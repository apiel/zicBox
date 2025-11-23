import crypto from 'crypto';
import { promises as fs } from 'fs';
import path from 'path';
// Assuming this is defined or handled by the environment (Canvas)
import { GOOGLE_AI_API_KEY } from './apikey.js';

// --- Configuration ---
// NOTE: Leave the API key blank. In the Canvas environment, the key is automatically provided during runtime.
const apiKey = GOOGLE_AI_API_KEY;
const apiUrl =
    'https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash-preview-09-2025:generateContent';

const ignore = [
    'node_modules',
    '.git',
    'build',
    'dist',
    'os',
    'libs/httplib/',
    'libs/nlohmann',
    'config/pixel',
    'config/grid',
    'config/grain',
    'config/desktop',
];
const extensions = ['.cpp', '.h', '.ts'];

// --- Progress Bar Utility ---
function updateProgress(current, total) {
    const barLength = 30;
    const percentage = current / total;
    const filled = Math.round(barLength * percentage);
    const empty = barLength - filled;

    const bar = '█'.repeat(filled) + '-'.repeat(empty);
    const pctText = (percentage * 100).toFixed(1);

    process.stdout.write(`\rProgress: [${bar}] ${pctText}% (${current}/${total})`);

    if (current === total) {
        process.stdout.write('\n'); // newline when done
    }
}

// --- Utility Functions ---
function isIgnored(file) {
    const lower = file.toLowerCase();
    return ignore.some((i) => lower.includes(i.toLowerCase()));
}

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function callGeminiApi(payload, maxRetries = 5) {
    for (let attempt = 0; attempt < maxRetries; attempt++) {
        const delay = Math.pow(2, attempt) * 1000 + Math.random() * 1000;

        try {
            const response = await fetch(`${apiUrl}?key=${apiKey}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload),
            });

            if (response.status === 429 && attempt < maxRetries - 1) {
                console.warn(
                    `[Attempt ${
                        attempt + 1
                    }/${maxRetries}] Rate limit exceeded (429). Retrying in ${Math.round(
                        delay / 1000
                    )}s...`
                );
                await sleep(delay);
                continue;
            }

            if (!response.ok) {
                const errorText = await response.text();
                throw new Error(`API call failed with status ${response.status}: ${errorText}`);
            }

            return await response.json();
        } catch (error) {
            console.error(`Error during API call on attempt ${attempt + 1}:`, error.message);
            if (attempt < maxRetries - 1) {
                console.warn(`Retrying in ${Math.round(delay / 1000)}s...`);
                await sleep(delay);
            } else {
                throw new Error('Failed to call Gemini API after multiple retries.');
            }
        }
    }
}

function calculateSha256(content) {
    return crypto.createHash('sha256').update(content, 'utf8').digest('hex');
}

function extractHeaderAndContent(content) {
    const singleHeaderPattern = /\/\*\*\s*Description[\s\S]*?\*\/|sha:\s*([a-fA-F0-9]{64})/;

    let contentWithoutHeaders = content;
    let lastRemovedSha = null;
    let headersRemoved = 0;

    while (true) {
        const match = contentWithoutHeaders.match(singleHeaderPattern);

        if (match && contentWithoutHeaders.indexOf(match[0]) === 0) {
            const shaMatch = match[0].match(/sha: ([a-fA-F0-9]{64})/);
            if (shaMatch) lastRemovedSha = shaMatch[1];

            contentWithoutHeaders = contentWithoutHeaders.substring(match[0].length).trimStart();
            headersRemoved++;
        } else {
            break;
        }
    }

    return {
        headerExists: headersRemoved > 0,
        existingSha: lastRemovedSha,
        cleanContent: contentWithoutHeaders,
    };
}

async function getFiles(dir = '.', fileList = []) {
    const files = await fs.readdir(dir);

    for (const file of files) {
        const filePath = path.join(dir, file);
        const stat = await fs.stat(filePath);

        if (isIgnored(filePath)) continue;

        if (stat.isDirectory()) {
            fileList = await getFiles(filePath, fileList);
        } else {
            const fileExtension = path.extname(filePath).toLowerCase();
            if (extensions.includes(fileExtension)) {
                fileList.push(filePath);
            }
        }
    }

    return fileList;
}

async function processFile(filePath) {
    console.log(`\n--- Processing file: ${filePath} ---`);

    let fileContent;
    try {
        fileContent = await fs.readFile(filePath, 'utf-8');
    } catch (error) {
        console.error(`\nError: Could not read file at ${filePath}. Details: ${error.message}`);
        return;
    }

    const { existingSha, cleanContent, headerExists } = extractHeaderAndContent(fileContent);
    const currentSha = calculateSha256(cleanContent);

    if (headerExists && existingSha === currentSha) {
        console.log(`\t✅ Skipped: SHA matched (${currentSha}).`);
        return;
    }

    const action = headerExists ? 'Updating' : 'Generating';
    console.log(`\tℹ️ ${action} explanation. New SHA: ${currentSha}.`);

    const fileExtension = path.extname(filePath).toLowerCase();
    const language =
        fileExtension === '.cpp'
            ? 'C++'
            : fileExtension === '.ts'
            ? 'TypeScript'
            : fileExtension === '.h'
            ? 'C/C++ Header'
            : 'Code';

    const systemPrompt = `You are an expert ${language} code analyst and technical writer. Your task is to provide a comprehensive, structured explanation of the provided code.
The target audience should be non-developer or beginners. There should be no code inside, however, a basic idea of how the code works should be included.
It should not exceed 2000 characters.
`;

    const userQuery = `Analyze the following ${language} code and generate the required description:\n\n\`\`\`${language.toLowerCase()}\n${cleanContent}\n\`\`\``;

    const payload = {
        contents: [{ parts: [{ text: userQuery }] }],
        systemInstruction: { parts: [{ text: systemPrompt }] },
    };

    let explanationText = `Error: Failed to generate explanation.`;

    try {
        const result = await callGeminiApi(payload);
        const candidate = result.candidates?.[0];

        if (candidate && candidate.content?.parts?.[0]?.text) {
            explanationText = candidate.content.parts[0].text.trim();
            console.log(`\t✅ API Explanation successful. Length: ${explanationText.length}`);
        } else {
            console.error('\t❌ Error: API response contained no generated text.');
        }
    } catch (error) {
        console.error('\t❌ Critical error during API processing:', error.message);
    }

    const newHeader = `/** Description:
${explanationText}

sha: ${currentSha} 
*/`;

    try {
        const newFileContent = `${newHeader.trim()}\n${cleanContent}`;
        await fs.writeFile(filePath, newFileContent, 'utf-8');
        console.log(`\t📝 File updated.`);
    } catch (writeError) {
        console.error(`\t❌ Error writing file ${filePath}: ${writeError.message}`);
    }
}

// --- Main execution ---
(async () => {
    console.log('🚀 Starting Code Explainer Script...');
    console.log(`Scanning for extensions: ${extensions.join(', ')}`);
    console.log(`Ignoring paths containing: ${ignore.join(', ')}`);

    try {
        const allFiles = await getFiles(process.cwd());

        if (allFiles.length === 0) {
            console.log('\nNo matching code files found.');
            return;
        }

        console.log(`\nFound ${allFiles.length} files to process.\n`);

        let index = 0;

        for (const file of allFiles) {
            index++;
            updateProgress(index, allFiles.length);
            await processFile(file);
        }

        console.log('\n✨ All files processed successfully.');
    } catch (error) {
        console.error('\n[CRITICAL ERROR] Failure during execution:', error.message);
    }
})();
