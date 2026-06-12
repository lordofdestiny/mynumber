declare module '*.node' {
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
    allSolutions(): Solution[];
    toString(): string;
  }
}
