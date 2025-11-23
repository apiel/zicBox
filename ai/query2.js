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

    // const fileMap = [];
    let idCounter = 0;

    // Use a simple array for reliable iteration
    for (const filePath of files) {
        // 1. Define the unique, stable numeric ID for this file
        const currentId = idCounter++;

        let raw;
        try {
            raw = await fs.readFile(filePath, 'utf-8');
        } catch {
            continue;
        }

        // 2. Register the mapping BEFORE the async FlexSearch call.
        // This ensures the map is populated sequentially and correctly.
        // fileMap.set(currentId, filePath);
        // fileMap.push(filePath);

        // 3. Add the content to the FlexSearch index using the same ID.
        // We await this, but the map entry is already guaranteed.
        await index.addAsync(currentId, raw);
    }

    // console.log(fileMap);
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
