"""Safety and reproducibility contracts for explicit teaching bundles."""
import importlib.util
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('bundles', Path(__file__).with_name('build_bundles.py'))
bundles = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundles)

class BundleContracts(unittest.TestCase):
    def test_unsafe_paths_rejected(self):
        for path in ('/etc/passwd', '../secret', 'a/../b', 'a\\b', 'a//b', '.git/config', 'build/a.cpp', 'a/./b', 'a\x00b'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                bundles.safe_relative(path)

    def test_binaries_and_nonknowledge_sources_rejected(self):
        for path in ('web/package.json', 'knowledge/example.exe', 'knowledge/example.zip'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                bundles.source_bytes(path)

    def test_deterministic_bytes_and_exact_manifest(self):
        import json
        entries = json.loads((bundles.HERE / 'specifications.json').read_text())['bundles']
        for entry in entries:
            with self.subTest(bundle=entry['id']):
                first, record = bundles.build_bundle(entry)
                self.assertEqual(first, bundles.build_bundle(entry)[0])
                self.assertEqual(first, (bundles.ROOT / record['path']).read_bytes())
                self.assertEqual(len(record['files']), len(record['sourceFiles']))

    def test_duplicate_members_rejected(self):
        import json
        entry = json.loads((bundles.HERE / 'specifications.json').read_text())['bundles'][0]
        entry['files'].append(entry['files'][0])
        with self.assertRaises(ValueError):
            bundles.build_bundle(entry)

if __name__ == '__main__':
    unittest.main()
