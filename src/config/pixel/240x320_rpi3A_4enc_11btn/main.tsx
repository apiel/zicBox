import { writeFileSync } from 'fs';

import { audio } from './audio';
import { ui } from './ui';

const output = JSON.stringify({ audio, ...ui }, null, 2);

// console.log(JSON.stringify({ audio, ui }, null, 2));
// write file to config.json

writeFileSync('config.json', output);
