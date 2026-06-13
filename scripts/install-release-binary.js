'use strict';

const fs = require('node:fs');
const path = require('node:path');
const https = require('node:https');
const { execSync } = require('node:child_process');

const root = path.join(__dirname, '..');
const pkg = JSON.parse(fs.readFileSync(path.join(root, 'package.json'), 'utf8'));

/** @returns {string | undefined} */
function releasePlatform() {
  if (process.platform === 'darwin') {
    return 'macos';
  }
  if (process.platform === 'linux') {
    return 'linux';
  }
  return undefined;
}

/** @param {string} url @param {string} dest */
function downloadFile(url, dest) {
  return new Promise((resolve, reject) => {
    const request = (target) => {
      https
        .get(target, (response) => {
          if (
            response.statusCode &&
            response.statusCode >= 300 &&
            response.statusCode < 400 &&
            response.headers.location
          ) {
            request(response.headers.location);
            return;
          }
          if (response.statusCode !== 200) {
            reject(new Error(`HTTP ${response.statusCode ?? 'unknown'} for ${target}`));
            return;
          }
          const file = fs.createWriteStream(dest);
          response.pipe(file);
          file.on('finish', () => file.close(() => resolve()));
          file.on('error', reject);
        })
        .on('error', reject);
    };
    request(url);
  });
}

/** @returns {Promise<boolean>} */
async function tryInstallFromNodeRelease() {
  const version = pkg.version;
  const platform = releasePlatform();
  const repository = pkg.repository?.url ?? '';
  const match = repository.match(/github\.com[:/](.+?)\/(.+?)(?:\.git)?$/);
  if (!version || !platform || !match) {
    return false;
  }

  const owner = match[1];
  const repo = match[2];
  const archive = `mynumber-node-${version}-${platform}.tar.gz`;
  const url = `https://github.com/${owner}/${repo}/releases/download/v${version}/${archive}`;
  const tmpDir = path.join(root, 'build', 'release-download');
  const archivePath = path.join(tmpDir, archive);

  fs.mkdirSync(tmpDir, { recursive: true });
  if (fs.existsSync(archivePath)) {
    fs.rmSync(archivePath);
  }

  try {
    await downloadFile(url, archivePath);
  } catch {
    return false;
  }

  const napiVersion = pkg.binary?.napi_versions?.[0] ?? process.versions.napi;
  const bindingDir = path.join(root, 'lib', 'binding', `napi-v${napiVersion}`);
  fs.mkdirSync(bindingDir, { recursive: true });

  execSync(`tar -xzf "${archivePath}" -C "${bindingDir}" mynumber.node`, { stdio: 'inherit' });
  return fs.existsSync(path.join(bindingDir, 'mynumber.node'));
}

module.exports = { tryInstallFromNodeRelease };
