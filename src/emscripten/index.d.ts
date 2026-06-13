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

export interface MojbrojModule {
  Combination: typeof Combination;
  Solution: typeof Solution;
}

export function load(): Promise<MojbrojModule>;
