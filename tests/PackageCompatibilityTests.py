"""Published package metadata keeps host ranges explicit and well formed."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', ROOT / 'sdk/tools/generate_package.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class CompatibilityTests(unittest.TestCase):
    def generate(self, folder_name='effect', **values):
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary) / folder_name
            folder.mkdir()
            source = dict(parameters=[dict(name='GAIN', min=0, max=100, default=50)], **values)
            (folder / 'parameters.json').write_text(json.dumps(source))
            output = Path(temporary) / 'output'
            generator.generate(folder, output, 'test')
            return json.loads((output / 'manifest.json').read_text())

    def test_defaults_and_custom_range(self):
        defaults = self.generate()
        self.assertEqual(defaults['minimum_host_version'], '0.1.0')
        self.assertEqual(defaults['maximum_host_version'], '0.2.0')
        custom = self.generate(minimum_host_version='0.1.3', maximum_host_version='2.0.0')
        self.assertEqual(custom['minimum_host_version'], '0.1.3')

    def test_invalid_ranges(self):
        for low, high in [('0.1', '1.0.0'), ('01.1.0', '1.0.0'), ('0.1.0', '0.1.0'),
                          ('2.0.0', '1.0.0'), ('0.1.0', '4294967296.0.0'), (True, '1.0.0')]:
            with self.subTest(low=low, high=high), self.assertRaises(ValueError):
                self.generate(minimum_host_version=low, maximum_host_version=high)

    def test_no_editor_in_hyphenated_project_folder(self):
        metadata = self.generate(folder_name='my-effect', id='yourcompany.youreffect',
                                 process='gain_process', reset='gain_reset')
        self.assertEqual(metadata['effect_id'], 'yourcompany.youreffect')

    def test_invalid_explicit_callback_still_rejected(self):
        for key in ('process', 'reset', 'stereo_process', 'render'):
            with self.subTest(callback=key), self.assertRaises(ValueError):
                self.generate(**{key: 'invalid-callback'})


if __name__ == '__main__':
    unittest.main()
