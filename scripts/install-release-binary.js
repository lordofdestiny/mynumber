'use strict';

const fs = require('node:fs');
const path = require('node:path');
const https = require('node:https');
const { execSync } = require('node:child_process');
const { releasePlatform, isWindows } = require('./native-platform');

const root = path.join(__dirname, '..');
const pkg = JSON.parse(fs.readFileSync(path.join(root, 'package.json'), 'utf8'));

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

/** @param {string} archivePath @param {string} bindingDir */
function extractMynumberNode(archivePath, bindingDir) {
  if (archivePath.endsWith('.zip')) {
    if (isWindows()) {
      const dest = path.join(bindingDir, 'mynumber.node');
      execSync(
        `powershell -NoProfile -Command "Expand-Archive -Path '${archivePath.replace(/'/g, "''")}' -DestinationPath '${bindingDir.replace(/'/g, "''")}' -Force"`,
        { stdio: 'inherit' },
      );
      if (!fs.existsSync(dest)) {
        throw new Error(`expected mynumber.node after extracting ${archivePath}`);
      }
      return;
    }

    execSync(`unzip -o -j "${archivePath}" mynumber.node -d "${bindingDir}"`, { stdio: 'inherit' });
    return;
  }

  execSync(`tar -xzf "${archivePath}" -C "${bindingDir}" mynumber.node`, { stdio: 'inherit' });
}

/**
 * @param {string} owner
 * @param {string} repo
 * @param {string} version
 * @param {string} platform
 * @param {string} extension
 * @returns {string}
 */
function releaseAssetUrl(owner, repo, version, platform, extension) {
  const archive = `mynumber-node-${version}-${platform}.${extension}`;
  return {
    archive,
    url: `https://github.com/${owner}/${repo}/releases/download/v${version}/${archive}`,
  };
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
  const extensions = platform === 'windows' ? ['zip', 'tar.gz'] : ['tar.gz', 'zip'];
  const tmpDir = path.join(root, 'build', 'release-download');
  fs.mkdirSync(tmpDir, { recursive: true });

  const napiVersion = pkg.binary?.napi_versions?.[0] ?? process.versions.napi;
  const bindingDir = path.join(root, 'lib', 'binding', `napi-v${napiVersion}`);
  fs.mkdirSync(bindingDir, { recursive: true });

  for (const extension of extensions) {
    const { archive, url } = releaseAssetUrl(owner, repo, version, platform, extension);
    const archivePath = path.join(tmpDir, archive);
    if (fs.existsSync(archivePath)) {
      fs.rmSync(archivePath);
    }

    try {
      await downloadFile(url, archivePath);
    } catch {
      continue;
    }

    try {
      extractMynumberNode(archivePath, bindingDir);
      if (fs.existsSync(path.join(bindingDir, 'mynumber.node'))) {
        return true;
      }
    } catch {
      // Try the next archive format.
    }
  }

  return false;
}

module.exports = { tryInstallFromNodeRelease };
