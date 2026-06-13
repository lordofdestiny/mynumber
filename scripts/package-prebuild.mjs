#!/usr/bin/env node
'use strict';

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const pkgDir = path.join(root, 'dist/npm/mynumber');
const addon = path.join(pkgDir, 'mynumber.node');

if (!fs.existsSync(addon)) {
  console.error('dist/npm/mynumber/mynumber.node not found. Run: npm run stage:npm');
  process.exit(1);
}

const pkg = JSON.parse(fs.readFileSync(path.join(pkgDir, 'package.json'), 'utf8'));
const napiVersion = pkg.binary?.napi_versions?.[0] ?? process.versions.napi;
const bindingDir = path.join(pkgDir, `lib/binding/napi-v${napiVersion}`);
fs.mkdirSync(bindingDir, { recursive: true });
fs.copyFileSync(addon, path.join(bindingDir, 'mynumber.node'));

execSync('npx node-pre-gyp package', { cwd: pkgDir, stdio: 'inherit' });
