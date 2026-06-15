const { spawnSync, execSync } = require('node:child_process');
const util = require('node:util');

function runAt(cwd, command, args) {
  const result = spawnSync(command, args, { cwd, stdio: 'inherit' });
  if (result.status !== 0) {
    throw new Error(`${command} ${args.join(' ')} failed with status ${result.status ?? 'unknown'}`);
  }
}

module.exports = { runAt }