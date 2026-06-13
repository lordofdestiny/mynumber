'use strict';

const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const pkgPath = path.join(root, 'package.json');
const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));
const moduleName = pkg.binary?.module_name ?? 'mynumber';
const napiVersions = pkg.binary?.napi_versions ?? [8];

/** @returns {string | undefined} */
function findBindingPath() {
  const bindingRoot = path.join(root, 'lib', 'binding');
  if (!fs.existsSync(bindingRoot)) {
    return undefined;
  }

  for (const dir of fs.readdirSync(bindingRoot)) {
    const candidate = path.join(bindingRoot, dir, `${moduleName}.node`);
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  for (const version of napiVersions) {
    const candidate = path.join(root, `build-tmp-napi-v${version}`, 'Release', `${moduleName}.node`);
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  const releaseCandidate = path.join(root, 'build', 'Release', `${moduleName}.node`);
  if (fs.existsSync(releaseCandidate)) {
    return releaseCandidate;
  }

  const colocated = path.join(root, `${moduleName}.node`);
  if (fs.existsSync(colocated)) {
    return colocated;
  }

  return undefined;
}

/** @returns {boolean} */
function hasBinding() {
  return findBindingPath() !== undefined;
}

module.exports = { findBindingPath, hasBinding };
