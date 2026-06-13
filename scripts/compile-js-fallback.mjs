import { execSync } from 'node:child_process';

execSync('tsc -p tsconfig.js-fallback.json', { stdio: 'inherit' });
