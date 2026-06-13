export type MynumberImplementation = 'wasm';

/** Active backend. */
export const implementation: MynumberImplementation;

export interface MynumberFeatures {
  /** Enumerate all exact solutions (not available in WASM; exceptionally inefficient). */
  allSolutions: boolean;
}

/** Feature flags for the WASM backend. */
export const features: MynumberFeatures;

export interface ICombination {
  target: number;
  numbers: number[];
}

export class Solution {
  readonly value: number;
  expression(): string;
  toString(): string;
  delete(): void;
}

export class Combination implements ICombination {
  target: number;
  numbers: number[];

  constructor(target?: number, numbers?: number[]);

  static generate(): Combination;

  solve(): Solution;
  toString(): string;
  toJSON(): ICombination;
  delete(): void;
}

export interface MynumberModule {
  Combination: typeof Combination;
  Solution: typeof Solution;
  features: MynumberFeatures;
  implementation: MynumberImplementation;
}

export function load(): Promise<MynumberModule>;
