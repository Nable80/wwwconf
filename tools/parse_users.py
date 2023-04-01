#!/usr/bin/python3

from ctypes import (
    LittleEndianStructure,
    c_byte, c_char,
    c_int8, c_int16, c_int32,
    c_uint8, c_uint16, c_uint32,
    sizeof
)
import os.path as op
from struct import pack, unpack
import sys

"""
Custom DB consists of the following dirs and files:

data/internals/activitylog1.dat - TODO
data/internals/activitylog2.dat - TODO
data/internals/antispam.dat - TODO
data/internals/authuser.dat - TODO

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
data/profiles/profcnicks.db - TODO
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

class SActivityLogRecord(LittleEndianStructure):
    _pack_ = 4
    _fields_ = [
        ('ip', c_uint32),
        ('count', c_uint32),
        ('time', c_uint32),
    ]
assert sizeof(SActivityLogRecord) == 12

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

def main(pathname):
    assert op.basename(pathname) == 'profindex.db'
    with open(pathname, 'rb') as inf:
        user_data = inf.read()
    # 2 DWORDs, then array of SProfile_UserInfo entries
    assert len(user_data) % sizeof(SProfile_UserInfo) == 8

    last_uid, total_users = unpack('<2I', user_data[:8])
    num_entries = (len(user_data) - 8) // sizeof(SProfile_UserInfo)
    print(f'last_uid={last_uid}, total_users={total_users}, num_entries={num_entries}')
    for start in range(8, len(user_data), sizeof(SProfile_UserInfo)):
        user_info = SProfile_UserInfo.from_buffer_copy(user_data, start)
        print(f'0x{start:08X}', user_info.username.decode('cp1251', errors='replace'))

if __name__ == '__main__':
    main(sys.argv[1])
