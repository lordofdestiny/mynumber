'use strict';

const { spawnSync, execSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');
const {
  cmakeBuildConfigArgs,
  cmakeConfigureArgs,
  findBuiltStaticLib,
  staticLibFileName,
} = require('./native-platform');

const root = path.join(__dirname, '..');
const nativeLibDir = path.join(root, 'native-lib');
const libPath = path.join(nativeLibDir, staticLibFileName());
const cmakeBuildDir = path.join(nativeLibDir, 'cmake-build');

/** @param {string} command @param {string[]} args */
function run(command, args) {
  const result = spawnSync(command, args, { cwd: root, stdio: 'inherit' });
  if (result.status !== 0) {
    throw new Error(`${command} ${args.join(' ')} failed with status ${result.status ?? 'unknown'}`);
  }
}

function hasCmake() {
  try {
    execSync('cmake --version', { stdio: 'ignore' });
    return true;
  } catch {
    return false;
  }
}

if (fs.existsSync(libPath)) {
  process.exit(0);
}

if (!hasCmake()) {
  console.warn(
    'install-build-lib: cmake not found; node-gyp compile fallback may fail without a staged static library',
  );
  process.exit(0);
}

const jobs = Math.max(1, Number(process.env.JOBS) || require('node:os').cpus().length || 4);

run('cmake', [
  '-B',
  'native-lib/cmake-build',
  ...cmakeConfigureArgs(),
  '-DMYNUMBER_BUILD_CLI=OFF',
]);
run('cmake', [
  '--build',
  'native-lib/cmake-build',
  ...cmakeBuildConfigArgs(),
  '--target',
  'mynumber_static',
  '--parallel',
  String(jobs),
]);

const builtLib = findBuiltStaticLib(cmakeBuildDir);
if (!builtLib) {
  throw new Error(`install-build-lib: expected static library under ${cmakeBuildDir}`);
}

fs.mkdirSync(nativeLibDir, { recursive: true });
fs.copyFileSync(builtLib, libPath);

if (!fs.existsSync(libPath)) {
  throw new Error(`install-build-lib: expected static library at ${libPath}`);
}

fs.rmSync(cmakeBuildDir, { recursive: true, force: true });
