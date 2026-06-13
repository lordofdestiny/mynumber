'use strict';

const fs = require('fs');
const path = require('path');

/** @returns {string} */
function resolveAddon() {
  const candidates = [
    path.join(__dirname, '../../dist/node/mynumber.node'),
    path.join(__dirname, '../../build/Release/mynumber.node'),
    path.join(__dirname, 'mynumber.node'),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  throw new Error('mynumber.node not built. Run npm run build:node');
}

const addon = require(resolveAddon());

exports.features = { allSolutions: true };
exports.implementation = 'native';
exports.Combination = addon.Combination;
if (addon.Solution !== undefined) {
  exports.Solution = addon.Solution;
}
