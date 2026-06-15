'use strict';

const fs = require('node:fs');
const path = require('node:path');

const root = path.join(__dirname, '..');
const pkgPath = path.join(root, 'package.json');
const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));
const moduleName = 'mynumber';

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

  const prebuiltRoot = path.join(root, 'build', 'prebuilt', `${moduleName}.node`);
  if (fs.existsSync(prebuiltRoot)) {
    return prebuiltRoot;
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
