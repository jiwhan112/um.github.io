#!/usr/bin/env bash
# Preserve the reviewed Playwright browser, OS dependencies and fallback fonts.
# Cache downloads, not the installed OS state: every fresh runner still installs deps.
set -euo pipefail
mkdir -p render-environment
apt_cache="/tmp/atlas-apt"
# /tmp is traversable by apt's _apt downloader, unlike the runner's private HOME.
mkdir -p "$apt_cache"
apt_config=$(mktemp "${RUNNER_TEMP:-/tmp}/atlas-apt.XXXXXX")
apt_sources=$(mktemp -d /tmp/atlas-apt-sources.XXXXXX)
chmod 0755 "$apt_sources"
trap 'rm -f "$apt_config"; rm -rf "$apt_sources"' EXIT
# Isolate only this invocation's source view. Keep suites, components, keys,
# security updates and all third-party repositories exactly as configured.
python3 - /etc/apt "$apt_sources" <<'PY_SOURCES'
import pathlib, re, sys
source, destination = map(pathlib.Path, sys.argv[1:])
(destination / "sources.list.d").mkdir(mode=0o755)
replacements = {
    "http://azure.archive.ubuntu.com/ubuntu": "https://archive.ubuntu.com/ubuntu",
    "https://azure.archive.ubuntu.com/ubuntu": "https://archive.ubuntu.com/ubuntu",
    "mirror+file:/etc/apt/apt-mirrors.txt": "https://archive.ubuntu.com/ubuntu",
    "mirror+file:/etc/apt/apt-mirrors-security.txt": "https://security.ubuntu.com/ubuntu",
}
paths = [source / "sources.list", *sorted((source / "sources.list.d").glob("*.list")),
         *sorted((source / "sources.list.d").glob("*.sources"))]
for path in paths:
    if not path.is_file():
        continue
    original = path.read_text()
    lines = []
    for line in original.splitlines(keepends=True):
        if not line.lstrip().startswith("#"):
            for old, new in replacements.items():
                line = re.sub(re.escape(old) + r"(?=/?(?:\s|$))", new, line)
            if re.search(r"mirror\+file:/etc/apt/apt-mirrors[^\s]*", line):
                raise SystemExit(f"Unrecognized Ubuntu mirror list in {path}; refusing to guess")
        lines.append(line)
    target = destination / path.relative_to(source)
    target.write_text("".join(lines))
    target.chmod(0o644)
# An absent legacy sources.list is valid for deb822-only Ubuntu runners.
(destination / "sources.list").touch(exist_ok=True)
PY_SOURCES
cat > "$apt_config" <<APT
Acquire::Retries "2";
Acquire::http::Timeout "30";
Acquire::https::Timeout "30";
Dir::Cache::archives "$apt_cache";
Dir::Etc::sourcelist "$apt_sources/sources.list";
Dir::Etc::sourceparts "$apt_sources/sources.list.d";
APT::Keep-Downloaded-Packages "true";
Binary::apt-get::APT::Keep-Downloaded-Packages "true";
APT
# Run the lockfile-installed CLI as root so its apt calls inherit this temporary
# configuration. Do not persist settings, change repository trust or skip deps.
echo "Installing browser OS dependencies from official Ubuntu HTTPS archives"
timeout --kill-after=15s 420s sudo env "PATH=$PATH" "APT_CONFIG=$apt_config" \
  node node_modules/playwright/cli.js install-deps chromium
# apt checks signed indexes and package hashes, including cached downloads.
echo 'Downloading and installing Noto CJK from official Ubuntu HTTPS archives'
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
