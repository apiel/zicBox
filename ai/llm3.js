// --- Code to replace your cloud API fetch ---


// NEED to have ollama running in background: ollama run qwen3:0.6b

import { promises as fs } from 'fs'; // Import Node's file system module
import { Ollama } from 'ollama'; // You would install the Ollama JavaScript library

// 1. Point the client to the local server address
const ollama = new Ollama({ host: 'http://localhost:11434' });

/**
 * Generates an explanation using the local model.
 * @param {string} codeToExplain - The raw code content.
 */
async function explainCodeLocally(codeToExplain) {
    // Modified prompt to request a simple, non-technical explanation for a beginner
    const prompt = `Explain the following code in a very simple, non-technical way, suitable for a beginner developer or a non-developer. Describe its main purpose and how it achieves it in straightforward language:\n\n\`\`\`\n${codeToExplain}\n\`\`\``;

    console.log("Contacting local Ollama server...");

    try {
        // This is a placeholder for the actual connection to your local Ollama instance
        const response = await ollama.generate({
            model: 'qwen3:0.6b',
            prompt: prompt,
            stream: false, // Wait for the full response
        });

        // The response.response field contains the model's text output
        console.log("\n✅ Local AI Explanation (Simple Version):");
        console.log("=========================================");
        console.log(response.response);
        console.log("=========================================");

    } catch (error) {
        console.error("❌ Error communicating with local Ollama server. Is Ollama running on http://localhost:11434?");
        console.error("Details:", error.message);
    }
}

// --- Main execution logic ---
(async () => {
    // Check for file path argument passed via command line
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.error("\nUsage: node node_local_integration.js <path_to_code_file>");
        console.error("Example: node node_local_integration.js example.cpp");
        return;
    }
    
    const filePath = args[0];
    console.log(`\n--- Analyzing file: ${filePath} ---`);

    let fileContent;
    try {
        // Read the content of the file
        fileContent = await fs.readFile(filePath, 'utf-8');
    } catch (error) {
        console.error(`\nError: Could not read file at ${filePath}.`);
        console.error("Please ensure the path is correct and the file exists.");
        console.error(`Details: ${error.message}`);
        return;
    }
    
    // Removed the file extension check for maximum simplicity.
    // The model will now determine the language from the code itself.
    
    await explainCodeLocally(fileContent);
})();