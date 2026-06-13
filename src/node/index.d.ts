export type MynumberImplementation = 'native' | 'js';

/** Active backend: `native` (`.node` addon) or `js` (pure-JS fallback). */
export const implementation: MynumberImplementation;

export interface MynumberFeatures {
  /** Enumerate all exact solutions (native addon only; exceptionally inefficient). */
  allSolutions: boolean;
}

/** Feature flags for the active backend. */
export const features: MynumberFeatures;

export interface ICombination {
  target: number;
  numbers: [number, number, number, number, number, number];
}

export class Solution {
  readonly value: number;
  expression(): string;
  toString(): string;
}

export class Combination implements ICombination {
  target: number;
  numbers: [number, number, number, number, number, number];

  constructor(comb: ICombination);
  constructor(target: number, numbers: [number, number, number, number, number, number]);

  static generate(): Combination;

  solve(): Solution;
  /** Available when `features.allSolutions` is `true`. */
  allSolutions?(): Solution[];
  toString(): string;
  toJSON(): ICombination;
}
