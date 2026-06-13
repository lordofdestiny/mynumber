import { StateValue } from "./StateValue";

export class Solution {  
  private state: StateValue;

  constructor(state: StateValue) {
    this.state = state;
  }

  get value(): number {
    return this.state.value;
  }

  expression(): string {
    return this.state.reconstruct();
  }

  toString(): string {
    return `Solution: ${this.expression()} = ${this.value}`;
  }
}
