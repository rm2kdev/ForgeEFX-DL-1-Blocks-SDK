"""Generate matching ABI descriptors and package metadata from an effect folder."""
import argparse
import json
import re
from pathlib import Path


def generate(folder, output, developer):
    folder, output = Path(folder).resolve(), Path(output)
    effect = json.loads((folder / 'parameters.json').read_text(encoding='utf-8'))
    category_path = folder.parent / 'category.json'
    category = json.loads(category_path.read_text(encoding='utf-8')) if category_path.exists() else {}
    effect_id = effect.get('id', folder.name)
    minimum = effect.get('minimum_host_version', '0.1.0')
    maximum = effect.get('maximum_host_version', '0.2.0')
    version_pattern = r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)'
    def version(value):
        if not isinstance(value, str) or not re.fullmatch(version_pattern, value):
            raise ValueError('Host versions must use major.minor.patch')
        result = tuple(map(int, value.split('.')))
        if any(part > 4294967295 for part in result):
            raise ValueError('Host version component exceeds uint32')
        return result
    if version(minimum) >= version(maximum):
        raise ValueError('Host version range must be nonempty (maximum is exclusive)')
    if not re.fullmatch(r'[a-zA-Z0-9_.-]+', effect_id):
        raise ValueError('Invalid permanent effect ID')
    params = effect['parameters']
    if not 1 <= len(params) <= 64 or len({p['name'] for p in params}) != len(params):
        raise ValueError('Expected 1..64 parameters with unique keys')
    words = effect.get('state_words', 32772)
    if type(words) is not int or not 1 <= words <= 67108864:
        raise ValueError('Invalid state_words')
    alignment = effect.get('state_alignment', 8)
    if alignment not in (4, 8):
        raise ValueError('ABI v1 supports state_alignment 4 or 8')
    for key in ('tempo_param', 'sync_param', 'trails_param'):
        if not -1 <= effect.get(key, -1) < len(params):
            raise ValueError('Invalid ' + key)
    if not -1 <= effect.get('tail_seconds', 12) <= 3600:
        raise ValueError('Invalid tail_seconds')
    metadata = dict(effect, schema_version=1, abi_version=1,
                    minimum_host_version=minimum, maximum_host_version=maximum,
                    effect_id=effect_id, developer_id=developer,
                    package_version=effect.get('package_version', '1.0.0'),
                    category_id=folder.parent.name,
                    category_name=category.get('name', folder.parent.name.upper()),
                    category_short_name=category.get('short_name', folder.parent.name[:3].upper()),
                    category_order=category.get('order', 1000), state_alignment=alignment,
                    state_words=words, sample_rate=48000)
    metadata.pop('inspired_by', None)
    q = json.dumps
    lines = ['#include "forgeefx_block.h"', '_Static_assert(sizeof(int) == sizeof(int32_t), "ABI v1 requires 32-bit int");']
    rows = []
    for index, p in enumerate(params):
        if any(type(p[k]) is not int for k in ('min', 'max', 'default')) or not -1000000 <= p['min'] <= p['max'] <= 1000000 or p['max'] - p['min'] > 1000000:
            raise ValueError('Parameter bounds must be within +/-1000000 with span at most 1000000')
        if not p['min'] <= p['default'] <= p['max']:
            raise ValueError('Parameter default outside bounds')
        labels = p.get('labels', [])
        if 'labels' in p and len(labels) != p['max'] - p['min'] + 1:
            raise ValueError('Choice count does not match range')
        if labels:
            lines.append(f'static const char *const choices_{index}[] = {{' + ','.join(map(q, labels)) + '};')
        fmt = 3 if labels else {'number': 0, 'knob': 1, 'signed': 2}.get(p.get('format'), 1 if p.get('unit', '') == '' and p['min'] == 0 and p['max'] == 100 else 0)
        rows.append('{' + ','.join([q(p['name']), q(p.get('display_name', p['name'])), q(p.get('unit', '')), str(p['min']), str(p['max']), str(p['default']), str(fmt), str(len(labels)), f'choices_{index}' if labels else '0']) + '}')
    lines.append('static const ForgeEFXParameter parameters[] = {' + ','.join(rows) + '};')
    process, reset = effect.get('process', folder.name + '_process'), effect.get('reset', folder.name + '_reset')
    stereo = effect.get('stereo_process', '0')
    render = effect.get('render', folder.name + '_render' if (folder / 'ui.c').is_file() else '0')
    for symbol in (process, reset, stereo, render):
        if symbol != '0' and not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', symbol):
            raise ValueError('Invalid callback symbol')
    lines += [f'int {process}(int, int *, int *);', f'void {reset}(int *);']
    if stereo != '0':
        lines.append(f'void {stereo}(int,int,int *,int *,int *,int *,int *);')
    renderer = '0'
    if (folder / 'ui.c').is_file():
        lines += [f'void {render}(const BlockUI *);',
                  'static void render_bridge(const BlockUI *ui, const ForgeEFXRenderServices *services) {',
                  'const ForgeEFXRenderServices *previous;',
                  'if (!services || services->abi_version != FORGEEFX_BLOCK_ABI_VERSION || services->struct_size < sizeof(ForgeEFXRenderServices)) return;',
                  'previous = forgeefx_set_render_services(services);', f'{render}(ui);',
                  'forgeefx_set_render_services(previous);', '}']
        renderer = 'render_bridge'
    fields = ['FORGEEFX_BLOCK_ABI_VERSION', 'sizeof(ForgeEFXBlockApi)', q(developer), q(effect_id), q(effect.get('name', folder.name.upper())), q(metadata['package_version']), q(metadata['category_id']), q(metadata['category_name']), q(metadata['category_short_name']), str(metadata['category_order']), str(effect.get('order', 1000)), str(effect.get('index', -1)), str(len(params)), 'parameters', str(words), str(alignment), 'FORGEEFX_BLOCK_SAMPLE_RATE', *[str(effect.get(k, -1)) for k in ('tempo_param', 'sync_param', 'trails_param')], str(effect.get('tail_seconds', 12)), process, reset, stereo, renderer]
    lines += ['static const ForgeEFXBlockApi api = {' + ','.join(fields) + '};',
              'FORGEEFX_BLOCK_EXPORT const ForgeEFXBlockApi *forgeefx_get_block_api(uint32_t requested_abi) { return requested_abi == FORGEEFX_BLOCK_ABI_VERSION ? &api : 0; }']
    output.mkdir(parents=True, exist_ok=True)
    for name, value in [('descriptor.c', '\n'.join(lines) + '\n'), ('manifest.json', json.dumps(metadata, indent=2) + '\n')]:
        path = output / name
        if not path.exists() or path.read_text(encoding='utf-8') != value:
            path.write_text(value, encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--developer', required=True)
    args = parser.parse_args()
    generate(args.folder, args.output, args.developer)
