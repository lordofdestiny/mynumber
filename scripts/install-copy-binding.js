'use strict';

const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const pkgPath = path.join(root, 'package.json');
const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));
const moduleName = pkg.binary?.module_name ?? 'mynumber';
const napiVersions = pkg.binary?.napi_versions ?? [8];

/** @returns {string | undefined} */
function findTargetPath() {
  try {
    const preGyp = require('@mapbox/node-pre-gyp');
    return preGyp.find(pkgPath);
  } catch {
    return undefined;
  }
}

/** @returns {string[]} */
function sourceCandidates() {
  const sources = [
    path.join(root, 'build', 'Release', `${moduleName}.node`),
    path.join(root, 'build', 'Debug', `${moduleName}.node`),
  ];

  for (const version of napiVersions) {
    sources.push(
      path.join(root, `build-tmp-napi-v${version}`, 'Release', `${moduleName}.node`),
      path.join(root, `build-tmp-napi-v${version}`, 'Debug', `${moduleName}.node`),
    );
  }

  return sources;
}

const target = findTargetPath();
if (!target || fs.existsSync(target)) {
  process.exit(0);
}

for (const source of sourceCandidates()) {
  if (!fs.existsSync(source)) {
    continue;
  }

  fs.mkdirSync(path.dirname(target), { recursive: true });
  fs.copyFileSync(source, target);
  process.exit(0);
}
