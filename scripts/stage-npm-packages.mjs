#!/usr/bin/env node
'use strict';

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execSync } from 'node:child_process';

import { readProject, syncVersionToPackageJson } from './project-meta.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');

syncVersionToPackageJson();

/** @param {string} src @param {string} dest */
function copyFile(src, dest) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  fs.copyFileSync(src, dest);
}

/** @param {string} src @param {string} dest */
function copyDir(src, dest) {
  fs.mkdirSync(dest, { recursive: true });
  for (const entry of fs.readdirSync(src, { withFileTypes: true })) {
    const from = path.join(src, entry.name);
    const to = path.join(dest, entry.name);
    if (entry.isDirectory()) {
      copyDir(from, to);
    } else {
      copyFile(from, to);
    }
  }
}

/** @param {string} dir */
function emptyDir(dir) {
  fs.rmSync(dir, { recursive: true, force: true });
  fs.mkdirSync(dir, { recursive: true });
}

/** @param {string} templatePath @param {Record<string, string>} replacements */
function loadTemplate(templatePath, replacements) {
  const raw = fs.readFileSync(templatePath, 'utf8');
  let text = raw;
  for (const [key, value] of Object.entries(replacements)) {
    text = text.replaceAll(key, value);
  }
  return JSON.parse(text);
}

/** @param {string} dest @param {Record<string, unknown>} pkg */
function writePkg(dest, pkg) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  fs.writeFileSync(dest, `${JSON.stringify(pkg, null, 2)}\n`);
}

const rootPkg = JSON.parse(fs.readFileSync(path.join(root, 'package.json'), 'utf8'));
const project = readProject();
const { owner, repo } = project.github;
const nativeOnly = process.argv.includes('--native-only');

const distNative = path.join(root, 'dist/npm/mynumber');
const distWasm = path.join(root, 'dist/npm/mynumber-wasm');

const nativeTemplate = loadTemplate(path.join(root, 'packaging/npm/mynumber.package.json'), {
  __GITHUB_OWNER__: owner,
  __GITHUB_REPO__: repo,
});

emptyDir(distNative);

copyFile(path.join(root, 'src/node/index.publish.js'), path.join(distNative, 'index.js'));
copyFile(path.join(root, 'src/node/index.d.ts'), path.join(distNative, 'index.d.ts'));
copyFile(path.join(root, 'binding.gyp'), path.join(distNative, 'binding.gyp'));
copyFile(path.join(root, 'packaging/native-cmake/CMakeLists.txt'), path.join(distNative, 'CMakeLists.txt'));
copyFile(path.join(root, 'packaging/native-cmake/cmake/mynumber.pc.in'), path.join(distNative, 'cmake/mynumber.pc.in'));
copyFile(path.join(root, 'packaging/native-cmake/cmake/mynumberConfig.cmake.in'), path.join(distNative, 'cmake/mynumberConfig.cmake.in'));
copyFile(path.join(root, 'packaging/project.json'), path.join(distNative, 'packaging/project.json'));
copyDir(path.join(root, 'include'), path.join(distNative, 'include'));
copyDir(path.join(root, 'src/api'), path.join(distNative, 'src/api'));
copyDir(path.join(root, 'src/impl'), path.join(distNative, 'src/impl'));
copyDir(path.join(root, 'src/wrapper'), path.join(distNative, 'src/wrapper'));
copyDir(path.join(root, 'src/polyfill'), path.join(distNative, 'src/polyfill'));
copyFile(path.join(root, 'scripts/common.js'), path.join(distNative, 'scripts/common.js'));
copyFile(path.join(root, 'scripts/install.js'), path.join(distNative, 'scripts/install.js'));
copyFile(path.join(root, 'scripts/install-release-binary.js'), path.join(distNative, 'scripts/install-release-binary.js'));
copyFile(path.join(root, 'scripts/install-binding.js'), path.join(distNative, 'scripts/install-binding.js'));
copyFile(path.join(root, 'scripts/install-build-lib.js'), path.join(distNative, 'scripts/install-build-lib.js'));
copyFile(path.join(root, 'scripts/install-copy-binding.js'), path.join(distNative, 'scripts/install-copy-binding.js'));
copyFile(path.join(root, 'scripts/install-cleanup.js'), path.join(distNative, 'scripts/install-cleanup.js'));
copyFile(path.join(root, 'scripts/native-platform.js'), path.join(distNative, 'scripts/native-platform.js'));
copyFile(path.join(root, 'scripts/copy-native-static-lib.js'), path.join(distNative, 'scripts/copy-native-static-lib.js'));

execSync('npm run build:js-fallback:publish', { cwd: root, stdio: 'inherit' });
const jsFallbackOut = path.join(root, 'build/js-fallback-publish/index.js');
if (!fs.existsSync(jsFallbackOut)) {
  console.error('build/js-fallback-publish/index.js not found. Run: npm run build:js-fallback:publish');
  process.exit(1);
}

fs.mkdirSync(path.join(distNative, 'lib/binding'), { recursive: true });
copyFile(jsFallbackOut, path.join(distNative, 'lib/js/index.js'));

writePkg(path.join(distNative, 'package.json'), {
  ...nativeTemplate,
  version: project.version,
  author: rootPkg.author,
  license: rootPkg.license,
  repository: rootPkg.repository ?? {
    type: 'git',
    url: `git+https://github.com/${owner}/${repo}.git`,
  },
});

const wasmJs = path.join(root, 'dist/wasm/mynumber.js');
const wasmBin = path.join(root, 'dist/wasm/mynumber.wasm');
if (!nativeOnly) {
  if (!fs.existsSync(wasmJs) || !fs.existsSync(wasmBin)) {
    console.error('dist/wasm/mynumber.{js,wasm} not found. Run: npm run build:wasm');
    process.exit(1);
  }

  emptyDir(distWasm);

  copyFile(path.join(root, 'src/emscripten/index.publish.js'), path.join(distWasm, 'index.js'));
  copyFile(path.join(root, 'src/emscripten/index.d.ts'), path.join(distWasm, 'index.d.ts'));
  copyFile(wasmJs, path.join(distWasm, 'mynumber.js'));
  copyFile(wasmBin, path.join(distWasm, 'mynumber.wasm'));

  const wasmTemplate = JSON.parse(
    fs.readFileSync(path.join(root, 'packaging/npm/mynumber-wasm.package.json'), 'utf8'),
  );

  writePkg(path.join(distWasm, 'package.json'), {
    ...wasmTemplate,
    version: project.version,
    author: rootPkg.author,
    license: rootPkg.license,
    repository: rootPkg.repository ?? {
      type: 'git',
      url: `git+https://github.com/${owner}/${repo}.git`,
    },
  });
}

console.log(`Staged npm packages:
  ${distNative}${nativeOnly ? '' : `\n  ${distWasm}`}`);
