'use strict';

const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const { hasBinding } = require('./install-binding');

if (!hasBinding()) {
  console.warn('install-cleanup: no native binding found; skipping artifact cleanup');
  process.exit(0);
}

const targets = [
  path.join(root, 'native-lib'),
  path.join(root, 'build'),
  path.join(root, 'mynumber.node'),
];

for (const target of targets) {
  if (!fs.existsSync(target)) {
    continue;
  }

  fs.rmSync(target, { recursive: true, force: true });
}
