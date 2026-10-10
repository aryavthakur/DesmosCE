"""Retain upstream source/notices without redistributing example font files."""
from pathlib import Path
import sys
import tarfile

if len(sys.argv) != 3:
    raise SystemExit('usage: package_dependency_source.py upstream.tar.gz output.tar.gz')
fonts = {'.ttf', '.otf', '.woff', '.woff2', '.eot', '.pfb', '.pcf', '.bdf', '.fnt'}
omitted = []
with tarfile.open(sys.argv[1], 'r:gz') as source, tarfile.open(sys.argv[2], 'w:gz') as target:
    for member in source:
        if Path(member.name).suffix.lower() in fonts:
            omitted.append(member.name)
            continue
        target.addfile(member, source.extractfile(member) if member.isfile() else None)
print('Upstream source retained; omitted example/test font files:', len(omitted))
for name in omitted:
    print(name)
