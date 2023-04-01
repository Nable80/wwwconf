#!/usr/bin/python3

import os.path as op
from struct import pack, unpack
import sys

HASHINDEX_BLOCK_SIZE = 2000
BLOCK_HDR_SIZE = 8

def main(pathname):
    assert op.basename(pathname) == 'profindex.idx'
    with open(pathname, 'rb') as inf:
        idx_data = inf.read()
    assert len(idx_data) % HASHINDEX_BLOCK_SIZE == 0

    for start in range(0, len(idx_data), HASHINDEX_BLOCK_SIZE):
        block = idx_data[start: start + HASHINDEX_BLOCK_SIZE]
        used, next_block = unpack('<H 2x I', block[:BLOCK_HDR_SIZE])
        assert BLOCK_HDR_SIZE <= used < HASHINDEX_BLOCK_SIZE
        assert next_block == 0xFFFFFFFF or (next_block % HASHINDEX_BLOCK_SIZE == 0 and next_block < len(idx_data))
        assert block[used] == 0
        print(start, used, next_block)
        pos = BLOCK_HDR_SIZE
        while pos < used:
            assert block[pos] == 0x0A # 10
            name_start = pos + 1
            name_end = block.index(b'\x0D', name_start) # 13
            name_bytes = block[name_start: name_end]
            name_index = unpack('<I', block[name_end + 1: name_end + 5])[0]
            print('> %08X "%s"' % (name_index, name_bytes.decode('cp1251', errors='replace')))
            pos = name_end + 5
        assert pos == used

if __name__ == '__main__':
    main(sys.argv[1])
