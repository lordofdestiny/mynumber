'use strict';

const fs = require('fs');
const path = require('path');

/**
 * @param {string} packageRoot
 * @param {{ binary?: { module_name?: string; napi_versions?: number[] } }} pkg
 * @returns {string | undefined}
 */
function findCompiledBinding(packageRoot, pkg) {
  const moduleName = pkg.binary?.module_name ?? 'mynumber';
  const napiVersions = pkg.binary?.napi_versions ?? [8];
  const candidates = [
    path.join(packageRoot, 'build', 'Release', `${moduleName}.node`),
    path.join(packageRoot, 'build', 'Debug', `${moduleName}.node`),
  ];

  for (const version of napiVersions) {
    candidates.push(
      path.join(packageRoot, `build-tmp-napi-v${version}`, 'Release', `${moduleName}.node`),
      path.join(packageRoot, `build-tmp-napi-v${version}`, 'Debug', `${moduleName}.node`),
    );
  }

  const bindingRoot = path.join(packageRoot, 'lib', 'binding');
  if (fs.existsSync(bindingRoot)) {
    for (const dir of fs.readdirSync(bindingRoot)) {
      candidates.push(path.join(bindingRoot, dir, `${moduleName}.node`));
    }
  }

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return undefined;
}

/**
 * Tier 0 (staging): colocated binary copied by stage-npm for local file: installs.
 * @returns {{ Combination: unknown; Solution?: unknown } | undefined}
 */
function loadColocated() {
  const addon = path.join(__dirname, 'mynumber.node');
  if (fs.existsSync(addon)) {
    return require(addon);
  }
}

/**
 * Tier 1 (release): binary downloaded by install-release-binary.js into lib/binding/.
 * @returns {{ Combination: unknown; Solution?: unknown } | undefined}
 */
function loadInstalledBinding() {
  const preGyp = require('@mapbox/node-pre-gyp');
  const pkgPath = path.join(__dirname, 'package.json');

  try {
    const binding = preGyp.find(pkgPath);
    if (!fs.existsSync(binding)) {
      return undefined;
    }
    return require(binding);
  } catch {
    return undefined;
  }
}

/**
 * Tier 2 (compile): binary built locally by node-gyp during install.
 * node-gyp leaves N-API builds in build-tmp-napi-v8/Release (and similar dirs).
 * @returns {{ Combination: unknown; Solution?: unknown } | undefined}
 */
function loadCompiled() {
  const pkgPath = path.join(__dirname, 'package.json');
  const pkg = require(pkgPath);
  const binding = findCompiledBinding(__dirname, pkg);

  if (!binding) {
    return undefined;
  }

  return require(binding);
}

/**
 * Tier 3 (pure JS): TypeScript fallback compiled to lib/js/index.js.
 * @returns {{ Combination: unknown; Solution?: unknown } | undefined}
 */
function loadJsFallback() {
  const js = path.join(__dirname, 'lib', 'js', 'index.js');
  if (!fs.existsSync(js)) {
    return undefined;
  }

  return require(js);
}

/** @returns {{ addon: { Combination: unknown; Solution?: unknown }; features: { allSolutions: boolean }; implementation: 'native' | 'js' }} */
function resolveAddon() {
  const nativeLoaders = [loadColocated, loadInstalledBinding, loadCompiled];
  const errors = [];

  for (const load of nativeLoaders) {
    try {
      const addon = load();
      if (addon?.Combination) {
        return { addon, features: { allSolutions: true }, implementation: 'native' };
      }
    } catch (err) {
      errors.push(/** @type {Error} */ (err));
    }
  }

  try {
    const addon = loadJsFallback();
    if (addon?.Combination) {
      return { addon, features: { allSolutions: false }, implementation: 'js' };
    }
  } catch (err) {
    errors.push(/** @type {Error} */ (err));
  }

  throw new Error(
    `Failed to load mynumber native addon:\n${errors.map((e) => e.message).join('\n')}`,
  );
}

const { addon, features, implementation } = resolveAddon();

exports.features = features;
exports.implementation = implementation;
exports.Combination = addon.Combination;
if (addon.Solution !== undefined) {
  exports.Solution = addon.Solution;
}
