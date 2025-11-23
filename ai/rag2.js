import { pipeline } from '@xenova/transformers';
import fs from 'fs/promises';
import path from 'path';

import { extractHeaderAndContent, getFiles, updateProgress } from './lib.js';

const OUTPUT_DIR = 'ai/rag';

// Ensure output directory exists
async function ensureOutputDir() {
    await fs.mkdir(OUTPUT_DIR, { recursive: true });
}

// Extract Description from header block
function extractDescriptionFromHeader(header) {
    const match = header.match(/\/\*\*\s*Description:\s*([\s\S]*?)sha:/i);
    if (!match) return null;
    return match[1].trim();
}

// Check existing RAG file SHA
async function getExistingSha(ragPath) {
    try {
        const raw = await fs.readFile(ragPath, 'utf-8');
        const json = JSON.parse(raw);
        return json.sha || null;
    } catch {
        return null;
    }
}

// Process a single file
async function processFile(filePath, embedder) {
    let raw;
    try {
        raw = await fs.readFile(filePath, 'utf-8');
    } catch (err) {
        console.error(`❌ Could not read ${filePath}:`, err.message);
        return;
    }

    const { headerExists, cleanContent, existingSha: headerSha } = extractHeaderAndContent(raw);
    if (!headerExists) {
        console.log(`⚠️ No description header found in ${filePath}, skipping.`);
        return;
    }

    const headerMatch = raw.match(/\/\*\*\s*Description:[\s\S]*?\*\//i);
    if (!headerMatch) {
        console.log(`⚠️ Header parse failed in ${filePath}, skipping.`);
        return;
    }

    const description = extractDescriptionFromHeader(headerMatch[0]);
    if (!description) {
        console.log(`⚠️ Description missing in header of ${filePath}, skipping.`);
        return;
    }

    const safeName = filePath.replace(/[\/\\]/g, '_');
    const outPath = path.join(OUTPUT_DIR, safeName + '.json');

    // Check existing RAG file SHA
    const existingRagSha = await getExistingSha(outPath);
    if (existingRagSha && existingRagSha === headerSha) {
        console.log(`\t✅ SHA matched in RAG file (${headerSha}), skipping embedding.`);
        return;
    }

    try {
        // Generate embedding using Xenova transformers
        const result = await embedder(description);

        // Manual mean pooling
        const tokens = result.data.length;
        const dims = result.data[0].length;
        const embedding = Array(dims).fill(0);
        for (let i = 0; i < tokens; i++) {
            for (let j = 0; j < dims; j++) {
                embedding[j] += result.data[i][j];
            }
        }
        for (let j = 0; j < dims; j++) embedding[j] /= tokens;

        // Save embedding + SHA
        await fs.writeFile(
            outPath,
            JSON.stringify(
                {
                    file: filePath,
                    description,
                    sha: headerSha,
                    embedding,
                },
                null,
                2
            )
        );

        console.log(`\n🧠 Saved embedding → ${outPath}`);
    } catch (err) {
        console.error(`❌ Failed to embed ${filePath}:`, err.message);
    }
}

// Main
(async () => {
    console.log('🚀 Starting Local Embedding Generator (Xenova)…');

    await ensureOutputDir();

    // Load embedding model once
    console.log('📦 Loading embedding model (Xenova/all-MiniLM-L6-v2)...');
    const embedder = await pipeline('feature-extraction', 'Xenova/all-MiniLM-L6-v2');

    // Find files
    const files = await getFiles(process.cwd());
    if (files.length === 0) {
        console.log('No files found.');
        return;
    }

    console.log(`\nFound ${files.length} files.`);

    let index = 0;
    for (const file of files) {
        index++;
        updateProgress(index, files.length);
        await processFile(file, embedder);
    }

    console.log('\n✨ Embedding generation complete!');
})();
