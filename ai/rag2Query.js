import { pipeline } from '@xenova/transformers';
import fs from 'fs/promises';
import path from 'path';
import process from 'process';
import { normalizeEmbedding } from './lib.js';

// Compute cosine similarity
function cosineSimilarity(a, b) {
    let dot = 0;
    let normA = 0;
    let normB = 0;
    for (let i = 0; i < a.length; i++) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    return dot / (Math.sqrt(normA) * Math.sqrt(normB));
}

// Load all embeddings from ai/rag
async function loadRagEmbeddings(dir = 'ai/rag') {
    const files = await fs.readdir(dir);
    const embeddings = [];

    for (const file of files) {
        if (!file.endsWith('.json')) continue;
        const raw = await fs.readFile(path.join(dir, file), 'utf-8');
        const json = JSON.parse(raw);
        embeddings.push(json);
    }

    return embeddings;
}

(async () => {
    const query = process.argv.slice(2).join(' ');
    if (!query) {
        console.log('❌ Please provide a query as an argument.');
        process.exit(1);
    }

    console.log(`🚀 Querying RAG for: "${query}"`);

    // Load RAG embeddings
    const ragEmbeddings = await loadRagEmbeddings();
    if (ragEmbeddings.length === 0) {
        console.log('❌ No RAG embeddings found.');
        return;
    }

    // Load embedding model
    console.log('📦 Loading embedding model...');
    const embedder = await pipeline('feature-extraction', 'Xenova/all-MiniLM-L6-v2');

    // Embed the query
    const rawQueryEmbedding = await embedder(query);
    let queryEmbedding = normalizeEmbedding(rawQueryEmbedding);
    if (queryEmbedding instanceof Float32Array) queryEmbedding = Array.from(queryEmbedding);

    // Compute similarity
    const results = ragEmbeddings.map((item) => {
        const score = cosineSimilarity(queryEmbedding, item.embedding);
        return { file: item.file, score, description: item.description };
    });

    // Sort by descending similarity
    results.sort((a, b) => b.score - a.score);

    // Show top 5 results
    console.log('\nTop matches:');
    results.slice(0, 5).forEach((r, idx) => {
        console.log(`\n[${idx + 1}] ${r.file} (score: ${r.score.toFixed(4)})`);
        // console.log(r.description.slice(0, 500) + (r.description.length > 500 ? '...' : ''));
    });
})();
