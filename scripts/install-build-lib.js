'use strict';

const { execSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const nativeLibDir = path.join(root, 'native-lib');
const libPath = path.join(nativeLibDir, 'libmynumber.a');
const cmakeBuildDir = path.join(nativeLibDir, 'cmake-build');

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
    'install-build-lib: cmake not found; node-gyp compile fallback may fail without libmynumber.a',
  );
  process.exit(0);
}

const jobs = process.env.JOBS || require('node:os').cpus().length;

execSync('cmake -B native-lib/cmake-build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DMYNUMBER_BUILD_CLI=OFF', {
  cwd: root,
  stdio: 'inherit',
});

execSync(`cmake --build native-lib/cmake-build --target mynumber_static --parallel ${jobs}`, {
  cwd: root,
  stdio: 'inherit',
});

const builtLib = path.join(cmakeBuildDir, 'libmynumber.a');
if (!fs.existsSync(builtLib)) {
  throw new Error(`install-build-lib: expected static library at ${builtLib}`);
}

fs.mkdirSync(nativeLibDir, { recursive: true });
fs.copyFileSync(builtLib, libPath);

if (!fs.existsSync(libPath)) {
  throw new Error(`install-build-lib: expected static library at ${libPath}`);
}
