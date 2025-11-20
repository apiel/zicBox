// rag.js
// Complete RAG system: small-chunk (30-50 lines), code-aware embeddings, hybrid scoring.
// Usage:
//   node rag.js                -> generate/update chunk embeddings
//   node rag.js "your query"   -> query and print top matches
//
// Requirements:
//   npm i @xenova/transformers
//
// NOTE: this is ESM (uses `import`). Run with `node --experimental-fetch` or Node 18+.

import { pipeline } from '@xenova/transformers';
import crypto from 'crypto';
import { lstatSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'fs';
import path from 'path';

// ---------------- Settings ----------------
const ignore = ['node_modules', '.git', 'build', 'dist', 'os', 'libs/httplib/', 'libs/nlohmann'];
const extensions = ['.cpp', '.h', '.ts'];
const rootFolder = '.';
const ragFolder = 'rag';
const chunkMinLines = 30; // small chunks as requested
const chunkMaxLines = 50;
const mergeLineGap = 5; // merge nearby chunks within 5 lines
const topK = 8; // number of top chunks to consider/print
const embedModel = 'Xenova/all-MiniLM-L6-v2'; // code-aware model
const hybridWeights = { cosine: 0.75, keywords: 0.25 };

// ---------------- Utilities ----------------
function isIgnored(file) {
    const lower = file.toLowerCase();
    return ignore.some((i) => lower.includes(i.toLowerCase()));
}

function isAllowed(file) {
    const lower = file.toLowerCase();
    return extensions.some((e) => lower.endsWith(e));
}

function ensureDir(file) {
    mkdirSync(path.dirname(file), { recursive: true });
}

function fileHash(content) {
    return crypto.createHash('sha256').update(content).digest('hex');
}

function safeReadJSON(p) {
    try {
        return JSON.parse(readFileSync(p, 'utf8'));
    } catch {
        return null;
    }
}

// walk files
async function extractFiles(folder, files = []) {
    for (const entry of readdirSync(folder)) {
        const fullPath = path.join(folder, entry);
        if (isIgnored(fullPath)) continue;
        const stat = lstatSync(fullPath);
        if (stat.isDirectory()) {
            await extractFiles(fullPath, files);
        } else if (isAllowed(fullPath)) {
            files.push(fullPath);
        }
    }
    return files;
}

// ---------------- Embedding normalizer (robust) ----------------
function normalizeEmbedding(output) {
    // Case: [ Float32Array(...) ]
    if (Array.isArray(output) && output.length === 1 && output[0] instanceof Float32Array) {
        return Array.from(output[0]);
    }
    // Case: [[...], ...]
    if (
        Array.isArray(output) &&
        Array.isArray(output[0]) &&
        (typeof output[0][0] === 'number' || output[0][0] instanceof Number)
    ) {
        return Array.from(output[0]);
    }
    // Case: [ { data: [...] } ]
    if (
        Array.isArray(output) &&
        output.length === 1 &&
        output[0].data &&
        Array.isArray(output[0].data)
    ) {
        return Array.from(output[0].data[0]);
    }
    // Case: { data: [ [..] ] } or { output: [ [..] ] }
    if (output && output.data && Array.isArray(output.data)) {
        return Array.from(output.data[0]);
    }
    if (output && output.output && Array.isArray(output.output)) {
        return Array.from(output.output[0]);
    }
    // Case: tensor-like { data: Float32Array, dims: [tokens, dim] }
    if (output && output.data instanceof Float32Array && Array.isArray(output.dims)) {
        const dim = output.dims[1] || output.dims[0];
        return Array.from(output.data.slice(0, dim));
    }

    console.error('❌ Unsupported embedding format (dump):', output);
    throw new Error('Could not normalize embedding');
}

// ---------------- Chunking helpers ----------------

// Split TS by top-level functions/classes and fallback
function chunkTypeScript(content) {
    const lines = content.split('\n');
    // naive regex for function / class / export const / arrow functions
    const boundaries = [];
    const funcRegex = /^\s*(export\s+)?(async\s+)?(function|class)\s+([A-Za-z0-9_]+)/;
    const exportAssignRegex = /^\s*(export\s+)?(const|let|var)\s+([A-Za-z0-9_]+)\s*[:=]/;
    for (let i = 0; i < lines.length; i++) {
        if (funcRegex.test(lines[i]) || exportAssignRegex.test(lines[i])) boundaries.push(i);
    }
    return windowedChunksFromBoundaries(lines, boundaries);
}

// ---------------- Safe C++ chunking ----------------
function chunkCpp(content) {
    const lines = content.split('\n');
    const boundaries = [];
    const funcRegex = /^\s*([A-Za-z0-9_<>:~\*\&\s]+)\s+([A-Za-z0-9_]+)\s*\(.*\)\s*(const)?\s*{/;
    const classRegex = /^\s*(class|struct)\s+[A-Za-z0-9_]+/;

    for (let i = 0; i < lines.length; i++) {
        if (funcRegex.test(lines[i]) || classRegex.test(lines[i])) boundaries.push(i);
    }

    return windowedChunksFromBoundaries(lines, boundaries);
}

// ---------------- Safe sliding window with tiny file handling ----------------
function windowedChunksFromBoundaries(lines, boundaries) {
    const chunks = [];
    const total = lines.length;

    // tiny file: just one chunk
    if (total <= chunkMaxLines) {
        chunks.push({ startLine: 1, endLine: total, content: lines.join('\n') });
        return chunks;
    }

    // no boundaries? use sliding window
    if (!boundaries || boundaries.length === 0) {
        let start = 0;
        while (start < total) {
            const end = Math.min(total, start + chunkMaxLines);
            chunks.push({
                startLine: start + 1,
                endLine: end,
                content: lines.slice(start, end).join('\n'),
            });
            // advance at least 1 line to avoid infinite loop
            start = end - Math.floor(chunkMinLines / 2);
            if (start <= start) start = end;
        }
        return chunks;
    }

    // boundaries exist: chunk by boundaries with sliding windows
    for (let i = 0; i < boundaries.length; i++) {
        const start = boundaries[i];
        const nextBoundary = boundaries[i + 1] || total;
        let s = start;
        while (s < nextBoundary) {
            const e = Math.min(nextBoundary, s + chunkMaxLines);
            chunks.push({ startLine: s + 1, endLine: e, content: lines.slice(s, e).join('\n') });
            s = e; // always advance to prevent infinite loop
        }
    }

    // optionally include head (before first boundary)
    if (boundaries[0] > 0) {
        let s = 0;
        while (s < boundaries[0]) {
            const e = Math.min(boundaries[0], s + chunkMaxLines);
            chunks.push({ startLine: s + 1, endLine: e, content: lines.slice(s, e).join('\n') });
            s = e;
        }
    }

    // optionally include tail (after last boundary)
    const lastBound = boundaries[boundaries.length - 1] || 0;
    if (lastBound < total - 1) {
        let s = lastBound;
        while (s < total) {
            const e = Math.min(total, s + chunkMaxLines);
            chunks.push({ startLine: s + 1, endLine: e, content: lines.slice(s, e).join('\n') });
            s = e;
        }
    }

    return chunks;
}

// Make deterministic filename for a chunk
function chunkFilename(filePath, startLine, endLine) {
    const safe = filePath.replace(/[\/\\]/g, '__').replace(/[^A-Za-z0-9_\-\.]/g, '_');
    return `${safe}__L${startLine}_L${endLine}.json`;
}

// ---------------- Save chunk metadata + embedding ----------------
function saveChunk(filePath, startLine, endLine, content, embedding) {
    const name = chunkFilename(filePath, startLine, endLine);
    const outPath = path.join(ragFolder, name);
    ensureDir(outPath);
    const hash = fileHash(content);
    try {
        const existing = safeReadJSON(outPath);
        if (existing && existing.hash === hash) {
            // unchanged
            // console.log(`Chunk unchanged: ${filePath}:${startLine}-${endLine}`);
            return false;
        }
    } catch {}
    const payload = { file: filePath, startLine, endLine, content, embedding, hash };
    writeFileSync(outPath, JSON.stringify(payload), 'utf8');
    // console.log(`Saved chunk: ${filePath}:${startLine}-${endLine}`);
    return true;
}

// ---------------- Cosine similarity (a and b are JS arrays) ----------------
function cosineSimilarity(a, b) {
    if (!Array.isArray(a) || !Array.isArray(b) || a.length !== b.length) return -1;
    let dot = 0,
        na = 0,
        nb = 0;
    for (let i = 0; i < a.length; i++) {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }
    if (na === 0 || nb === 0) return -1;
    return dot / (Math.sqrt(na) * Math.sqrt(nb));
}

// ---------------- Keyword score (simple token overlap) ----------------
function keywordScore(query, text) {
    if (!query || !text) return 0;
    const qTokens = query
        .toLowerCase()
        .split(/[^a-z0-9_]+/)
        .filter(Boolean);
    if (qTokens.length === 0) return 0;
    const textLower = text.toLowerCase();
    let hits = 0;
    for (const t of qTokens) {
        if (textLower.includes(t)) hits++;
    }
    return hits / qTokens.length; // 0..1
}

// ---------------- Merge consecutive chunks within a gap ----------------
function mergeTopChunks(scoredChunks) {
    // scoredChunks: {file, startLine, endLine, score, content}
    const byFile = {};
    for (const s of scoredChunks) {
        (byFile[s.file] ||= []).push(s);
    }
    const merged = [];
    for (const file of Object.keys(byFile)) {
        const arr = byFile[file].sort((a, b) => a.startLine - b.startLine);
        let cur = null;
        for (const c of arr) {
            if (!cur) {
                cur = { ...c };
                continue;
            }
            if (c.startLine <= cur.endLine + mergeLineGap) {
                // merge
                cur.endLine = Math.max(cur.endLine, c.endLine);
                cur.score = Math.max(cur.score, c.score);
                cur.content += '\n' + c.content;
            } else {
                merged.push(cur);
                cur = { ...c };
            }
        }
        if (cur) merged.push(cur);
    }
    return merged;
}

// ---------------- Main: generate chunks and embeddings ----------------
async function generateRAG() {
    mkdirSync(ragFolder, { recursive: true });
    console.log('Loading embedding model...');
    const embedder = await pipeline('feature-extraction', embedModel);

    const files = await extractFiles(rootFolder);
    console.log(`Found ${files.length} code files\n`);

    let total = 0,
        saved = 0;
    for (const file of files) {
        console.log(`Chunking file: ${file}`);
        const content = readFileSync(file, 'utf8');
        const ext = path.extname(file).toLowerCase();
        let chunks = [];
        // console.log('chunking...');
        if (ext === '.ts') chunks = chunkTypeScript(content);
        else if (ext === '.cpp' || ext === '.h') chunks = chunkCpp(content);
        else chunks = windowedChunksFromBoundaries(content.split('\n'), []); // fallback

        console.log(`Processing file: ${file} (${chunks.length} chunks)`);
        for (const c of chunks) {
            total++;
            const chunkName = chunkFilename(file, c.startLine, c.endLine);
            const outPath = path.join(ragFolder, chunkName);
            const currentHash = fileHash(c.content);
            let existingHash = null;
            try {
                const existing = safeReadJSON(outPath);
                existingHash = existing && existing.hash;
            } catch {}
            if (existingHash === currentHash) {
                // skip embedding
                continue;
            }
            // compute embedding
            let raw = await embedder(c.content);
            let emb = normalizeEmbedding(raw);
            // convert Float32Array to plain array if needed
            if (emb instanceof Float32Array) emb = Array.from(emb);
            saveChunk(file, c.startLine, c.endLine, c.content, emb) && saved++;
            // free memory
            emb = null;
            raw = null;
        }
    }

    console.log(`\nChunks processed: ${total}, new/updated: ${saved}`);
    console.log('RAG chunk generation complete ✅');
}

// ---------------- Query mode ----------------
async function queryRAG(question) {
    console.log(`\n🔍 Querying RAG for: "${question}"\n`);
    const embedder = await pipeline('feature-extraction', embedModel);
    let rawQ = await embedder(question);
    let qEmb = normalizeEmbedding(rawQ);
    if (qEmb instanceof Float32Array) qEmb = Array.from(qEmb);

    const files = readdirSync(ragFolder);
    const scored = [];
    for (const file of files) {
        if (!file.endsWith('.json')) continue;
        const data = safeReadJSON(path.join(ragFolder, file));
        if (!data || !data.embedding) continue;
        const chunkEmb = data.embedding;
        if (!Array.isArray(chunkEmb) || chunkEmb.length !== qEmb.length) {
            // skip mismatch (models sometimes change dims); alternatively you could project/resize but skip for now
            continue;
        }
        const cosRaw = cosineSimilarity(qEmb, chunkEmb);
        const cos = Math.max(0, cosRaw); // clamp negative to 0
        const kw = keywordScore(question, data.content);
        const final = hybridWeights.cosine * cos + hybridWeights.keywords * kw;
        scored.push({
            file: data.file,
            startLine: data.startLine,
            endLine: data.endLine,
            content: data.content,
            cosine: cos,
            keyword: kw,
            score: final,
        });
    }

    if (scored.length === 0) {
        console.log(
            'No indexed chunks found or embedding dimension mismatch. Try running `node rag.js` to (re)generate chunks.'
        );
        return;
    }

    scored.sort((a, b) => b.score - a.score);
    const top = scored.slice(0, Math.max(topK, 20)); // take a larger set before merging

    // Merge neighboring chunks into regions
    const merged = mergeTopChunks(top).sort((a, b) => b.score - a.score);
    const outTop = merged.slice(0, 6);

    console.log('📄 Top relevant regions:\n');
    for (const item of outTop) {
        console.log(`file: ${item.file}`);
        console.log(`lines: ${item.startLine} - ${item.endLine}`);
        console.log(
            `score: ${item.score.toFixed(4)} (cosine=${item.cosine?.toFixed(4) || 'n/a'}, kw=${
                item.keyword?.toFixed(3) || 'n/a'
            })`
        );
        console.log('snippet:');
        console.log('---------------------------------------------');
        // Print up to first 30 lines of the region
        const lines = item.content.split('\n').slice(0, 50);
        for (const l of lines) console.log(l);
        console.log('---------------------------------------------\n');
    }

    console.log('Done.');
}

// ---------------- Entry point ----------------
const userQuestion = process.argv.slice(2).join(' ');

if (userQuestion) {
    queryRAG(userQuestion).catch((err) => {
        console.error('Query failed:', err);
    });
} else {
    generateRAG().catch((err) => {
        console.error('RAG generation failed:', err);
    });
}
