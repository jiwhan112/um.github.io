#!/usr/bin/env python3
"""Build/check explicit, source-only teaching ZIPs. Never extract or execute them."""
import argparse
import hashlib
import io
import json
import re
import zipfile
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / 'knowledge/examples'
SUFFIXES = {'.c', '.h', '.cpp', '.hpp', '.cxx', '.hxx', '.cc', '.hh', '.py',
            '.sh', '.hlsl', '.fx', '.rc', '.manifest', '.cmake', '.txt', '.md',
            '.json', '.csv', '.html', '.ini', '.toml', '.png', '.bmp'}


def safe_relative(value):
    if not isinstance(value, str) or not value or '\\' in value or ':' in value or re.search(r'[\x00-\x1f\x7f]', value):
        raise ValueError(f'Invalid relative path: {value!r}')
    p = PurePosixPath(value)
    if p.is_absolute() or any(x in ('', '.', '..') for x in value.split('/')):
        raise ValueError(f'Unsafe path: {value}')
    if any(x in ('.git', 'node_modules', '__pycache__', '__MACOSX', 'build') for x in p.parts):
        raise ValueError(f'Non-source directory: {value}')
    return p


def source_bytes(value):
    relative = safe_relative(value)
    if relative.parts[0] != 'knowledge' or relative.suffix.lower() not in SUFFIXES:
        raise ValueError(f'Not an allowed teaching source: {value}')
    path = ROOT / str(relative)
    for parent in (path, *path.parents):
        if parent == ROOT:
            break
        if parent.is_symlink():
            raise ValueError(f'Symlink is not a source input: {value}')
    if not path.resolve().is_relative_to(ROOT) or not path.is_file():
        raise ValueError(f'Missing/outside source: {value}')
    data = path.read_bytes()
    if len(data) > 2_000_000:
        raise ValueError(f'Unexpectedly large teaching file: {value}')
    if relative.suffix.lower() not in {'.png', '.bmp'}:
        data.decode('utf-8')
        if b'\x00' in data:
            raise ValueError(f'Binary disguised as source: {value}')
    return data


def build_bundle(spec):
    ident = spec['id']
    if not re.fullmatch(r'[a-z0-9]+(?:-[a-z0-9]+)*', ident):
        raise ValueError(f'Invalid bundle ID: {ident}')
    members, provenance = {}, []
    for entry in spec['files']:
        relative = safe_relative(entry['path'])
        name = f'{ident}/{relative}'
        if name.casefold() in {x.casefold() for x in members}:
            raise ValueError(f'Duplicate/case-colliding member: {name}')
        data = source_bytes(entry['source'])
        members[name] = data
        provenance.append({'path': name, 'source': entry['source'],
                           'sha256': hashlib.sha256(data).hexdigest()})
    if f'{ident}/README.md' not in members:
        raise ValueError(f'Bundle needs an unpack/build README: {ident}')
    output = io.BytesIO()
    with zipfile.ZipFile(output, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(members.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    payload = output.getvalue()
    pages = spec['pages']
    if not pages or len(set(pages)) != len(pages) or any(not re.fullmatch(r'[a-z0-9-]+', p) for p in pages):
        raise ValueError(f'Invalid page mapping: {ident}')
    return payload, {'id': ident, 'title': spec['title'],
                     'path': f'knowledge/examples/{ident}.zip',
                     'sha256': hashlib.sha256(payload).hexdigest(), 'bytes': len(payload),
                     'files': sorted(members), 'pages': pages,
                     'readme': spec['readmeSource'],
                     'sourceFiles': sorted(provenance, key=lambda x: x['path'])}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Read-only exact deterministic archive/catalog check')
    args = parser.parse_args()
    specs = json.loads((HERE / 'specifications.json').read_text())
    if specs['version'] != 1:
        raise ValueError('Unsupported specifications version')
    bundles, ids = [], set()
    for spec in specs['bundles']:
        if spec['id'] in ids:
            raise ValueError('Duplicate bundle ID')
        ids.add(spec['id'])
        payload, record = build_bundle(spec)
        path = ROOT / record['path']
        if args.check:
            if path.is_symlink() or path.read_bytes() != payload:
                raise ValueError(f'Archive differs from deterministic source inputs: {path}')
        else:
            if path.is_symlink():
                raise ValueError(f'Archive destination is a symlink: {path}')
            path.write_bytes(payload)
        bundles.append(record)
    result = {'version': 1, 'bundles': sorted(bundles, key=lambda x: x['id'])}
    encoded = (json.dumps(result, ensure_ascii=False, indent=2) + '\n').encode()
    catalog = HERE / 'catalog.json'
    if args.check:
        if catalog.read_bytes() != encoded:
            raise ValueError('Catalog differs from source-derived records')
    else:
        if catalog.is_symlink():
            raise ValueError('Catalog destination is a symlink')
        catalog.write_bytes(encoded)
    print(f'{"Verified" if args.check else "Built"} {len(bundles)} deterministic source-only bundles')


if __name__ == '__main__':
    main()
