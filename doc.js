const { lstatSync, mkdirSync, readdirSync, readFileSync, rmSync, writeFileSync } = require('fs');
const path = require('path');

const ignore = [
    // './README.md',
    'NOTE.md',
    'node_modules',
    'waveshare/',
    'plugins/components/__deprecated_SDL',
    'plugins/components/__deprecated_Pixel',
    'plugins/components/__toBeFixed_Pixel',
    'plugins/controllers/__deprecated',
    'plugins/audio/__deprecated',
    'hardware/ZicPad/neotrillis/.pio',
    'hardware/encoderModule/.pio',
    '.git',
    'dustscript',
    'wiki',
    'plugins/config',
    'libs',
    'os/',
    'plugins/audio/',
    'pico/pico-',
    'build/',
    'zicBlock',
    'managed_components',
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
const reg1 = /\*md\s((?:.|\n)*?)\s*\*\//g; //--> /*md content */
const reg2 = /\/\/md\s(.*)/g; //--> //md content
const reg3 = /"(\w+)".*\/\/eg:\s*(.*)/g; //--> textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"
const reg4 = /\/\/\/\s(.*)/g; //--> /// description

const reg = new RegExp(
    reg1.source + '|' + reg2.source + '|' + reg3.source + '|' + reg4.source,
    'g'
);

function extractMdComment(content) {
    const result = [];
    let match;
    // while ((match = reg1.exec(content)) !== null || (match = reg2.exec(content)) !== null) {
    while ((match = reg.exec(content)) !== null) {
        if (match[5]) {
            result.push(`  // ${match[5]}`);
        } else if (match[3] && match[4]) {
            const value = match[4][0] === '"' ? match[4] : `{${match[4]}}`;
            result.push(`  ${match[3]}=${value}`);
        } else {
            const extractedContent = match[1] || match[2];
            const mdConfigMacro = 'md_config:';
            const mdConfigEndMacro = 'md_config_end';
            if (extractedContent.startsWith(mdConfigMacro)) {
                const componentName = extractedContent.slice(mdConfigMacro.length);
                result.push(`
\`\`\`tsx
<${componentName}`);
            } else if (extractedContent.startsWith(mdConfigEndMacro)) {
                result.push(`/>
\`\`\`
                `);
            } else {
                result.push(extractedContent);
            }
        }
    }
    return result.join('\n');
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

function extractTitle2(lines) {
    return lines
        .filter((line) => line.trim().startsWith('## '))
        .map((line) => line.replace('## ', ''));
}

const fileList = [];

function docs(folder) {
    const contents = [];
    const files = readdirSync(folder);
    for (const file of files) {
        try {
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
        } catch (error) {
            console.error(`Error in file ${folder}/${file}`);
            throw error;
        }
    }
    if (contents.length) {
        const lines = contents.join('\n\n').split('\n');
        const header = lines.shift();
        const content = lines.join('\n');
        const title2 = extractTitle2(lines);
        const filename = header.replace(/^#+\s+/, '').replaceAll(' ', '-');
        const wikiFile = path.join(docsFolder, filename, `${filename}.md`);

console.log(wikiFile);

        // if filename in fileList, prepend data to file because parent folder will always come last
        const fileListItem = fileList.find((f) => f.filename === filename);
        if (fileListItem) {
            fileListItem.title2 = [...title2, ...fileListItem.title2];
            const appendContent = readFileSync(wikiFile, 'utf8');
            writeFileSync(wikiFile, content + '\n\n' + appendContent);
        } else {
            fileList.push({ filename, title2 });
            ensureDir(wikiFile);
            writeFileSync(wikiFile, content);
        }
    }
}

docs(rootFolder);

fileList.sort((a, b) => a.filename.localeCompare(b.filename));

for (const [index, fileActive] of fileList.entries()) {
    const sidebar =
        `[Home: Getting started](https://github.com/apiel/zicBox/wiki)\n\n` +
        fileList
            .map((file) => {
                if (file.filename === fileActive.filename) {
                    const title2 = file.title2
                        .map(
                            (title) =>
                                ` - [${title}](https://github.com/apiel/zicBox/wiki/${
                                    file.filename
                                }#${title.toLowerCase().replaceAll(' ', '-')})`
                        )
                        .join('\n');

                    return `**${file.filename}**\n${title2}`;
                }
                return `[${file.filename}](https://github.com/apiel/zicBox/wiki/${file.filename})`;
            })
            .join('\n\n');
    writeFileSync(path.join(docsFolder, fileActive.filename, '_Sidebar.md'), sidebar);

    const previous =
        index > 0
            ? `<b>Previous</b>: <a href="https://github.com/apiel/zicBox/wiki/${
                  fileList[index - 1].filename
              }">${fileList[index - 1].filename}</a>`
            : `<b>Previous</b>: <a href="https://github.com/apiel/zicBox/wiki">Home</a>`;
    const next =
        index < fileList.length - 1
            ? '<b>Next</b>: <a href="https://github.com/apiel/zicBox/wiki/' +
              fileList[index + 1].filename +
              '">' +
              fileList[index + 1].filename +
              '</a>'
            : '';
    const footer = `
<p align="center">
    ${previous} ${previous && next ? ' | ' : ''} ${next}
</p>
    `;

    writeFileSync(path.join(docsFolder, fileActive.filename, '_Footer.md'), footer);
}

const sidebar = fileList
    .map(({ filename }) => `[${filename}](https://github.com/apiel/zicBox/wiki/${filename})`)
    .join('\n\n');
writeFileSync(path.join(docsFolder, '..', '_Sidebar.md'), sidebar);

const root = readFileSync('README.md', 'utf8');
const home = root.split('\n').slice(1).join('\n');
writeFileSync(path.join('wiki', 'Home.md'), home);
