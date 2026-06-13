import { Combination, features, implementation, type ICombination } from 'mynumber';
import { load, type MynumberModule } from 'mynumber/wasm';

const numbers: [number, number, number, number, number, number] = [3, 3, 8, 8, 2, 2];

const comb = new Combination(24, numbers);

const config: ICombination = { target: 24, numbers };
const fromConfig = new Combination(config);
const generated = Combination.generate();

generated.solve().expression();
if (features.allSolutions && implementation === 'native') {
  generated.allSolutions?.();
}
comb.solve().value;
fromConfig.toJSON();

async function wasmSmoke(): Promise<void> {
  const mod: MynumberModule = await load();
  const impl: typeof mod.implementation = mod.implementation;
  if (impl !== 'wasm') throw new Error('expected wasm');
  const wasmComb = new mod.Combination(24, numbers);

  try {
    const solution = wasmComb.solve();
    try {
      solution.expression();
      solution.value;
    } finally {
      solution.delete();
    }
  } finally {
    wasmComb.delete();
  }
}

void wasmSmoke();
