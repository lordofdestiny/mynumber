import { deepFreeze } from './Util';

export const OperatorEnum = deepFreeze({
  NONE: { op: null, prec: 0 },
  ADD: { op: '+', prec: 1 },
  SUB: { op: '-', prec: 1 },
  MUL: { op: '*', prec: 2 },
  DIV: { op: '/', prec: 2 },
} as const);

export type Operator = typeof OperatorEnum[keyof typeof OperatorEnum];

export class StateValue {
  value: number;
  op: Operator;
  left: StateValue | null;
  right: StateValue | null;

  constructor(value: number, op: Operator, left: StateValue | null, right: StateValue | null) {
    this.value = value;
    this.op = op;
    this.left = left;
    this.right = right;
  }

  private static calculate(left: number, right: number, op: Operator): number {
    switch (op.op) {
      case '+':
        return left + right;
      case '-':
        return left - right;
      case '*':
        return left * right;
      case '/':
        return left / right;
      default:
        throw new Error(`Invalid operator: ${op}`);
    }
  }

  static combine(left: StateValue, right: StateValue, op: Operator): StateValue {
    const value = this.calculate(left.value, right.value, op);
    return new StateValue(value, op, left, right);
  }

  reconstruct(): string {
    return this.reconstruct_impl(this.op, false);
  }

  private reconstruct_impl(op: Operator, is_right: boolean): string {
    if (this.op === OperatorEnum.NONE) {
      return this.value.toString();
    }

    if (this.left === null || this.right === null) {
      throw new Error("Left or right is null");
    }

    const left = this.left.reconstruct_impl(this.op, false);
    const right = this.right.reconstruct_impl(this.op, true);
    const result = `${left} ${this.op.op} ${right}`;

    return this.needs_parens(op, is_right) ? `(${result})` : result;
  }

  private needs_parens(op: Operator, is_right: boolean): boolean {
    if (op === OperatorEnum.NONE) {
      return false;
    }

    const current_prec = this.op.prec;
    const parent_prec = op.prec;

    return current_prec < parent_prec 
      || (current_prec === parent_prec 
        && is_right 
        && (op === OperatorEnum.SUB || op === OperatorEnum.DIV));
  }
}
