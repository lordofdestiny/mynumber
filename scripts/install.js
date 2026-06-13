'use strict';

const { execSync } = require('node:child_process');
const path = require('node:path');

const root = path.join(__dirname, '..');
const { hasBinding } = require('./install-binding');
const { tryInstallFromNodeRelease } = require('./install-release-binary');

function run(command) {
  execSync(command, { cwd: root, stdio: 'inherit' });
}

async function main() {
  if (!hasBinding()) {
    await tryInstallFromNodeRelease();
  }

  if (!hasBinding()) {
    run('node scripts/install-build-lib.js');
    run('node-pre-gyp rebuild');
    run('node scripts/install-copy-binding.js');
  }

  run('node scripts/install-cleanup.js');
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
