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
            prev_end = prev_start + prev_size
            end = start + size
            distance = start - prev_end
            if distance < 0:
                print(f'overlapping ranges: [0x{prev_start:08X}:0x{prev_end:08X}) [0x{start:08X}:0x{end:08X})')

            if distance <= 0:
                result[-1] = (max(prev_end, end) - prev_start, prev_start)
            else:
                result.append((size, start))

    # Write result:
    with open(pathname + '.new', 'wb') as fout:
        for entry in result:
            fout.write(pack('<II', *entry))

if __name__ == '__main__':
    main(sys.argv[1])
