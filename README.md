# mynumber

Number combination puzzle solver implemented in C++20, with Node.js native addon, WebAssembly, and pure JavaScript fallback builds.

Given a **target** and **six numbers**, the solver finds arithmetic expressions using `+`, `-`, `*`, and `/` (with parentheses) that evaluate to the target. This is the classic “numbers game” / Countdown-style puzzle.

**Live demo**: [lordofdestiny.github.io/mynumber](https://lordofdestiny.github.io/mynumber/)

## Published packages

This repository is a **private development monorepo**. Consumers install the scoped npm packages:


| Package                                                                                      | Description                                           |
| -------------------------------------------------------------------------------------------- | ----------------------------------------------------- |
| `[@lordofdestiny/mynumber](https://www.npmjs.com/package/@lordofdestiny/mynumber)`           | Node.js addon — native binary with automatic fallback |
| `[@lordofdestiny/mynumber-wasm](https://www.npmjs.com/package/@lordofdestiny/mynumber-wasm)` | WebAssembly build for Node.js and browsers            |


Install the package that matches your runtime:

```bash
npm install @lordofdestiny/mynumber          # Node.js
npm install @lordofdestiny/mynumber-wasm     # Node.js or browser (via bundler)
```

`@lordofdestiny/mynumber` requires **Node.js ≥ 18**. `@lordofdestiny/mynumber-wasm` has no Node engine requirement when used in the browser.

---

## `@lordofdestiny/mynumber` (native)

High-performance native addon. On `npm install`, the package:

1. **Downloads a prebuild** from [GitHub Releases](https://github.com/lordofdestiny/mynumber/releases) when one is available (see [release assets](#github-release-assets))
2. **Compiles from source** if no prebuild is available (requires cmake, a C++20 compiler, and optional `node-gyp` deps)
3. **Falls back to pure JS** at runtime if no native binary can be loaded

At runtime the loader tries, in order: colocated binary → prebuild → compiled binary → `lib/js` fallback.

### Usage

```js
const { Combination, features, implementation } = require('@lordofdestiny/mynumber');

// implementation === 'native' | 'js'

// Solve a specific puzzle
const comb = new Combination(24, [3, 3, 8, 8, 2, 2]);
const solution = comb.solve();

console.log(solution.expression()); // e.g. "8 * 3"
console.log(solution.value);        // 24

// Generate and solve a random puzzle
const random = Combination.generate();
const answer = random.solve();
console.log(random.target, answer.expression());

// List all exact solutions (native addon only — see features)
if (features.allSolutions) {
  const all = comb.allSolutions();
}
```

`implementation` is `'native'` when the `.node` addon loaded, or `'js'` on the pure-JS fallback. Use `features` to check whether optional methods are available.

### TypeScript

Types are included:

```ts
import { Combination, features, type ICombination } from '@lordofdestiny/mynumber';

const config: ICombination = { target: 24, numbers: [3, 3, 8, 8, 2, 2] };
const comb = new Combination(config);

if (features.allSolutions) {
  comb.allSolutions?.();
}
```

### API


| Export                   | Description                                                                                                 |
| ------------------------ | ----------------------------------------------------------------------------------------------------------- |
| `implementation`         | `'native'` or `'js'` — which backend is active                                                              |
| `features`               | Feature flags (e.g. `{ allSolutions: boolean }`)                                                            |
| `Combination`            | Puzzle state — target + six numbers                                                                         |
| `Combination.generate()` | Random solvable puzzle                                                                                      |
| `comb.solve()`           | Best solution (exact or closest)                                                                            |
| `comb.allSolutions()`    | All exact solutions — when `features.allSolutions` is `true` (native addon only; exceptionally inefficient) |
| `Solution.expression()`  | Human-readable expression string                                                                            |
| `Solution.value`         | Evaluated result                                                                                            |


Check `features` before calling optional methods:

```js
if (features.allSolutions) {
  comb.allSolutions();
}
```

---

## `@lordofdestiny/mynumber-wasm`

WebAssembly build of the same solver. No native compile step — use in **Node.js** or **browsers** (bundle `mynumber.js` and `mynumber.wasm` with your app, or import from the npm package).

### Node.js usage

```js
const { load, features, implementation } = require('@lordofdestiny/mynumber-wasm');

async function main() {
  const mod = await load();
  // implementation === 'wasm'
  // mod.implementation === 'wasm'

  const { Combination } = mod;

  const comb = new Combination(24, [3, 3, 8, 8, 2, 2]);
  try {
    const solution = comb.solve();
    try {
      console.log(solution.expression());
    } finally {
      solution.delete();
    }
  } finally {
    comb.delete();
  }
}

main();
```

Wasm objects are backed by Emscripten handles — call `.delete()` when done (same pattern as the C++ API).

### Browser usage

Bundle the package with your frontend tooling (Vite, webpack, etc.) and call `load()`:

```ts
import { load } from '@lordofdestiny/mynumber-wasm';

const { Combination } = await load();
const comb = new Combination(24, [3, 3, 8, 8, 2, 2]);
// ... same API as Node.js; call .delete() when done
```

Ensure `mynumber.wasm` is served alongside your bundle (most bundlers copy it from `node_modules/@lordofdestiny/mynumber-wasm/`). Alternatively, download `mynumber-wasm-{version}.zip` from [GitHub Releases](https://github.com/lordofdestiny/mynumber/releases) (see [release assets](#github-release-assets)) and host `mynumber.js` + `mynumber.wasm` yourself.

### TypeScript

```ts
import { load, features, implementation, type MynumberModule } from '@lordofdestiny/mynumber-wasm';

const mod: MynumberModule = await load();
// implementation === 'wasm'
```

### API


| Export           | Description                                                                      |
| ---------------- | -------------------------------------------------------------------------------- |
| `implementation` | `'wasm'` — at package level and on the object returned by `load()`               |
| `features`       | Feature flags (e.g. `{ allSolutions: false }`)                                   |
| `load()`         | Initialize WASM and return `{ Combination, Solution, features, implementation }` |
| `Combination`    | Same puzzle API as the native package (without `allSolutions`)                   |
| `Solution`       | Result with `.expression()`, `.value`, `.delete()`                               |


`features.allSolutions` is always `false` in the WASM build.

---

## Backends

Every package exports `implementation` (which backend is active) and `features` (boolean capability flags).


| `implementation` | Package                        | When                                                          |
| ---------------- | ------------------------------ | ------------------------------------------------------------- |
| `'native'`       | `@lordofdestiny/mynumber`      | `.node` addon loaded (prebuild, compile, or colocated binary) — [release assets](#github-release-assets) |
| `'js'`           | `@lordofdestiny/mynumber`      | Pure-JS fallback (no native binary available)                 |
| `'wasm'`         | `@lordofdestiny/mynumber-wasm` | WebAssembly module (Node.js or browser)                       |


```js
const { implementation, features } = require('@lordofdestiny/mynumber');
// implementation === 'native' | 'js'

if (implementation === 'native' && features.allSolutions) {
  comb.allSolutions();
}
```

```js
const { implementation } = require('@lordofdestiny/mynumber-wasm');
// implementation === 'wasm' (also on the object returned by load())
```

---

## Choosing a package


| Runtime                                    | Package                        | Notes                                                                   |
| ------------------------------------------ | ------------------------------ | ----------------------------------------------------------------------- |
| Node.js (fastest)                          | `@lordofdestiny/mynumber`      | Native addon; [release assets](#github-release-assets) when available, compile or JS fallback otherwise |
| Node.js (no native toolchain)              | `@lordofdestiny/mynumber-wasm` | Pure WASM, no compile step                                              |
| Browser                                    | `@lordofdestiny/mynumber-wasm` | Bundle or host `mynumber.js` + `mynumber.wasm`                          |
| Node.js (portable, same binary everywhere) | `@lordofdestiny/mynumber-wasm` | Identical WASM artifact everywhere                                      |


`@lordofdestiny/mynumber` also includes a pure-JS fallback when no native binary is available, but WASM is usually faster if you want a non-native path.

---

## Repository layout

```
mojbroj/
├── demo/             Showcase app (GitHub Pages; not for consumption)
├── dist/             Build outputs (gitignored)
├── include/          C++ headers
├── packaging/
│   ├── npm/          Published package templates
│   └── project.json  Canonical version + GitHub metadata
├── scripts/          Staging, install, prebuild packaging
├── src/
│   ├── emscripten/   WASM bindings
│   ├── impl/         Core solver (C++)
│   ├── js-fallback/  Pure TypeScript fallback
│   ├── node/         Dev + publish entry loaders
│   └── wrapper/      Node-API bindings
└── test/             Node test suite
```

---

## Development

### Prerequisites

- **C++20** toolchain (clang/gcc)
- **CMake** ≥ 3.20
- **Node.js** ≥ 18
- **Emscripten** (`em++` on PATH) for WASM builds

### Setup

```bash
git clone https://github.com/lordofdestiny/mynumber.git
cd mynumber
npm ci
```

### Build

```bash
npm run build:node      # Native addon → dist/node/
npm run build:wasm      # WASM → dist/wasm/
npm run build:js-fallback
npm run build:all       # Everything + staged npm packages
npm test
```

### Native CLI

```bash
npm run app
# or
make native main && ./build/native/mynumber
```

### Local demo (development only)

```bash
npm run demo
# Serves the showcase app at http://localhost:8080 — not part of the published API
```

### Stage npm packages (without publishing)

```bash
npm run stage:npm
# Output: dist/npm/mynumber, dist/npm/mynumber-wasm
npm run lint:packages
```

Link into a consumer project:

```bash
npm install /path/to/mojbroj/dist/npm/mynumber
```

---

## Releases

Pushing a `v*` tag triggers `[.github/workflows/release.yml](.github/workflows/release.yml)`, which:

- Builds native CPack `.tar.gz` archives
- Builds Node prebuild tarballs for `node-pre-gyp`
- Publishes `@lordofdestiny/mynumber` and `@lordofdestiny/mynumber-wasm` to npm
- Deploys the browser demo to GitHub Pages
- Attaches five [release assets](#github-release-assets)

### Version bumps

Version is defined in `[packaging/project.json](packaging/project.json)`. Bump with:

```bash
npm version patch   # or minor / major
```

This updates `package.json`, syncs `packaging/project.json`, creates a git commit, and tags `v{version}`.

To bump manually: edit `packaging/project.json`, then run `npm run sync-version`.

---

## GitHub Release assets

Each [release](https://github.com/lordofdestiny/mynumber/releases) attaches five archives:

| Pattern | Contents |
|---------|----------|
| `mynumber-{version}-{platform}.tar.gz` | Native C++ library and CLI |
| `mynumber-node-{version}-{platform}.zip` | Node addon: `mynumber.node` + `index.d.ts` |
| `mynumber-wasm-{version}.zip` | WASM: `mynumber.js` + `mynumber.wasm` + `index.d.ts` |

**Platforms** (native and Node)

- macOS — `Darwin` in `.tar.gz`, `macos` in `.zip`
- Linux — `Linux` in `.tar.gz`, `linux` in `.zip`

WASM is platform-independent.


---

## License

ISC
