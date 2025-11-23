import fs from 'fs/promises';
import path from 'path';

const RAG_DIR = 'ai/rag';

function normalize(text) {
    return text.toLowerCase();
}

// Count query words present in text
function scoreMatch(text, query) {
    const words = query.toLowerCase().split(/\s+/);
    let score = 0;
    for (const word of words) {
        if (text.includes(word)) score++;
    }
    return score / words.length; // fraction of words matched
}

async function loadRagDescriptions() {
    const files = await fs.readdir(RAG_DIR);
    const data = [];
    for (const file of files) {
        if (!file.endsWith('.json')) continue;
        const json = JSON.parse(await fs.readFile(path.join(RAG_DIR, file), 'utf-8'));
        data.push({ file: json.file, description: json.description });
    }
    return data;
}

async function main() {
    const query = process.argv.slice(2).join(' ').toLowerCase();
    if (!query) {
        console.log('Provide a query as argument.');
        return;
    }

    const ragData = await loadRagDescriptions();

    const scored = ragData
        .map((item) => ({
            file: item.file,
            description: item.description,
            score: scoreMatch(normalize(item.description), query),
        }))
        .filter((x) => x.score > 0)
        .sort((a, b) => b.score - a.score);

    console.log('Top matches:');
    scored.slice(0, 5).forEach((r, idx) => {
        console.log(`\n[${idx + 1}] ${r.file} (score: ${r.score.toFixed(2)})`);
        // console.log(r.description.slice(0, 300) + (r.description.length > 300 ? '...' : ''));
    });
}

main();
