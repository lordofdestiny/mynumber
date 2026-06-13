'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { findBuiltStaticLib, staticLibFileName } = require('./native-platform');

const root = path.join(__dirname, '..');
const cmakeBuildDir = process.argv[2];

if (!cmakeBuildDir) {
  console.error('usage: node scripts/copy-native-static-lib.js <cmake-build-dir>');
  process.exit(1);
}

const builtLib = findBuiltStaticLib(path.resolve(root, cmakeBuildDir));
if (!builtLib) {
  console.error(`copy-native-static-lib: no static library found under ${cmakeBuildDir}`);
  process.exit(1);
}

const nativeLibDir = path.join(root, 'native-lib');
const stagedLib = path.join(nativeLibDir, staticLibFileName());

fs.mkdirSync(nativeLibDir, { recursive: true });
fs.copyFileSync(builtLib, stagedLib);

console.log(`copy-native-static-lib: ${builtLib} -> ${stagedLib}`);
