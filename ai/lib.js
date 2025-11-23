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

export function normalizeEmbedding(output) {
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