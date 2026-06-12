import { Combination, type ICombination } from '..';

const numbers: [number, number, number, number, number, number] = [3, 3, 8, 8, 2, 2];
const comb = new Combination(24, numbers);

const config: ICombination = { target: 24, numbers };
const fromConfig = new Combination(config);
const generated = Combination.generate();

generated.solve().expression();
generated.allSolutions();
comb.solve().value;
fromConfig.toJSON();
