'use strict';

const { spawnSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');

const {runAt} = require('./common');
const {
  cmakeBuildConfigArgs,
  cmakeConfigureArgs,
} = require('./native-platform');

const root = path.join(__dirname, '..');
const distNodeDir = path.join(root, 'dist/node');
const addonBin = path.join(root, 'build/Release/mynumber.node');
const jobs = Math.max(1, Number(process.env.JOBS) || require('node:os').cpus().length || 4);
const nodeGyp = path.join(root, 'node_modules', 'node-gyp', 'bin', 'node-gyp.js');

const run = (command, args) => runAt(root, command, args);

run('cmake', [
  '-B',
  'build/native',
  ...cmakeConfigureArgs(),
  '-DMYNUMBER_BUILD_CLI=OFF',
]);
run('cmake', [
  '--build',
  'build/native',
  ...cmakeBuildConfigArgs(),
  '--target',
  'mynumber_static',
  '--parallel',
  String(jobs),
]);
run('node', ['scripts/copy-native-static-lib.js', 'build/native']);
run('node', [nodeGyp, 'configure']);
run('node', [nodeGyp, 'build', '-j', String(jobs), "--verbose"]);

if (!fs.existsSync(addonBin)) {
  throw new Error(`build-dist-node: expected addon at ${addonBin}`);
}

fs.mkdirSync(distNodeDir, { recursive: true });
fs.copyFileSync(addonBin, path.join(distNodeDir, 'mynumber.node'));
fs.copyFileSync(path.join(root, 'src/node/index.js'), path.join(distNodeDir, 'index.js'));
fs.copyFileSync(path.join(root, 'src/node/index.d.ts'), path.join(distNodeDir, 'index.d.ts'));

console.log(`build-dist-node: staged ${distNodeDir}`);
