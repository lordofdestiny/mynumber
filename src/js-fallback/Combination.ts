import type { ICombination } from './types';
import { Solution } from './Solution';
import { OperatorEnum, StateValue } from './StateValue';
import { cartesian2, randomInt, range, submask_complement_pairs } from './Util';

const MIDDLE_NUMBERS = [10, 15, 20] as const;
const LARGE_NUMBERS = [25, 50, 75, 100] as const;

function copyNumbers(numbers: ICombination['numbers']): ICombination['numbers'] {
  return [
    numbers[0] ?? 0,
    numbers[1] ?? 0,
    numbers[2] ?? 0,
    numbers[3] ?? 0,
    numbers[4] ?? 0,
    numbers[5] ?? 0,
  ];
}

export class Combination implements ICombination {
  target: number = 0;
  numbers: ICombination['numbers'] = [0, 0, 0, 0, 0, 0];

  constructor(targetOrConfig?: number | ICombination, numbers?: ICombination['numbers']) {
    const argc = arguments.length;

    if (argc === 0) {
      return;
    }

    if (argc === 1 && typeof targetOrConfig === 'object' && targetOrConfig !== null) {
      if (typeof targetOrConfig.target !== 'number' || !Array.isArray(targetOrConfig.numbers)) {
        throw new TypeError('Invalid arguments.');
      }
      this.target = targetOrConfig.target;
      this.numbers = copyNumbers(targetOrConfig.numbers);
      return;
    }

    if (argc === 2) {
      if (typeof targetOrConfig !== 'number' || !Array.isArray(numbers)) {
        throw new TypeError('Invalid arguments.');
      }
      this.target = targetOrConfig;
      this.numbers = copyNumbers(numbers);
      return;
    }

    throw new TypeError('Too many arguments');
  }

  static generate(): Combination {
    const target = randomInt(1, 999);
    const numbers: ICombination['numbers'] = [
      randomInt(1, 9),
      randomInt(1, 9),
      randomInt(1, 9),
      randomInt(1, 9),
      MIDDLE_NUMBERS[randomInt(0, MIDDLE_NUMBERS.length - 1)],
      LARGE_NUMBERS[randomInt(0, LARGE_NUMBERS.length - 1)],
    ];
    return new Combination(target, numbers);
  }

  solve(): Solution | null {
    const states: Map<number, StateValue>[] = Array.from({ length: 64 }, () => new Map());

    for (const i of range(0, 6)) {
      states[1 << i].set(
        this.numbers[i],
        new StateValue(this.numbers[i], OperatorEnum.NONE, null, null),
      );
    }

    let bestDiff = Number.POSITIVE_INFINITY;
    let bestMatch: StateValue | null = null;

    for (const mask of range(1, 64)) {
      for (const [submask1, submask2] of submask_complement_pairs(mask)) {
        const statePairs = cartesian2(states[submask1].values(), states[submask2].values());

        for (const [state1, state2] of statePairs) {
          const val1 = state1.value;
          const val2 = state2.value;
          const newStates: StateValue[] = [
            StateValue.combine(state1, state2, OperatorEnum.ADD),
            StateValue.combine(state1, state2, OperatorEnum.MUL),
          ];

          if (val1 > val2) {
            newStates.push(StateValue.combine(state1, state2, OperatorEnum.SUB));
          }
          if (val2 > val1) {
            newStates.push(StateValue.combine(state2, state1, OperatorEnum.SUB));
          }
          if (val2 !== 0 && val1 % val2 === 0) {
            newStates.push(StateValue.combine(state1, state2, OperatorEnum.DIV));
          }
          if (val1 !== 0 && val2 % val1 === 0) {
            newStates.push(StateValue.combine(state2, state1, OperatorEnum.DIV));
          }

          for (const newState of newStates) {
            if (states[mask].has(newState.value)) {
              continue;
            }

            states[mask].set(newState.value, newState);

            const diff = Math.abs(this.target - newState.value);
            if (diff >= bestDiff) {
              continue;
            }

            bestDiff = diff;
            bestMatch = newState;
            if (diff === 0) {
              return new Solution(newState);
            }
          }
        }
      }
    }

    return bestMatch ? new Solution(bestMatch) : null;
  }

  toString(): string {
    return `Combination { target: ${this.target}, numbers : [${this.numbers.join(', ')}] }`;
  }

  toJSON(): ICombination {
    return {
      target: this.target,
      numbers: this.numbers,
    };
  }
}
