#!/usr/bin/python3

import os.path as op
from struct import pack, unpack
import sys

from bitarray import bitarray

def main(pathname):
    assert op.basename(pathname) in (
        'freeindex.msg', 'freemess.msg',
        'profbfree.db', 'profifree.db'
    )

    # Read current ranges, each entry is a pair of DWORDs (size, start):
    free_ranges = []
    with open(pathname, 'rb') as fin:
        while True:
            entry = fin.read(8)
            if not entry:
                break
            size, start = unpack('<2I', entry)
            # Skip empty entries:
            if size > 0:
                free_ranges.append((size, start))

    # Find the maximum end address and allocate bitmap:
    bmap_size = max(x[0] + x[1] for x in free_ranges, default=0)
    if bmap_size == 0:
        return
    bmap = bitarray(bmap_size, endian='big')
    bmap.setall(0)
    for size, start in free_ranges:
        assert not bmap[start: start + size].any(), 'overlapping ranges'
        bmap[start: start + size] = 1

    # Write result:
    with open(pathname + '.bmap', 'wb') as fout:
        bmap.tofile(fout)

if __name__ == '__main__':
    main(sys.argv[1])
