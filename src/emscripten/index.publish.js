'use strict';

const path = require('path');

/** @type {Promise<import('./index').MynumberModule> | undefined} */
let modulePromise;

/**
 * Load the WebAssembly module and return exported classes.
 * Publish layout: mynumber.js is colocated in the package root.
 * @returns {Promise<import('./index').MynumberModule>}
 */
async function load() {
  if (!modulePromise) {
    const createModule = require(path.join(__dirname, 'mynumber.js'));
    const Module = await createModule();
    modulePromise = Promise.resolve({
      Combination: Module.Combination,
      Solution: Module.Solution,
    });
  }
  return modulePromise;
}

module.exports = { load };
