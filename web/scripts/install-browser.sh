#!/usr/bin/env bash
# Use exactly the same dependency set that produced the reviewed screenshots.
# install chromium alone omits fallback fonts (e.g. Unifont) installed by --with-deps.
set -euo pipefail
npx playwright install --with-deps chromium
sudo apt-get -o Acquire::Retries=1 -o Acquire::http::Timeout=25 -o Acquire::https::Timeout=25 install -y --no-install-recommends fonts-noto-cjk
mkdir -p render-environment
{
  node --version
  npx playwright --version
  printf '\nKorean fallback: '
  fc-match 'sans-serif:lang=ko'
  printf '\nJapanese fallback: '
  fc-match 'sans-serif:lang=ja'
  printf '\nInstalled fonts:\n'
  fc-list : family | sort -u
} | tee render-environment/fonts.txt
