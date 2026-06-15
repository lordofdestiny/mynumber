'use strict';

const path = require('node:path');

const { runAt } = require('./common')
const { hasBinding } = require('./install-binding');
const { isWindows } = require('./native-platform')
const { tryInstallFromNodeRelease } = require('./install-release-binary');

const root = path.join(__dirname, '..');

const run = (command, args) => runAt(root, command, args);

async function main() {
  if (!hasBinding()) {
    await tryInstallFromNodeRelease();
  }

  if (!hasBinding()) {
    run('node', ['scripts/install-build-lib.js']);
  }

  run('node', ['scripts/install-copy-binding.js']);
  run('node', ['scripts/install-cleanup.js']);
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
