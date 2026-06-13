export type { ICombination } from './types';
export { Solution } from './Solution';
export { Combination } from './Combination';

/** Feature flags for this backend (JS fallback — no native addon). */
export const features = { allSolutions: false };

export const implementation = 'js' as const;
