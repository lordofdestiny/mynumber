'use strict';

const fs = require('fs');
const path = require('path');

const FEATURES = { allSolutions: false };
const IMPLEMENTATION = 'wasm';

/** @type {Promise<import('./index').MynumberModule> | undefined} */
let modulePromise;

/** @returns {string} */
function resolveMynumberJs() {
  const candidates = [
    path.join(__dirname, '../../dist/wasm/mynumber.js'),
    path.join(__dirname, 'mynumber.js'),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  throw new Error('mynumber.js not built. Run npm run build:wasm');
}

/**
 * Load the WebAssembly module and return exported classes.
 * @returns {Promise<import('./index').MynumberModule>}
 */
async function load() {
  if (!modulePromise) {
    const createModule = require(resolveMynumberJs());
    const Module = await createModule();
    modulePromise = Promise.resolve({
      Combination: Module.Combination,
      Solution: Module.Solution,
      features: FEATURES,
      implementation: IMPLEMENTATION,
    });
  }
  return modulePromise;
}

module.exports = { load, features: FEATURES, implementation: IMPLEMENTATION };
