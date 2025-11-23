import crypto from 'crypto';
import { promises as fs } from 'fs';
import path from 'path';

export const ignore = [
    'node_modules',
    '.git',
    'build',
    'dist',
    'os',
    'libs/httplib/',
    'libs/nlohmann',
    'config/pixel',
    'config/grid',
    'config/grain',
    'config/desktop',
];
export const extensions = ['.cpp', '.h', '.ts'];

export function isIgnored(file) {
    const lower = file.toLowerCase();
    return ignore.some((i) => lower.includes(i.toLowerCase()));
}

// --- Progress Bar Utility ---
export function updateProgress(current, total) {
    const barLength = 30;
    const percentage = current / total;
    const filled = Math.round(barLength * percentage);
    const empty = barLength - filled;

    const bar = '█'.repeat(filled) + '-'.repeat(empty);
    const pctText = (percentage * 100).toFixed(1);

    process.stdout.write(`\rProgress: [${bar}] ${pctText}% (${current}/${total})`);

    if (current === total) {
        process.stdout.write('\n'); // newline when done
    }
}

export function calculateSha256(content) {
    return crypto.createHash('sha256').update(content, 'utf8').digest('hex');
}

export function extractHeaderAndContent(content) {
    const singleHeaderPattern = /\/\*\*\s*Description[\s\S]*?\*\/|sha:\s*([a-fA-F0-9]{64})/;

    let contentWithoutHeaders = content;
    let lastRemovedSha = null;
    let headersRemoved = 0;

    while (true) {
        const match = contentWithoutHeaders.match(singleHeaderPattern);

        if (match && contentWithoutHeaders.indexOf(match[0]) === 0) {
            const shaMatch = match[0].match(/sha: ([a-fA-F0-9]{64})/);
            if (shaMatch) lastRemovedSha = shaMatch[1];

            contentWithoutHeaders = contentWithoutHeaders.substring(match[0].length).trimStart();
            headersRemoved++;
        } else {
            break;
        }
    }

    return {
        headerExists: headersRemoved > 0,
        existingSha: lastRemovedSha,
        cleanContent: contentWithoutHeaders,
    };
}


export async function getFiles(dir = '.', fileList = []) {
    const files = await fs.readdir(dir);

    for (const file of files) {
        const filePath = path.join(dir, file);
        const stat = await fs.stat(filePath);

        if (isIgnored(filePath)) continue;

        if (stat.isDirectory()) {
            fileList = await getFiles(filePath, fileList);
        } else {
            const fileExtension = path.extname(filePath).toLowerCase();
            if (extensions.includes(fileExtension)) {
                fileList.push(filePath);
            }
        }
    }

    return fileList;
}