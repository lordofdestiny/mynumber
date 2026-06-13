
export function deepFreeze<T extends object>(obj: T): Readonly<T> {
  const propNames = Reflect.ownKeys(obj);

  for (const name of propNames) {
    const value = (obj as Record<string | symbol, unknown>)[name];
    if (value && typeof value === 'object') {
      deepFreeze(value);
    }
  }

  return Object.freeze(obj);
}

export function randomInt(min: number, max: number): number {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

export function* range(start: number, end: number): Generator<number> {
  for(let i = start; i < end; i++) {
    yield i;
  }
}

export function* submasks(mask: number): Generator<number> {
  let submask = mask;
  while(submask != 0) {
    submask = (submask - 1) & mask;
    yield submask;
  }
}

export function* submask_complement_pairs(mask: number, symmetry = false): Generator<[number, number]> {
  for (const submask of submasks(mask)) {
    const complement = mask ^ submask;
    if (!symmetry || submask < complement) {
      yield [submask, complement];
    }
  }
}

export function* cartesian2<T, U>(iter1: Iterable<T>, iter2: Iterable<U>): Generator<[T, U]> {
  const right = [...iter2];
  for (const left of iter1) {
    for (const item of right) {
      yield [left, item];
    }
  }
}
