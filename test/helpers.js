/**
 * Evaluates a puzzle expression for test assertions.
 * Only allows digits and arithmetic operators.
 * @param {string} expr
 * @returns {number}
 */
function evalExpression(expr) {
  if (!/^[\d\s+\-*/().]+$/.test(expr)) {
    throw new Error(`unsafe expression: ${expr}`);
  }

  // eslint-disable-next-line no-new-func
  return Function(`"use strict"; return (${expr});`)();
}

module.exports = { evalExpression };
