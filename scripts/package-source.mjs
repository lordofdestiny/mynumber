#!/usr/bin/env node
'use strict';

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execSync } from 'node:child_process';
import { getVersion } from './project-meta.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const version = getVersion();
const bundleRoot = `mynumber-cmake-${version}`;
const stageDir = path.join(root, 'dist/release', bundleRoot);
const outDir = path.join(root, 'dist/release');
const outZip = path.join(outDir, `mynumber-cmake-${version}.zip`);
const outTarGz = path.join(outDir, `mynumber-cmake-${version}.tar.gz`);

/** @param {string} src @param {string} dest */
function copyFile(src, dest) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  fs.copyFileSync(src, dest);
}

/** @param {string} src @param {string} dest */
function copyDir(src, dest) {
  fs.mkdirSync(dest, { recursive: true });
  for (const entry of fs.readdirSync(src, { withFileTypes: true })) {
    const from = path.join(src, entry.name);
    const to = path.join(dest, entry.name);
    if (entry.isDirectory()) {
      copyDir(from, to);
    } else {
      copyFile(from, to);
    }
  }
}

fs.rmSync(stageDir, { recursive: true, force: true });
fs.mkdirSync(stageDir, { recursive: true });

copyFile(path.join(root, 'packaging/native-cmake/CMakeLists.txt'), path.join(stageDir, 'CMakeLists.txt'));
copyDir(path.join(root, 'packaging/native-cmake/cmake'), path.join(stageDir, 'cmake'));
copyFile(path.join(root, 'packaging/project.json'), path.join(stageDir, 'packaging/project.json'));
copyFile(path.join(root, 'include/mynumber.hpp'), path.join(stageDir, 'include/mynumber.hpp'));
copyDir(path.join(root, 'include/impl'), path.join(stageDir, 'include/impl'));
copyDir(path.join(root, 'include/polyfill'), path.join(stageDir, 'include/polyfill'));
copyDir(path.join(root, 'src/impl'), path.join(stageDir, 'src/impl'));
copyFile(path.join(root, 'src/main.cpp'), path.join(stageDir, 'src/main.cpp'));

fs.mkdirSync(outDir, { recursive: true });
for (const archive of [outZip, outTarGz]) {
  if (fs.existsSync(archive)) {
    fs.rmSync(archive);
  }
}

execSync(`zip -r "${outZip}" "${bundleRoot}"`, {
  cwd: outDir,
  stdio: 'inherit',
});

execSync(`tar -czf "${outTarGz}" "${bundleRoot}"`, {
  cwd: outDir,
  stdio: 'inherit',
});

console.log(`Created ${outZip}`);
console.log(`Created ${outTarGz}`);
