import { promises as fs } from 'fs';
import path from 'path';
import { GOOGLE_AI_API_KEY } from './apikey.js';

// --- Configuration for Gemini API Call ---
// NOTE: Leave the API key blank. In the Canvas environment, the key is automatically provided during runtime.
const apiKey = GOOGLE_AI_API_KEY;
const apiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash-preview-09-2025:generateContent";

/**
 * Sleeps for a given number of milliseconds.
 * @param {number} ms - The number of milliseconds to sleep.
 * @returns {Promise<void>}
 */
const sleep = (ms) => new Promise(resolve => setTimeout(resolve, ms));

/**
 * Calls the Gemini API with exponential backoff.
 * @param {object} payload - The request payload.
 * @param {number} maxRetries - Maximum number of retries.
 * @returns {Promise<object>} The API response JSON.
 */
async function callGeminiApi(payload, maxRetries = 5) {
    for (let attempt = 0; attempt < maxRetries; attempt++) {
        const delay = Math.pow(2, attempt) * 1000 + Math.random() * 1000; // Exponential backoff + jitter
        
        try {
            const response = await fetch(`${apiUrl}?key=${apiKey}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });

            if (response.status === 429 && attempt < maxRetries - 1) {
                console.warn(`[Attempt ${attempt + 1}/${maxRetries}] Rate limit exceeded (429). Retrying in ${Math.round(delay / 1000)}s...`);
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
                throw new Error("Failed to call Gemini API after multiple retries.");
            }
        }
    }
}

/**
 * Generates an explanation for the given code file using the Gemini API.
 * @param {string} filePath - The path to the code file.
 */
async function explainCode(filePath) {
    console.log(`\n--- Analyzing file: ${filePath} ---`);

    let fileContent;
    try {
        fileContent = await fs.readFile(filePath, 'utf-8');
    } catch (error) {
        console.error(`\nError: Could not read file at ${filePath}.`);
        console.error("Please ensure the path is correct and the file exists.");
        console.error(`Details: ${error.message}`);
        return;
    }

    const fileExtension = path.extname(filePath).toLowerCase();
    const language = (fileExtension === '.cpp') ? 'C++' : 
                     (fileExtension === '.ts') ? 'TypeScript' : 
                     'Code';
    
    // --- System Prompt and User Query Construction ---
    // The system prompt defines the AI's role and output format, crucial for RAG.
    const systemPrompt = `You are an expert ${language} code analyst and technical writer. Your task is to provide a comprehensive, structured explanation of the provided code.
The target audience should be non developer or beginners. There should be no code inside, however a baisc idea of how the code works should be included.
It should not exceed 2000 characters.
`;

    const userQuery = `Analyze the following ${language} code and generate the required description:\n\n\`\`\`${language.toLowerCase()}\n${fileContent}\n\`\`\``;

    const payload = {
        contents: [{ parts: [{ text: userQuery }] }],
        systemInstruction: { parts: [{ text: systemPrompt }] },
    };

    console.log(`Sending ${language} code for explanation to Gemini...`);
    
    try {
        const result = await callGeminiApi(payload);
        const candidate = result.candidates?.[0];

        if (candidate && candidate.content?.parts?.[0]?.text) {
            const explanationText = candidate.content.parts[0].text;
            
            console.log("\n✅ AI Explanation Generated Successfully:");
            console.log("=========================================");
            console.log(explanationText);
            console.log("=========================================");
            
            // Extract grounding sources (if any, though not expected for code analysis)
            let sources = [];
            const groundingMetadata = candidate.groundingMetadata;
            if (groundingMetadata && groundingMetadata.groundingAttributions) {
                sources = groundingMetadata.groundingAttributions
                    .map(attribution => ({
                        uri: attribution.web?.uri,
                        title: attribution.web?.title,
                    }))
                    .filter(source => source.uri && source.title);
            }
            
            if (sources.length > 0) {
                console.log("\n--- Citation Sources (for context): ---");
                sources.forEach((s, index) => console.log(`${index + 1}. [${s.title}](${s.uri})`));
            }
            console.log();

        } else {
            console.error("\n❌ Error: API response contained no generated text.");
            console.error(JSON.stringify(result, null, 2));
        }

    } catch (error) {
        console.error("\n❌ A critical error occurred during processing:", error.message);
    }
}

// --- Main execution logic ---
(async () => {
    // Check for file path argument
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.error("\nUsage: node code_explainer.js <path_to_code_file>");
        console.error("Example: node code_explainer.js example.cpp");
        return;
    }

    const filePath = args[0];
    await explainCode(filePath);
})();