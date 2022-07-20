/***************************************************************************
                          main.h  -  main module include
                             -------------------
    begin                : Wed Mar 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

// bit mask for proc making html header of board
#define HEADERSTRING_POST_NEW_MESSAGE           0x000001u
#define HEADERSTRING_CONFIGURE                  0x000002u
#define HEADERSTRING_RETURN_TO_MAIN_PAGE        0x000004u
#define HEADERSTRING_REFRESH_TO_MAIN_PAGE       0x000008u
#define HEADERSTRING_WELCOME_INFO               0x000010u
#define HEADERSTRING_SHOWBODY_JS                0x000020u
#define HEADERSTRING_DISABLE_LOGOFF             0x000040u
#define HEADERSTRING_DISABLE_SEARCH             0x000080u
#define HEADERSTRING_ENABLE_REGISTER            0x000100u
#define HEADERSTRING_DISABLE_LOGIN              0x000200u
#define HEADERSTRING_DISABLE_ALL                0x000400u
#define HEADERSTRING_REG_USER_LIST              0x000800u
#define HEADERSTRING_REFRESH_TO_THREAD          0x001000u
#define HEADERSTRING_ENABLE_TO_MESSAGE          0x002000u
#define HEADERSTRING_ENABLE_RESETNEW            0x004000u
#define HEADERSTRING_REDIRECT_NOW               0x008000u
#define HEADERSTRING_NO_CACHE_THIS              0x010000u
#define HEADERSTRING_DISABLE_PRIVATEMSG         0x020000u
#define HEADERSTRING_DISABLE_FAVOURITES         0x040000u
#define HEADERSTRING_DISABLE_END_TABLE          0x080000u
#define HEADERSTRING_ENABLE_REPLY_LINK          0x100000u
#define HEADERSTRING_POST_JS                    0x200000u

// button codes
#define ACTION_BUTTON_POST                      0x0001u
#define ACTION_BUTTON_PREVIEW                   0x0002u
#define ACTION_BUTTON_EDIT                      0x0004u
#define ACTION_BUTTON_FAKEREPLY                 0x0008u

#define MAX_DELTA_POST_MESSAGE                  15000

#define MAINPAGE_INDEX                          0xFFFFFFFF

#endif
