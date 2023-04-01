#!/usr/bin/python3

import os.path as op
from struct import pack, unpack
import sys

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
            free_ranges.append(unpack('<2I', entry))

    # Sort ranges by start address:
    free_ranges.sort(key=lambda x: x[1])

    # Compactify:
    result = []
    for size, start in free_ranges:
        # Omit empty entries:
        if size == 0:
            continue

        if len(result) == 0:
            # Append first non-empty entry as-is:
            result.append((size, start))
        else:
            # Check for overlaps and merge adjacent entries:
            prev_size, prev_start = result[-1]
            distance = start - (prev_start + prev_size)
            assert distance >= 0, 'overlapping ranges'
            if distance == 0:
                result[-1] = (prev_size + size, prev_start)
            else:
                result.append((size, start))

    # Write result:
    with open(pathname, 'wb') as fout:
        for entry in result:
            fout.write(pack('<II', *entry))

if __name__ == '__main__':
    main(sys.argv[1])
