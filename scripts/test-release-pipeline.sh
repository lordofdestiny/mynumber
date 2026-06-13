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
CMAKE_TAR="dist/release/mynumber-cmake-${VERSION}.tar.gz"
check "$CMAKE_ZIP" "cmake source zip"
check "$CMAKE_TAR" "cmake source tar.gz"

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

NODE_BASENAME="mynumber-node-${VERSION}-${NODE_PLATFORM}"
mkdir -p dist/release
NODE_ZIP="dist/release/${NODE_BASENAME}.zip"
NODE_TAR="dist/release/${NODE_BASENAME}.tar.gz"
(cd dist/node && zip -j "../release/${NODE_BASENAME}.zip" mynumber.node index.d.ts)
(cd dist/node && tar -czf "../release/${NODE_BASENAME}.tar.gz" mynumber.node index.d.ts)
check "$NODE_ZIP" "node release zip"
check "$NODE_TAR" "node release tar.gz"
check_zip_contains "$NODE_ZIP" "mynumber.node" "node zip contains mynumber.node"
check_zip_contains "$NODE_ZIP" "index.d.ts" "node zip contains index.d.ts"
tar -tzf "$NODE_TAR" | grep -qx "mynumber.node" && echo "  OK  node tar contains mynumber.node" && PASS=$((PASS + 1)) || { echo "  FAIL node tar contains mynumber.node"; FAIL=$((FAIL + 1)); }

rm -rf dist/npm/mynumber/build dist/npm/mynumber/lib/binding

echo
echo "--- build-wasm ---"
if command -v em++ >/dev/null 2>&1; then
  make dist-wasm
  check dist/wasm/mynumber.js "wasm mynumber.js"
  check dist/wasm/mynumber.wasm "wasm mynumber.wasm"
  WASM_ZIP="dist/release/mynumber-wasm-${VERSION}.zip"
  WASM_TAR="dist/release/mynumber-wasm-${VERSION}.tar.gz"
  (cd dist/wasm && zip -j "../release/mynumber-wasm-${VERSION}.zip" mynumber.js mynumber.wasm index.d.ts)
  (cd dist/wasm && tar -czf "../release/mynumber-wasm-${VERSION}.tar.gz" mynumber.js mynumber.wasm index.d.ts)
  check "$WASM_ZIP" "wasm release zip"
  check "$WASM_TAR" "wasm release tar.gz"
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
echo "--- release asset collection ---"
mkdir -p dist/release-assets-test
cp "$CMAKE_ZIP" "$CMAKE_TAR" dist/release-assets-test/
cp "${NATIVE_TGZ[0]}" dist/release-assets-test/
cp "$NODE_ZIP" "$NODE_TAR" dist/release-assets-test/ 2>/dev/null || true
cp "dist/release/mynumber-wasm-${VERSION}.zip" "dist/release/mynumber-wasm-${VERSION}.tar.gz" dist/release-assets-test/ 2>/dev/null || true
RELEASE_COUNT=$(find dist/release-assets-test -type f | wc -l | tr -d ' ')
echo "Collected ${RELEASE_COUNT} release asset(s):"
ls -la dist/release-assets-test/
if [[ "$RELEASE_COUNT" -ge 7 ]]; then
  echo "  OK  release asset collection (local macOS; CI expects 10 with both platforms)"
  PASS=$((PASS + 1))
else
  echo "  FAIL release asset collection (expected >= 7 on local macOS-only run)"
  FAIL=$((FAIL + 1))
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
