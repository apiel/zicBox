import fs from 'fs/promises';
import lunr from 'lunr';
import { extractHeaderAndContent, getFiles, updateProgress } from './lib.js';

async function buildIndex() {
    const files = await getFiles(process.cwd());

    const docs = [];
    let counter = 0;

    for (const file of files) {
        counter++;
        updateProgress(counter, files.length);

        let raw;
        try {
            raw = await fs.readFile(file, 'utf-8');
        } catch {
            continue;
        }

        const { cleanContent } = extractHeaderAndContent(raw);
        const headerMatch = raw.match(/\/\*\*\s*Description:[\s\S]*?\*\//i);
        const description = headerMatch
            ? headerMatch[0].replace(/\/\*\*|\*\//g, '').trim()
            : '';

        const content = description + '\n' + cleanContent;

        docs.push({ id: counter, file, content });
    }

    // Build Lunr index
    const idx = lunr(function () {
        this.ref('id');
        this.field('content');

        docs.forEach(doc => this.add(doc));
    });

    console.log('\n✅ Lunr index built successfully!');
    return { idx, docs };
}

async function searchIndex(query, idx, docs) {
    const results = idx.search(query); // default OR search
    return results.map(r => {
        const doc = docs.find(d => d.id === parseInt(r.ref));
        return { file: doc.file, content: doc.content, score: r.score };
    }).slice(0, 5); // top 5
}

(async () => {
    const query = process.argv.slice(2).join(' ');
    if (!query) {
        console.log('❌ Provide a query as argument.');
        process.exit(1);
    }

    console.log(`🔍 Building index and searching for: "${query}"...`);

    const { idx, docs } = await buildIndex();
    const matches = await searchIndex(query, idx, docs);

    if (matches.length === 0) {
        console.log('No matches found.');
        return;
    }

    console.log('\nTop matches:');
    matches.forEach((m, idx) => {
        console.log(`\n[${idx + 1}] ${m.file} (score: ${m.score.toFixed(4)})`);
    });
})();
