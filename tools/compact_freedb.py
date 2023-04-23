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
    write_result = True
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
            if distance < 0:
                print(f'overlapping ranges: [{prev_start}:{prev_start + prev_size}) [{start}:{start + size})')
                write_result = False
                continue
            if distance == 0:
                result[-1] = (prev_size + size, prev_start)
            else:
                result.append((size, start))

    # Write result:
    if not write_result:
        sys.exit(1)

    with open(pathname, 'wb') as fout:
        for entry in result:
            fout.write(pack('<II', *entry))

if __name__ == '__main__':
    main(sys.argv[1])
