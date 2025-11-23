import fs from 'fs/promises';
import { extractHeaderAndContent, getFiles } from './lib.js';

// Compute a simple match score: fraction of query words found in text
function scoreMatch(text, query) {
    const words = query.toLowerCase().split(/\s+/);
    const textLower = text.toLowerCase();
    let score = 0;
    for (const word of words) {
        if (textLower.includes(word)) score++;
    }
    return score / words.length;
}

async function searchCode(query) {
    const files = await getFiles(process.cwd()); // recursively get all code files
    const results = [];

    for (const filePath of files) {
        let raw;
        try {
            raw = await fs.readFile(filePath, 'utf-8');
        } catch {
            continue; // skip unreadable files
        }

        const { cleanContent } = extractHeaderAndContent(raw);

        // Extract description header if exists
        const headerMatch = raw.match(/\/\*\*\s*Description:[\s\S]*?\*\//i);
        const description = headerMatch
            ? headerMatch[0].replace(/\/\*\*|\*\//g, '').trim()
            : '';

        // Combine description + code for searching
        const searchText = description + '\n' + cleanContent;

        const score = scoreMatch(searchText, query);
        if (score > 0) {
            results.push({ file: filePath, score, description });
        }
    }

    results.sort((a, b) => b.score - a.score);
    return results.slice(0, 5); // top 5 matches
}

// --- Main ---
(async () => {
    const query = process.argv.slice(2).join(' ');
    if (!query) {
        console.log('❌ Provide a query as argument.');
        process.exit(1);
    }

    console.log(`🔍 Searching for: "${query}"...`);

    const topMatches = await searchCode(query);

    if (topMatches.length === 0) {
        console.log('No matches found.');
        return;
    }

    console.log('\nTop matches:');
    topMatches.forEach((r, idx) => {
        console.log(`\n[${idx + 1}] ${r.file} (score: ${r.score.toFixed(2)})`);
    });
})();
