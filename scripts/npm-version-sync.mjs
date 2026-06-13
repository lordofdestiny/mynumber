#!/usr/bin/env node
'use strict';

import { execSync } from 'node:child_process';
import { syncVersionFromPackageJson } from './project-meta.mjs';

syncVersionFromPackageJson();

try {
  execSync('git add packaging/project.json', { stdio: 'inherit' });
} catch {
  // Allow npm version outside a git worktree.
}
