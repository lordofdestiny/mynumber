const { describe, it } = require('node:test');
const assert = require('node:assert/strict');
const fs = require('fs');
const path = require('path');

const { evalExpression } = require('./helpers');

const wasmJs = path.join(__dirname, '../dist/wasm/mynumber.js');
const wasmBuilt = fs.existsSync(wasmJs);

/** @type {[number, number, number, number, number, number]} */
const SMALL_NUMBERS = [3, 3, 8, 8, 2, 2];

describe('wasm', { skip: !wasmBuilt && 'wasm not built (run npm run build:wasm)' }, () => {
  it('loads and solves a combination', async () => {
    const { load } = require('mynumber/wasm');
    const { Combination } = await load();

    const comb = new Combination(24, SMALL_NUMBERS);
    try {
      assert.equal(comb.target, 24);
      assert.deepEqual(comb.numbers, SMALL_NUMBERS);

      const solution = comb.solve();
      try {
        assert.equal(solution.value, 24);
        assert.equal(evalExpression(solution.expression()), 24);
      } finally {
        solution.delete();
      }
    } finally {
      comb.delete();
    }
  });

  it('generates a random puzzle', async () => {
    const { load } = require('mynumber/wasm');
    const { Combination } = await load();

    const comb = Combination.generate();
    try {
      assert.equal(comb.numbers.length, 6);
      assert.ok(comb.target > 0);

      const solution = comb.solve();
      try {
        assert.equal(solution.value, comb.target);
      } finally {
        solution.delete();
      }
    } finally {
      comb.delete();
    }
  });
});
