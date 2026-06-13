'use strict';

const { execSync } = require('node:child_process');
const path = require('node:path');

const root = path.join(__dirname, '..');
const pkgPath = path.join(root, 'package.json');
const { hasBinding } = require('./install-binding');

function run(command) {
  execSync(command, { cwd: root, stdio: 'inherit' });
}

if (!hasBinding()) {
  try {
    run('node-pre-gyp install');
  } catch {
    // Prebuild download failed; compile fallback below.
  }
}

if (!hasBinding()) {
  run('node scripts/install-build-lib.js');
  run('node-pre-gyp rebuild');
  run('node scripts/install-copy-binding.js');
}

run('node scripts/install-cleanup.js');
