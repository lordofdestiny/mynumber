#!/usr/bin/env node
'use strict';

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import esbuild from 'esbuild';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const outDir = path.join(root, 'build/js-fallback-publish');
const outFile = path.join(outDir, 'index.js');

fs.rmSync(outDir, { recursive: true, force: true });
fs.mkdirSync(outDir, { recursive: true });

await esbuild.build({
  entryPoints: [path.join(root, 'src/js-fallback/index.ts')],
  outfile: outFile,
  bundle: true,
  minify: true,
  platform: 'node',
  target: 'node18',
  format: 'cjs',
  sourcemap: false,
  logLevel: 'info',
});
