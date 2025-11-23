import FlexSearch from 'flexsearch';
import fs from 'fs/promises';
import { extractHeaderAndContent, getFiles, updateProgress } from './lib.js';

async function buildIndex() {
    const files = await getFiles(process.cwd()); // recursively get all code files

    // Node-friendly FlexSearch Index
    const index = new FlexSearch.Index({
        tokenize: 'forward',
        encode: 'advanced',
        threshold: 0,
        resolution: 9,
        doc: {
            id: 'id',
            field: ['content'],
        },
    });

    const docs = [];
    let counter = 0;

    for (const file of files) {
        counter++;
        updateProgress(counter, files.length);

        let raw;
        try {
            raw = await fs.readFile(file, 'utf-8');
        } catch {
            continue; // skip unreadable files
        }

        const { cleanContent } = extractHeaderAndContent(raw);

        const headerMatch = raw.match(/\/\*\*\s*Description:[\s\S]*?\*\//i);
        const description = headerMatch ? headerMatch[0].replace(/\/\*\*|\*\//g, '').trim() : '';

        const content = description + '\n' + cleanContent;

        const doc = { id: counter, file, content };
        docs.push(doc);
        index.add(doc);
    }

    console.log('\n✅ Index built successfully!');
    return { index, docs };
}

async function searchIndex(query, index, docs) {
    const results = index.search(query, { limit: 5, enrich: true });
    if (!results || results.length === 0) return [];

    // Flatten enrich results
    const flat = results.flatMap((r) => r.result);
    return flat.map((r) => {
        const doc = docs.find((d) => d.id === r);
        return { file: doc.file, content: doc.content };
    });
}

(async () => {
    const query = process.argv.slice(2).join(' ');
    if (!query) {
        console.log('❌ Provide a query as argument.');
        process.exit(1);
    }

    console.log(`🔍 Building index and searching for: "${query}"...`);

    const { index, docs } = await buildIndex();
    const matches = await searchIndex(query, index, docs);

    if (matches.length === 0) {
        console.log('No matches found.');
        return;
    }

    console.log('\nTop matches:');
    matches.forEach((m, idx) => {
        console.log(`\n[${idx + 1}] ${m.file}`);
        console.log(m.content.slice(0, 300) + (m.content.length > 300 ? '...' : ''));
    });
})();
