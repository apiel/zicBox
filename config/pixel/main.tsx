import { writeFileSync } from 'fs';
import 'tsconfig-paths/register'; // To solve importedalias

console.log('starting...');

import { generateAudioMakefile } from '@/libs/audio';
import { stringifyWithCompactedKeys } from '@/libs/stringify';
import { generateComponentMakefile } from '@/libs/ui';
import { audio } from './audio';
import { ui } from './ui';

// const output = JSON.stringify({ audio, ...ui }, null, 2);
// const output = stringifyWithLimitedIndentation({ audio, ...ui });
const output = stringifyWithCompactedKeys({ audio, ...ui });

writeFileSync('data/config.json', output);
generateAudioMakefile('makeconf/audio.mk');
generateComponentMakefile('makeconf/component.mk');
console.log('done');

