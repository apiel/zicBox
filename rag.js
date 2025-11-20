import { pipeline } from '@xenova/transformers';
import crypto from 'crypto';
import { lstatSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'fs';
import path from 'path';

// --------------------------------------------
// Settings
// --------------------------------------------
const ignore = ['node_modules', '.git', 'build', 'dist', 'os'];
const extensions = ['.cpp', '.h', '.ts'];
const rootFolder = '.';
const ragFolder = 'rag';

// --------------------------------------------
// Utility functions
// --------------------------------------------
function isIgnored(file) {
    const lower = file.toLowerCase();
    return ignore.some(i => lower.includes(i.toLowerCase()));
}

function isAllowed(file) {
    const lower = file.toLowerCase();
    return extensions.some(e => lower.endsWith(e));
}

function ensureDir(file) {
    mkdirSync(path.dirname(file), { recursive: true });
}

function fileHash(content) {
    return crypto.createHash('sha256').update(content).digest('hex');
}

async function extractFiles(folder, files = []) {
    for (const entry of readdirSync(folder)) {
        const fullPath = path.join(folder, entry);
        if (isIgnored(fullPath)) continue;

        if (lstatSync(fullPath).isDirectory()) {
            await extractFiles(fullPath, files);
        } else if (isAllowed(fullPath)) {
            files.push(fullPath);
        }
    }
    return files;
}

// ============================================
// Embedding Normalizer — VERY IMPORTANT
// ============================================
function normalizeEmbedding(output) {
    // Case 1: Array wrapping with Float32Array
    // Example: [ Float32Array(259) ]
    if (Array.isArray(output) &&
        output.length === 1 &&
        output[0] instanceof Float32Array) {
        return Array.from(output[0]); // flatten into normal JS array
    }

    // Case 2: { data: [...] }
    if (output.data && Array.isArray(output.data)) {
        return Array.from(output.data[0]);
    }

    // Case 3: { output: [...] }
    if (output.output && Array.isArray(output.output)) {
        return Array.from(output.output[0]);
    }

    // Case 4: tensor object: { data: Float32Array, dims: [tokens, dim] }
    if (output.data instanceof Float32Array && output.dims) {
        const [tokens, dim] = output.dims;
        const first = output.data.slice(0, dim);
        return Array.from(first);
    }

    // Debugging fallback
    console.error("❌ Unsupported embedding format:", output);
    throw new Error("Could not normalize embedding");
}

// --------------------------------------------
// Save RAG file
// --------------------------------------------
function saveRagFile(filePath, content, embedding) {
    const name = filePath.replace(/[\/\\]/g, '_') + '.json';
    const outPath = path.join(ragFolder, name);
    ensureDir(outPath);

    const hash = fileHash(content);

    // Skip if unchanged
    try {
        const existing = JSON.parse(readFileSync(outPath, 'utf8'));
        if (existing.hash === hash) {
            console.log(`Skipped ${filePath} (unchanged)`);
            return false;
        }
    } catch {}

    writeFileSync(outPath, JSON.stringify({ path: filePath, embedding, hash }));
    console.log(`Saved/Updated ${filePath}`);
    return true;
}

// --------------------------------------------
// Cosine similarity
// --------------------------------------------
function cosineSimilarity(a, b) {
    const dot = a.reduce((sum, v, i) => sum + v * b[i], 0);
    const normA = Math.sqrt(a.reduce((sum, v) => sum + v*v, 0));
    const normB = Math.sqrt(b.reduce((sum, v) => sum + v*v, 0));
    return dot / (normA * normB);
}

// ============================================
// MAIN: RAG generator
// ============================================
async function generateRAG() {
    mkdirSync(ragFolder, { recursive: true });

    console.log('Loading embedding model...');
    const embedder = await pipeline('feature-extraction', 'Xenova/all-MiniLM-L6-v2');

    const files = await extractFiles(rootFolder);
    console.log(`Found ${files.length} code files\n`);

    for (const file of files) {
        const content = readFileSync(file, 'utf8');

        const name = file.replace(/[\/\\]/g, '_') + '.json';
        const outPath = path.join(ragFolder, name);

        let existingHash;
        try {
            const existing = JSON.parse(readFileSync(outPath, 'utf8'));
            existingHash = existing.hash;
        } catch {}

        const currentHash = fileHash(content);
        if (existingHash === currentHash) {
            console.log(`Skipped ${file} (unchanged)`);
            continue;
        }

        let raw = await embedder(content);
        let embedding = normalizeEmbedding(raw);

        saveRagFile(file, content, embedding);
    }

    console.log('\nRAG generation complete ✅');
}

// ============================================
// QUERY MODE
// ============================================
async function queryRAG(question) {
    const embedder = await pipeline('feature-extraction', 'Xenova/all-MiniLM-L6-v2');
    let raw = await embedder(question);
    let queryEmbedding = normalizeEmbedding(raw);

    const files = readdirSync(ragFolder);
    console.log(`\n🔍 Querying RAG for: "${question} (${files.length} files)"\n`);
    const scored = [];

    for (const file of files) {
        if (!file.endsWith('.json')) continue;

        const data = JSON.parse(readFileSync(path.join(ragFolder, file), 'utf8'));

        const sim = cosineSimilarity(queryEmbedding, data.embedding);

        scored.push({ path: data.path, score: sim });
    }

    scored.sort((a, b) => b.score - a.score);
    console.log(scored);

    console.log('📄 Top relevant files:\n');
    for (const item of scored.slice(0, 5)) {
        console.log(`• ${item.path}   (score: ${item.score.toFixed(4)})`);
    }
}

// ============================================
// Entry point
// ============================================
const userQuestion = process.argv.slice(2).join(' ');

if (userQuestion) {
    queryRAG(userQuestion);
} else {
    generateRAG();
}
