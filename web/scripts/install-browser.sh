#!/usr/bin/env bash
# Preserve the reviewed Playwright browser, OS dependencies and fallback fonts.
# Cache downloads, not the installed OS state: every fresh runner still installs deps.
set -euo pipefail
mkdir -p render-environment
apt_cache="/tmp/atlas-apt"
# /tmp is traversable by apt's _apt downloader, unlike the runner's private HOME.
mkdir -p "$apt_cache"
apt_config=$(mktemp "${RUNNER_TEMP:-/tmp}/atlas-apt.XXXXXX")
trap 'rm -f "$apt_config"' EXIT
cat > "$apt_config" <<APT
Acquire::Retries "2";
Acquire::http::Timeout "30";
Acquire::https::Timeout "30";
Dir::Cache::archives "$apt_cache";
APT::Keep-Downloaded-Packages "true";
Binary::apt-get::APT::Keep-Downloaded-Packages "true";
APT
# Run the lockfile-installed CLI as root so its apt calls inherit this temporary
# configuration. Do not persist apt settings, replace repositories or skip deps.
timeout --kill-after=15s 420s sudo env "PATH=$PATH" "APT_CONFIG=$apt_config" \
  node node_modules/playwright/cli.js install-deps chromium
# The measured cold mirror rate (~134 kB/s) needs about eight minutes for 61 MB.
timeout --kill-after=15s 600s sudo env "APT_CONFIG=$apt_config" \
  apt-get install -y --no-install-recommends fonts-noto-cjk
# Actions caches only public .deb files, not root-only apt lock/partial state.
PLAYWRIGHT_DOWNLOAD_CONNECTION_TIMEOUT=60000 timeout --kill-after=15s 180s \
  node node_modules/playwright/cli.js install chromium
{
  node --version
  node node_modules/playwright/cli.js --version
  printf '\nKorean fallback: '
  fc-match 'sans-serif:lang=ko'
  printf '\nJapanese fallback: '
  fc-match 'sans-serif:lang=ja'
  printf '\nInstalled fonts:\n'
  fc-list : family | sort -u
} | tee render-environment/fonts.txt
