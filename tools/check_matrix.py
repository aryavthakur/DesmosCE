"""Validate release checklist numbering, columns and the required status vocabulary."""
from collections import Counter
from pathlib import Path
import re

path = Path(__file__).resolve().parents[1] / 'docs/feature-matrix.md'
allowed = {'IMPLEMENTED AND TESTED', 'IMPLEMENTED BUT UNVERIFIED ON HARDWARE',
           'PARTIALLY IMPLEMENTED', 'NOT IMPLEMENTED', 'HARDWARE-INAPPLICABLE'}
rows = []
for line in path.read_text().splitlines():
    if re.match(r'^\|\s*\d+\s*\|', line):
        cells = [cell.strip() for cell in line.split('|')[1:-1]]
        assert len(cells) == 7, (len(cells), line)
        assert cells[4] in allowed, cells
        assert all(cells), cells
        rows.append(cells)
assert [int(row[0]) for row in rows] == list(range(1,274)), 'Expected exactly items 1 through 273'
print('Feature matrix: 273 numbered entries; required columns/statuses present.')
for status, count in sorted(Counter(row[4] for row in rows).items()):
    print(f'{status}: {count}')
