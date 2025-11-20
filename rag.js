import { pipeline } from '@xenova/transformers';
import crypto from 'crypto';
import { lstatSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'fs';
import path from 'path';

const ignore = ['node_modules', '.git', 'build', 'dist'];
const extensions = ['.cpp', '.h', '.ts'];
const rootFolder = '.';
const ragFolder = 'rag';

// ---------------- Utility functions ----------------
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

// ---------------- RAG file save ----------------
function saveRagFile(filePath, content, embedding) {
    const name = filePath.replace(/[\/\\]/g, '_') + '.json';
    const outPath = path.join(ragFolder, name);
    ensureDir(outPath);

    const hash = fileHash(content);

    // Skip if hash matches existing
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

// ---------------- Cosine similarity ----------------
export function cosineSimilarity(a, b) {
    const dot = a.reduce((sum, v, i) => sum + v * b[i], 0);
    const normA = Math.sqrt(a.reduce((sum, v) => sum + v*v, 0));
    const normB = Math.sqrt(b.reduce((sum, v) => sum + v*v, 0));
    return dot / (normA * normB);
}

// ---------------- Main RAG generator ----------------
async function generateRAG() {
    // Optional: clean rag folder completely
    // rmSync(ragFolder, { recursive: true, force: true });
    mkdirSync(ragFolder, { recursive: true });

    console.log('Loading embedding model...');
    const embedder = await pipeline('feature-extraction', 'Xenova/all-MiniLM-L6-v2');

    const files = await extractFiles(rootFolder);
    console.log(`Found ${files.length} code files`);

    for (const file of files) {
        const content = readFileSync(file, 'utf8');

        // Compute embedding only if changed
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

        const embedding = await embedder(content);

        saveRagFile(file, content, embedding);
    }

    console.log('RAG generation complete ✅');
}

generateRAG();
