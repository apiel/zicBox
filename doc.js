const { copyFileSync, rmSync, readdirSync, lstatSync, readFileSync, writeFileSync, mkdirSync } = require('fs');
const path = require('path');

const ignore = ['README.md', 'NOTE.md', 'node_modules', 'waveshare/', 'hardware/neotrillis/.pio', '.git'];
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

function dirExists(folder) {
    try {
        return lstatSync(folder).isDirectory();
    } catch {
        return false;
    }
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
            copyFileSync(filepath, distPath);
        } else if (isAllowedExtension(filepath)) {
            const content = readFileSync(filepath, 'utf8');
            const md = extractMdComment(content);
            if (md) {
                const mdFile = path.format({ ...path.parse(filepath), base: '', ext: '.md' });
                const mdPath = path.join(docsFolder, mdFile);
                const mdFolder = path.dirname(mdPath);
                if (!dirExists(mdFolder)) {
                    mkdirSync(mdFolder, { recursive: true });
                }
                writeFileSync(mdPath, md);
            }
        }
    }
}

docs(rootFolder);
