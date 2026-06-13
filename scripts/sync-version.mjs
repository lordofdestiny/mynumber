#!/usr/bin/env node
'use strict';

import { syncVersionToPackageJson, getVersion } from './project-meta.mjs';

const changed = syncVersionToPackageJson();
const suffix = changed ? ' (synced to package.json)' : '';
console.log(`Project version: ${getVersion()}${suffix}`);
console.log('Edit packaging/project.json to bump manually, or run: npm version patch|minor|major');

