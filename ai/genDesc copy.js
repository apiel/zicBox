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

const ignore = ['node_modules', '.git', 'build', 'dist', 'os', 'libs/httplib/', 'libs/nlohmann', 'ai'];
const extensions = ['.cpp', '.h', '.ts'];

// --- Utility Functions ---

/**
 * Checks if a file or directory path should be ignored.
 * @param {string} file - The file or directory path.
 * @returns {boolean} True if the path should be ignored.
 */
function isIgnored(file) {
    const lower = file.toLowerCase();
    // Check if any part of the path contains an ignored pattern
    return ignore.some((i) => lower.includes(i.toLowerCase()));
}

/**
 * Sleeps for a given number of milliseconds.
 * @param {number} ms - The number of milliseconds to sleep.
 * @returns {Promise<void>}
 */
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

/**
 * Calls the Gemini API with exponential backoff.
 * @param {object} payload - The request payload.
 * @param {number} maxRetries - Maximum number of retries.
 * @returns {Promise<object>} The API response JSON.
 */
async function callGeminiApi(payload, maxRetries = 5) {
    for (let attempt = 0; attempt < maxRetries; attempt++) {
        // Exponential backoff + jitter
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
                continue; // Retry
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

/**
 * Calculates the SHA-256 hash of a string.
 * @param {string} content - The content to hash.
 * @returns {string} The hex digest of the SHA-256 hash.
 */
function calculateSha256(content) {
    return crypto.createHash('sha256').update(content, 'utf8').digest('hex');
}

/**
 * Extracts the existing explanation header and the clean code content.
 * @param {string} content - The full file content.
 * @returns {{headerExists: boolean, existingHeader: string, existingSha: string | null, cleanContent: string}}
 */
function extractHeaderAndContent(content) {
    // Regex to match the mandatory header structure at the very beginning of the file
    const headerRegex = /^(\/\*\* Description:\n[\s\S]*?\nsha: [a-fA-F0-9]{64}\n\*\/)\s*/;
    const match = content.match(headerRegex);

    if (match) {
        // match[1] is the full header block
        const header = match[1];
        // content.substring(match[0].length) skips the header and any trailing whitespace
        const cleanContent = content.substring(match[0].length);

        // Extract SHA from the header
        const shaMatch = header.match(/sha: ([a-fA-F0-9]{64})/);
        const existingSha = shaMatch ? shaMatch[1] : null;

        return {
            headerExists: true,
            existingHeader: header,
            existingSha: existingSha,
            cleanContent: cleanContent,
        };
    }

    // No header found, the entire content is clean content
    return {
        headerExists: false,
        existingHeader: '',
        existingSha: null,
        cleanContent: content,
    };
}

/**
 * Recursively finds all code files in the current directory, respecting ignore and extension lists.
 * @param {string} dir - The directory to start searching from.
 * @param {string[]} fileList - Accumulator for file paths.
 * @returns {Promise<string[]>}
 */
async function getFiles(dir = '.', fileList = []) {
    const files = await fs.readdir(dir);

    for (const file of files) {
        const filePath = path.join(dir, file);
        const stat = await fs.stat(filePath);

        // Skip ignored paths
        if (isIgnored(filePath)) {
            continue;
        }

        if (stat.isDirectory()) {
            // Recursively search subdirectories
            fileList = await getFiles(filePath, fileList);
        } else {
            // Check if the file extension is included
            const fileExtension = path.extname(filePath).toLowerCase();
            if (extensions.includes(fileExtension)) {
                fileList.push(filePath);
            }
        }
    }

    return fileList;
}

/**
 * Processes a single code file: checks for updates, calls the API if necessary, and rewrites the file.
 * @param {string} filePath - The path to the code file.
 */
async function processFile(filePath) {
    console.log(`\n--- Processing file: ${filePath} ---`);

    let fileContent;
    try {
        fileContent = await fs.readFile(filePath, 'utf-8');
    } catch (error) {
        console.error(`\nError: Could not read file at ${filePath}. Details: ${error.message}`);
        return;
    }

    // 1. Extract existing header and clean content
    const { existingSha, cleanContent, headerExists } = extractHeaderAndContent(fileContent);

    // 2. Calculate the SHA of the *current* clean content
    const currentSha = calculateSha256(cleanContent);

    // 3. Check if we need to call the API
    if (headerExists && existingSha === currentSha) {
        console.log(
            `\t✅ Skipped: SHA matched (${currentSha}). Code has not changed since last run.`
        );
        return;
    }

    const action = headerExists ? 'Updating' : 'Generating';
    console.log(`\t⚠️ ${action} explanation. New SHA: ${currentSha}.`);

    const fileExtension = path.extname(filePath).toLowerCase();
    const language =
        fileExtension === '.cpp'
            ? 'C++'
            : fileExtension === '.ts'
            ? 'TypeScript'
            : fileExtension === '.h'
            ? 'C/C++ Header'
            : 'Code';

    // --- System Prompt and User Query Construction ---
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
            console.log(`\t✅ API Explanation successful. Text length: ${explanationText.length}`);
        } else {
            console.error('\t❌ Error: API response contained no generated text.');
        }
    } catch (error) {
        console.error('\t❌ A critical error occurred during API processing:', error.message);
    }

    // 4. Construct the new header
    const newHeader = `/** Description:
${explanationText}

sha: ${currentSha} 
*/`;

    // 5. Rewrite the file
    try {
        const newFileContent = `${newHeader}\n\n${cleanContent}`;
        await fs.writeFile(filePath, newFileContent, 'utf-8');
        console.log(`\t📝 File successfully updated with new description and SHA.`);
    } catch (writeError) {
        console.error(`\t❌ Error writing file ${filePath}: ${writeError.message}`);
    }
}

// --- Main execution logic ---
(async () => {
    console.log('🚀 Starting Code Explainer Script...');
    console.log(`Scanning for extensions: ${extensions.join(', ')}`);
    console.log(`Ignoring paths containing: ${ignore.join(', ')}`);

    try {
        const allFiles = await getFiles(process.cwd()); // Start from the current working directory

        if (allFiles.length === 0) {
            console.log('\nNo matching code files found based on extensions and ignore patterns.');
            return;
        }

        console.log(`\nFound ${allFiles.length} files to process. Starting generation...`);

        // Process files sequentially to avoid rate-limiting issues
        for (const file of allFiles) {
            await processFile(file);
        }

        console.log('\n✨ All code files processed successfully.');
    } catch (error) {
        console.error(
            '\n[CRITICAL ERROR] Failed during file system traversal or main loop:',
            error.message
        );
    }
})();
