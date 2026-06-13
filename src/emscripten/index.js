'use strict';

const path = require('path');

/** @type {Promise<import('./index').MojbrojModule> | undefined} */
let modulePromise;

/**
 * Load the WebAssembly module and return exported classes.
 * @returns {Promise<import('./index').MojbrojModule>}
 */
async function load() {
  if (!modulePromise) {
    const wasmPath = path.join(__dirname, '../../out/emscripten/mojbroj.js');
    const createModule = require(wasmPath);
    const Module = await createModule();
    modulePromise = Promise.resolve({
      Combination: Module.Combination,
      Solution: Module.Solution,
    });
  }
  return modulePromise;
}

module.exports = { load };
