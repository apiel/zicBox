import { writeFileSync } from 'fs';
import 'tsconfig-paths/register'; // To solve importedalias

console.log('starting...');

import { stringifyWithCompactedKeys } from '@/libs/stringify';
import { audio } from './audio';
import { ui } from './ui';

// const output = JSON.stringify({ audio, ...ui }, null, 2);
// const output = stringifyWithLimitedIndentation({ audio, ...ui });
const output = stringifyWithCompactedKeys({ audio, ...ui });

writeFileSync('config.json', output);

console.log('done');

