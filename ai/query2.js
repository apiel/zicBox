// query2.js (Only the indexFiles function changed)

import { Index } from 'flexsearch';
import fs from 'fs/promises';
import { getFiles } from './lib.js';

(async () => {
    const query = process.argv.slice(2).join(' ');
    if (!query) {
        console.log('❌ Provide a query as argument.');
        process.exit(1);
    }

    console.log(`\n🔍 Searching for: "${query}"...`);

    console.log('Building search index...');
    const files = await getFiles(process.cwd());

    const index = new Index({
        tokenize: 'forward',
        cache: true,
        context: true,
        async: true,
    });

    for (let i = 0; i < files.length; i++) {
        const filePath = files[i];
        let raw;
        try {
            raw = await fs.readFile(filePath, 'utf-8');
        } catch {
            continue;
        }
        await index.addAsync(i, raw);
    }

    console.log(`Index built for ${files.size} files.`);

    const results = await index.searchAsync(query, {
        limit: 5,
        suggest: true,
    });

    console.log(`Found ${results.length} results.`, results);

    for (const result of results) {
        const fileId = Number(result);
        const filePath = files[fileId];
        console.log(`${filePath} [score: ${result.score || 1}]`);
    }
})();
