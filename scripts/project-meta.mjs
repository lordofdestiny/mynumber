#!/usr/bin/env node
'use strict';

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const projectPath = path.join(root, 'packaging/project.json');
const packagePath = path.join(root, 'package.json');

/** @returns {{ version: string; github: { owner: string; repo: string } }} */
export function readProject() {
  return JSON.parse(fs.readFileSync(projectPath, 'utf8'));
}

/** @returns {string} */
export function getVersion() {
  return readProject().version;
}

/** @returns {boolean} true when package.json was updated */
export function syncVersionToPackageJson() {
  const project = readProject();
  const pkg = JSON.parse(fs.readFileSync(packagePath, 'utf8'));

  if (pkg.version === project.version) {
    return false;
  }

  pkg.version = project.version;
  fs.writeFileSync(packagePath, `${JSON.stringify(pkg, null, 2)}\n`);
  return true;
}

/** @returns {boolean} true when project.json was updated */
export function syncVersionFromPackageJson() {
  const pkg = JSON.parse(fs.readFileSync(packagePath, 'utf8'));
  const project = readProject();

  if (project.version === pkg.version) {
    return false;
  }

  project.version = pkg.version;
  fs.writeFileSync(projectPath, `${JSON.stringify(project, null, 2)}\n`);
  return true;
}
