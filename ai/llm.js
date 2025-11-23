import { pipeline } from '@xenova/transformers';
import fs from 'fs';

async function summarize(code) {
  const summarizer = await pipeline(
    'text-generation',
    'meta-llama/Llama-3.2-1B',
    { quantized: true }
  );

  const prompt = `<|user|>
Summarize this code in plain English (purpose, structure, behavior):
${code}
<|assistant|>`;

  const resp = await summarizer(prompt, {
    max_new_tokens: 200,
    temperature: 0.1
  });

  return resp[0].generated_text;
}

async function run() {
  const file = fs.readFileSync('/home/alex/Music/zicBox/helpers/GpioEncoder.h', 'utf8');
  const summary = await summarize(file);
  console.log('Summary:\n', summary);
}

run();
