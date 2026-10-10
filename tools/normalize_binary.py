"""Normalize only the TI transfer-file comment; leave program data/checksum intact."""
from pathlib import Path
import struct

path = Path(__file__).resolve().parents[1] / 'bin/DESMOSCE.8xp'
data = bytearray(path.read_bytes())
assert data[:11] == b'**TI83F*\x1a\n\x00', 'Unexpected TI transfer header'
length = struct.unpack_from('<H', data, 53)[0]
assert len(data) == 55 + length + 2, 'Invalid transfer length'
assert sum(data[55:55+length]) & 0xffff == struct.unpack_from('<H', data, 55+length)[0], 'Bad checksum'
comment = b'DesmosCE experimental alpha; CE v15.0'
data[11:53] = comment.ljust(42, b'\0')
path.write_bytes(data)
print('Normalized timestamp comment; calculator program payload unchanged.')
