const {
    copyFileSync,
    rmSync,
    readdirSync,
    lstatSync,
    readFileSync,
    writeFileSync,
    mkdirSync,
} = require('fs');
const path = require('path');

const ignore = [
    'NOTE.md',
    'node_modules',
    'waveshare/',
    'hardware/neotrillis/.pio',
    '.git',
    'hardware',
    'dustscript',
];
const extensions = ['.c', '.h', '.cpp'];

const docsFolder = 'docs';
const rootFolder = process.argv[2] || '.';
if (rootFolder === '.') {
    rmSync(docsFolder, { recursive: true, force: true });
}

function isIgnored(file) {
    file = file.toLowerCase();
    for (const i of ignore) {
        if (file.includes(i.toLowerCase())) {
            return true;
        }
    }
    return false;
}

function isAllowedExtension(file) {
    file = file.toLowerCase();
    for (const e of extensions) {
        if (file.endsWith(e)) {
            return true;
        }
    }
    return false;
}

// Should extract content from those comment format:
/*md content */
/*md
content
*/
const reg = /\/\*md\s*((.*?\n)+|(.*?))\s*\*\//g;
function extractMdComment(content) {
    const result = [];
    let match;
    while ((match = reg.exec(content)) !== null) {
        result.push(match[1]);
    }
    return result.join('\n\n');
}

function ensureDir(file) {
    try {
        if (lstatSync(folder).isDirectory()) {
            return;
        }
    } catch {}
    const folder = path.dirname(file);
    mkdirSync(folder, { recursive: true });
}

function docs(folder) {
    const files = readdirSync(folder);
    for (const file of files) {
        const filepath = path.join(folder, file);
        if (isIgnored(filepath)) {
            continue;
        }
        if (lstatSync(filepath).isDirectory()) {
            docs(filepath);
        } else if (filepath.endsWith('.md')) {
            const distPath = path.join(docsFolder, filepath);
            ensureDir(distPath);
            copyFileSync(filepath, distPath);
        } else if (isAllowedExtension(filepath)) {
            const content = readFileSync(filepath, 'utf8');
            const md = extractMdComment(content);
            if (md) {
                const mdFile = path.format({ ...path.parse(filepath), base: '', ext: '.md' });
                const mdPath = path.join(docsFolder, mdFile);
                ensureDir(mdPath);
                writeFileSync(mdPath, md);
            }
        }
    }
}

docs(rootFolder);
