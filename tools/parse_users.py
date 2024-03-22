#!/usr/bin/python3

from ctypes import (
    LittleEndianStructure,
    c_byte, c_char,
    c_int8, c_int16, c_int32,
    c_uint8, c_uint16, c_uint32,
    sizeof
)
import datetime as dt
import os.path as op
from struct import pack, unpack
import sys

"""
Custom DB consists of the following dirs and files:

data/internals/activitylog1.dat - TODO
data/internals/activitylog2.dat - TODO
data/internals/antispam.dat - TODO
data/internals/authuser.dat - user sessions, array of SSavedAuthSeq

data/messages/freeindex.msg - TODO, array of SFreeDBEntry
data/messages/freemess.msg - TODO, array of SFreeDBEntry
data/messages/globalann.msg - TODO
data/messages/index.msg - TODO
data/messages/messages.msg - TODO
data/messages/ra_index.msg - TODO
data/messages/vra_index.msg - TODO

data/profiles/pic/ - not used

data/profiles/profbfree.db - freed ranges of profbody.db, array of SFreeDBEntry
data/profiles/profbody.db - TODO
data/profiles/profcnicks.db - array of AltNamesStruct
data/profiles/profifree.db - freed ranges of profindex.db, array of SFreeDBEntry
data/profiles/profindex.db - two dwords (last_uid, total_users), then array of SProfile_UserInfo
data/profiles/profindex.idx - custom hash table, 2000 bytes per bucket
data/profiles/profpers.db - array of SPersonalMessage

data/settings/badwords.txt - words that are auto-replaced with ###, one word per line
data/settings/banned.txt - banlist, list of triplet strings:
    "ip-template type optional-comment", where type is:
        * - no posts are allowed at all
        p - (from 'proxy') disallow posts from unregs
        = - whitelisted IP, don't check further entries

data/wwwconf.log - text log with errors, warnings and other informational messages
"""

PROFILES_MAX_USERNAME_LENGTH            = 30
PROFILES_MAX_PASSWORD_LENGTH            = 33
PROFILES_MAX_ICQ_LEN                    = 16
PROFILES_MAX_ALT_DISPLAY_NAME           = 90
PROFILES_FAV_THREADS_COUNT              = 20 # 4*20 = 80 bytes real size
PROFILES_MAX_SIGNATURE_LENGTH           = 255
PROFILES_FULL_USERINFO_MAX_NAME         = 255
PROFILES_FULL_USERINFO_MAX_EMAIL        = 255
PROFILES_FULL_USERINFO_MAX_HOMEPAGE     = 70
PROFILES_FULL_USERINFO_MAX_SELECTEDUSR  = 185
PROFILE_PERSONAL_MESSAGE_LENGTH = 385
# hashindex constants:
HASHINDEX_BLOCK_SIZE = 2000
HASHINDEX_BLOCK_HDR_SIZE = 8
# constants for AltNamesStruct:
MAX_REAL_NICK_SIZE = 30
MAX_ALT_NICK_SIZE = 300
# flag in SSavedAuthSeq.uid:
SEQUENCE_IP_CHECK_DISABLED = 0xF0000000

class AltNamesStruct(LittleEndianStructure):
    _pack_ = 1
    _fields_ = (
        ('uid', c_uint32),
        ('rname', c_char * MAX_REAL_NICK_SIZE),
        ('aname', c_char * MAX_ALT_NICK_SIZE),
    )
assert sizeof(AltNamesStruct) == 334

class SActivityLogRecord(LittleEndianStructure):
    _pack_ = 4
    _fields_ = [
        ('ip', c_uint32),
        ('count', c_uint32),
        ('time', c_uint32),
    ]
assert sizeof(SActivityLogRecord) == 12

class SSavedAuthSeq(LittleEndianStructure):
    _pack_ = 4
    _fields_ = [
        ('sid', c_uint32 * 2), # seq = (sid[0] << 32) | sid[1]
        ('uid', c_uint32), # upper 4 bits (why?) are used as a flag (disable IP check)
        ('ip', c_uint32),
        ('prof_index', c_uint32), # index in profindex.db
        ('expiration', c_uint32), # time_t
    ]
assert sizeof(SSavedAuthSeq) == 24

class SPersonalMessage(LittleEndianStructure):
    _pack_ = 4
    _fields_ = [
        ('prev', c_uint32),
        ('poster_prev', c_uint32),
        ('name_from', c_char * PROFILES_MAX_USERNAME_LENGTH),
        ('uid_from', c_uint32),
        ('name_to', c_char * PROFILES_MAX_USERNAME_LENGTH),
        ('uid_to', c_uint32),
        ('date', c_uint32),
        ('msg', c_char * PROFILE_PERSONAL_MESSAGE_LENGTH),
    ]
assert sizeof(SPersonalMessage) == 472

class SViewSettings(LittleEndianStructure):
    _pack_ = 1
    _fields_ = (
        ('dsm', c_uint16),
        ('topics', c_uint32),
        ('tv', c_uint16),
        ('tc', c_uint16),
        # Next fields should be uint8/int8 but 'tt' crosses a byte boundary
        # and ctypes handles it differently from C, should I file it as a bug?
        ('ss', c_uint16, 3),
        ('lsel', c_uint16, 2),
        ('tt', c_uint16, 4),
        ('tz', c_int16, 5),
        ('reserved', c_uint16, 2),
    )
assert sizeof(SViewSettings) == 12

class SProfile_FullUserInfo(LittleEndianStructure):
    _pack_ = 1
    _fields_ = (
        ('full_name', c_char * PROFILES_FULL_USERINFO_MAX_NAME),
        ('email', c_char * PROFILES_FULL_USERINFO_MAX_EMAIL),
        ('homepage', c_char * PROFILES_FULL_USERINFO_MAX_HOMEPAGE),
        ('selected_users', c_char * PROFILES_FULL_USERINFO_MAX_SELECTEDUSR),
        ('signature', c_char * PROFILES_MAX_SIGNATURE_LENGTH),
        ('create_date', c_uint32), # time_t
        ('size', c_uint32),
        ('about', c_uint32) # somehow it's char *
    )
assert sizeof(SProfile_FullUserInfo) == 1032

class SProfile_UserInfo(LittleEndianStructure):
    _pack_ = 1
    _fields_ = (
        ('username', c_char * PROFILES_MAX_USERNAME_LENGTH),
        ('password', c_char * PROFILES_MAX_PASSWORD_LENGTH),
        ('vs', SViewSettings),
        ('icqnumber', c_char * PROFILES_MAX_ICQ_LEN),
        ('altdisplayname', c_char * PROFILES_MAX_ALT_DISPLAY_NAME),
        ('favs', c_uint32 * PROFILES_FAV_THREADS_COUNT),
        ('refresh_count', c_uint32), # number of CUserLogin::CheckSession calls
        ('readpersmescnt', c_uint16), # received persmsg read count
        ('persmescnt', c_uint16), # received persmsg current count
        ('postedmescnt', c_uint16), # posted persmsg count
        ('persmsg', c_uint32), # received personal message index (or 0xFFFFFFFF if does not exist)
        ('postedpersmsg', c_uint32), # posted personal message index (or 0xFFFFFFFF)
        ('status', c_uint8),
        ('last_ip', c_uint32), # IP of last login
        ('secheader', c_uint8), # security level for message header, unused
        ('align1', c_uint8 * 3),
        ('uniq_id', c_uint32), # unique user identifier
        ('flags', c_uint32), # flags for user (e.g.: have picture... etc.)
        ('secur', c_uint8), # security level of user (for message body), unused
        ('align2', c_uint8 * 3),
        ('right', c_uint32), # user permissions
        ('postcount', c_uint32), # number of posts to conference
        ('login_date', c_uint32), # time_t timestamp
        ('fullinfo_id', c_uint32), # index in profile bodies file of structure with common user information
    )
assert sizeof(SProfile_UserInfo) == 316

def decode_str(raw_bytes):
    return raw_bytes.decode('cp1251', errors='replace')

def decode_ip(u32_val):
    return '.'.join(str(x) for x in pack('<I', u32_val))

def show_profiles(pathname):
    with open(pathname, 'rb') as inf:
        user_data = inf.read()
    # 2 DWORDs, then array of SProfile_UserInfo entries
    assert len(user_data) % sizeof(SProfile_UserInfo) == 8

    last_uid, total_users = unpack('<2I', user_data[:8])
    num_entries = (len(user_data) - 8) // sizeof(SProfile_UserInfo)
    print(f'last_uid={last_uid}, total_users={total_users}, num_entries={num_entries}')
    for start in range(8, len(user_data), sizeof(SProfile_UserInfo)):
        user_info = SProfile_UserInfo.from_buffer_copy(user_data, start)
        print(f'0x{start:08X}:', decode_str(user_info.username))

def show_hashindex(pathname):
    with open(pathname, 'rb') as inf:
        idx_data = inf.read()
    assert len(idx_data) % HASHINDEX_BLOCK_SIZE == 0

    entries = []
    # block offset -> hash value for entries in this block
    hash_value_map = { i * HASHINDEX_BLOCK_SIZE: i for i in range(256) }
    for start in range(0, len(idx_data), HASHINDEX_BLOCK_SIZE):
        hash_value = hash_value_map[start]
        block = idx_data[start: start + HASHINDEX_BLOCK_SIZE]
        used, next_block = unpack('<H 2x I', block[:HASHINDEX_BLOCK_HDR_SIZE])
        assert HASHINDEX_BLOCK_HDR_SIZE <= used < HASHINDEX_BLOCK_SIZE
        if next_block != 0xFFFFFFFF:
            assert next_block % HASHINDEX_BLOCK_SIZE == 0 and next_block < len(idx_data)
            hash_value_map[next_block] = hash_value
        assert block[used] == 0
        print(f'bucket at 0x{start:08X}: used={used}, next_block=0x{next_block:08X}')
        pos = HASHINDEX_BLOCK_HDR_SIZE
        while pos < used:
            assert block[pos] == 0x0A # 10
            name_start = pos + 1
            name_end = block.index(b'\x0D', name_start) # 13
            name_bytes = block[name_start: name_end]
            assert sum(name_bytes) & 0xFF == hash_value
            name = decode_str(name_bytes)
            name_index = unpack('<I', block[name_end + 1: name_end + 5])[0]
            name_index_comment = '' if name_index % sizeof(SProfile_UserInfo) == 8 else ' (invalid)'
            print(f'> "{name}" -> 0x{name_index:08X}{name_index_comment}')
            entries.append((name_index, name))
            pos = name_end + 5
        assert pos == used
    # Check for overlaps:
    entries.sort()
    for i in range(len(entries) - 1):
        name_index, name = entries[i]
        next_name_index, next_name = entries[i + 1]
        if name_index + sizeof(SProfile_UserInfo) > next_name_index:
            print(f'Entry for "{name}" (0x{name_index:08X}) overlaps "{next_name}" (0x{next_name_index:08X})')

def show_altnames(pathname):
    with open(pathname, 'rb') as inf:
        data = inf.read()
    # array of AltNamesStruct entries
    assert len(data) % sizeof(AltNamesStruct) == 0
    for start in range(0, len(data), sizeof(AltNamesStruct)):
        entry = AltNamesStruct.from_buffer_copy(data, start)
        print(f'0x{start:08X}: uid={entry.uid}, rname="{decode_str(entry.rname)}", aname="{decode_str(entry.aname)}"')

def show_freedb(pathname):
    fname = op.basename(pathname)
    # Read ranges, each entry is a pair of DWORDs (size, start):
    free_ranges = []
    with open(pathname, 'rb') as fin:
        while True:
            entry = fin.read(8)
            if not entry:
                break
            free_ranges.append(unpack('<2I', entry))
    # Check for definitely invalid entries:
    if fname == 'profifree.db':
        for size, start in free_ranges:
            if start % sizeof(SProfile_UserInfo) != 8:
                print(f'bad range start: 0x{start:08X}')
            if size % sizeof(SProfile_UserInfo):
                print(f'bad range size: 0x{size:08X}')
    # Sort ranges and merge adjacent entries:
    free_ranges.sort(key=lambda x: x[1])
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
    # Print result:
    print('Free ranges:')
    for size, start in result:
        print(f'[0x{start:08X}:0x{start + size:08X})')

def show_sessions(pathname):
    with open(pathname, 'rb') as inf:
        data = inf.read()
    with open(op.join(op.dirname(op.dirname(pathname)), 'profiles/profindex.db'), 'rb') as inf:
        profile_ids = {} # offset -> uid
        profile_names = {} # uid -> name
        inf.seek(8)
        while True:
            offset = inf.tell()
            buf = inf.read(sizeof(SProfile_UserInfo))
            if len(buf) == 0:
                prof_db_len = offset
                break
            prof = SProfile_UserInfo.from_buffer_copy(buf)
            profile_ids[offset] = prof.uniq_id
            profile_names[prof.uniq_id] = decode_str(prof.username)
    # array of SSavedAuthSeq entries
    assert len(data) % sizeof(SSavedAuthSeq) == 0
    for start in range(0, len(data), sizeof(SSavedAuthSeq)):
        entry = SSavedAuthSeq.from_buffer_copy(data, start)
        assert entry.uid & SEQUENCE_IP_CHECK_DISABLED in (0, SEQUENCE_IP_CHECK_DISABLED)
        uid = entry.uid & ~SEQUENCE_IP_CHECK_DISABLED
        ignore_ip = ' (ignored)' if (entry.uid & SEQUENCE_IP_CHECK_DISABLED) else ''
        ipaddr = decode_ip(entry.ip)
        assert entry.prof_index % sizeof(SProfile_UserInfo) == 8 and entry.prof_index < prof_db_len
        prof_uid = profile_ids[entry.prof_index]
        prof_name = profile_names.get(uid, '-')
        prof_check = f' (INVALID, prof_uid={prof_uid})' if prof_uid != uid else ''
        exp_dt = dt.datetime.utcfromtimestamp(entry.expiration)
        print(f'seq={entry.sid[0]:08x}{entry.sid[1]:08x}, uid={uid} ({prof_name}), ip={ipaddr}{ignore_ip}, prof=0x{entry.prof_index:08X}{prof_check}, exp={exp_dt:%Y-%m-%d %H:%M}')

def main(pathname):
    fname = op.basename(pathname)
    if fname == 'profindex.db':
        show_profiles(pathname)
    elif fname == 'profindex.idx':
        show_hashindex(pathname)
    elif fname == 'profcnicks.db':
        show_altnames(pathname)
    elif fname in ('freeindex.msg', 'freemess.msg', 'profbfree.db', 'profifree.db'):
        show_freedb(pathname)
    elif fname == 'authuser.dat':
        show_sessions(pathname)

if __name__ == '__main__':
    main(sys.argv[1])
