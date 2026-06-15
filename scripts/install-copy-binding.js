'use strict';

const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const moduleName = 'mynumber';

/** @returns {string | undefined} */
function findTargetPath() {
  return path.join(root, 'lib', 'binding', `${moduleName}.node`);
}

/** @returns {string[]} */
function sourceCandidates() {
  const sources = [
    path.join(root, 'build', 'Release', `${moduleName}.node`),
    path.join(root, 'build', 'Debug', `${moduleName}.node`),
    path.join(root, 'build', 'prebuilt', `${moduleName}.node`),
  ];

  return sources;
}

const target = findTargetPath();
if (!target || fs.existsSync(target)) {
  process.exit(0);
}

for (const source of sourceCandidates()) {
  if (!fs.existsSync(source)) {
    continue;
  }

  fs.mkdirSync(path.dirname(target), { recursive: true });
  fs.copyFileSync(source, target);
  process.exit(0);
}
