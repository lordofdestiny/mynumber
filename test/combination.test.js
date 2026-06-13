const { describe, it } = require('node:test');
const assert = require('node:assert/strict');

const { Combination } = require('..');
const { evalExpression } = require('./helpers');

/** @type {[number, number, number, number, number, number]} */
const SMALL_NUMBERS = [3, 3, 8, 8, 2, 2];
/** @type {[number, number, number, number, number, number]} */
const CLASSIC_NUMBERS = [100, 75, 50, 25, 6, 3];

describe('Combination', () => {
  describe('construction', () => {
    it('creates from target and numbers array', () => {
      const comb = new Combination(24, SMALL_NUMBERS);

      assert.equal(comb.target, 24);
      assert.deepEqual(comb.numbers, SMALL_NUMBERS);
    });

    it('creates from config object', () => {
      const comb = new Combination({ target: 24, numbers: SMALL_NUMBERS });

      assert.equal(comb.target, 24);
      assert.deepEqual(comb.numbers, SMALL_NUMBERS);
    });

    it('throws on invalid arguments', () => {
      assert.throws(() => {
        // @ts-expect-error intentional invalid input
        new Combination('bad', []);
      }, /Invalid arguments/);
      assert.throws(() => {
        // @ts-expect-error intentional invalid input
        new Combination({});
      }, /Invalid arguments/);
      assert.throws(() => {
        // @ts-expect-error intentional invalid input
        new Combination(1, 2, 3);
      }, /Too many arguments/);
    });
  });

  describe('properties', () => {
    it('allows reading and writing target and numbers', () => {
      /** @type {[number, number, number, number, number, number]} */
      const numbers = [1, 2, 3, 4, 5, 6];
      const comb = new Combination(5, numbers);

      comb.target = 10;
      comb.numbers = [2, 2, 2, 2, 2, 2];

      assert.equal(comb.target, 10);
      assert.deepEqual(comb.numbers, [2, 2, 2, 2, 2, 2]);
      assert.equal(comb.solve().value, 10);
    });
  });

  describe('solve', () => {
    it('finds an exact solution', () => {
      const comb = new Combination(24, SMALL_NUMBERS);
      const solution = comb.solve();

      assert.equal(solution.value, 24);
      assert.equal(evalExpression(solution.expression()), 24);
    });

    it('finds the classic 952 solution', () => {
      const comb = new Combination(952, CLASSIC_NUMBERS);
      const solution = comb.solve();

      assert.equal(solution.value, 952);
      assert.equal(evalExpression(solution.expression()), 952);
    });

    it('returns the closest value when exact match is impossible', () => {
      /** @type {[number, number, number, number, number, number]} */
      const ones = [1, 1, 1, 1, 1, 1];
      const comb = new Combination(10, ones);
      const solution = comb.solve();

      assert.equal(solution.value, 9);
      assert.equal(evalExpression(solution.expression()), 9);
    });

    it('formats solution as a readable string', () => {
      const comb = new Combination(24, SMALL_NUMBERS);
      const solution = comb.solve();
      const text = solution.toString();

      assert.match(text, /^Solution: /);
      assert.match(text, / = 24$/);
      assert.ok(text.includes(solution.expression()));
    });
  });

  describe('allSolutions', () => {
    it('returns only exact solutions', () => {
      const comb = new Combination(24, SMALL_NUMBERS);
      const allSolutions = comb.allSolutions;
      if (!allSolutions) {
        throw new Error('allSolutions requires native addon');
      }
      const solutions = allSolutions.call(comb);

      assert.ok(solutions.length > 0);
      for (const solution of solutions) {
        assert.equal(solution.value, 24);
        assert.equal(evalExpression(solution.expression()), 24);
      }
    });

    it('includes a simple exact solution', () => {
      const comb = new Combination(24, SMALL_NUMBERS);
      const allSolutions = comb.allSolutions;
      if (!allSolutions) {
        throw new Error('allSolutions requires native addon');
      }
      const expressions = allSolutions.call(comb).map((s) => s.expression());

      assert.ok(expressions.includes('3 * 8'));
    });
  });

  describe('generate', () => {
    it('returns a combination with valid countdown-style numbers', () => {
      const comb = Combination.generate();

      assert.ok(comb instanceof Combination);
      assert.ok(comb.target >= 1 && comb.target <= 999);
      assert.equal(comb.numbers.length, 6);

      const [a, b, c, d, middle, large] = comb.numbers;
      for (const digit of [a, b, c, d]) {
        assert.ok(digit >= 1 && digit <= 9);
      }
      assert.ok([10, 15, 20].includes(middle));
      assert.ok([25, 50, 75, 100].includes(large));
    });

    it('produces a solvable puzzle', () => {
      const comb = Combination.generate();
      const solution = comb.solve();

      assert.ok(solution.value > 0);
      assert.equal(evalExpression(solution.expression()), solution.value);
    });
  });

  describe('serialization', () => {
    it('stringifies and serializes to JSON', () => {
      const comb = new Combination(24, SMALL_NUMBERS);

      assert.match(comb.toString(), /Combination \{ target: 24/);
      assert.match(comb.toString(), /numbers/);

      assert.deepEqual(comb.toJSON(), {
        target: 24,
        numbers: SMALL_NUMBERS,
      });
    });
  });
});
