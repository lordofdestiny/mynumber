'use strict';

const fs = require('node:fs');
const path = require('node:path');

/** @returns {string | undefined} */
function releasePlatform() {
  switch (process.platform) {
    case 'darwin':
      return 'macos';
    case 'linux':
      return 'linux';
    case 'win32':
      return 'windows';
    default:
      return undefined;
  }
}

/** @returns {boolean} */
function isWindows() {
  return process.platform === 'win32';
}

/** @returns {string} */
function staticLibFileName() {
  return isWindows() ? 'mynumber.lib' : 'libmynumber.a';
}

/** @returns {string[]} */
function cmakeConfigureArgs() {
  if (isWindows()) {
    return [];
  }
  return ['-G', 'Unix Makefiles', '-DCMAKE_BUILD_TYPE=Release'];
}

/** @returns {string[]} */
function cmakeBuildConfigArgs() {
  return isWindows() ? ['--config', 'Release'] : [];
}

/**
 * @param {string} cmakeBuildDir
 * @returns {string | undefined}
 */
function findBuiltStaticLib(cmakeBuildDir) {
  const name = staticLibFileName();
  const candidates = [
    path.join(cmakeBuildDir, 'Release', name),
    path.join(cmakeBuildDir, name),
    path.join(cmakeBuildDir, staticLibFileName()),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return undefined;
}

/** @returns {string} */
function sharedLibFileName() {
  if (isWindows()) {
    return 'mynumber.dll';
  }
  if (process.platform === 'darwin') {
    return 'libmynumber.dylib';
  }
  return 'libmynumber.so';
}

/**
 * @param {string} cmakeBuildDir
 * @returns {string | undefined}
 */
function findBuiltSharedLib(cmakeBuildDir) {
  const name = sharedLibFileName();
  const candidates = [
    path.join(cmakeBuildDir, 'Release', name),
    path.join(cmakeBuildDir, name),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return undefined;
}

/** @returns {string} */
function cliFileName() {
  return isWindows() ? 'mynumber.exe' : 'mynumber';
}

module.exports = {
  releasePlatform,
  isWindows,
  staticLibFileName,
  sharedLibFileName,
  cliFileName,
  cmakeConfigureArgs,
  cmakeBuildConfigArgs,
  findBuiltStaticLib,
  findBuiltSharedLib,
};
