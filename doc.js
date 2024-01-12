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
    './README.md',
    'NOTE.md',
    'node_modules',
    'waveshare/',
    'hardware/neotrillis/.pio',
    '.git',
    'hardware',
    'dustscript',
    'wiki',
];
const extensions = ['.c', '.h', '.cpp'];

const docsFolder = 'wiki/docs';
const rootFolder = '.';

function folderExists(folder) {
    try {
        if (lstatSync(folder).isDirectory()) {
            return true;
        }
    } catch {}
    return false;
}

if (folderExists(docsFolder)) {
    rmSync(docsFolder, { recursive: true, force: true });
} else {
    console.error(`${docsFolder} not found. Are you sure you loaded wiki submodule?`);
    if (!process.argv.includes('--force')) {
        process.exit(1);
    }
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
// use https://regexr.com/ to test
const reg = /\/\*md\s*((.*?[^\*\/]\n)+|(.*?))\s*\*\//g;
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
    const contents = [];
    const files = readdirSync(folder);
    for (const file of files) {
        const filepath = path.join(folder, file);
        if (isIgnored(filepath)) {
            continue;
        }
        if (lstatSync(filepath).isDirectory()) {
            docs(filepath);
        } else if (filepath.toLowerCase().endsWith('readme.md')) {
            // Ignore root readme
            if (filepath.toLowerCase() === 'readme.md') {
                continue;
            }
            const content = readFileSync(filepath, 'utf8');
            contents.unshift(content);
        } else if (filepath.endsWith('.md')) {
            const content = readFileSync(filepath, 'utf8');
            contents.push(content);
        } else if (isAllowedExtension(filepath)) {
            const content = readFileSync(filepath, 'utf8');
            const md = extractMdComment(content);
            if (md) {
                contents.push(md);
            }
        }
    }
    if (contents.length) {
        const lines = contents.join('\n\n').split('\n');
        const header = lines.shift();
        const content = lines.join('\n');
        const filename = header.replace(/^#+\s+/, '').replace(' ', '-');
        const wikiFile = path.join(docsFolder, `${filename}.md`);
        ensureDir(wikiFile);
        writeFileSync(wikiFile, content);
    }
}

docs(rootFolder);
