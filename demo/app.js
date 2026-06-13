'use strict';

/** @type {{ Combination: typeof Combination, Solution: typeof Solution } | null} */
let api = null;

const targetInput = document.getElementById('target');
const numberInputs = [...document.querySelectorAll('#numbers input')];
const statusEl = document.getElementById('status');
const resultEl = document.getElementById('result');
const solveBtn = document.getElementById('solve');
const generateBtn = document.getElementById('generate');

function readNumbers() {
  return numberInputs.map((input) => Number(input.value));
}

function writeCombination(comb) {
  targetInput.value = String(comb.target);
  comb.numbers.forEach((n, i) => {
    numberInputs[i].value = String(n);
  });
}

function showSolution(comb, solution) {
  const exact = solution.value === comb.target;
  resultEl.textContent =
    solution.expression() +
    '\n= ' +
    solution.value +
    (exact ? '' : ' (closest to ' + comb.target + ')');
}

function showError(message) {
  statusEl.textContent = message;
  resultEl.textContent = '';
}

function deleteNative(obj) {
  if (obj && typeof obj.delete === 'function') {
    obj.delete();
  }
}

function runSolve(comb) {
  const solution = comb.solve();
  try {
    showSolution(comb, solution);
    statusEl.textContent = '';
  } finally {
    deleteNative(solution);
  }
}

function onSolveClick() {
  if (!api) return;

  const target = Number(targetInput.value);
  const numbers = readNumbers();
  const comb = new api.Combination(target, numbers);

  try {
    runSolve(comb);
  } finally {
    deleteNative(comb);
  }
}

function onGenerateClick() {
  if (!api) return;

  const comb = api.Combination.generate();
  try {
    writeCombination(comb);
    runSolve(comb);
  } finally {
    deleteNative(comb);
  }
}

function setReady() {
  statusEl.textContent = 'Ready';
  solveBtn.disabled = false;
  generateBtn.disabled = false;
}

function init() {
  solveBtn.addEventListener('click', onSolveClick);
  generateBtn.addEventListener('click', onGenerateClick);
  solveBtn.disabled = true;
  generateBtn.disabled = true;

  createMynumberModule()
    .then((Module) => {
      api = { Combination: Module.Combination, Solution: Module.Solution };
      setReady();
    })
    .catch((err) => {
      showError('Failed to load WASM. Run: npm run build:demo');
      console.error(err);
    });
}

init();
