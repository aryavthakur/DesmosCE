"""Fail closed on the selected CE v15 static layout, not a runtime safety proof."""
from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parents[1]
output = subprocess.check_output(['size', str(root / 'bin/DESMOSCE.obj')], text=True)
text, data, bss = map(int, output.splitlines()[1].split()[:3])
program = root / 'bin/DESMOSCE.8xp'
# Conservative full .8xv file sizes: GRAPHX + FILEIOC + KEYPADC + LIBLOAD v15.
runtime_upper_bound = 15511
loader_reserve = 6144
code_region = 0xD40000 - 0xD1A87F
report = {
    'binary_bytes': program.stat().st_size,
    'text_bytes': text, 'data_bytes': data, 'bss_bytes': bss,
    'runtime_library_upper_bound': runtime_upper_bound,
    'loader_reserve_bytes': loader_reserve,
    'code_region_bytes': code_region,
    'estimated_code_margin_bytes': code_region - text - data - runtime_upper_bound,
    'bss_heap_region_bytes': 0xD13FD8 - 0xD052C6,
    'estimated_heap_remainder_bytes': 0xD13FD8 - 0xD052C6 - bss,
    'stack_region_bytes': 0xD1A87E - 0xD13FD8,
    'hardware_tested': False,
    'warning': 'Static budget only. Library relocation, fragmentation, stack high-water and launcher behavior require device validation.'
}
assert report['binary_bytes'] < 65535, report
assert text + data + runtime_upper_bound + loader_reserve <= code_region, report
assert bss <= 55000, report
(root / 'build').mkdir(exist_ok=True)
(root / 'build/memory-report.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
