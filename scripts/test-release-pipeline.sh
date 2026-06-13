#!/usr/bin/env bash
# Simulates release.yml build steps locally and verifies expected artifacts.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

VERSION="$(node -p "JSON.parse(require('fs').readFileSync('packaging/project.json','utf8')).version")"
export GITHUB_REF_NAME="v${VERSION}"
PLATFORM="$(uname -s | tr '[:upper:]' '[:lower:]')"
case "$PLATFORM" in
  darwin) NODE_PLATFORM=macos ;;
  linux) NODE_PLATFORM=linux ;;
  *) NODE_PLATFORM="$PLATFORM" ;;
esac

PASS=0
FAIL=0

check() {
  local path="$1"
  local label="$2"
  if [[ -e "$path" ]]; then
    echo "  OK  $label"
    echo "       $path"
    PASS=$((PASS + 1))
  else
    echo "  FAIL $label"
    echo "       missing: $path"
    FAIL=$((FAIL + 1))
  fi
}

check_zip_contains() {
  local zip="$1"
  local member="$2"
  local label="$3"
  if unzip -l "$zip" | awk '{print $4}' | grep -qx "$member"; then
    echo "  OK  $label"
    PASS=$((PASS + 1))
  else
    echo "  FAIL $label (expected $member in $zip)"
    FAIL=$((FAIL + 1))
  fi
}

echo "=== Release pipeline local test (v${VERSION}, platform=${NODE_PLATFORM}) ==="

echo
echo "--- package-source ---"
node scripts/package-source.mjs
CMAKE_ZIP="dist/release/mynumber-cmake-${VERSION}.zip"
check "$CMAKE_ZIP" "cmake source zip"

echo
echo "--- build-native (from cmake zip) ---"
NATIVE_STAGE="dist/release/native-pipeline-test"
rm -rf "$NATIVE_STAGE"
mkdir -p "$NATIVE_STAGE"
cp "$CMAKE_ZIP" "$NATIVE_STAGE/"
(
  cd "$NATIVE_STAGE"
  unzip -q "mynumber-cmake-${VERSION}.zip"
  cd "mynumber-cmake-${VERSION}"
  cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
  cmake --build build --parallel
  cmake --build build --target package
)
NATIVE_TGZ=(dist/release/native-pipeline-test/mynumber-cmake-"${VERSION}"/build/mynumber-"${VERSION}"-*.tar.gz)
check "${NATIVE_TGZ[0]}" "native CPack .tar.gz"

echo
echo "--- build-node ---"
cmake -B build/native -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --target mynumber_static --parallel
mkdir -p native-lib
cp build/native/libmynumber.a native-lib/libmynumber.a
make dist-node
npm run stage:npm:native
npm run package:prebuild

PREBUILD=(dist/npm/mynumber/build/stage/**/*.tar.gz)
# bash glob
shopt -s nullglob
PREBUILD_FILES=(dist/npm/mynumber/build/stage/**/*.tar.gz)
shopt -u nullglob
if ((${#PREBUILD_FILES[@]} > 0)); then
  check "${PREBUILD_FILES[0]}" "node-pre-gyp prebuild tarball"
else
  echo "  FAIL node-pre-gyp prebuild tarball"
  echo "       missing: dist/npm/mynumber/build/stage/**/*.tar.gz"
  FAIL=$((FAIL + 1))
fi

mkdir -p dist/release
NODE_ZIP="dist/release/mynumber-node-${VERSION}-${NODE_PLATFORM}.zip"
(cd dist/node && zip -j "../release/mynumber-node-${VERSION}-${NODE_PLATFORM}.zip" mynumber.node index.d.ts)
check "$NODE_ZIP" "node release zip"
check_zip_contains "$NODE_ZIP" "mynumber.node" "node zip contains mynumber.node"
check_zip_contains "$NODE_ZIP" "index.d.ts" "node zip contains index.d.ts"

rm -rf dist/npm/mynumber/build dist/npm/mynumber/lib/binding

echo
echo "--- build-wasm ---"
if command -v em++ >/dev/null 2>&1; then
  make dist-wasm
  check dist/wasm/mynumber.js "wasm mynumber.js"
  check dist/wasm/mynumber.wasm "wasm mynumber.wasm"
  WASM_ZIP="dist/release/mynumber-wasm-${VERSION}.zip"
  (cd dist/wasm && zip -j "../release/mynumber-wasm-${VERSION}.zip" mynumber.js mynumber.wasm index.d.ts)
  check "$WASM_ZIP" "wasm release zip"
  check_zip_contains "$WASM_ZIP" "mynumber.js" "wasm zip contains mynumber.js"
  check_zip_contains "$WASM_ZIP" "mynumber.wasm" "wasm zip contains mynumber.wasm"
  check_zip_contains "$WASM_ZIP" "index.d.ts" "wasm zip contains index.d.ts"
else
  echo "  SKIP wasm (em++ not on PATH)"
fi

echo
echo "--- build-demo ---"
if [[ -f dist/wasm/mynumber.js && -f dist/wasm/mynumber.wasm ]]; then
  mkdir -p dist/demo
  cp demo/index.html demo/style.css demo/app.js dist/demo/
  cp dist/wasm/mynumber.js dist/wasm/mynumber.wasm dist/demo/
  check dist/demo/index.html "demo index.html"
  check dist/demo/mynumber.js "demo wasm js"
  check dist/demo/mynumber.wasm "demo wasm binary"
else
  echo "  SKIP demo (wasm not built)"
fi

echo
echo "--- publish-npm staging (no publish) ---"
npm run stage:npm
npm run lint:packages
check dist/npm/mynumber/package.json "staged mynumber package.json"
check dist/npm/mynumber-wasm/package.json "staged mynumber-wasm package.json"
check dist/npm/mynumber-wasm/mynumber.js "staged wasm js"
check dist/npm/mynumber-wasm/mynumber.wasm "staged wasm binary"

echo
echo "=== Summary: ${PASS} passed, ${FAIL} failed ==="
exit "$FAIL"
