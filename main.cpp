/***************************************************************************
                          main.cpp  -  main module
                             -------------------
    begin                : Thu Mar 14 21:54:15 MSK 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru

 ***************************************************************************/

#include "basetypes.h"
#include "dbase.h"
#include "error.h"
#include "main.h"
#include "messages.h"
#include "speller.h"
#include "security.h"
#include "boardtags.h"
#include "profiles.h"
#include "logins.h"
#include "searcher.h"
#include "sendmail.h"
#include "announces.h"
#include "colornick.h"
#include "activitylog.h"
#include "statfs.h" 


char *ConfTitle;

const char *MESSAGEHEAD_timetypes[4] = {
        "час(а)",       // coded as "1"
        "день/дня",     // coded as "2"
        "неделю(и)",    // coded as "3"
        "месяц(а)"      // coded as "4"
};

unsigned LENGTH_timetypes_hours[4] = {
        1,
        24,
        168,
        720
};


#if TOPICS_SYSTEM_SUPPORT
const char *Topics_List[TOPICS_COUNT] = {
        "без темы",
        "Мурзилка",
        "Программирование",
        "Работа",
        "Учеба",
        "Обсуждение",
        "Куплю",
        "Движок борды",
        "Развлечения",
        "Продам",
        "Temp",
        "Услуги",
        "BSD/Linux",
        "Windows",
        "Проблемы сети",
        "Новости",
        "Голосование",
        "Потеряно/Найдено",
        "Спорт"
};
DWORD Topics_List_map[TOPICS_COUNT] = {
        0, //"без темы",
        4, //"Учеба",
        3, //"Работа",
        1, //"Мурзилка",
        5, //"Обсуждение",
        15,//"Новости"
        18, //"Спорт"
        8, //"Развлечения",
        7, //"Движок борды",
        2, //"Программирование",
        6, //"Куплю",
        9, //"Продам",
        11,//"Услуги",
        13,//"Windows",
        12,//"BSD/Linux",
        14,//"Проблемы сети",
        16,//"Голосование",
        17,//"lost/found",
        10,//"Temp",
};
#endif

const char *UserStatus_List[USER_STATUS_COUNT] = {
        "Новичок",
        "Дурачок",
        "Эксперт",
        "Гуру",
        "Флудер"
};

const char *UserRight_List[USERRIGHT_COUNT] = {
        "SUPERUSER",
        "VIEW_MESSAGE",
        "MODIFY_MESSAGE",
        "CLOSE_MESSAGE",
        "OPEN_MESSAGE",
        "CREATE_MESSAGE",
        "CREATE_MESSAGE_THREAD",
        "ALLOW_HTML",
        "PROFILE_MODIFY",
        "PROFILE_CREATE",
        "ROLL_MESSAGE",
        "UNROLL_MESSAGE",
        "POST_GLOBAL_ANNOUNCE",
        "ALT_DISPLAY_NAME"
};

int GlobalNewSession = 1;

enum {
        ACTION_POST,
        ACTION_PREVIEW,
        ACTION_EDIT
};

char* strget(char *par,const char *find, WORD maxl, char end, bool argparsing = true);

int getAction(char* par)
{
#define JS_POST_FIELD "jpost"
        int i = -1;
        const char* szActions[] = {
                "post",
                "preview",
                "edit"
        };
        int cActions = sizeof(szActions) / sizeof(char*);
        char* st = strget(par, JS_POST_FIELD"=", MAX_STRING, '&');
        if(st && *st) {
                for(i = 0; i < cActions; i++)
                        if(!strcmp(st, szActions[i]))
                                break;
        } else {
                char buf[MAX_STRING];
                for(i = 0; i < cActions; i++)
                {
                        sprintf(buf, "%s=", szActions[i]);
                        st = strget(par, buf, MAX_STRING, '&');
                        if(st && *st)
                                break;
                }
        }
        if(st)
                free(st);
        if(i < cActions)
                return i;
        return -1;
}

static void HttpRedirect(char *url)
{
        printf("Status: 302 Moved\n");
        printf("Pragma: no-cache\n");
        printf("Location: %s\n\n",url);
}

static void PrintBoardError(const char *s1, const char *s2, int code, DWORD msgid = 0)
{
        printf(DESIGN_GLOBAL_BOARD_MESSAGE, s1, s2);
        if(code & HEADERSTRING_REFRESH_TO_MAIN_PAGE)
                printf("%s", MESSAGEMAIN_browser_return);
        if(code & HEADERSTRING_REFRESH_TO_THREAD && msgid != 0)
                printf(MESSAGEMAIN_browser_to_thread, msgid);
}

static void PrintBanList()
{
        FILE *f;
        size_t readed;
        void *buf = malloc(MAX_HTML_FILE_SIZE);
        if((f = fopen(F_BANNEDIP, FILE_ACCESS_MODES_R)) == NULL) printhtmlerror();
        if((readed = fread(buf, 1, MAX_HTML_FILE_SIZE, f)) == 0) printhtmlerror();
        printf(DESIGN_BAN_FORM);
        if(fwrite(buf, 1, readed, stdout) != readed) printhtmlerror();
        printf(DESIGN_BAN_FORM2);
        free(buf);
        fclose(f);
}

/* Prints post message form
 */
static void PrintMessageForm(SMessage *msg, char *body, DWORD s, int code, DWORD flags = 0)
{
        char tstr[2][100];

        printf("<CENTER><FORM METHOD=POST NAME=\"postform\" onSubmit=\"return false;\" ACTION=\"%s?xpost=%lu\">",
                        MY_CGI_URL, s);

        printf(DESIGN_POST_NEW_MESSAGE_TABLE "<TR><TD COLSPAN=2 ALIGN=CENTER><BIG>");
        if(code & ACTION_BUTTON_EDIT) printf(MESSAGEMAIN_post_editmessage);
        else if(code & ACTION_BUTTON_FAKEREPLY) printf("<A NAME=Reply>" MESSAGEMAIN_post_replymessage "</A>");
        else printf(MESSAGEMAIN_post_newmessage);

        printf("</BIG></TD></TR>");
        printf("<TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=\"80%%\" NOSHADE></TR>");

        if ((currentdsm & CONFIGURE_bot) == 0 && ULogin.LU.ID[0] == 0) {
                printf("<TR class=\"nd\"><TD COLSPAN=2 ALIGN=CENTER><b>" MESSAGEMAIN_post_bot "</b></TR>");
                printf("<TR class=\"nd\"><TD ALIGN=CENTER><b>Name:</b></TD>"
                       "<TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"name\" SIZE=22 MAXLENGTH=%d>&nbsp;&nbsp;&nbsp;"
                       "<b>E-mail: </b><input type=\"TEXT\" name=\"email\" size=\"35\" maxlength=\"%d\">"
                       "</TD></TR>",
                       AUTHOR_NAME_LENGTH - 1, PROFILES_FULL_USERINFO_MAX_EMAIL - 1);
        }

        if(!(code & ACTION_BUTTON_EDIT)) {
                if(ULogin.LU.ID[0] == 0) {
                        int ti = msg->AuthorName[0] ? 3 : 1;
                        // print name/password form
                        printf("<TR><TD ALIGN=CENTER>%s</TD><TD ALIGN=LEFT>"\
                               "<INPUT TYPE=TEXT NAME=\"amen\" SIZE=22 MAXLENGTH=%d VALUE=\"%s\" tabindex=\"%d\">",
                               MESSAGEMAIN_post_you_name, AUTHOR_NAME_LENGTH - 1, msg->AuthorName, ti);
                        // print password
                        printf("&nbsp;&nbsp;&nbsp;%s <INPUT TYPE=PASSWORD NAME=\"pswd\" SIZE=22 MAXLENGTH=%d VALUE=\"\" tabindex=\"%d\">"\
                               "&nbsp;&nbsp;&nbsp;%s <INPUT TYPE=CHECKBOX NAME=\"lmi\" tabindex=\"%d\"></TD></TR>\n",
                               MESSAGEMAIN_post_your_password, PROFILES_MAX_PASSWORD_LENGTH - 1, ti, MESSAGEMAIN_post_login_me, ti);
                }
                else {
                        // print name
                        char *aname = FilterBiDi(msg->AuthorName);
                        printf("<TR><TD ALIGN=CENTER>%s</TD><TD ALIGN=LEFT>"\
                                "<B>%s</B>&nbsp;&nbsp;&nbsp;<A HREF=\"%s?login=logoff\"><SMALL>[%s] </SMALL></A></TD></TR>\n",
                                MESSAGEMAIN_post_you_name, aname, MY_CGI_URL, MESSAGEHEAD_logoff);
                        if (aname)
                                free(aname);
                }
        }
        else {
                // print edit name and ip
                if(ULogin.LU.ID[0] != 0 && (ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) {
                        printf("<TR><TD ALIGN=CENTER>%s &nbsp;" \
                                "<INPUT TYPE=TEXT NAME=\"name\" SIZE=29 MAXLENGTH=%d VALUE=\"%s\" tabindex=\"3\"></TD>" \
                                "<TD>%s &nbsp;<INPUT TYPE=TEXT NAME=\"host\" SIZE=40 MAXLENGTH=%d" \
                                " VALUE=\"%s\"></TD></TR>", MESSAGEMAIN_post_you_name, AUTHOR_NAME_LENGTH - 1,
                                msg->AuthorName, MESSAGEMAIN_post_hostname, HOST_NAME_LENGTH - 1, msg->HostName);
                }
                
        }


        printf("<TR><TD ALIGN=CENTER>%s</TD><TD ALIGN=LEFT>", MESSAGEMAIN_post_message_subject);
        
#if TOPICS_SYSTEM_SUPPORT
        // subject and topic
        if(s == 0) {
                // Only for ROOT messages
                printf("<SELECT NAME=\"topic\" tabindex=\"1\">");
                for(DWORD i = 0; i < TOPICS_COUNT; i++) {
                        if(Topics_List_map[i] == msg->Topics) {
                                // define default choise
                                printf("<OPTION VALUE=\"%lu\"" LISTBOX_SELECTED ">%s\n",
                                        Topics_List_map[i], Topics_List[Topics_List_map[i]]);
                        }
                        else {
                                printf("<OPTION VALUE=\"%lu\">%s\n", Topics_List_map[i], Topics_List[Topics_List_map[i]]);
                        }
                }
                printf("</SELECT>");
        }

        printf("<INPUT TYPE=TEXT NAME=\"subject\" SIZE=%d MAXLENGTH=%d VALUE=\"%s\" tabindex=\"1\""
	       "onfocus=\"last = document.postform.subject;\"></TD></TR>\n", s ? 88 : 62, MESSAGE_HEADER_LENGTH - 1, msg->MessageHeader);
#else
        printf("<INPUT TYPE=TEXT NAME=\"subject\" SIZE=88 MAXLENGTH=%d VALUE=\"%s\" tabindex=\"1\""
	       "onfocus=\"last = document.postform.subject;\"></TD></TR>\n", MESSAGE_HEADER_LENGTH - 1, msg->MessageHeader);
#endif                         

        DESIGN_STYLE_BUTTONS_BEGIN()
                DESIGN_STYLE_BUTTONS_ADD_WRAP("b", "30px", "b", "жирный текст: [b]текст[/b] (alt+b)", "[b]", "[/b]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("i", "30px", "i", "курсивный текст: [i]текст[/i] (alt+i)", "[i]", "[/i]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("u", "30px", "u", "подчеркнутый текст: [u]текст[/u] (alt+u)", "[u]", "[/u]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("q", "30px", "q", "цитата: [q]текст[/q] (alt+q)", "[q]", "[/q]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("p", "40px", "pic", "изображение: [pic]http://ссылка[/pic] (alt+p)", "[pic]", "[/pic]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("w", "40px", "url", "ссылка: [url=http://ссылка]название[/url] (alt+w)", "[url=", "]ссылка[/url]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("h", "30px", "h", "заголовок: [h]текст[/h] (alt+h)", "[h]", "[/h]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("s", "30px", "s", "мелкий текст: [s]текст[/s] (alt+s)", "[s]", "[/s]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("6", "40px", "sup", "верхний индекс: [sup]текст[/sup] (alt+6)", "[sup]", "[/sup]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("-", "40px", "sub", "нижний индекс: [sub]текст[/sub] (alt+-)", "[sub]", "[/sub]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("=", "55px", "strike", "перечеркнутый текст: [strike]текст[/strike] (alt+=)", "[strike]", "[/strike]", 1)
                DESIGN_STYLE_BUTTONS_NEWLINE()
                DESIGN_STYLE_BUTTONS_ADD_WRAP("3", "55px", "color", "цветной текст: [color=#цвет]текст[/color] (alt+3)", "[color=#00FF00]", "[/color]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("r", "40px", "red", "красный текст: [red]текст[/red] (alt+r)", "[red]", "[/red]", 1)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("f", "40px", "pre", "преформатированный текст: [pre]текст[/pre] (alt+f)", "[pre]", "[/pre]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("c", "60px", "center", "центрированный текст: [center]текст[/center] (alt+c)", "[center]", "[/center]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("t", "40px", "tex", "TEX-формула: [tex]текст[/tex] (alt+t)", "[tex]", "[/tex]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP("y", "40px", "tub", "YouTube-видео: [tub]идентификатор видео[/tub] (alt+y)", "[tub]", "[/tub]", 0)
                DESIGN_STYLE_BUTTONS_ADD_WRAP(".", "65px", "spoiler", "спойлер: [spoiler]текст[/spoiler] (alt+.)", "[spoiler]", "[/spoiler]", 1)
                DESIGN_STYLE_BUTTONS_ADD_INSERT("l", "35px", "hr", "горизонтальная линия: [hr] (alt+l)", "[hr]", 0)
                DESIGN_STYLE_BUTTONS_ADD_SHOWSMILE()
                DESIGN_STYLE_SMILES_BEGIN()
                        DESIGN_STYLE_SMILES_ADD("smile.gif", ":)")
                        DESIGN_STYLE_SMILES_ADD("bigsmile.gif", ":))")
                        DESIGN_STYLE_SMILES_ADD("frown.gif", ":(")
                        DESIGN_STYLE_SMILES_NEWLINE()
                        DESIGN_STYLE_SMILES_ADD("wink.gif", ";)")
                        DESIGN_STYLE_SMILES_ADD("lol.gif", ":!!")
                        DESIGN_STYLE_SMILES_ADD("smirk.gif", ":\\\\")
                        DESIGN_STYLE_SMILES_NEWLINE()
                        DESIGN_STYLE_SMILES_ADD("smoke.gif", ":SMOKE")
                        DESIGN_STYLE_SMILES_ADD("no.gif", ":NO")
                        DESIGN_STYLE_SMILES_ADD("yes.gif", ":YES")
                        DESIGN_STYLE_SMILES_NEWLINE()
                        DESIGN_STYLE_SMILES_ADD("bored.gif", ":BORED")
                        DESIGN_STYLE_SMILES_ADD("crazy.gif", ":CRAZY")
                        DESIGN_STYLE_SMILES_ADD("mad.gif", ":MAD")
                        DESIGN_STYLE_SMILES_NEWLINE()
                        DESIGN_STYLE_SMILES_ADD("draznit.gif", ":DRAZNIT")
                        DESIGN_STYLE_SMILES_ADD("redface.gif", ":o")
                        DESIGN_STYLE_SMILES_ADD("rolleyes.gif", ":ROLLEYES")
                        DESIGN_STYLE_SMILES_NEWLINE()
                        DESIGN_STYLE_SMILES_ADD("figa.gif", ":FIGA")
                        DESIGN_STYLE_SMILES_ADD("devil.gif", ":DEVIL")
                        DESIGN_STYLE_SMILES_ADD("iq.gif", ":CIQ")
                DESIGN_STYLE_SMILES_END()
        DESIGN_STYLE_BUTTONS_END();
        

        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><TEXTAREA COLS=75 ROWS=12 NAME=\"body\" "
	       "CLASS=\"post\" tabindex=\"1\" onfocus=\"last = document.postform.body;\">%s</TEXTAREA></TD></TR>", body);

        tstr[0][0] = tstr[1][0] = 0;
        if(flags & MSG_CHK_DISABLE_SMILE_CODES) strcpy(tstr[0], RADIO_CHECKED);
        if(flags & MSG_CHK_DISABLE_WWWCONF_TAGS) strcpy(tstr[1], RADIO_CHECKED);

        printf("<TR><TD COLSPAN=2 ALIGN=RIGHT class=cl>"
               "%s<INPUT TYPE=CHECKBOX NAME=\"dct\"%s class=cl><BR />"
               "%s<INPUT TYPE=CHECKBOX NAME=\"dst\"%s class=cl>",
               MESSAGEMAIN_post_disable_wwwconf_tags, tstr[1],
               MESSAGEMAIN_post_disable_smile_tags, tstr[0]);
                                                  
        if(ULogin.LU.ID != 0) {
                tstr[0][0] = 0;
                if(flags & MSG_CHK_ENABLE_EMAIL_ACKNL) strcpy(tstr[0], RADIO_CHECKED);
                printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"wen\"%s class=cl>",
                         MESSAGEMAIN_post_reply_acknl, tstr[0]);
        }

        printf("</TD></TR><TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=\"80%%\" NOSHADE></TR>");
                                                                 
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER>");

        printf(SCRIPT_FORM_MESSAGE_BUTTON);
        printf(SCRIPT_FORM_MESSAGE_QEDIT);

        if(code & ACTION_BUTTON_EDIT) {
                printf(DESIGN_FORM_MESSAGE_BUTTON, "edit", MESSAGEMAIN_post_edit_message, 1);
        }
        else {
                if(code & ACTION_BUTTON_PREVIEW) {
                        printf(DESIGN_FORM_MESSAGE_BUTTON, "preview",MESSAGEMAIN_post_preview_message, 2);
                }
                if(code & ACTION_BUTTON_POST) {
                        printf(DESIGN_FORM_MESSAGE_BUTTON, "post", MESSAGEMAIN_post_post_message, 1);
                }
        }
        printf("<INPUT TYPE=HIDDEN NAME=\"jpost\" VALUE=\"%s\">", "");

        printf("</TD></TR></TABLE></FORM></CENTER><P>&nbsp;");
}

static void PrintLoginForm()
{
        printf("<P><FORM METHOD=POST ACTION=\"%s?login=action\">", MY_CGI_URL);
        
        printf(DESIGN_BEGIN_LOGIN_OPEN);

        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD><INPUT TYPE=TEXT NAME=\"mname\" SIZE=20 " \
               "MAXLENGTH=%d VALUE=\"%s\"></TD></TR><TR><TD ALIGN=RIGHT>%s</TD><TD>" \
               "<INPUT TYPE=PASSWORD NAME=\"mpswd\" SIZE=20 MAXLENGTH=30 VALUE=\"\"></TD></TR>",
                MESSAGEMAIN_login_loginname, AUTHOR_NAME_LENGTH - 1,
                FilterHTMLTags(cookie_name, 1000, 0), MESSAGEMAIN_login_password);

        printf("<TR><TD COLSPAN=2><CENTER><INPUT TYPE=CHECKBOX NAME=\"ipoff\" VALUE=1>" \
                MESSAGEMAIN_login_ipcheck "</CENTER></TD></TR>");
        
        printf("<P><TR><TD COLSPAN=2 ALIGN=CENTER><INPUT TYPE=SUBMIT VALUE=\"Enter\"></TD></TR>" DESIGN_END_LOGIN_CLOSE "</FORM>");

        printf(MESSAGEMAIN_login_lostpassw);
}

static void PrintPrivateMessageForm(char *name, const char *body)
{
        printf("<CENTER><FORM METHOD=POST ACTION=\"%s?persmsgpost\"><P>&nbsp;<P>", MY_CGI_URL);

        printf(DESIGN_POST_NEW_MESSAGE_TABLE "<TH COLSPAN=2><BIG>" MESSAGEMAIN_privatemsg_send_msg_hdr
                "</BIG></TR><TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=80%% NOSHADE></TR>");
        printf("<TR><TH ALIGN=RIGHT>%s </TH><TD>"
                "<INPUT TYPE=TEXT NAME=\"name\" SIZE=30 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>"
                "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG>", 
                MESSAGEMAIN_privatemsg_send_msg_usr, PROFILES_MAX_USERNAME_LENGTH - 1,
                name, MESSAGEMAIN_privatemsg_send_msg_bdy);

        printf("<BR><TEXTAREA COLS=50 ROWS=7 NAME=\"body\" WRAP=VIRTUAL>%s</TEXTAREA></TD></TR>", body);

        printf("<TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=80%% NOSHADE></TR><BR><TR><TD COLSPAN=2 ALIGN=CENTER>"
                "<INPUT TYPE=SUBMIT NAME=\"Post\" VALUE=\"%s\">&nbsp;<INPUT TYPE=SUBMIT NAME=\"Post\" VALUE=\"%s\"></TD></TR></TABLE></FORM></CENTER>",
                MESSAGEMAIN_privatemsg_prev_msg_btn, MESSAGEMAIN_privatemsg_send_msg_btn);
}

static void PrintAnnounceForm(const char *body, int ChangeAnn = 0)
{
        printf("<CENTER><FORM METHOD=POST ACTION=\"%s?globann=post\"><P>&nbsp;<P>", MY_CGI_URL);

        printf(DESIGN_POST_NEW_MESSAGE_TABLE "<TH COLSPAN=2><BIG>%s</BIG></TR><TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=80%% NOSHADE></TR>",
                ChangeAnn ? MESSAGEMAIN_globann_upd_ann_hdr : MESSAGEMAIN_globann_send_ann_hdr);

        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><BR>"
                "<TEXTAREA COLS=50 ROWS=7 NAME=\"body\" WRAP=VIRTUAL>%s</TEXTAREA></TD></TR>",
                        MESSAGEMAIN_globann_send_ann_body, body);

        if(ChangeAnn) printf("<TR><TD COLSPAN=2><CENTER><INPUT TYPE=CHECKBOX NAME=\"refid\" VALUE=1>" \
                MESSAGEMAIN_globann_upd_ann_id "</CENTER></TD></TR>");
        printf("<TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=80%% NOSHADE></TR><BR><TR><TD COLSPAN=2 ALIGN=CENTER>");
        if(ChangeAnn) printf("<INPUT TYPE=HIDDEN NAME=\"cgann\" VALUE=\"%d\">", ChangeAnn);
        printf("<INPUT TYPE=SUBMIT NAME=\"Post\" VALUE=\"%s\"><INPUT TYPE=SUBMIT NAME=\"Post\" VALUE=\"%s\"></TD></TR></TABLE></FORM></CENTER>",
                MESSAGEMAIN_globann_prev_ann_btn, MESSAGEMAIN_globann_send_ann_btn);
}

static void PrintLostPasswordForm()
{
        printf("<P><FORM METHOD=POST ACTION=\"%s?login=lostpasswaction\">", MY_CGI_URL);

        printf(DESIGN_BEGIN_LOSTPASSW_OPEN);

        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD><INPUT TYPE=TEXT NAME=\"mname\" SIZE=20 MAXLENGTH=%d "
                "VALUE=\"%s\"></TD></TR><TR><TD ALIGN=RIGHT>%s</TD><TD><INPUT TYPE=TEXT NAME=\"memail\" SIZE=20 "
                "MAXLENGTH=%d VALUE=\"\"></TD></TR>", MESSAGEMAIN_lostpassw_loginname, AUTHOR_NAME_LENGTH - 1,
                FilterHTMLTags(cookie_name, 1000, 0), MESSAGEMAIN_lostpassw_email, PROFILES_FULL_USERINFO_MAX_EMAIL - 1);

        printf("<P><TR><TD COLSPAN=2 ALIGN=CENTER><INPUT TYPE=SUBMIT VALUE=\"" MESSAGEMAIN_lostpassw_getpassw "\"><CENTER></TD></TR>" DESIGN_END_LOSTPASSW_CLOSE "</FORM>");
}

/* print configuration form */
static void PrintConfig()        
{
        char str1[20], str2[20], str3[20], str4[20], str5[20], str6[20], str7[20], str8[20], str9[20];
        int i;

        printf("<TABLE align=center width=\"100%%\"><tr><td><FORM METHOD=POST ACTION=\"%s?configure=action\" name=\"configure\">",
                MY_CGI_URL);
        
        printf("<CENTER><P><B>%s</B><BR><P>", MESSAGEHEAD_configure);
        
        printf("<BR><BR>%s<BR>", MESSAGEHEAD_configure_showmsgs);

        str1[0] = str2[0] = str3[0] = str4[0] = str5[0] = 0;
        if(currentlsel == 1) strcpy(str1, RADIO_CHECKED);

        switch(currenttt) {
        case 1:
                strcpy(str2, LISTBOX_SELECTED);
                break;
        case 2:
                strcpy(str3, LISTBOX_SELECTED);
                break;
        case 3:
                strcpy(str4, LISTBOX_SELECTED);
                break;
        case 4:
                strcpy(str5, LISTBOX_SELECTED);
                break;
        }
        printf("<INPUT TYPE=RADIO NAME=lsel VALUE=1 %s>%s" \
                "<INPUT TYPE=TEXT NAME=\"tv\" SIZE=2 VALUE=%lu><SELECT NAME=\"tt\">" \
                "<OPTION VALUE=\"1\"%s>%s<OPTION VALUE=\"2\"%s>%s<OPTION VALUE=\"3\" %s>" \
                "%s<OPTION VALUE=\"4\"%s>%s</SELECT><BR>",
                str1, MESSAGEHEAD_configure_msgslast, currenttv,
                str2, MESSAGEHEAD_timetypes[0], str3, MESSAGEHEAD_timetypes[1], str4,
                MESSAGEHEAD_timetypes[2], str5, MESSAGEHEAD_timetypes[3]
        );

        str1[0] = str2[0] = str3[0] = str4[0] = str5[0] = 0;
        if(currentlsel == 2) strcpy(str1, RADIO_CHECKED);

        switch(currentss) {
        case 1:
                strcpy(str2, LISTBOX_SELECTED);
                break;
        case 2:
                strcpy(str3, LISTBOX_SELECTED);
                break;
        case 3:
                strcpy(str4, LISTBOX_SELECTED);
                break;
        case 4:
                strcpy(str5, LISTBOX_SELECTED);
                break;
        }

        printf("<INPUT TYPE=RADIO NAME=lsel VALUE=2 %s>%s<INPUT TYPE=TEXT NAME=\"tc\" SIZE=3 VALUE=%lu>"
                "<BR><BR>%s<BR><SELECT NAME=\"ss\"><OPTION VALUE=\"2\" %s>%s<OPTION VALUE=\"3\" %s>%s<OPTION VALUE=\"4\"%s>"
                "%s</SELECT>",
                str1, MESSAGEHEAD_configure_lastnum, currenttc, 
                MESSAGEHEAD_configure_showstyle, str3,
                MESSAGEHEAD_configure_showhronbackward, str4,
                MESSAGEHEAD_configure_showhronwothreads, str5,
                MESSAGEHEAD_configure_showhrononlyheaders);

        printf("<BR><BR>Часовой пояс:<BR> <SELECT NAME=\"tz\">");
        for(i = -12; i <= 12; i++)
                printf("<OPTION VALUE=\"%d\" %s>GMT%s%02d", i,
                (i == currenttz) ? LISTBOX_SELECTED : "", (i>=0) ? "+" : "-", (i>0)? i : -i);
        printf("</SELECT><BR>");


#if TOPICS_SYSTEM_SUPPORT
        // use str4 for temporaty buffer
        printf("<BR><P><TABLE BORDER=1 CELLSPACING=0 CELLPADDING=6 BGCOLOR=\"#BBAAAA\">\
<TR><TD ALIGN=RIGHT>" DESIGN_CONFIGURE_CHECKALL "</TD></TR><TR><TD ALIGN=RIGHT>");
        for(DWORD i = 0; i < TOPICS_COUNT; i++)
        {
                if(currenttopics & (1<<Topics_List_map[i]))
                        strcpy(str4, RADIO_CHECKED);
                else str4[0] = 0;
                printf("%s <INPUT TYPE=CHECKBOX NAME=\"topic%lu\" %s><br>\n",
                        Topics_List[Topics_List_map[i]], Topics_List_map[i], str4);
        }
        printf("</TD></TR></TABLE>");
#endif

        if((currentdsm & CONFIGURE_dsm) != 0) strcpy(str3, RADIO_CHECKED);
        else str3[0] = 0;
        if((currentdsm & CONFIGURE_onh) != 0) strcpy(str2, RADIO_CHECKED);
        else str2[0] = 0;
        if((currentdsm & CONFIGURE_plu) != 0) strcpy(str1, RADIO_CHECKED);
        else str1[0] = 0;
        if((currentdsm & CONFIGURE_host) != 0) strcpy(str6, RADIO_CHECKED);
        else str6[0] = 0;
        if((currentdsm & CONFIGURE_nalt) != 0) strcpy(str7, RADIO_CHECKED);
        else str7[0] = 0;
        if((currentdsm & CONFIGURE_dsig) != 0) strcpy(str8, RADIO_CHECKED);
        else str8[0] = 0;
        if((currentdsm & CONFIGURE_shrp) != 0) strcpy(str9, RADIO_CHECKED);
        else str9[0] = 0;
        if((currentdsm & CONFIGURE_clr) != 0) strcpy(str4, RADIO_CHECKED);
        else str4[0] = 0;
        if((currentdsm & CONFIGURE_bot) != 0) strcpy(str5, RADIO_CHECKED);
        else str5[0] = 0;

        printf("<TABLE><TR><TD ALIGN=RIGHT>%s<INPUT TYPE=CHECKBOX NAME=\"dsm\" VALUE=1 %s>",
                MESSAGEHEAD_configure_disablesmiles, str3);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"onh\" VALUE=1 %s>",
                MESSAGEHEAD_configure_ownpostshighlight, str2);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"host\" VALUE=1 %s>",
                MESSAGEHEAD_configure_showhostnames, str6);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"nalt\" VALUE=1 %s>",
                MESSAGEHEAD_configure_showaltnames, str7);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"dsig\" VALUE=1 %s>",
                MESSAGEHEAD_configure_showsign, str8);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"shrp\" VALUE=1 %s>",
                MESSAGEHEAD_configure_showreplyform, str9);
#if ALLOW_MARK_NEW_MESSAGES == 2
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"plu\" VALUE=1 %s>",
                MESSAGEHEAD_configure_plus_is_href, str1);
#endif
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"clr\" VALUE=1 %s>",
                MESSAGEHEAD_configure_disablecolor, str4);
        printf("<BR>%s<INPUT TYPE=CHECKBOX NAME=\"bot\" VALUE=1 %s>",
                MESSAGEHEAD_configure_disablebot, str5);
        printf("</TD></TR></TABLE>");

        if(ULogin.LU.ID[0] && (ULogin.pui->Flags & PROFILES_FLAG_VIEW_SETTINGS) )
                        printf("<P>" MESSAGEHEAD_configure_saving_to_profile "<BR>");
        else printf("<P>" MESSAGEHEAD_configure_saving_to_browser "<BR>");
        if(ULogin.LU.ID[0]) printf(MESSAGEHEAD_configure_view_saving "<BR>");
        
        printf("<P><INPUT TYPE=SUBMIT VALUE=\"%s\"></CENTER></FORM></td></tr></table>",
                MESSAGEHEAD_configure_applysettings);
        
}

void PrintTopString(DWORD c, DWORD ind, DWORD ret)
{
        /* print init code */
        printf(DESIGN_COMMAND_TABLE_BEGIN);
        
        if(c & HEADERSTRING_ENABLE_TO_MESSAGE) {
                printf("<A HREF=\"#%ld\">%s</A>", ret, MESSAGEHEAD_to_message);

        }

        if((c & HEADERSTRING_ENABLE_TO_MESSAGE) || (c & HEADERSTRING_ENABLE_REPLY_LINK)) {
                if((c & HEADERSTRING_ENABLE_REPLY_LINK))
                        printf("<A HREF=\"%s?form=%ld\">%s</A>",
                                MY_CGI_URL, ret, MESSAGEMAIN_post_replymessage);
                else printf("<A HREF=\"#Reply\">%s</A>",
                                MESSAGEMAIN_post_replymessage);

        }

        if(c & HEADERSTRING_RETURN_TO_MAIN_PAGE) {
                if(ind == MAINPAGE_INDEX || ind == 0) printf("<A HREF=\"%s?index\">%s</A>",
                        MY_CGI_URL, MESSAGEHEAD_return_to_main_page);
                else printf("<A HREF=\"%s?index#%ld\" style=\"color:red;\">%s</A>",
                        MY_CGI_URL, ind, MESSAGEHEAD_return_to_main_page);

        }
        
        if(c & HEADERSTRING_POST_NEW_MESSAGE) {
                printf("<A HREF=\"%s?form\">%s</A>", MY_CGI_URL, MESSAGEHEAD_post_new_message);
        }

        if((c & HEADERSTRING_DISABLE_SEARCH) == 0) {
                 printf("<a HREF=\"http://zlo.rt.mipt.ru:7500/search?site=9\">%s</a>", MESSAGEHEAD_search);
        }
        
        if(c & HEADERSTRING_CONFIGURE) {
                printf("<A HREF=\"%s?configure=form\">%s</A>", MY_CGI_URL, MESSAGEHEAD_configure);
        }

        if(c & HEADERSTRING_ENABLE_RESETNEW) {
        //        printf("<A HREF=\"%s?resetnew\" onclick=\"run_resetnew(); return false;\">%s</A>", MY_CGI_URL, MESSAGEHEAD_resetnew);
                printf("<A HREF=\"%s?resetnew\" >%s</A>", MY_CGI_URL, MESSAGEHEAD_resetnew);
        }

        if((c & HEADERSTRING_DISABLE_REGISTER) == 0) {
                if(ULogin.LU.ID[0] == 0)
                        printf("<A HREF=\"%s?register=form\">%s</A>", MY_CGI_URL, MESSAGEHEAD_register);
        }

        if(c & HEADERSTRING_REG_USER_LIST) {
                printf("<A HREF=\"%s?userlist\">%s</A>", MY_CGI_URL, MESSAGEHEAD_userlist);
        }
        
        if( (ULogin.LU.right & USERRIGHT_SUPERUSER) != 0  && HEADERSTRING_REG_USER_LIST) {
                printf("<A HREF=\"%s?banlist\">%s</a>", MY_CGI_URL, MESSAGEHEAD_banlist);
        }

        if((ULogin.LU.ID[0] == 0) && ((c & HEADERSTRING_DISABLE_LOGIN) == 0)) {
                printf("<A HREF=\"%s?login=form\">%s</A>", MY_CGI_URL, MESSAGEHEAD_login);
        }

#if USER_FAVOURITES_SUPPORT
        if(ULogin.LU.ID[0] != 0 && (c & HEADERSTRING_DISABLE_FAVOURITES) == 0) {
                printf("<A HREF=\"%s?favs\">%s</A>",
                                MY_CGI_URL, MESSAGEHEAD_favourites);
        }
#endif

        if(ULogin.LU.ID[0] != 0 && (c & HEADERSTRING_DISABLE_PRIVATEMSG) == 0) {
                if(ULogin.pui->persmescnt - ULogin.pui->readpersmescnt > 0)
                        printf("<A HREF=\"%s?persmsg\"><FONT COLOR=RED><B>%s(%d)</B></FONT></A>",
                                MY_CGI_URL, MESSAGEHEAD_personalmsg, ULogin.pui->persmescnt - ULogin.pui->readpersmescnt);
                else
                        printf("<A HREF=\"%s?persmsg\">%s</A>", MY_CGI_URL, MESSAGEHEAD_personalmsg);
        }

        if((ULogin.LU.right & USERRIGHT_POST_GLOBAL_ANNOUNCE) != 0 && (c & HEADERSTRING_POST_NEW_MESSAGE) != 0) {
                printf("<A HREF=\"%s?globann=form\">%s</A>", MY_CGI_URL, MESSAGEHEAD_makeannounce);
        }
        
        if((ULogin.LU.ID[0] != 0) && ((c & HEADERSTRING_DISABLE_LOGOFF) == 0)) {
                printf("<A HREF=\"%s?login=logoff\">%s</A>", MY_CGI_URL, MESSAGEHEAD_logoff);
        }

        printf(DESIGN_COMMAND_TABLE_END);
}

/* print HTML header of file, header placed in topbanner.html */
void PrintHTMLHeader(DWORD code, DWORD curind, DWORD retind = 0)
{
        if(!HPrinted) {
                printf("Content-type: text/html\n");
                HPrinted = 1; // header have been printed
        }


        if((code & HEADERSTRING_NO_CACHE_THIS) != 0) printf("Cache-Control: no-cache\nPragma: no-cache\n");

        printf("Set-Cookie: " COOKIE_NAME_STRING "name=%s|lsel=%lu|tc=%lu|tt=%lu|tv=%lu|ss=%lu|" \
                "lm=%ld|fm=%ld|lt=%ld|ft=%ld|dsm=%lu|seq=%08lx%08lx|topics=%lu|lann=%lu|tovr=%lu|tz=%ld&;" \
                " expires=" COOKIE_EXPIRATION_DATE "path=" COOKIE_SERVER_PATH
                "\nSet-Cookie: " COOKIE_SESSION_NAME "on&; path=" COOKIE_SERVER_PATH "\n\n", 
                CodeHttpString(cookie_name, 0), cookie_lsel, cookie_tc, cookie_tt, cookie_tv,
                cookie_ss, currentlm, currentfm, currentlt, currentft, cookie_dsm, ULogin.LU.ID[0], ULogin.LU.ID[1],
                cookie_topics, currentlann, topicsoverride,cookie_tz);

        printf(HTML_START);

        if((code & HEADERSTRING_REDIRECT_NOW)) {
                printf("<meta http-equiv=\"Refresh\" content=\"0; url=%s?index\"></meta></head></html>", MY_CGI_URL);
                return;
        }

        if(code & HEADERSTRING_REFRESH_TO_MAIN_PAGE) {
                if(curind == MAINPAGE_INDEX || curind == 0)
                        printf("<meta http-equiv=\"Refresh\" content=\"%d; url=%s?index\">",
                        AUTO_REFRESH_TIME, MY_CGI_URL);
                else
                        printf("<meta http-equiv=\"Refresh\" content=\"%d; url=%s?index#%ld\">",
                        AUTO_REFRESH_TIME, MY_CGI_URL, curind);
        }

        // print output encoding (charset)
        printf(HTML_ENCODING_HEADER, GetBoardUrl());

        // print title
#if STABLE_TITLE == 0
        printf("<title>%s</title>", ConfTitle);
#else
        printf("<title>%s</title>", TITLE_StaticTitle);
#endif

        printf(HTML_STYLE_HEADER);

        /* print top string (navigation) */
        if((HEADERSTRING_DISABLE_ALL & code) == 0)
                PrintTopString(code, curind, retind);
}

/* print bottom lines from file (banners, etc.) */
void PrintBottomLines()
{
#if USE_HTML_BOTTOMBANNER
        printf(HTML_BOTTOMBANNER);
#endif
        printf(HTML_END);
}

/* print moderation toolbar and keys
 * 
 */
int PrintAdminToolbar(DWORD root, int mflag, DWORD UID)
{
        DWORD fl = 0;        // store bit mask for keys

        /* allow to superuser or author */
        if((ULogin.LU.right & USERRIGHT_SUPERUSER) || (ULogin.LU.ID[0] != 0 &&
                ULogin.LU.UniqID == UID)) {

                if(((mflag & MESSAGE_IS_CLOSED) == 0) && (ULogin.LU.right & USERRIGHT_CLOSE_MESSAGE))
                        fl = fl | 0x0001;

                if((mflag & MESSAGE_IS_CLOSED) && (ULogin.LU.right & USERRIGHT_OPEN_MESSAGE))
                        fl = fl | 0x0002;

                if(ULogin.LU.right & USERRIGHT_MODIFY_MESSAGE)
                        fl = fl | 0x0004;
        }

        /* allow only to superuser */
        if(ULogin.LU.right & USERRIGHT_SUPERUSER) {
                
                fl = fl | 0x0080; /* delete */

                if((mflag & MESSAGE_IS_INVISIBLE) == 0)
                        fl = fl | 0x0008;
                
                if(mflag & MESSAGE_IS_INVISIBLE)
                        fl = fl | 0x0010;
                
                if((mflag & MESSAGE_COLLAPSED_THREAD) == 0)
                        fl = fl | 0x0020;
                
                if(mflag & MESSAGE_COLLAPSED_THREAD)
                        fl = fl | 0x0040;
        }

        int g = 0;
        if(fl) {
                /* print administration table */
                printf("<BR><CENTER><TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1"
                        "BGCOLOR=\"#eeeeee\"><TR><TD ALIGN=CENTER>[ <SMALL>");
                
                /* close thread */
                if(fl & 0x0001) {
                        printf("<A HREF=\"%s?close=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_close_thread);
                        g = 1;
                }

                /* open thread */
                if(fl & 0x0002) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?unclose=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_unclose_thread);
                        g = 1;
                }
                
                // change message 
                if(fl & 0x0004) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?changemsg=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_change_message);
                }

                /* collapse thread */
                if(fl & 0x0020) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?roll=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_roll);
                }
                        
                /* uncollapse thread */
                if(fl & 0x0040) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?roll=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_unroll);
                }

                /* hide thread */
                if(fl & 0x0008) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?hide=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_hide_thread);
                        g = 1;
                }
                
                /* unhide thread */
                if(fl & 0x0010) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?unhide=%ld\"><font color=\"#AF0000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_unhide_thread);
                        g =1;
                }

                /* delete thread */
                if(fl & 0x0080) {
                        if(g) printf(DESIGN_BUTTONS_DIVIDER);
                        printf("<A HREF=\"%s?delmsg=%lu\"><font color=\"#FF1000\">%s</font></A>",
                                MY_CGI_URL, root, MESSAGEMAIN_moderate_delete_thread);
                }

                printf("</SMALL> ]</TD></TR></TABLE></CENTER>");
        }
        return 0;
}

void PrintSearchForm(const char *s, DB_Base *db, int start = 0)
{
        printf("<FORM METHOD=POST ACTION=\"%s?search=action\">",
                MY_CGI_URL);
        
        printf("<CENTER><P><B>%s</B><BR><P>", MESSAGEHEAD_search);

        if(start) {
                FILE *f;
                char LastMsgStr[500];
                DWORD LastMsg = 0, LastDate = 0;
                f = fopen(F_SEARCH_LASTINDEX, FILE_ACCESS_MODES_R);
                if(f != NULL) {
                        if (fscanf(f, "%lu %lu", &LastMsg, &LastDate) == EOF && ferror(f))
                                printhtmlerror();
                        fclose(f);
                }
                if(LastMsg != 0) {
                        SMessage mes;
                        if(ReadDBMessage(db->TranslateMsgIndex(LastMsg), &mes)) {
                                char *s;
                                s = ConvertTime(mes.Date);
                                sprintf(LastMsgStr, "%lu (%s)", LastMsg, s);
                        }
                        else strcpy(LastMsgStr, MESSAGEMAIN_search_indexerror);
                }
                else {
                        strcpy(LastMsgStr, MESSAGEMAIN_search_notindexed);
                }
                printf("<P>%s<P>%s : %s<P>", MESSAGEMAIN_search_howtouse, MESSAGEMAIN_search_lastindexed, LastMsgStr);
        }

        printf("<P><EM>%s</EM><BR>", MESSAGEMAIN_search_searchmsg);
        printf("<INPUT TYPE=RADIO NAME=sel VALUE=1 CHECKED>%s <FONT FACE=\"Courier\">"
                "<INPUT TYPE=TEXT NAME=\"find\" SIZE=45 VALUE=\"%s\"></FONT>", 
                        MESSAGEMAIN_search_containing, s);

        
        printf("<P><INPUT TYPE=SUBMIT VALUE=\"%s\"></CENTER></FORM>",
                MESSAGEHEAD_search);
}

#if STABLE_TITLE == 0
void Tittle_cat(const char *s)
{
        // set title
        ConfTitle = (char*)realloc(ConfTitle, strlen(ConfTitle) + strlen(TITLE_divider) + strlen(s) + 1);
        strcat(ConfTitle, TITLE_divider);
        strcat(ConfTitle, s);
}
#else
#define Tittle_cat(x) {}
#endif

/* print create or edit(delete) profile form (depend of flags)
 * if flags == 1 - Edit profile, otherwise create profile
 */
void PrintEditProfileForm(SProfile_UserInfo *ui, SProfile_FullUserInfo *fui, DWORD flags)
{
        char str1[20], str2[20], str3[20], str4[20], str5[20];
        printf("<FORM METHOD=POST ACTION=\"%s?register=action\">",
                MY_CGI_URL);

        if(ULogin.LU.ID[0] == 0) {
                printf(DESIGN_BEGIN_REGISTER_OPEN "<TR><TD COLSPAN=2 ALIGN=CENTER>"
                        "<BIG>%s</BIG></TD></TR>", MESSAGEMAIN_register_intro);
        }
        else
                printf(DESIGN_BEGIN_REGISTER_OPEN "<TR><TD COLSPAN=2 ALIGN=CENTER>"
                        "<BIG>%s</BIG></TD></TR>", MESSAGEMAIN_register_chg_prof_intro);

        printf("<TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=\"80%%\" NOSHADE></TD></TR><TR><TD ALIGN=RIGHT>%s </TD>",
                        MESSAGEMAIN_register_login);

        if( (ULogin.LU.ID[0] == 0) || (ULogin.LU.ID[0] != 0 && ((ULogin.pui->right & USERRIGHT_SUPERUSER) != 0)) ) {
                printf("<TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"login\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                        AUTHOR_NAME_LENGTH - 1, FilterHTMLTags(ui->username, 1000, 0));
        }
        else {
                printf("<TD ALIGN=LEFT>%s</TD></TR>", FilterHTMLTags(ui->username, 1000, 0));
        }

        if( ULogin.LU.ID[0] != 0 && ((ULogin.LU.right & USERRIGHT_ALT_DISPLAY_NAME) != 0) ) {
                printf("<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"dispnm\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                        MESSAGEMAIN_register_displayname, PROFILES_MAX_ALT_DISPLAY_NAME - 1, ui->altdisplayname);
        }

        if((flags & 0x01) == 0) {
                printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>" MESSAGEMAIN_register_oldpass_req "</STRONG></TR>" \
                           "<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=PASSWORD NAME=\"opswd\" SIZE=35 " \
                           "MAXLENGTH=%d VALUE=\"\"></TD></TR>",
                                MESSAGEMAIN_register_oldpassword, PROFILES_MAX_PASSWORD_LENGTH - 1);
        }

        if(ui->Flags & PROFILES_FLAG_VISIBLE_EMAIL)
                strcpy(str1, RADIO_CHECKED);
        else *str1 = 0;

        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>" MESSAGEMAIN_register_if_want_change "</STRONG></TR>");
        
        printf("<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=PASSWORD NAME=\"pswd1\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>" \
                "<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=PASSWORD NAME=\"pswd2\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                MESSAGEMAIN_register_password1, PROFILES_MAX_PASSWORD_LENGTH - 1, "",
                MESSAGEMAIN_register_password2, PROFILES_MAX_PASSWORD_LENGTH - 1, "");

        printf("<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"name\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                MESSAGEMAIN_register_full_name, PROFILES_FULL_USERINFO_MAX_NAME - 1,
                FilterHTMLTags(fui->FullName, 1000, 0));
                
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>" MESSAGEMAIN_register_validemail_req "</STRONG></TR>");

        if ((currentdsm & CONFIGURE_bot) == 0 && ULogin.LU.ID[0] == 0)
                printf("<TR class=\"nd\"><TD COLSPAN=2 ALIGN=CENTER><STRONG>" MESSAGEMAIN_register_bot "</STRONG></TR>");

        printf("<TR><TD ALIGN=RIGHT>" MESSAGEMAIN_register_email " </TD>");

        printf("<TD ALIGN=LEFT>");

        if ((currentdsm & CONFIGURE_bot) == 0 && ULogin.LU.ID[0] == 0)
                printf("<INPUT TYPE=TEXT class=\"nd\" NAME=\"email\" SIZE=35 MAXLENGTH=%d><span class\"nd\"><BR></span>",
                       PROFILES_FULL_USERINFO_MAX_EMAIL - 1);

        printf("<INPUT TYPE=TEXT NAME=\"emchk\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>"
               "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>" MESSAGEMAIN_register_email_pub 
               "</STRONG><INPUT TYPE=CHECKBOX NAME=\"pem\" VALUE=1%s>",
               PROFILES_FULL_USERINFO_MAX_EMAIL - 1, FilterHTMLTags(fui->Email, 1000, 0), str1);
        
        printf("</TD></TR>");

        printf("<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"hpage\" SIZE=35 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                MESSAGEMAIN_register_homepage, PROFILES_FULL_USERINFO_MAX_HOMEPAGE - 1,
                FilterHTMLTags(fui->HomePage, 1000, 0));
                
        printf("<TR><TD ALIGN=RIGHT>%s </TD><TD ALIGN=LEFT><INPUT TYPE=TEXT NAME=\"icq\" SIZE=15 MAXLENGTH=%d VALUE=\"%s\"></TD></TR>",
                  MESSAGEMAIN_register_icq, PROFILES_MAX_ICQ_LEN - 1, FilterHTMLTags(ui->icqnumber, 1000, 0));

                        
        fui->SelectedUsers[PROFILES_FULL_USERINFO_MAX_SELECTEDUSR - 1] = 0;        // FIX
   
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><BR>"\
                "<TEXTAREA COLS=60 ROWS=3 NAME=\"about\" WRAP=VIRTUAL>%s</TEXTAREA></TD></TR>",
                 MESSAGEMAIN_register_about, FilterHTMLTags(fui->AboutUser, 1000, 0));
                 
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><BR>"\
                "<TEXTAREA COLS=60 ROWS=3 NAME=\"sign\" WRAP=VIRTUAL>%s</TEXTAREA></TD></TR>",
                MESSAGEMAIN_register_signature, FilterHTMLTags(fui->Signature, 1000, 0));
                
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><BR>"\
                "<TEXTAREA COLS=30 ROWS=4 NAME=\"susr\" WRAP=VIRTUAL>%s</TEXTAREA></TD></TR>",
                MESSAGEMAIN_register_selectedusers,  FilterHTMLTags(fui->SelectedUsers, 1000, 0));


        if((ui->Flags & PROFILES_FLAG_INVISIBLE) == 0)
                strcpy(str1, RADIO_CHECKED);
        else *str1 = 0;

    if((ui->Flags & PROFILES_FLAG_PERSMSGDISABLED) != 0)
                strcpy(str2, RADIO_CHECKED);
    else *str2 = 0;

    if((ui->Flags & PROFILES_FLAG_PERSMSGTOEMAIL) != 0)
                strcpy(str3, RADIO_CHECKED);
        else *str3 = 0;

        if((ui->Flags & PROFILES_FLAG_ALWAYS_EMAIL_ACKN) != 0)
                strcpy(str4, RADIO_CHECKED);
        else *str4 = 0;

        if((ui->Flags & PROFILES_FLAG_VIEW_SETTINGS) != 0)
                strcpy(str5, RADIO_CHECKED);
        else *str5 = 0;
                                                                                                                                                                                
                
        printf("<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><INPUT TYPE=CHECKBOX NAME=\"vprf\" VALUE=1 %s></TD></TR>" \
                "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><INPUT TYPE=CHECKBOX NAME=\"apem\" VALUE=1 %s></TD></TR>"        \
                "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><INPUT TYPE=CHECKBOX NAME=\"pdis\" VALUE=1 %s></TD></TR>" \
                "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><INPUT TYPE=CHECKBOX NAME=\"peml\" VALUE=1 %s></TD></TR>" \
                "<TR><TD COLSPAN=2 ALIGN=CENTER><STRONG>%s</STRONG><INPUT TYPE=CHECKBOX NAME=\"vprs\" VALUE=1 %s></TD></TR>",
                        MESSAGEMAIN_register_private_prof, str1,
                        MESSAGEMAIN_register_always_emlackn, str4,
                        MESSAGEMAIN_register_pmsg_disable, str2,
                        MESSAGEMAIN_register_pmsg_email, str3,
                        MESSAGEMAIN_register_view_saving, str5
                );

        printf("<TR><TD COLSPAN=2></TR><TR><TD COLSPAN=2 ALIGN=CENTER><B><FONT COLOR=RED>" \
                   MESSAGEMAIN_register_req_fields "</FONT></B></TR><TR><TD COLSPAN=2>" \
                   "<HR ALIGN=CENTER WIDTH=\"80%%\" NOSHADE></TR><TR><TD COLSPAN=2 ALIGN=CENTER>");

        /* print buttons */
        if(flags & 0x01)
                printf("<INPUT TYPE=SUBMIT NAME=\"register\" VALUE=\"%s\">&nbsp;", MESSAGEMAIN_register_register);

        if(flags & 0x02)
                printf("<INPUT TYPE=SUBMIT NAME=\"register\" VALUE=\"%s\">&nbsp;",  MESSAGEMAIN_register_edit);

        if(flags & 0x04)
                printf(
                "<INPUT TYPE=SUBMIT NAME=\"register\" VALUE=\"" MESSAGEMAIN_register_delete "\">" 
                "<INPUT TYPE=CHECKBOX NAME=\"" CONFIRM_DELETE_CHECKBOX_TEXT "\" VALUE=\"true\">" 
                MESSAGEMAIN_register_confirm_delete);

        printf("</TD></TR>" DESIGN_END_REGISTER_CLOSE "</FORM>");
}


void PrintSessionsList(DWORD Uid)
{
        char **buf = NULL;
        DWORD sc = 0, i;
        DWORD seqid[2], userid;
        if(ULogin.GenerateListSessionForUser(&buf, &sc, Uid)){

                if(sc){
                        printf(DESIGN_BEGIN_USERINFO_INTRO_OPEN);
                        for(i = 0; i < sc; i++) {
                                unsigned char *seqip = (unsigned char*)(buf[i]+4);
                                seqid[0] = *((DWORD*)(buf[i]+8));
                                seqid[1] = *((DWORD*)(buf[i]+12));
                                userid = *((DWORD*)(buf[i]+16));
                                time_t seqtime = *((DWORD*)(buf[i]));
                                char *seqdate;
                                if (seqtime > time(NULL))
                                        seqdate = (char*)ConvertFullTime(seqtime-USER_SESSION_LIVE_TIME);
                                else
                                        seqdate = (char*)ConvertFullTime(seqtime);

                                printf("<TR><TD ALIGN=RIGHT>%ld. "MESSAGEMAIN_session_ip"</TD><TD"
                                        " ALIGN=LEFT><STRONG>%u.%u.%u.%u</STRONG> %s</TD></TR>",
                                        i+1, seqip[0] & 0xff, seqip[1] & 0xff, seqip[2] & 0xff, seqip[3] & 0xff, 
                                        userid & SEQUENCE_IP_CHECK_DISABLED ? MESSAGEMAIN_session_ip_nocheck : MESSAGEMAIN_session_ip_check );
                                         
                                printf("<TR><TD ALIGN=RIGHT>"MESSAGEMAIN_session_date"</TD><TD ALIGN=LEFT>"
                                        " <STRONG>%s</STRONG></TD></TR>", seqdate);
                                printf("<TR><TD ALIGN=RIGHT>"MESSAGEMAIN_session_state"</TD><TD ALIGN=LEFT><STRONG>");

                                if( seqtime > time(NULL) ) {
                                        printf(MESSAGEMAIN_session_state_active);

                                        if( (ULogin.LU.right & USERRIGHT_SUPERUSER) != 0  || ( ULogin.LU.UniqID == Uid && ( ULogin.LU.right & USERRIGHT_PROFILE_MODIFY) != 0 ) )
                                                printf(" [<a href=\"%s?clsession1=%ld&clsession2=%ld\">"MESSAGEMAIN_session_state_toclose"</a>]", MY_CGI_URL, seqid[0], seqid[1]);
                                }
                                else  printf(MESSAGEMAIN_session_state_closed);
                                printf("</STRONG></TD></TR><TR><TD><BR></TD></TR>");
                                free(buf[i]);
                        }
                        printf(DESIGN_END_USERINFO_INTRO_CLOSE);
                }
                else printf(DESIGN_BEGIN_USERINFO_INTRO_OPEN "<TR><TD ALIGN=CENTER><B>"
                        MESSAGEMAIN_session_no "</B></TD></TR>" DESIGN_END_USERINFO_INTRO_CLOSE);
        }
        else printhtmlerror();
        if(buf) free(buf);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
int PrintAboutUserInfo(char *name)
{
        char *nickname;
        CProfiles *mprf = new CProfiles();
        if(mprf->errnum != PROFILE_RETURN_ALLOK) {
#if ENABLE_LOG >= 1
                print2log("error working with profiles database (init)");
#endif
                return 0;
        }
        SProfile_FullUserInfo fui;
        SProfile_UserInfo ui;

        nickname = FilterHTMLTags(name, 1000);

        if(mprf->GetUserByName(name, &ui, &fui, NULL) != PROFILE_RETURN_ALLOK)
        {
                delete mprf;
                char *nickname_f = FilterBiDi(nickname);
                printf(MESSAGEMAIN_profview_no_user, nickname_f);
                if (nickname_f)
                        free(nickname_f);
                return 1;
        }

        printf("<P></P>" DESIGN_BEGIN_USERINFO_INTRO_OPEN
               "<TR><TD COLSPAN=2><BIG><span>%s <span class=\"nick\">%s</span></span></BIG></TD></TR>", MESSAGEMAIN_profview_intro, nickname);
        printf("<TR><TD COLSPAN=2><HR ALIGN=CENTER WIDTH=\"80%%\" NOSHADE></TR>");
        char *nickname_f = FilterBiDi(nickname);
        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG><span class=\"nick\">%s</span></STRONG><SMALL>", MESSAGEMAIN_profview_login, nickname_f);
        if (nickname_f)
                free(nickname_f);

        if(ULogin.LU.UniqID == ui.UniqID){
                printf(" <A HREF=\"%s?register=form\">(%s)</A>", MY_CGI_URL, MESSAGEMAIN_profview_editinfo);
        }
        
        if(ULogin.LU.ID[0] && ULogin.LU.UniqID != ui.UniqID){
                printf(" <A HREF=\"%s?persmsgform=%s\">(%s)</A>", MY_CGI_URL, CodeHttpString(name), MESSAGEMAIN_profview_postpersmsg);
        }

        printf("</SMALL></TD></TR>");

        if((ui.Flags & PROFILES_FLAG_ALT_DISPLAY_NAME) != 0) {
                char *st;
                if(!PrepareTextForPrint(ui.altdisplayname, &st, 1, MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE)) {
                        st = (char*)malloc(1000);
                        strcpy(st, ui.altdisplayname);
                }
                printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>",
                                MESSAGEMAIN_profview_altname, st);
                free(st);
        }

        if(((ui.Flags & PROFILES_FLAG_INVISIBLE) == 0) || (ULogin.LU.right & USERRIGHT_SUPERUSER) ||
                (ULogin.LU.UniqID == ui.UniqID) )
        {
                printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>",
                        MESSAGEMAIN_profview_fullname,  FilterHTMLTags(fui.FullName, 1000, 0));
                        
        //        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD><STRONG><A HREF=\"%s\">%s</A></STRONG></TD></TR>",
        //                MESSAGEMAIN_profview_homepage,        CodeHttpString(fui.HomePage, 0, URL_ENCODE), FilterHTMLTags(fui.HomePage, 1000, 0));
                int need_http = 0;
                if(strlen(fui.HomePage) < 7 || strncmp(fui.HomePage, "http://", 7)) need_http = 1;
                //else fui.HomePage;

                                
                printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG><A HREF=\"%s%s\">%s%s</A></STRONG></TD></TR>",
                        MESSAGEMAIN_profview_homepage,        
                        need_http ? "http://": "", CodeHttpString(fui.HomePage, 0, URL_ENCODE),
                        need_http ? "http://": "", FilterHTMLTags(fui.HomePage, 1000, 0));
        
        
                
                /* if invisible mail - allow view only for same user or superuser */
                if((ui.Flags & PROFILES_FLAG_VISIBLE_EMAIL) || (ULogin.LU.right & USERRIGHT_SUPERUSER) ||
                        (ULogin.LU.UniqID == ui.UniqID)) {
                        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG><A HREF=\"mailto:%s\">%s</A></STRONG></TD></TR>",
                                MESSAGEMAIN_profview_email, CodeHttpString(fui.Email, 0, MAIL_ENCODE), FilterHTMLTags(fui.Email, 1000, 0));
                }
                else printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG><FONT COLOR=\"#0000F0\">%s</FONT></STRONG></TD></TR>",
                        MESSAGEMAIN_profview_email, MESSAGEMAIN_profview_privacy_inf);
                
                /* possible error (with malloc) here */
                char *regdate, *logdate = (char*)malloc(255), *ustatus = (char*)malloc(255);
                /*************************************/
                if(!ui.LoginDate) strcpy(logdate, "Never logged in");
                else strcpy(logdate, (char*)ConvertFullTime(ui.LoginDate));
                regdate = (char*)ConvertFullTime(fui.CreateDate);

                // set up ustatus
        //        if((ui.right & USERRIGHT_SUPERUSER) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) || (strcmp(name, "www") /*&& strcmp(name, "Jul'etka") */))) {
                 if(ui.right & USERRIGHT_SUPERUSER) {
                        strcpy(ustatus, MESSAGEMAIN_profview_u_moderator);
                }
                else {
                        strcpy(ustatus, MESSAGEMAIN_profview_u_user);
                }

                //        check for too high value in status
//                if(ui.Status >= USER_STATUS_COUNT) ui.Status = USER_STATUS_COUNT - 1;        // maximum status

                //strcat(ustatus, " ( ");
                //strcat(ustatus, UserStatus_List[ui.Status]);
                //strcat(ustatus, " )");

                ui.icqnumber[15] = 0;        //        FIX
/*
                char icqpic[1000];
                icqpic[0] = 0;

                if(strlen(ui.icqnumber) > 0) {
                        sprintf(icqpic, "<TR><TD ALIGN=RIGHT>%s</TD><TD><STRONG>" \
                                "<IMG src=\"http://online.mirabilis.com/scripts/online.dll?icq=%s&amp;img=5\" alt=\"icq status\">"\
                                "%s</STRONG></TD></TR>",
                                MESSAGEMAIN_profview_user_icq, ui.icqnumber, ui.icqnumber);
                }

*/

                char *about = NULL;
                DWORD tmp;
                char *st = FilterHTMLTags(fui.AboutUser, MAX_PARAMETERS_STRING);
                if(st) {
                        DWORD enabled_smiles = 0;
                        if((currentdsm & CONFIGURE_dsm) == 0)
                                enabled_smiles = MESSAGE_ENABLED_SMILES;
                        if(FilterBoardTags(st, &about, 0, MAX_PARAMETERS_STRING,
                                        enabled_smiles | MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE |
                                        BOARDTAGS_EXPAND_ENTER, &tmp) == 0) {
                                about = (char*)malloc(strlen(st) + 1);
                                strcpy(about, st);
                        }
                        free(st);
                }
                

                printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR><BR>" \
                        "<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>" \
                        "<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%ld</STRONG></TD></TR>" \
                        "<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>" \
                        "<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>" \
                        "<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>",
                           MESSAGEMAIN_profview_user_icq, FilterHTMLTags(ui.icqnumber, 1000, 0),
                           MESSAGEMAIN_profview_user_status,        ustatus,
                           MESSAGEMAIN_profview_postcount,                ui.postcount,
                           MESSAGEMAIN_profview_reg_date,                regdate,
                           MESSAGEMAIN_profview_login_date,                logdate,
                           MESSAGEMAIN_profview_about_user,                about);

                free(about);

                if((ULogin.LU.right & USERRIGHT_SUPERUSER) || (ULogin.LU.UniqID == ui.UniqID) ) {
                        char hname[10000];
                        hostent *he;
                        unsigned char *aa = (unsigned char *)(&ui.lastIP);
                        sprintf(hname, "%u.%u.%u.%u", aa[0] & 0xff, aa[1] & 0xff, aa[2] & 0xff, aa[3] & 0xff);
                        if((he = gethostbyaddr((char*)(&ui.lastIP), 4, AF_INET)) != NULL) {
                                // prevent saving bad hostname
                                if(strlen(he->h_name) > 0) {
                                        char tmp[1000];

                                        strcpy(tmp, hname);
                                        strncpy(hname, he->h_name, 9999);
                                        strcat(hname, " (");
                                        strcat(hname, tmp);
                                        strcat(hname, ")");
                                }
                        }
                        // only for admin :)
                        if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0)
                                printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%lu</STRONG></TD></TR>", 
                                        MESSAGEMAIN_profview_refreshcnt, ui.RefreshCount);
                        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%s</STRONG></TD></TR>",
                                MESSAGEMAIN_profview_lastip, hname);
                        printf("<TR><TD ALIGN=RIGHT>%s</TD><TD ALIGN=LEFT><STRONG>%d(%d)</STRONG></TD></TR>",
                                MESSAGEMAIN_profview_persmsgcnt, ui.persmescnt, ui.readpersmescnt);
                }

                free(logdate);
        //        free(ustatus);
        }
        else {
                printf("<TR><TD COLSPAN=2><BIG>%s</BIG></TD></TR><BR>", MESSAGEMAIN_profview_privacy_prof);
        }
        printf(DESIGN_END_USERINFO_INTRO_CLOSE);


        printf("<BR>");

  if((ULogin.LU.right & USERRIGHT_SUPERUSER) || (ULogin.LU.UniqID == ui.UniqID) ) {
        PrintSessionsList(ui.UniqID);

  }

        //
        //        if admin print profile modification form
        //
        if((ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                char sel[100];
                BYTE i;
                printf("<BR><CENTER><FORM METHOD=POST ACTION=\"%s?changeusr=action\"> User Status: ", MY_CGI_URL);

                printf("<SELECT NAME=\"ustat\">");
                for(i = 0; i < USER_STATUS_COUNT; i++) {

                        if(i == ui.Status) strcpy(sel, LISTBOX_SELECTED);
                        else strcpy(sel, "");

                        printf("<OPTION VALUE=\"%d\" %s>%s", i, sel, UserStatus_List[i]);
                }
                printf("</SELECT>");

                printf("<INPUT TYPE=HIDDEN SIZE=0 NAME=\"name\" VALUE=\"%s\">", nickname);

                // user rights here
                puts("<BR><P><TABLE BORDER=1 CELLSPACING=0 CELLPADDING=6 BGCOLOR=\"#FFFFFF\"><TR><TH ALIGN=RIGHT>");
                for(i = 0; i < USERRIGHT_COUNT; i++)
                {
                        if( (ui.right & (1<<i)) != 0)
                                strcpy(sel, RADIO_CHECKED);
                        else sel[0] = 0;
                        printf("%s <INPUT TYPE=CHECKBOX NAME=\"right%d\" %s><BR>",
                                UserRight_List[i], i, sel);
                }
                puts("</TH></TR></TABLE>");

                printf("<BR><BR><INPUT TYPE=SUBMIT NAME=\"update\" VALUE=\"%s\">", MESSAGEMAIN_register_edit);

                printf("</FORM></CENTER>");
        }

        free(nickname);
        delete mprf;

        return 1;
}

// string compare
static int cmp_name(const void *p1, const void *p2)
{
        char upper[2][AUTHOR_NAME_LENGTH];
        strcpy(upper[0], (*(char **)p1) + 20);
        strcpy(upper[1], (*(char **)p2) + 20);
        for(int i = 0; i < 2; i++) 
                toupperstr(upper[i]);
        
        return strcmp(upper[0], upper[1]);
}

// by last ip
static int cmp_ip(const void *p1, const void *p2)
{
        return int( ntohl((*((DWORD**)p1))[0]) - ntohl((*((DWORD**)p2))[0]) );
}

// by postcount
static int cmp_postcount(const void *p1, const void *p2)
{
        return int( (*((DWORD**)p2))[1] - (*((DWORD**)p1))[1] );
}

// by refresh count
static int cmp_refreshcount(const void *p1, const void *p2)
{
        return int( (*((DWORD**)p2))[3] - (*((DWORD**)p1))[3] );
}

// by last login date
static int cmp_date(const void *p1, const void *p2)
{
        return int( (*((DWORD**)p2))[2] - (*((DWORD**)p1))[2] );
}

// by security right
static int cmp_right(const void *p1, const void *p2)
{
        return int( (*((DWORD**)p1))[4] - (*((DWORD**)p2))[4] );
}

void PrintUserList(DB_Base *dbb, int code)
{
        char **buf = NULL;
        char name[1000];
        DWORD uc = 0, i;
        CProfiles uprof;

        if(!uprof.GenerateUserList(&buf, &uc))
                printhtmlerror();

        // Print header of user list
        printf("<CENTER><P><B>%s</B><BR>%s%lu<P>", MESSAGEHEAD_userlist, MESSAGEMAIN_total_user_count, uc);

        switch(code) {
                case 1:
                        qsort((void *)buf, uc, sizeof(char*), cmp_name);
                        break;
                case 2:
                        qsort((void *)buf, uc, sizeof(char*), cmp_postcount);
                        break;
                case 3:
                        qsort((void *)buf, uc, sizeof(char*), cmp_ip);
                        break;
                case 4:
                        qsort((void *)buf, uc, sizeof(char*), cmp_date);
                        break;
                case 5:
                        qsort((void *)buf, uc, sizeof(char*), cmp_refreshcount);
                        break;
                case 6:
                        qsort((void *)buf, uc, sizeof(char*), cmp_right);
                        break;
        }

        // print sort link bar
        if((ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                printf("<B>%s</B>", MESSAGEMAIN_userlist_sortby);
                if(code != 1) printf("<A HREF=\"%s?userlist=1\">%s</A> | ", MY_CGI_URL, MESSAGEMAIN_userlist_sortbyname);
                else printf("<B>%s</B> | ", MESSAGEMAIN_userlist_sortbyname);
                if(code != 2) printf("<A HREF=\"%s?userlist=2\">%s</A> | ", MY_CGI_URL, MESSAGEMAIN_userlist_sortbypcnt);
                else printf("<B>%s</B> | ", MESSAGEMAIN_userlist_sortbypcnt);
                if(code != 3) printf("<A HREF=\"%s?userlist=3\">%s</A> | ", MY_CGI_URL, MESSAGEMAIN_userlist_sortbyhost);
                else printf("<B>%s</B> | ", MESSAGEMAIN_userlist_sortbyhost);
                if(code != 4) printf("<A HREF=\"%s?userlist=4\">%s</A> | ", MY_CGI_URL, MESSAGEMAIN_userlist_sortbydate);
                else printf("<B>%s</B> | ", MESSAGEMAIN_userlist_sortbydate);
                if(code != 5) printf("<A HREF=\"%s?userlist=5\">%s</A> | ", MY_CGI_URL, MESSAGEMAIN_userlist_sortbyrefresh);
                else printf("<B>%s</B> | ", MESSAGEMAIN_userlist_sortbyrefresh);
                if(code != 6) printf("<A HREF=\"%s?userlist=6\">%s</A><BR><BR>", MY_CGI_URL, MESSAGEMAIN_userlist_sortbyright);
                else printf("<B>%s</B><BR><BR>", MESSAGEMAIN_userlist_sortbyright);
        }

        DWORD oldval = 0;
        if(uc) {
                unsigned char *aa = (unsigned char*)buf[0];
                switch(code) {
                        case 2:
                                oldval = *((DWORD*)(buf[0] + 4));
                                printf("<B>(%lu)</B><BR>", *((DWORD*)(buf[0] + 4)));
                                break;
                        case 3:
                                oldval = *((DWORD*)(buf[0]));
                                printf("<B>(%u.%u.%u.%u)</B><BR>", aa[0] & 0xff, aa[1] & 0xff, aa[2] & 0xff, aa[3] & 0xff);
                                break;
                        case 4:
                                // not used yet
                                break;
                        case 5:
                                oldval = *((DWORD*)(buf[0] + 12));
                                printf("<B>(%lu)</B><BR>", *((DWORD*)(buf[0] + 12)));
                                break;
                        case 6:
                                oldval = *((DWORD*)(buf[0] + 16));
                                printf("<B>(%08lx)</B><BR>", *((DWORD*)(buf[0] + 16)));
                                break;
                }
        }

        int cc = 0;
        // print begin
        for(i = 0; i < uc; i++) {
               switch(code) {
               case 2:
                       if(oldval != *((DWORD*)(buf[i] + 4))) {
                               printf("<BR><BR><B>(%lu)</B><BR>", *((DWORD*)(buf[i] + 4)));
                               oldval = *((DWORD*)(buf[i] + 4));
                               cc = 0;
                       }
                       break;
               case 3:
                       if(oldval != *((DWORD*)(buf[i]))) {
                               unsigned char *aa = (unsigned char*)buf[i];
                               printf("<BR><BR><B>(%u.%u.%u.%u)</B><BR>", aa[0] & 0xff, aa[1] & 0xff, aa[2] & 0xff, aa[3] & 0xff);
                               oldval = *((DWORD*)(buf[i]));
                               cc = 0;
                       }
                       break;
               case 4:
                       break;
               case 5:
                       if(oldval != *((DWORD*)(buf[i] + 12))) {
                               printf("<BR><BR><B>(%lu)</B><BR>", *((DWORD*)(buf[i] + 12)));
                               oldval = *((DWORD*)(buf[i] + 12));
                               cc = 0;
                       }
                       break;
               case 6:
                       if(oldval != *((DWORD*)(buf[i] + 16))) {
                               printf("<BR><BR><B>(%08lx)</B><BR>", *((DWORD*)(buf[i] + 16)));
                               oldval = *((DWORD*)(buf[i] + 16));
                               cc = 0;
                       }
                       break;
               }
               if((cc % 10) != 0) printf(" | ");
               if(((cc % 10) == 0) && cc != 0)  printf("<BR>");
               
               cc++;
               
               dbb->Profile_UserName(buf[i] + 20, name, 1);
               printf("%s", name);
               free(buf[i]);
        }
        if(buf) free(buf);
        printf("</CENTER>");
}

/* create or update user profile
 * if op == 1 - create
 * if op == 2 - update
 * if op == 3 - delete
 */
int CheckAndCreateProfile(SProfile_UserInfo *ui, SProfile_FullUserInfo *fui, char *p2, char *oldp, int op, char *deal)
{
        CProfiles *uprof;
        DWORD err = 0, needregisteraltnick = 0;

        /* password check */
        if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                /* old password */
                if(op != 1 && (ULogin.LU.ID[0] == 0 || (strcmp(oldp, ULogin.pui->password) != 0) ||
                        (strcmp(ULogin.pui->username, ui->username) != 0)))
                        return PROFILE_CHK_ERROR_INVALID_PASSWORD;
        }
        /* password and confirm password */
        if(op != 3 && (strcmp(ui->password, p2) != 0 || ((strlen(p2) < PROFILES_MIN_PASSWORD_LENGTH) && strlen(p2) != 0)))
                return PROFILE_CHK_ERROR_INVALID_PASSWORD_REP;
        
        if(strlen(p2) == 0) {
                if(ULogin.LU.ID[0] != 0) strcpy(ui->password, ULogin.pui->password);
                else return PROFILE_CHK_ERROR_INVALID_PASSWORD_REP;
        }

        // ************ delete ************
        if(op == 3) {
                if((ULogin.LU.ID[0] == 0 || strcmp(ULogin.pui->username, ui->username) != 0 ||
                        (ULogin.LU.right & USERRIGHT_PROFILE_MODIFY) == 0) && (!(ULogin.LU.right & USERRIGHT_SUPERUSER)))
                        return PROFILE_CHK_ERROR_CANNOT_DELETE_USR;

                uprof = new CProfiles();

                SProfile_UserInfo nui;
                err = uprof->GetUserByName(ui->username, &nui, fui, NULL);
                if(err == PROFILE_RETURN_ALLOK) {
                        // check for special admitions
                        if(nui.altdisplayname[0] != 0 && (nui.Flags & PROFILES_FLAG_ALT_DISPLAY_NAME) ) {
                                // need to delete alternative spelling too
                                ui->UniqID = nui.UniqID;
                                needregisteraltnick = 1;
                        }

                        // deleting user
                        err = uprof->DeleteUser(ui->username);
                        // check result of operation
                        if(err == PROFILE_RETURN_ALLOK) ULogin.ForceCloseSessionForUser(nui.UniqID); // delete all sessions.
                }
                goto cleanup_and_parseerror;
        }

        /* Now we know that it can't be user deletion, so check common parameters */

        if(fui->Signature[0] != 0) {
                ui->Flags = ui->Flags | PROFILES_FLAG_HAVE_SIGNATURE;
        }

        if(ui->altdisplayname[0] != 0 && /*security check*/ (ULogin.LU.ID[0] != 0 && (ULogin.LU.right & USERRIGHT_ALT_DISPLAY_NAME) != 0 ) ) {
                ui->Flags |= PROFILES_FLAG_ALT_DISPLAY_NAME;
                needregisteraltnick = 1;
        }
        else {
                ui->Flags &= (~PROFILES_FLAG_ALT_DISPLAY_NAME);
                if( (ULogin.LU.ID[0] != 0 && (ULogin.LU.right & USERRIGHT_ALT_DISPLAY_NAME) != 0 ) ) needregisteraltnick = 1;
        }

        /* common email check (if requred vaild email) */
        if(IsMailCorrect(fui->Email) == 0)
                return PROFILE_CHK_ERROR_INVALID_EMAIL;

        uprof = new CProfiles();

        // ********** update **********
        if(op == 2) {
                err = uprof->ModifyUser(ui, fui, NULL);
                goto cleanup_and_parseerror;
        }

        // ********** create **********
        if(op == 1) {
                ui->lastIP = Nip;
                err = uprof->AddNewUser(ui, fui, NULL);
                goto cleanup_and_parseerror;
        }

        delete uprof;

        printhtmlerror();

cleanup_and_parseerror:

        delete uprof;

        switch(err) {
        case PROFILE_RETURN_ALLOK:
                {
                        // Do post user creation/modificaton job
#if USER_ALT_NICK_SPELLING_SUPPORT
                        if(op == 1 || (op == 2 && needregisteraltnick)) {
                                if(ui->altdisplayname[0] != 0)
                                        AltNames.AddAltName(ui->UniqID, ui->username, ui->altdisplayname);
                                else
                                        AltNames.DeleteAltName(ui->UniqID);
                        }
                        else if(op == 3 && needregisteraltnick) {
                                AltNames.DeleteAltName(ui->UniqID);
                        }
#endif
                }
                return PROFILE_CHK_ERROR_ALLOK;

        case PROFILE_RETURN_ALREADY_EXIST:
                return PROFILE_CHK_ERROR_ALREADY_EXIST;

        case PROFILE_RETURN_DB_ERROR:
#if ENABLE_LOG >= 1
                print2log("Profiles database error: DB ERROR, deal=%s", deal);
#endif 
                printhtmlerror();

        case PROFILE_RETURN_INVALID_FORMAT:
#if ENABLE_LOG >= 1
                print2log("Profiles database error: INVALID FORMAT, deal=%s", deal);
#endif
                printhtmlerror();

        case PROFILE_RETURN_INVALID_LOGIN:
                if(op == 1 || op == 2)
                        return PROFILE_CHK_ERROR_INVALID_LOGIN_SPELL;
                return PROFILE_CHK_ERROR_CANNOT_DELETE_USR;

        case PROFILE_RETURN_INVALID_PASSWORD:
                return PROFILE_CHK_ERROR_INVALID_PASSWORD;

        case PROFILE_RETURN_PASSWORD_SHORT:
                return PROFILE_CHK_ERROR_SHORT_PASSWORD;

        case PROFILE_RETURN_UNKNOWN_ERROR:
        default:
#if ENABLE_LOG >= 1
                print2log("profiles database error : UNKNOWN, deal=%s", deal);
#endif 
                printhtmlerror();
        }

        return 0; //abnormal
}

/* complete operation with user profile and print 
 * result of execution
 * op == 1 create
 * op == 2 update
 * op == 3 delete
 */
void DoCheckAndCreateProfile(SProfile_UserInfo *ui, SProfile_FullUserInfo *fui, char *passwdconfirm, char *oldpasswd, int op, char *deal)
{
        int err;
        
        if(op == 1 ) { 
                char *f = FilterWhitespaces(ui->username);
                memmove(ui->username, f, strlen(f) + 1);
        }

        print2log("DoCheckAndCreateProfile: user '%s', op=%d, (by %s)", ui->username, op, ULogin.LU.ID[0] != 0 ? ULogin.pui->username : "anonymous");
        err = CheckAndCreateProfile(ui, fui, passwdconfirm, oldpasswd, op, deal);

        if(err != PROFILE_CHK_ERROR_ALLOK)
                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

        char *tmp;

        switch(err) {
        case PROFILE_CHK_ERROR_ALLOK:
                if(op == 3) {
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                        MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_register_delete_ex, MESSAGEMAIN_register_delete_ex2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                }
                else {
                        if(op == 1) {
                                /* if session already opened - close it */
                                if(ULogin.LU.ID[0] != 0)
                                        ULogin.CloseSession(ULogin.LU.ID);

                                if(ULogin.OpenSession(ui->username, ui->password, NULL, Nip, 0) == 1) {
                                        // entered, set new cookie
                                        cookie_name = (char*)realloc(cookie_name, AUTHOR_NAME_LENGTH);
                                        strcpy(cookie_name, ui->username);
                                }
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                        MAINPAGE_INDEX);
                                PrintBoardError(MESSAGEMAIN_register_create_ex, MESSAGEMAIN_register_create_ex2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                        }
                        else {
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                        MAINPAGE_INDEX);
                                PrintBoardError(MESSAGEMAIN_register_edit_ex, MESSAGEMAIN_register_edit_ex2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                        }
                }

                PrintBottomLines();
                break;
        case PROFILE_CHK_ERROR_ALREADY_EXIST:
                PrintBoardError(MESSAGEMAIN_register_already_exit, MESSAGEMAIN_register_already_exit2, 0);
                break;
        case PROFILE_CHK_ERROR_NOT_EXIST:
                PrintBoardError(MESSAGEMAIN_register_invalid_psw, MESSAGEMAIN_register_invalid_psw2, 0);
                break;
        case PROFILE_CHK_ERROR_INVALID_LOGIN_SPELL:
                PrintBoardError(MESSAGEMAIN_register_invalid_lg_spell, MESSAGEMAIN_register_invalid_lg_spell2, 0);
                break;
        case PROFILE_CHK_ERROR_INVALID_PASSWORD:
                PrintBoardError(MESSAGEMAIN_register_invalid_psw, MESSAGEMAIN_register_invalid_psw2,
                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                break;
        case PROFILE_CHK_ERROR_INVALID_PASSWORD_REP:
                PrintBoardError(MESSAGEMAIN_register_invalid_n_psw, MESSAGEMAIN_register_invalid_n_psw2, 0);
                break;
        case PROFILE_CHK_ERROR_SHORT_PASSWORD:
                PrintBoardError(MESSAGEMAIN_register_invalid_n_psw, MESSAGEMAIN_register_invalid_n_psw2, 0);
                break;
        case PROFILE_CHK_ERROR_INVALID_EMAIL:
                PrintBoardError(MESSAGEMAIN_register_invalid_email, MESSAGEMAIN_register_invalid_email2, 0);
                break;
        case PROFILE_CHK_ERROR_CANNOT_DELETE_USR:
                /* possible BUG here */
                tmp = (char*)malloc(1000);
                /*********************/
                sprintf(tmp, MESSAGEMAIN_register_cannot_delete, ui->username);
                PrintBoardError(tmp, MESSAGEMAIN_register_cannot_delete2,
                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                free(tmp);
                break;
        default:
                printhtmlerror();
        }

        if(err != PROFILE_CHK_ERROR_ALLOK)
                PrintBottomLines();
}

void printaccessdenied(UNUSED(char *deal))
{
        Tittle_cat(TITLE_Error);
        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
#if ENABLE_LOG > 2
        print2log(LOG_UNKNOWN_URL, Cip, deal);
#endif
        PrintBoardError(MESSAGEMAIN_access_denied, MESSAGEMAIN_access_denied2, 0);
        PrintBottomLines();
}

void printpassworderror(UNUSED(char *deal))
{
        /* incorrect username or password */
#if ENABLE_LOG > 1
        print2log(LOG_PSWDERROR, Cip, deal);
#endif

        Tittle_cat(TITLE_IncorrectPassword);
                                
        /* incorrect login and/or password */
        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
        PrintBoardError(MESSAGEMAIN_incorrectpwd, MESSAGEMAIN_incorrectpwd2, 0);
        PrintBottomLines();
}

void printnomessage(UNUSED(char *deal))
{
        Tittle_cat(TITLE_Error);
#if ENABLE_LOG > 2
        print2log(LOG_UNKNOWN_URL, Cip, deal);
#endif
        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
        PrintBoardError(MESSAGEMAIN_nonexistingmsg, MESSAGEMAIN_nonexistingmsg2, 0);
        PrintBottomLines();
}

void printbadurl(UNUSED(char *deal))
{
        Tittle_cat(TITLE_Error);

#if ENABLE_LOG > 2
        print2log(LOG_UNKNOWN_URL, Cip, deal);
#endif
        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
        PrintBoardError(MESSAGEMAIN_requesterror, MESSAGEMAIN_requesterror2, 0);
        PrintBottomLines();
}

/* print message thread */
int PrintMessageThread(DB_Base *bd, DWORD root, DWORD Flag, DWORD Puser)
{
        // print admin toolbar
        PrintAdminToolbar(root, Flag, Puser);

        bd->DB_PrintMessageBody(root);
        bd->DB_PrintMessageThread(root);
        return 0;        
}

//rewritten for IIS compatibility
// Return not more than maxlen bytes (including '\0')
char* GetParams(DWORD maxlen)
{
        char* pCL = getenv("CONTENT_LENGTH");
        if(pCL != NULL)
        {
                DWORD nCL = strtoul(pCL, NULL, 10);
                if(nCL > maxlen) nCL = maxlen;
                if(nCL > 0)
                {
                        DWORD ret;
                        char* szBuf = (char*)malloc(nCL + 3);
                        if(szBuf == NULL) {
#if _DEBUG_ == 1
                                print2log("GetParams::malloc - out of memory");
#endif
                                return NULL;
                        }
                        if((ret = (DWORD)fread(szBuf, sizeof(char), nCL, stdin)) == 0) {
#if _DEBUG_ == 1
                                print2log("GetParams::fread - failed");
#endif
                                goto cleanup;        
                        }
                        szBuf[ret] = '&';        // patch string for our parser
                        szBuf[ret+1] = '\0';
//                        print2log("POST params = %s, readed = %d", szBuf, ret);
                        return szBuf;
cleanup:        
                        free(szBuf);
                }
        }
        else {
#if _DEBUG_ == 1
                print2log("No CONTENT_LENGTH env. value");
#endif
        }
        return NULL;
}

/* parse hex symbol to value */
char inline parsesym(char s)
{
        s = (char)toupper(s);
        if(s >= '0' && s <= '9') return (char)(s - '0');
        if(s >= 'A' && s <= 'F') return (char)(s - 'A' + 10);
        return 20;
}

/* find and return key [find] in string [par] between [find] and "&"
* par is limited to MAX_PARAMETERS_LENGTH - 1
*/
char* strget(char *par,const char *find, WORD maxl, char end, bool argparsing)
{
        bool bZend = false;
        char *res, *rres;
        char *s, *x;
        char a, b;
        if(par == NULL) return NULL;
        if(find == NULL) return NULL;
        if((s = strstr(par, find)) == NULL) return NULL;
        if((bZend = ((x = strchr(s, end)) == NULL)))
                x = strchr(s, 0);
        else
                *x = 0; // temporary change '&' to '\0'
        rres = res = (char *)malloc(maxl + 1);
        s = s + strlen(find);
        if(argparsing) {
                while(*s != '\0' && res - rres < maxl) {
                        if(*s == '%' && x > s + 2) {
                                if((a = parsesym(*(s + 1))) == 20) {
                                        s++; continue; // ignore invalid %
                                }
                                if((b = parsesym(*(s + 2))) == 20) {
                                        s++; continue; // ignore invalid %
                                }
                                *res = (char)(a*16 + b);
                                s+=2;
                        }
                        else if(*s == '+') *res = ' ';
                        else *res = *s;
                        res++;
                        s++;
                }
                *res = 0;
                rres = (char*)realloc(rres, strlen(rres) + 1);
        }
        else {
                strncpy(rres, s, maxl);
                rres[maxl] = 0;        // fix for any string
        }
        if(!bZend)
                *x = end;
        return rres;
}

// get cookie information, if avaliable
void ParseCookie()
{
        char *c, *s, *ss, *t, *st;
        DWORD tmp;
        long tmp_signed;
        
        GlobalNewSession = 1;
        currentft = 1;
        currentlt = 1;
        currentfm = 1;
        currentlm = 1;
        currentlann = 0;
        
        cookie_ss = CONFIGURE_SETTING_DEFAULT_ss;
        cookie_tt = CONFIGURE_SETTING_DEFAULT_tt;
        cookie_tv = CONFIGURE_SETTING_DEFAULT_tv;
        cookie_tc = CONFIGURE_SETTING_DEFAULT_tc;
        cookie_lsel = CONFIGURE_SETTING_DEFAULT_lsel;
        cookie_dsm = CONFIGURE_SETTING_DEFAULT_dsm;
        cookie_tz = DATETIME_DEFAULT_TIMEZONE;
        
#if        TOPICS_SYSTEM_SUPPORT
        cookie_topics = CONFIGURE_SETTING_DEFAULT_topics;
        topicsoverride = CONFIGURE_SETTING_DEFAULT_toverride;
#endif

        s = getenv("HTTP_COOKIE");
        //if(s) print2log(s);
        
        if(s != NULL) {
                c = (char*)malloc(strlen(s) + 1);
                strcpy(c, s);

                // After this strget() we will have all %XX parsed ! So we should
                // disable %XX parsing
                if((ss = strget(c, COOKIE_NAME_STRING, COOKIE_MAX_LENGTH, '&')) != NULL) {
                        ss = (char*)realloc(ss, strlen(ss)+2);
                        strcat(ss,"|");
                        
                        
                        if((t = strget(ss, "name=", AUTHOR_NAME_LENGTH - 1, '|', 0))){
                                cookie_name = t;
                        }

                        
                        if((t = strget(ss, "seq=", 30, '|', 0))){
                                cookie_seq = t;
                        }



                        // read lsel (show type selection)
                        if((t = strget(ss, "lsel=", 3, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(( (*t) != '\0' && *st == '\0') && errno != ERANGE && tmp <= 2 && tmp >= 1)
                                {
                                        cookie_lsel = tmp;
                                }
                                free(t);
                        }
                
                        // read tc (thread count)
                        if((t = strget(ss, "tc=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        cookie_tc = tmp;
                                }
                                free(t);
                        }

                        if(cookie_tc > CONFIGURE_SETTING_MAX_tc)
                                                cookie_tc = CONFIGURE_SETTING_MAX_tc;

                        // read tt (time type)
                        if((t = strget(ss, "tt=", 3, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp <= 4 && tmp > 0)
                                {
                                        cookie_tt = tmp;
                                }
                                free(t);
                        }

                        // read tv (time value)
                        if((t  = strget(ss, "tv=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
				if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        cookie_tv = tmp;
                                }
                                free(t);
                        }
                                
                        if(cookie_tv > CONFIGURE_SETTING_MAX_hours/LENGTH_timetypes_hours[cookie_tt-1]) 
                                                cookie_tv = CONFIGURE_SETTING_MAX_hours/LENGTH_timetypes_hours[cookie_tt-1];

                        // read ss (style string)
                        if((t = strget(ss, "ss=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0 && tmp <= 4)
                                {
                                        cookie_ss = tmp;
                                }
                                free(t);
                        }
                

                        // read lt (last thread)
                        if((t = strget(ss, "lt=", 12, '|', 0))){
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        currentlt = tmp;
                                }
                                free(t);
                        }
                                        
                        // read fm (first message)
                        if((t = strget(ss, "ft=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        currentft = tmp;
                                }
                                free(t);
                        }
                                
                        // read lm (last message)
                        if((t = strget(ss, "lm=", 12, '|', 0))){
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        currentlm = tmp;
                                }
                                free(t);
                        }
                        
                        // read fm (first message)
                        if((t = strget(ss, "fm=", 12, '|', 0))){
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                        currentfm = tmp;
                                }
                                free(t);
                        }
                        
                        // read dsm (globally disable smiles, picture, and 2-d link bar)
                        if((t = strget(ss, "dsm=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp > 0)
                                {
                                cookie_dsm = tmp;
                                }
                                free(t);
                        }
                        
                        // read topics
                        if((t = strget(ss, "topics=", 20, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE)
                                {
                                cookie_topics = tmp;
                                }
                                free(t);
                        }

                        // read topics override
                        if((t = strget(ss, "tovr=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE)
                                {
                                topicsoverride = tmp;
                                }
                                free(t);
                        }

                        // read lann (last hided announce)
                        if((t = strget(ss, "lann=", 12, '|', 0))) {
                                tmp = strtoul(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE)
                                {
					currentlann = tmp;
					ReadLastAnnounceNumber(&tmp);
					if(currentlann > tmp) currentlann = tmp;
                                }
                                free(t);
                        }
                        
                        // read timezone
                        if((t = strget(ss, "tz=", 12, '|', 0))) {
                                tmp_signed = strtol(t, &st, 10);
                                if(((*t) != '\0' && *st == '\0') && errno != ERANGE && tmp_signed >= -12 && tmp_signed <= 12)
                                {
                                        cookie_tz = tmp_signed;
                                }
                                free(t);
                        }

                        free(ss);
        
                }
                
        
                if((ss = strget(c, COOKIE_SESSION_NAME, 12, '&')) && strcmp(ss, "on") == 0) {
                        GlobalNewSession = 0;
                        free(ss);
                }
                        
                free(c);
        }

        if(!cookie_name){
                cookie_name = (char*)malloc(1);
                cookie_name[0] = 0;
        }
        
        if(!cookie_seq) {
                cookie_seq = (char*)malloc(1);
                cookie_seq[0] = 0;
        }
}

const char *GetBoardUrl()
{
        static char *url;
        char *h, *s;

        if (!url) {
                if ( (h = getenv("HTTP_HOST")) == NULL)
                        printhtmlerror();
                s = getenv("SCRIPT_NAME");
                url = (char*) malloc(strlen("http://") + strlen(h) + (s || *s ? strlen(s) : 1) + 1);
                strcpy(url, "http://");
                strcat(url, h);
                strcat(url, (s || *s ? s : "/"));
        }
        return url;        
}

static void PrepareActionResult(int action, const char **c_par1, const char **c_par2)
{
        switch(action) {
        case MSG_CHK_ERROR_NONAME :
                *c_par1 = MESSAGEMAIN_add_no_name;
                *c_par2 = MESSAGEMAIN_add_no_name2;
                break;
        case MSG_CHK_ERROR_NOMSGHEADER:
                *c_par1 = MESSAGEMAIN_add_no_subject;
                *c_par2 = MESSAGEMAIN_add_no_subject2;
                break;
        case MSG_CHK_ERROR_NOMSGBODY:
                *c_par1 = MESSAGEMAIN_add_no_body;
                *c_par2 = MESSAGEMAIN_add_no_body2;
                break;
        case MSG_CHK_ERROR_BADSPELLING:
                *c_par1 = MESSAGEMAIN_add_spelling;
                *c_par2 = MESSAGEMAIN_add_spelling2;
                break;
#if BANNED_CHECK
        case MSG_CHK_ERROR_BANNED:
                *c_par1 = MESSAGEMAIN_add_banned;
                *c_par2 = MESSAGEMAIN_add_banned2;
                break;
#endif
        case MSG_CHK_ERROR_CLOSED:
                *c_par1 = MESSAGEMAIN_add_closed;
                *c_par2 = MESSAGEMAIN_add_closed2;
                break;
        case MSG_CHK_ERROR_INVALID_NUMBER:
                *c_par1 = MESSAGEMAIN_nonexistingmsg;
                *c_par2 = MESSAGEMAIN_nonexistingmsg2;
                break;
        case MSG_CHK_ERROR_INVALID_PASSW:
                *c_par1 = MESSAGEMAIN_incorrectpwd;
                *c_par2 = MESSAGEMAIN_incorrectpwd2;
                break;
       	case MSG_CHK_ERROR_EDIT_DENIED:
                *c_par1 = MESSAGEMAIN_edit_denied;
                *c_par2 = MESSAGEMAIN_edit_denied2;
                break;
        case MSG_CHK_ERROR_INVISIBLE:
                *c_par1 = MESSAGEMAN_invisible;
                *c_par2 = MESSAGEMAN_invisible2;
        default:
                *c_par1 = MESSAGEMAIN_unknownerr;
                *c_par2 = MESSAGEMAIN_unknownerr2;
                break;
        }
}

int ConvertHex(char *s, char *res)
{
        int i = 0;
        if(!s) return 0;
        while(*s != 0 && *(s+1) != 0) {
#define FROM_HEX(x) ((x >= 'A' && x <= 'F') ? (x-'A' + 10) : \
        (x >= 'a' && x <= 'f') ? (x-'a' + 10) : (x - '0'))
                res[i] = FROM_HEX(*s)*16 + FROM_HEX(*(s+1));
                i++;
                s+=2;
        }
        return i;
}

int main()
{
        char *deal, *st, *mesb;
        char *par; // parameters string
        char *tmp;
        int initok = 0;
        DB_Base DB;
        SMessage mes;

        if(!isEnoughSpace()) {
                printf("Content-type: text/html\n\n"
                "Sorry guys, no space left for DB - wwwconf shutting down.");
                exit(1);
        }

#ifdef RT_REDIRECT
#define BADURL "/board/"
#define GOODURL "http://board.rt.mipt.ru/"
        if((st = getenv(REQUEST_URI)) != NULL)
        {
                deal = (char*)malloc(strlen(st) + 2);
                strcpy(deal, st);
                //fprintf(stderr,"req uri: %s\n",deal);
                if (strncmp(deal, BADURL, strlen(BADURL)) == 0 ) { 
                        tmp = (char*)malloc(strlen(deal) + strlen(GOODURL) + 10);
                        sprintf(tmp,"%s%s",GOODURL,deal + strlen(BADURL));
                        //fprintf(stderr,"redir: %s\n",tmp);
                        HttpRedirect(tmp);
                        free(tmp);
                        goto End_part;
                }        
                free(deal);
        }
#endif

#define UA_LINKS        "Links"
#define UA_LYNX                "Lynx"
#define UA_NN202        "Mozilla/2.02"
        // will we use <DIV> or <DL>?
        if( ((st = getenv("HTTP_USER_AGENT")) != NULL) && (*st != '\0') )
        {
                deal = (char*)malloc(strlen(st) + 2);
                strcpy(deal, st);
                if ( (strncmp(deal, UA_LINKS, strlen(UA_LINKS)) == 0) ||
                         (strncmp(deal, UA_LYNX,  strlen(UA_LYNX))  == 0) ||
                         (strncmp(deal, UA_NN202, strlen(UA_NN202)) == 0) ) {
                        strcpy(DESIGN_open_dl, DESIGN_OP_DL);
                        strcpy(DESIGN_open_dl_grey, DESIGN_OP_DL);
                        strcpy(DESIGN_open_dl_white, DESIGN_OP_DL);
                        strcpy(DESIGN_close_dl, DESIGN_CL_DL);
                        strcpy(DESIGN_break, DESIGN_DD);
                        strcpy(DESIGN_threads_divider, DESIGN_THREADS_DIVIDER_HR);
                        initok = 1;
                }
                free(deal);
        }
        if (!initok) {
                        strcpy(DESIGN_open_dl, DESIGN_OP_DIV);
                        strcpy(DESIGN_open_dl_grey, DESIGN_OP_DIV_grey);
                        strcpy(DESIGN_open_dl_white, DESIGN_OP_DIV_white);
                        strcpy(DESIGN_close_dl, DESIGN_CL_DIV);
                        strcpy(DESIGN_break, DESIGN_BR);
                        strcpy(DESIGN_threads_divider, DESIGN_THREADS_DIVIDER_IMG);
        }


#if USE_LOCALE
        /* set locale */
        setlocale(LC_ALL, LANGUAGE_LOCALE);
#endif

        /* get cookie string, if available, and parse it */
        ParseCookie();
        

#if STABLE_TITLE == 0
        // set default title
        ConfTitle = (char*)malloc(strlen(TITLE_WWWConfBegining) + 1);
        strcpy(ConfTitle, TITLE_WWWConfBegining);
#endif



        // get parameters with we have been run
        if((st = getenv(QUERY_STRING)) != NULL)
        {
                deal = (char*)malloc(strlen(st) + 2);
                strcpy(deal, st);
        }
        else deal = NULL;
        
        if(deal == NULL || (strcmp(deal,"") == 0))
        {
                deal = (char*)malloc(20);
                strcpy(deal,"index");
        }

        // detect IP
        if((tmp = getenv(REMOTE_ADDR)) != NULL)
        {
                Cip = (char*)malloc(strlen(tmp) + 1);
                strcpy(Cip, tmp);
        }
        else {
                Cip = (char*)malloc(strlen(TAG_IP_NOT_DETECTED) + 1);
                strcpy(Cip, TAG_IP_NOT_DETECTED);
        }

        // translate IP
        // if it fails, we will have Nip = 0
        char *tst, *tms;
        tst = Cip;
        {
                for(register DWORD i = 0; i < 4; i++)
                {
                        if((tms = strchr(tst,'.')) != NULL || (tms = strchr(tst,'\0')) != NULL)
                        {
                                *tms = '\0';
                                ((char*)(&Nip))[i] = (unsigned char)atoi(tst);
                                tst = tms + 1;
                                if(i < 3) *tms = '.';
                        }
                        else break;
                }
        }
        if(Nip == 0) Nip = 1;

#if ACTIVITY_LOGGING_SUPPORT
        // user activity logging
        DWORD hostcnt, hitcnt;
        RegisterActivityFrom(Nip, hitcnt, hostcnt);
#endif

#if _DEBUG_ == 1
        //        print2log("Entering from : %s, deal=%s", Cip, deal);
#endif

        strcat(deal,"&");

        /************ get user info from session ************/
        {
                DWORD tmp[2];
                if(strlen(cookie_seq) == 16 &&
                        ConvertHex(cookie_seq, (char*)&tmp) == 8)
                {
                        tmp[0] = ntohl(tmp[0]);        // use network order due printf()
                        tmp[1] = ntohl(tmp[1]);
                        // if session code not zero let's open session
                        if(tmp[0] != 0 && tmp[1] != 0)
                        {
                                // try to open sequence
                                if(ULogin.CheckSession(tmp, Nip, 0))
                                        strcpy(cookie_name, ULogin.pui->username);
                        }
                }
        }
        
        //        
        // checking settings were saved in profile or cookies and  restoring them.
        //

        if(ULogin.LU.ID[0] && (ULogin.pui->Flags & PROFILES_FLAG_VIEW_SETTINGS)  ) { 

                currentdsm = ULogin.pui->vs.dsm;
                currenttopics = ULogin.pui->vs.topics;
                currenttv = ULogin.pui->vs.tv;
                currenttc = ULogin.pui->vs.tc;
                currentss = ULogin.pui->vs.ss;
                currentlsel = ULogin.pui->vs.lsel;
                currenttt = ULogin.pui->vs.tt;
                currenttz = ULogin.pui->vs.tz;
        }
        else{
                currentlsel = cookie_lsel;
                currenttc = cookie_tc;
                currenttt = cookie_tt;
                currenttv = cookie_tv;
                currentss = cookie_ss;
                currentdsm = cookie_dsm;
                currenttopics = cookie_topics;
                currenttz = cookie_tz;
        }

        // calculate minimal message print time
        if(currentlsel == 1) current_minprntime = time(NULL) - 3600*currenttv*LENGTH_timetypes_hours[currenttt-1];


        //security check
        if(ULogin.LU.ID[0] && (ULogin.LU.right & USERRIGHT_SUPERUSER) )
                print2log("Superuser: %s from %s - %s", ULogin.pui->username, Cip, deal);


        //==========================        
        // detecting user wishes %)
        //==========================
        
        
        
        if(strncmp(deal, "resetnew", 8) == 0) {
                // apply old last message value
                currentlm = currentfm;
                currentlt = currentft;

                // set new read time value
                PrintHTMLHeader(HEADERSTRING_REDIRECT_NOW | HEADERSTRING_NO_CACHE_THIS, MAINPAGE_INDEX);
                
                goto End_part;
        }

        if(strncmp(deal, "index", 5) == 0)
        {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0)
                {
                        printaccessdenied(deal);
                        goto End_part;
                }

                int is_xml = 0;

                if((st = strget(deal,"index=", 16, '&')) != NULL)
                {
                        if(strcmp(st, "rss") == 0){
                                is_xml = 1;
                                currentss = 5;
                        }
#if TOPICS_SYSTEM_SUPPORT
                        int entok = 0;
                        if(strcmp(st, "all") == 0) {
                                topicsoverride = TOPICS_COUNT + 50;
                                entok = 1;
                        } else {
                                errno = 0;
                                char *ss;
                                DWORD tmp = strtol(st, &ss, 10);
                                if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE || tmp > TOPICS_COUNT)
                                {
                                        // just print common index
                                }
                                else {
                                        topicsoverride = tmp;
                                        entok = 1;
                                }
                        }
                        if(entok) {
                                PrintHTMLHeader(HEADERSTRING_REDIRECT_NOW | HEADERSTRING_NO_CACHE_THIS, MAINPAGE_INDEX);
                                goto End_part;
                        }
#endif
                }

#if TOPICS_SYSTEM_SUPPORT && STABLE_TITLE == 0
                // add current topic to index title
                if (topicsoverride > 0 && topicsoverride <= TOPICS_COUNT) {
                        Tittle_cat(Topics_List[topicsoverride-1]);
                } else if (topicsoverride > (TOPICS_COUNT + 1)) {
                        Tittle_cat(MESSAGEMAIN_WELCOME_ALLTOPICS);
                }
#endif


                //        Apply new last message and thread value
                DWORD readed = DB.VIndexCountInDB();
                DWORD mtc;
                //        Read main threads count
                if(!DB.ReadMainThreadCount(&mtc)) mtc=0;
                
                currentfm = readed;
                currentft = mtc;
                //
                //        Here we make a decision about new session of "+"
                //
                if(GlobalNewSession)
                {
                        // apply old last message value
                        currentlm = currentfm;
                        currentlt = currentft;
                }

                if(!is_xml){
                PrintHTMLHeader(HEADERSTRING_REG_USER_LIST | HEADERSTRING_POST_NEW_MESSAGE |
                        HEADERSTRING_CONFIGURE | HEADERSTRING_WELCOME_INFO |
                        HEADERSTRING_ENABLE_RESETNEW | HEADERSTRING_DISABLE_END_TABLE, MAINPAGE_INDEX);

                //        Prepare information about new message count and dispaly mode */
                char displaymode[500];                // display message mode
                char displaynewmsg[500];        // new message info
                char topicselect[2000];                // topic select: MAYBE BUG HERE IF TOO MANY TOPICS
                char privmesinfo[500];                // private message info
                char activityloginfo[500];        // user activity info
                topicselect[0] = 0;
                privmesinfo[0] = 0;
                activityloginfo[0] = 0;

                DWORD a = currentfm - currentlm;
                DWORD t = currentft >= currentlt ? currentft - currentlt : 0;
                DWORD totalcount = DB.MessageCountInDB();
                if(a) sprintf(displaynewmsg, MESSAGEMAIN_WELCOME_NEWTHREADS, t, a, totalcount);
                else sprintf(displaynewmsg, MESSAGEMAIN_WELCOME_NONEWTHREADS, totalcount);

                // current settings in welcome message
                if(currentlsel == 1)
                {
                        // hours
                        sprintf(displaymode, MESSAGEMAIN_WELCOME_DISPLAYTIME, currenttv, MESSAGEHEAD_timetypes[currenttt-1]);
                }
                else
                {
                        // threads
                        sprintf(displaymode, MESSAGEMAIN_WELCOME_DISPLAYTHREADS, currenttc);
                }

#if        TOPICS_SYSTEM_SUPPORT
                {
                char tmp[500], sel[50], sel2[50];
                sel[0] = sel2[0] = 0;
                //        Prepare topic list
                if(topicsoverride == 0) strcpy(sel, LISTBOX_SELECTED);
                if(topicsoverride > TOPICS_COUNT) strcpy(sel2, LISTBOX_SELECTED);
                sprintf(topicselect, DESIGN_WELCOME_QUICKNAV, sel, MESSAGEMAIN_WELCOME_YOURSETTINGS, sel2, MESSAGEMAIN_WELCOME_ALLTOPICS);
                for(DWORD i = 0; i < TOPICS_COUNT; i++) {
                        if(Topics_List_map[i] == (topicsoverride - 1)) strcpy(sel, LISTBOX_SELECTED);
                        else sel[0] = 0; // ""
                        sprintf(tmp, "<OPTION VALUE=\"?index=%lu\"%s>%s</OPTION>\n", Topics_List_map[i]+1, sel, Topics_List[Topics_List_map[i]]);
                        strcat(topicselect, tmp);
                }
                strcat(topicselect, "</SELECT>");
                }
#endif

                // print info about personal messages
                if(ULogin.LU.ID[0] != 0 && ULogin.pui->persmescnt - ULogin.pui->readpersmescnt > 0) {
                        sprintf( privmesinfo, ", <A HREF=\"" MY_CGI_URL "?persmsg\" STYLE=\"text-decoration:underline;\"><FONT COLOR=RED>" MESSAGEMAIN_privatemsg_newmsgann " %d " \
                        MESSAGEMAIN_privatemsg_newmsgann1 "</FONT></A>", (ULogin.pui->persmescnt - ULogin.pui->readpersmescnt) );
                }

#if ACTIVITY_LOGGING_SUPPORT
                sprintf(activityloginfo, MESSAGEMAIN_ACTIVITY_STATVIEW, hitcnt, hostcnt);
#endif

                if(ULogin.LU.ID[0] == 0) {
                        printf(MESSAGEMAIN_WELCOME_START, PROFILES_MAX_USERNAME_LENGTH,
                                FilterHTMLTags(cookie_name, 1000, 0),
                                PROFILES_MAX_PASSWORD_LENGTH, displaynewmsg, displaymode,
                                activityloginfo, topicselect);
                }
                else {
                        char uname[1000];
                        DB.Profile_UserName(ULogin.pui->username, uname, 1);
                        printf(MESSAGEMAIN_WELCOME_LOGGEDSTART, uname, privmesinfo, displaynewmsg,
                                displaymode, activityloginfo, topicselect);
                }


                
                // Announce going here
                {
                        SGlobalAnnounce *ga;
                        DWORD cnt, i;
                        if(ReadGlobalAnnounces(&ga, &cnt) != ANNOUNCES_RETURN_OK) printhtmlerror();
                        if(cnt) {
                                char uname[1000], del[1000], *date;
                                int something_printed = 0;
                                for(i = 0; i < cnt; i++) {
                                        if(ga[i].Number > currentlann) {
                                                DB.Profile_UserName(ga[i].From, uname, 1);

                                                char *st = FilterHTMLTags(ga[i].Announce, MAX_PARAMETERS_STRING - 1);
                                                char *st1 = NULL;
                                                DWORD retflg;
                                                DWORD enabled_smiles = 0;
                                                if((currentdsm & CONFIGURE_dsm) == 0)
                                                        enabled_smiles = MESSAGE_ENABLED_SMILES;
                                                if(FilterBoardTags(st, &st1, 0, MAX_PARAMETERS_STRING - 1, 
                                                        enabled_smiles | MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE |
                                                        BOARDTAGS_EXPAND_ENTER, &retflg) == 0)
                                                {
                                                        st1 = st;
                                                        st = NULL;
                                                }

                                                if(((ULogin.LU.right & USERRIGHT_POST_GLOBAL_ANNOUNCE) != 0 && ga[i].UIdFrom ==
                                                        ULogin.LU.UniqID) || (ULogin.LU.right & USERRIGHT_SUPERUSER) != 0)
                                                {
                                                        sprintf(del, "<A HREF=\"" MY_CGI_URL "?ganndel=%lu\" "
                                                                "STYLE=\"text-decoration:underline;\">" MESSAGEMAIN_globann_delannounce
                                                                "</A> <A HREF=\"" MY_CGI_URL "?globann=%lu\" "
                                                                "STYLE=\"text-decoration:underline;\">" MESSAGEMAIN_globann_updannounce
                                                                "</A>", ga[i].Number, ga[i].Number);
                                        }        
                                                else del[0] = 0;

                                                date = ConvertTime(ga[i].Date);

                                                printf(DESIGN_GLOBALANN_FRAME, st1, MESSAGEMAIN_globann_postedby, uname, date, del);
                                                                                                        

                                                if(st) free(st);
                                                if(st1) free(st1);
                                                something_printed = 1;
                                        }
                                }
                                if(something_printed) {
                                        printf("<CENTER><A HREF=\"?rann=%lu\" STYLE=\"text-decoration:underline;\" class=cl>" MESSAGEMAIN_globann_hidenewann "</A></CENTER><BR>", ga[cnt-1].Number);
                                }
                                else {
                                        // show all announces
                                        printf("<CENTER><A HREF=\"?rann=0\" STYLE=\"text-decoration:underline;\" class=cl>" MESSAGEMAIN_globann_showall "(%lu)</A></CENTER><BR>", cnt);
                                }
                        }
                        if(ga) free(ga);
                } // of announces

                }//if not xml
                else{
                        printf("Content-type: application/rss+xml\n\n");
                        printf(RSS_START, GetBoardUrl());
                }
                
#if TOPICS_SYSTEM_SUPPORT
                DWORD oldct = currenttopics;                
                if(topicsoverride) {
                        if(topicsoverride > TOPICS_COUNT) currenttopics = 0xffffffff;        // all
                        else currenttopics = (1<<(topicsoverride-1));
                }
#endif

                DB.DB_PrintHtmlIndex(mtc);

#if TOPICS_SYSTEM_SUPPORT
                currenttopics = oldct;
#endif
        
                if (!is_xml)
                        PrintBottomLines();
                else
                        printf(RSS_END);
                        
                goto End_part;
        }
        
        if(strncmp(deal, "read", 4) == 0)
        {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0)
                {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"read=", 16, '&')) != NULL)
                {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        DWORD x;
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || (x = DB.TranslateMsgIndex(tmp)) == NO_MESSAGE_CODE)
                        {
                                printnomessage(deal);
                        }
                        else
                        {
                                // read message
                                if(!ReadDBMessage(x, &mes)) printhtmlerror();

                                /* allow read invisible message only to SUPERUSER */
                                if((mes.Flag & MESSAGE_IS_INVISIBLE) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0)) {
                                        printnomessage(deal);
                                }
                                else
                                {
#if STABLE_TITLE == 0
                                        // change title
                                        char *an;
                                        DWORD xtmp;
                                        if(FilterBoardTags(mes.MessageHeader, &an, 1,
							   MAX_PARAMETERS_STRING, mes.Flag | BOARDTAGS_CUT_TAGS, &xtmp) != 1)
                                                an = mes.MessageHeader;

#if TOPICS_SYSTEM_SUPPORT
                                        if (mes.Topics < TOPICS_COUNT && mes.Topics != 0)
                                                Tittle_cat(Topics_List[mes.Topics]);
#endif

                                        Tittle_cat(an);                                        

                                        if(an != mes.MessageHeader) free(an);
#endif
                                        // tmpxx contains vindex of parent message if thread is rolled.
                                        // if some sub-thread is rolled, tmpxx contains vindex of MAIN parent of thread
                                        
                                        DWORD tmpxx;
                                        if (mes.Flag & MESSAGE_COLLAPSED_THREAD && mes.Level > 0){
                                                SMessage parmes;
                                                if(!ReadDBMessage(mes.ParentThread, &parmes)) printhtmlerror();
                                                tmpxx = parmes.ViIndex;
                                        }
                                        else tmpxx = tmp;

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_ENABLE_TO_MESSAGE |
                                                ((currentdsm & CONFIGURE_shrp) ? HEADERSTRING_ENABLE_REPLY_LINK : 0), tmpxx, tmp);
                                        
                                        PrintMessageThread(&DB, tmp, mes.Flag, mes.UniqUserID);

                                        /* allow post to closed message only to SUPERUSER and USER  */
                                        if( (((mes.Flag & MESSAGE_IS_CLOSED) == 0 &&
                                                (ULogin.LU.right & USERRIGHT_CREATE_MESSAGE) )  ||
                                                (ULogin.LU.right & USERRIGHT_SUPERUSER) != 0 ) && 
                                                ((currentfm < MAX_DELTA_POST_MESSAGE) || 
                                                (tmp > (currentfm - MAX_DELTA_POST_MESSAGE)) || 
                                                (tmp == 0)) && ((currentdsm & CONFIGURE_shrp) == 0))
                                        {
                                                strcpy(mes.AuthorName, cookie_name);
                                                mes.MessageHeader[0] = 0;
                                                mesb = (char*)malloc(1);
                                                *mesb = 0;
                                                PrintMessageForm(&mes, mesb, tmp, ACTION_BUTTON_POST | ACTION_BUTTON_PREVIEW | ACTION_BUTTON_FAKEREPLY);
                                                free(mesb);
                                        }

                                        PrintBottomLines();
                                }
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }

	if(strncmp(deal, "xmlread", 7) == 0) {
		/* security check */
		if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
			printaccessdenied(deal);
			goto End_part;
		}

		printf("Cache-Control: no-cache\nContent-type: application/xml\n\n");
		printf("<?xml version=\"1.0\" encoding=\"windows-1251\"?><reply>");

		if((st = strget(deal,"xmlread=", 16, '&')) != NULL) {
			errno = 0;
			char *ss;
			DWORD tmp = strtol(st, &ss, 10);
			DWORD x;
			if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
			   tmp < 1 || (x = DB.TranslateMsgIndex(tmp)) == NO_MESSAGE_CODE) {
				printf("<error msg=\"nomsgcode\" />");
				printf("</reply>");
				goto End_part;
			}
			free(st);

			// read message
			if(!ReadDBMessage(x, &mes)) {
				printf("<error msg=\"ioerr\"/>");
				printf("</reply>");
				goto End_part;
			}

			/* allow read invisible message only to SUPERUSER */
			if((mes.Flag & MESSAGE_IS_INVISIBLE) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0)) {
				printf("<error msg=\"accden\"/>");
				printf("</reply>");
				goto End_part;
			}
			printf("<data>");
			DB.DB_PrintMessageBody(tmp, 1);
			printf("</data>");
		}
		else
			printf("<error msg=\"badreq\"/>");
			
		printf("</reply>");
		goto End_part;
	}

        if(strncmp(deal, "form", 4) == 0)
        {
                DWORD repnum = 0;
                
                // read form= parameter (if reply form is required)
                if((st = strget(deal,"form=", 16, '&')) != NULL)
                {
                        errno = 0;
                        char *ss;
                        repnum = strtol(st, &ss, 10);
                        DWORD x;
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                repnum < 1 || (x = DB.TranslateMsgIndex(repnum)) == NO_MESSAGE_CODE)
                        {
                                printnomessage(deal);
                                free(st);
                                goto End_part; 
                        }
                        free(st);
                }

                /* security check */
                if( (repnum && (ULogin.LU.right & USERRIGHT_CREATE_MESSAGE) == 0 ) ||
                        ( !repnum && (ULogin.LU.right & USERRIGHT_CREATE_MESSAGE_THREAD) == 0) ){
                                printaccessdenied(deal);
                                goto End_part;
                }
                
                if(!repnum) Tittle_cat(TITLE_Form);
                else Tittle_cat(TITLE_WriteReply);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                
                strcpy(mes.AuthorName, FilterHTMLTags(cookie_name, 1000, 0));
                mes.MessageHeader[0] = 0;
                mesb = (char*)malloc(1);
                *mesb = 0;
                PrintMessageForm(&mes, mesb, repnum,
                        repnum ? ACTION_BUTTON_POST | ACTION_BUTTON_PREVIEW | ACTION_BUTTON_FAKEREPLY :
                        ACTION_BUTTON_POST | ACTION_BUTTON_PREVIEW);
                PrintBottomLines();
                goto End_part;
        }
        
        
        if(strncmp(deal, "xpost", 5) == 0) {
                char *ss = NULL, *st = NULL;
                char *passw, *c_host;
                DWORD ROOT = 0;
                DWORD CFlags = 0, LogMeIn = 0;

                // read method GET post params
                if((ss = strget(deal, "xpost=", 16, '&')) != NULL ) {
                        errno = 0;
                        char *st;
                        ROOT = strtol(ss, &st, 10);
                        if((!(*ss != '\0' && *st == '\0')) || errno == ERANGE) {
                                printnomessage(deal);
                                goto End_part;
                        }
                        free(ss);
                }
                else {
                        printbadurl(deal);
                        goto End_part;
                }

                // Check security rights
                if(! ((ULogin.LU.right & USERRIGHT_CREATE_MESSAGE) && ROOT) &&
                        !((ULogin.LU.right & USERRIGHT_CREATE_MESSAGE_THREAD) && ROOT == 0))
                {
                        printaccessdenied(deal);
                        goto End_part;
                }
                        
                        
                // make IP address
                if ((tmp = getenv("HTTP_X_FORWARDED_FOR")) != NULL) print2log("proxy HTTP_X_FORWARDED_FOR %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_X_FORWARDED")) != NULL) print2log("proxy HTTP_X_FORWARDED %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_FORWARDED")) != NULL) print2log("proxy HTTP_FORWARDED %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_VIA")) != NULL) print2log("proxy HTTP_VIA %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_USER_AGENT_VIA")) != NULL) print2log("proxy HTTP_USER_AGENT_VIA %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_COMING_FROM")) != NULL) print2log("proxy HTTP_COMING_FROM %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_X_COMING_FROM")) != NULL) print2log("proxy HTTP_X_COMING_FROM %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_CLIENT_IP")) != NULL) print2log("proxy HTTP_CLIENT_IP %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_FROM")) != NULL) print2log("proxy HTTP_FROM %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_PROXY_CONNECTION")) != NULL) print2log("proxy HTTP_PROXY_CONNECTION %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_XROXY_CONNECTION")) != NULL) print2log("proxy HTTP_XROXY_CONNECTION %s - %s", tmp, deal);
                if ((tmp = getenv("HTTP_PROXY_AUTHORIZATION")) != NULL) print2log("proxy HTTP_PROXY_AUTHORIZATION %s - %s", tmp, deal);


                if(Nip != 1) {
                /*        // resolve
                        char*tmp;
                        // Exception! if forwareder for localhost or 127.0.0.1 - ignore forwarder
                        if((tmp = getenv("HTTP_X_FORWARDED_FOR")) != NULL &&
                                strcmp(tmp, "127.0.0.1") != 0 && strcmp(tmp, "localhost") != 0 &&
                                strncmp(tmp, "192.168", 7) != 0 && strncmp(tmp, "10.", 3) != 0)
                        {
                                // TODO: more detailed log here
                                print2log("proxy %s - %s", tmp, deal);
                                
                                // TODO: we need to resolve DNS here
                                strncpy(mes.HostName, tmp, HOST_NAME_LENGTH - 1);
                                mes.HostName[HOST_NAME_LENGTH - 1] = 0;
                        } else
                */ 
                        if(!IP2HostName(Nip, mes.HostName, HOST_NAME_LENGTH - 1))
                                strcpy(mes.HostName, Cip);



                }
                else strcpy(mes.HostName, TAG_IP_NOT_DETECTED);


                mes.IPAddr = Nip;

#if HTTP_REFERER_CHECK == 1
                char *useragent = getenv("HTTP_USER_AGENT");
                if(
                        !useragent || 
                        (strncmp(useragent, UA_LYNX, strlen(UA_LYNX)) &&
                        strncmp(useragent, UA_LINKS, strlen(UA_LINKS)))
                ) {
                        char *tts = getenv("HTTP_REFERER");
                        if(tts == NULL || strstr(tts, ALLOWED_HTTP_REFERER) == NULL) {
                                // TODO: more detailed error here
                                print2log("bad referer, tts='%s'", tts);
                                printbadurl(deal);
                                goto End_part;
                        }
                }
#endif

                // get parameters
                par = GetParams(MAX_PARAMETERS_STRING);

                // read antibot fields
                st = strget(par, "name=", AUTHOR_NAME_LENGTH - 1, '&');
                ss = strget(par, "email=", AUTHOR_NAME_LENGTH - 1, '&');
                if ((st && st[0]) || (ss && ss[0])) {
                        print2log("xpost: A bot is trapped: name=%s email=%s", st ? st : "", ss ? ss : "");
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_register_already_exit, MESSAGEMAIN_register_already_exit2, 0);
                        PrintBottomLines();
                        exit(0);
                }

                // read name
                st = strget(par,"amen=",  AUTHOR_NAME_LENGTH - 1, '&');
                if(st == NULL) {
                        strcpy(mes.AuthorName, "");
                }
                else {
                        strncpy(mes.AuthorName, st, AUTHOR_NAME_LENGTH - 1);
                        mes.AuthorName[AUTHOR_NAME_LENGTH - 1] = 0;
                        free(st);
                }
        
                // read password
                passw = strget(par,"pswd=", PROFILES_MAX_PASSWORD_LENGTH - 1, '&');

                // read subject
                st = strget(par,"subject=", MESSAGE_HEADER_LENGTH - 1, '&');
                if(st == NULL) {
                        strcpy(mes.MessageHeader, "");
                }
                else {
                        strncpy(mes.MessageHeader, FilterWhitespaces(st), MESSAGE_HEADER_LENGTH - 1);
                        mes.MessageHeader[MESSAGE_HEADER_LENGTH - 1] = 0;
                        free(st);
                }

                // read host (for edit)
                c_host = strget(par,"host=", HOST_NAME_LENGTH - 1, '&');

                // read msg body
                mesb = strget(par,"body=", MAX_PARAMETERS_STRING, '&');
                
                // read dct (disable WWWConf Tags)
                st = strget(par,"dct=", 10, '&');
                if(st != NULL) {
                        if(strcmp(st, "on") == 0) {
                                CFlags = CFlags | MSG_CHK_DISABLE_WWWCONF_TAGS;
                        }
                        free(st);
                }
                
                // read dst (disable smile codes)
                st = strget(par,"dst=", 10, '&');
                if(st != NULL) {
                        if(strcmp(st, "on") == 0) {
                                CFlags = CFlags | MSG_CHK_DISABLE_SMILE_CODES;
                        }
                        free(st);
                }
                
                // read wen (acknol. by email)
                st = strget(par,"wen=", 10, '&');
                if(st != NULL) {
                        // mail ackn. allowed ONLY for threads with ROOT == 0
                        if(strcmp(st, "on") == 0 && ROOT == 0) {
                                CFlags = CFlags | MSG_CHK_ENABLE_EMAIL_ACKNL;
                        }
                        free(st);
                }

                // read lmi (login me)
                st = strget(par,"lmi=", 10, '&');
                if(st != NULL) {
                        if(strcmp(st, "on") == 0) {
                                LogMeIn = 1;
                        }
                        free(st);
                }

                mes.Topics = 0;
#if TOPICS_SYSTEM_SUPPORT
                {
                        char *ss;
                        DWORD topicID;
                        if((ss = strget(par, "topic=", 10, '&')) != NULL) {
                                errno = 0;
                                char *st;
                                topicID = strtol(ss, &st, 10);
                                if((!(*ss != '\0' && *st == '\0')) || errno == ERANGE || topicID > TOPICS_COUNT - 1)
                                {
                                        // default topic
                                        mes.Topics = TOPICS_DEFAULT_SELECTED;
                                }
                                mes.Topics = topicID;
                                free(ss);
                        }
                }
#endif

                // get user action (post/edit/preview)
                int action = getAction(par);
                free(par);

                // init current error
                int cr;
                switch(action) {
                case ACTION_POST:
                        {
                                //*************************************************
                                //        post message
                                //*************************************************

                                // antispam
                                if(CheckPostfromIPValidity(Nip, POST_TIME_LIMIT) == 0) {
#if ENABLE_LOG
                                        print2log(LOG_SPAM_TRY, Cip, deal);
#endif
                                        Tittle_cat(TITLE_Spamtry);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                        PrintBoardError(MESSAGEMAIN_spamtry, MESSAGEMAIN_spamtry2, 0);
                                        PrintBottomLines();
                                        if(passw) free(passw);
                                        goto End_part;
                                }

                                // do not allow posts to old threads
                                if ( ROOT > 0 && ((DB.VIndexCountInDB() - ROOT) > MAX_DELTA_POST_MESSAGE) ) {
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, ROOT);
                                        PrintBoardError(MESSAGEMAIN_add_closed, MESSAGEMAIN_add_closed2, 0);
                                        PrintBottomLines();
                                        if(passw) free(passw);
                                        goto End_part;

                                }
                        
                                mes.Date = time(NULL);        //        set current time
                                mes.MDate = (time_t)0;        //        haven't modified
                                char *banreason = NULL;
                                if((cr = DB.DB_InsertMessage(&mes, ROOT, strlen(mesb), &mesb, CFlags, passw, &banreason)) != MSG_CHK_ERROR_PASSED)
                                {
                                        const char *c_ActionResult1, *c_ActionResult2;
                                        PrepareActionResult(cr, &c_ActionResult1, &c_ActionResult2);
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, ROOT);
                                        PrintBoardError(c_ActionResult1, c_ActionResult2, 0);

                                        if(cr == MSG_CHK_ERROR_BANNED && banreason) {
                                                // print the reason if it exists
                                                printf(DESIGN_BAN_REASON_STYLE, MESSAGEMAIN_BANNED_REASON, banreason);
                                        }
                                        if(banreason) free(banreason);
                                }
                                else {
                                        //        Mark that IP as already posted
                                        CheckPostfromIPValidity(Nip, POST_TIME_LIMIT);

                                        //        posted, set new cookie
                                        cookie_name = (char*)realloc(cookie_name, AUTHOR_NAME_LENGTH);
                                        strcpy(cookie_name, mes.AuthorName);


                                        //
                                        //        Log in user if requested
                                        //
                                        if(LogMeIn && (passw != NULL && *passw != 0)) {
                                                LogMeIn = 0;
                                                /* if session already opened - close it */
                                                if(ULogin.LU.ID[0] != 0)
                                                        ULogin.CloseSession(ULogin.LU.ID);

                                                if(ULogin.OpenSession(mes.AuthorName, passw, NULL, Nip, 0) == 1) {
                                                        // entered, ok
                                                        LogMeIn = 1;
                                                }
                                        }
                                        else LogMeIn = 0;

                                        //
                                        //        Check if message was posted to rolled thread than we should change ROOT to main root of thread
                                        //
                                        DWORD tmpxx;
                                        if((mes.Flag & MESSAGE_COLLAPSED_THREAD)) {
                                                SMessage parmes;
                                                if(!ReadDBMessage(mes.ParentThread, &parmes)) printhtmlerror();
                                                tmpxx = parmes.ViIndex;
                                        }
                                        else tmpxx = mes.ViIndex;

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                                tmpxx, ROOT);
                                        if(LogMeIn) {
                                                // if we have logged in also
                                                PrintBoardError(MESSAGEMAIN_add_ok_login, MESSAGEMAIN_add_ok2, HEADERSTRING_REFRESH_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_THREAD, mes.ViIndex);
                                        }
                                        else PrintBoardError(MESSAGEMAIN_add_ok, MESSAGEMAIN_add_ok2, HEADERSTRING_REFRESH_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_THREAD, mes.ViIndex);

                                        PrintBottomLines();

                                        if(passw != NULL) free(passw);

                                        goto End_part;
                                }
                        }
                        break;
                case ACTION_PREVIEW:
                        {
                                //**********************************************
                                //        preview message
                                //**********************************************
                                DWORD rf;
                                SProfile_UserInfo UI;

                                //
                                //        set some fields of message
                                //
                                mes.ParentThread = NO_MESSAGE_CODE;
                                mes.Date = time(NULL);
                                mes.MDate = 0;
                                mes.ViIndex = 0;

                                if(ULogin.LU.ID[0] != 0) {
                                        memcpy(&UI, ULogin.pui, sizeof(UI));
                                        strcpy(mes.AuthorName, UI.username);
                                }
                                else {
                                        /* default user */
                                        UI.right = DEFAULT_NOBODY_RIGHT;
                                        UI.UniqID = 0;
                                        UI.username[0] = 0;
                                }

                                if(UI.right & USERRIGHT_ALLOW_HTML) CFlags = CFlags | MSG_CHK_ALLOW_HTML;
                                mes.UniqUserID = UI.UniqID;

                                char *banreason = NULL;
                                if((cr = CheckSpellingBan(&mes, &mesb, &banreason, CFlags, &rf)) != MSG_CHK_ERROR_PASSED)
                                {
                                        const char *c_ActionResult1, *c_ActionResult2;
                                        PrepareActionResult(cr, &c_ActionResult1, &c_ActionResult2);
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, ROOT);
                                        PrintBoardError(c_ActionResult1, c_ActionResult2, 0);

                                        if(cr == MSG_CHK_ERROR_BANNED && banreason) {
                                                // print the reason if it exists
                                                printf(DESIGN_BAN_REASON_STYLE, MESSAGEMAIN_BANNED_REASON, banreason);
                                        }
                                }
                                if(banreason) free(banreason);

                                mes.Flag = rf;
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                printf(DESIGN_PREVIEW_PREVIEWMESSAGE, MESSAGEHEAD_preview_preview_message);

                                // fix message body size for print preview
                                mes.msize = 1;        // any constant greater than zero

                                DB.PrintHtmlMessageBody(&mes, mesb);

                                printf(DESIGN_PREVIEW_CHANGEMESSAGE, MESSAGEHEAD_preview_change_message);

                                PrintMessageForm(&mes, mesb, ROOT, ACTION_BUTTON_POST | ACTION_BUTTON_PREVIEW, CFlags);
                                free(mesb);
                        }
                        break;
                case ACTION_EDIT:
                        {
                                if((ULogin.pui->right & USERRIGHT_SUPERUSER) && c_host && c_host[0] != 0) {
                                        strcpy(mes.HostName, c_host);
                                        mes.HostName[HOST_NAME_LENGTH - 1] = 0;
                                }
                                mes.IPAddr = Nip;

                                char *banreason;
                                if((cr = DB.DB_ChangeMessage(ROOT, &mes, &mesb, CFlags, &banreason)) != MSG_CHK_ERROR_PASSED)
                                {
                                        const char *c_ActionResult1, *c_ActionResult2;
                                        PrepareActionResult(cr, &c_ActionResult1, &c_ActionResult2);
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, ROOT);
                                        PrintBoardError(c_ActionResult1, c_ActionResult2, 0);

                                        if(cr == MSG_CHK_ERROR_BANNED && banreason) {
                                                // print the reason if it exists
                                                printf(DESIGN_BAN_REASON_STYLE, MESSAGEMAIN_BANNED_REASON, banreason);
                                        }
                                }
                                else {
                                        //        posted, set new cookie
                                        cookie_name = (char*)realloc(cookie_name, AUTHOR_NAME_LENGTH);
                                        strcpy(cookie_name, mes.AuthorName);
                                        
                                        // Check if message was posted to rolled thread then we should change ROOT to main root of thread
                                        DWORD tmpxx;
                                        if (mes.Flag & MESSAGE_COLLAPSED_THREAD) {
                                                SMessage parmes;
                                                if (!ReadDBMessage(mes.ParentThread, &parmes))
                                                        printhtmlerror();
                                                tmpxx = parmes.ViIndex;
                                        } else
                                                tmpxx = ROOT;
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmpxx, ROOT);
                                        PrintBoardError(MESSAGEMAIN_add_ok, MESSAGEMAIN_add_ok2,
                                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_THREAD, ROOT);
                                }
                                if (mesb) {
                                        free(mesb);
                                        mesb = NULL;
                                }
                        }
                        break;
                default:
                        {
#if ENABLE_LOG > 1
                                print2log("Unknown parameter during message post, st = %s", st);
#endif
                                if(st) free(st);
                                if(passw) free(passw);
                                goto End_URLerror;
                        }
                }// switch(action)
                if(passw) free(passw);

                PrintBottomLines();
                goto End_part;
        }

        
        if(strncmp(deal, "configure", 9) == 0) {
                st = NULL;
                if((st = strget(deal,"configure=", 16, '&')) != NULL) {
                        if(strcmp(st, "action") == 0) {
                                free(st);
                                // get parameters
                                par = GetParams(MAX_PARAMETERS_STRING);

                                currentss = CONFIGURE_SETTING_DEFAULT_ss;
                                currenttt = CONFIGURE_SETTING_DEFAULT_tt;
                                currenttv = CONFIGURE_SETTING_DEFAULT_tv;
                                currenttc = CONFIGURE_SETTING_DEFAULT_tc;
                                currentlsel = CONFIGURE_SETTING_DEFAULT_lsel;
                                currentdsm = CONFIGURE_SETTING_DEFAULT_dsm;
                                currenttz = DATETIME_DEFAULT_TIMEZONE;
                                
                                if(par != NULL) {

#define READ_PARAM_MASK(param, var, mask) {                             \
        char *ss = strget(par, param, 20, '&');                         \
        if(ss && !strcmp(ss, "1"))                                      \
                var |= mask;                                            \
        else                                                            \
                var &= (~mask);                                         \
        free(ss);                                                       \
}

                                        // read disable smiles
                                        READ_PARAM_MASK("dsm=", currentdsm, CONFIGURE_dsm);
                                        // read onh (disable own nick highlighing)
                                        READ_PARAM_MASK("onh=", currentdsm, CONFIGURE_onh);
                                        // read plu (enable + acting like an href)
                                        READ_PARAM_MASK("plu=", currentdsm, CONFIGURE_plu);
                                        // read host (disable host displaying)
                                        READ_PARAM_MASK("host=", currentdsm, CONFIGURE_host);
                                        // read alt nick displaying
                                        READ_PARAM_MASK("nalt=", currentdsm, CONFIGURE_nalt);
                                        // read signature disable
                                        READ_PARAM_MASK("dsig=", currentdsm, CONFIGURE_dsig);
                                        // read show reply form
                                        READ_PARAM_MASK("shrp=", currentdsm, CONFIGURE_shrp);
                                        // read disable colors
                                        READ_PARAM_MASK("clr=", currentdsm, CONFIGURE_clr);
                                        // read disable bot defense
                                        READ_PARAM_MASK("bot=", currentdsm, CONFIGURE_bot);

#define READ_PARAM_NUM(param, var, vardefault) {                        \
       char *st, *ss = strget(par, param, 20, '&');                     \
       errno = 0;                                                       \
       var = vardefault;                                                \
       if (ss && *ss) {                                                 \
               DWORD tmp = strtol(ss, &st, 10);                         \
               if (!*st || !errno)                                      \
                       var = tmp;                                       \
       }                                                                \
       free(ss);                                                        \
}

                                        // read lsel (show type selection)
                                        READ_PARAM_NUM("lsel=", currentlsel, CONFIGURE_SETTING_DEFAULT_lsel);
                                        if(currentlsel != 1 && currentlsel != 2)
                                                currentlsel = CONFIGURE_SETTING_DEFAULT_lsel;

                                        // read tc (thread count)
                                        READ_PARAM_NUM("tc=", currenttc, CONFIGURE_SETTING_DEFAULT_tc);
                                        if(currenttc <= 0)
                                                currenttc = CONFIGURE_SETTING_DEFAULT_tc;
                                        if(currenttc > CONFIGURE_SETTING_MAX_tc)
                                                currenttc = CONFIGURE_SETTING_MAX_tc;

                                        // read tt (read time type)
                                        READ_PARAM_NUM("tt=", currenttt, CONFIGURE_SETTING_DEFAULT_tt);
                                        if(currenttt < 1 || currenttt > 4)
                                                currenttt = CONFIGURE_SETTING_DEFAULT_tt;
                                        // read tv (time value)
                                        READ_PARAM_NUM("tv=", currenttv, CONFIGURE_SETTING_DEFAULT_tv);
                                        if(currenttv <= 0)
                                                currenttv = CONFIGURE_SETTING_DEFAULT_tv;
                                        if(currenttv > CONFIGURE_SETTING_MAX_hours/LENGTH_timetypes_hours[currenttt-1]) 
                                                currenttv = CONFIGURE_SETTING_MAX_hours/LENGTH_timetypes_hours[currenttt-1];


                                        // read ss (read time type)
                                        READ_PARAM_NUM("ss=", currentss, CONFIGURE_SETTING_DEFAULT_ss);
                                        if(currentss < 1 || currentss > 4)
                                                currentss = CONFIGURE_SETTING_DEFAULT_ss;
                                        READ_PARAM_NUM("tz=", currenttz, DATETIME_DEFAULT_TIMEZONE);
                                        if(currenttz < -12 || currenttz > 12)
                                                currenttz = DATETIME_DEFAULT_TIMEZONE;

#if        TOPICS_SYSTEM_SUPPORT
                                        // read topics that should be displayed
                                        {
                                                currenttopics = 0;
                                                DWORD i;
                                                for(i = 0; i < TOPICS_COUNT; i++)
                                                {
                                                        char *ss;
                                                        char st[30];
                                                        sprintf(st, "topic%lu=", i);
                                                        if((ss = strget(par, st,  3, '&')) != NULL)
                                                        {
                                                                if(strcmp(ss, "on") == 0)
                                                                {
                                                                        currenttopics |= (1<<i);
                                                                }
                                                                free(ss);
                                                        }
                                                }
                                        }
#endif

                                        //
                                        // saving values - profile or cookie way
                                        //

                                        if((ULogin.LU.ID[0] != 0) && (ULogin.pui->Flags & PROFILES_FLAG_VIEW_SETTINGS) ){
                                                        
                                                ULogin.pui->vs.dsm = currentdsm;
                                                ULogin.pui->vs.topics = currenttopics;
                                                ULogin.pui->vs.tv = currenttv;
                                                ULogin.pui->vs.tc = currenttc;
                                                ULogin.pui->vs.ss = currentss;
                                                ULogin.pui->vs.lsel = currentlsel;
                                                ULogin.pui->vs.tt = currenttt;
                                                ULogin.pui->vs.tz = currenttz;
                                
                                                CProfiles *uprof;
                                                uprof = new CProfiles();
                                                uprof->ModifyUser(ULogin.pui, NULL, NULL);
                                                delete uprof;
                                        }
                                        else{
                                                // settings are not in profile. so new values should be in cookies
                                                cookie_lsel = currentlsel;
                                                cookie_tc = currenttc;
                                                cookie_tt = currenttt;
                                                cookie_tv = currenttv;
                                                cookie_ss = currentss;
                                                cookie_dsm = currentdsm;
                                                cookie_topics = currenttopics;
                                                cookie_tz = currenttz;
                                         }
                                        
                                }
                                free(par);
                                
                                
                                //
                                // Redirect user to the index page
                                //
                                PrintHTMLHeader(HEADERSTRING_REDIRECT_NOW | HEADERSTRING_NO_CACHE_THIS, MAINPAGE_INDEX);

                                goto End_part;
                        }
                        free(st);

                        Tittle_cat(TITLE_Configure);

                        /* print configuration form */
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintConfig();
                        PrintBottomLines();
                        /****************************/
                }
                goto End_part;
        }

        if(strncmp(deal, "login", 5) == 0) {
                st = NULL;
                if((st = strget(deal,"login=", 16, '&')) != NULL) {
                        
                        if(strcmp(st, "action") == 0) {
                                free(st);
                                // check for User name and password
                                // get parameters
                                par = GetParams(MAX_PARAMETERS_STRING);

                                /***************************************/
                                int disableipcheck = 0;
                                char *ipchk;
                                if((ipchk = strget(par, "ipoff=", 10, '&')) != NULL) {
                                        if(strcmp(ipchk, "1") == 0) {
                                                disableipcheck = 1;
                                        }
                                        free(ipchk);
                                }
                                st = strget(par,"mname=", PROFILES_MAX_USERNAME_LENGTH - 1, '&');
                                if(st != NULL) {
                                        char *ss;
                                        ss = strget(par,"mpswd=", PROFILES_MAX_PASSWORD_LENGTH - 1, '&');
                                        if(ss != NULL) {
                                                /* if session already opened - close it */
                                                if(ULogin.LU.ID[0] != 0)
                                                        ULogin.CloseSession(ULogin.LU.ID);

                                                if(ULogin.OpenSession(st, ss, NULL, Nip, disableipcheck) == 1) {
                                                        print2log("User '%s' was logged in (%s)", ULogin.pui->username, getenv(REMOTE_ADDR));

                                                        //        Prepare conference login greetings
                                                        char boardgreet[1000];
                                                        const char *greetnames[4] = {
                                                                MESSAGEMAIN_login_helloday,
                                                                MESSAGEMAIN_login_helloevn,
                                                                MESSAGEMAIN_login_hellonight,
                                                                MESSAGEMAIN_login_hellomor
                                                        };
                                                        int cur = 2;
                                                        time_t t = time(NULL) + 3600*currenttz;
                                                        tm *tt = gmtime(&t);
                                                        if(tt->tm_hour >= 6  && tt->tm_hour < 10) cur = 3;
                                                        if(tt->tm_hour >= 10 && tt->tm_hour < 18) cur = 0;
                                                        if(tt->tm_hour >= 18 && tt->tm_hour < 22) cur = 1;
                                                        sprintf(boardgreet, MESSAGEMAIN_login_ok, greetnames[cur], FilterHTMLTags(ULogin.pui->username, PROFILES_MAX_USERNAME_LENGTH, 0));


                                                        Tittle_cat(TITLE_Login);

                                                        // entered, set new cookie
                                                        cookie_name = (char*)realloc(cookie_name, AUTHOR_NAME_LENGTH);
                                                        strcpy(cookie_name, st);


                                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_LOGIN | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                        PrintBoardError(boardgreet, MESSAGEMAIN_login_ok2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                                        PrintBottomLines();

                                                        free(ss);
                                                        free(st);
                                                        goto End_part;
                                                }
                                                free(ss);
                                        }
                                        free(st);
                                }
print2log("incor pass %s", par);
                                printpassworderror(deal);
                                goto End_part;
                        }

                        if(strcmp(st, "lostpasswform") == 0) {
                                free(st);

                                Tittle_cat(TITLE_LostPassword);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_LOGIN, MAINPAGE_INDEX);
                                printf(DESIGN_LOSTPASSW_HEADER, MESSAGEMAIN_lostpassw_header);
                                PrintLostPasswordForm();
                                PrintBottomLines();

                                goto End_part;
                        }

                        if(strcmp(st, "lostpasswaction") == 0) {
                                free(st);

                                // check for User name and email address
                                // get parameters
                                par = GetParams(MAX_PARAMETERS_STRING);

                                /***************************************/
                                st = strget(par,"mname=", PROFILES_MAX_USERNAME_LENGTH - 1, '&');
                                if(st != NULL) {
                                        char *ss;
                                        ss = strget(par,"memail=", PROFILES_FULL_USERINFO_MAX_EMAIL - 1, '&');
                                        if(ss != NULL) {
                                                CProfiles uprof;
                                                SProfile_FullUserInfo Fui;
                                                SProfile_UserInfo ui;
                                                if(uprof.GetUserByName(st, &ui, &Fui, NULL) == PROFILE_RETURN_ALLOK) {
                                                        if(strcmp(Fui.Email, ss) == 0) {
                                                                //
                                                                //        We should send password to the user
                                                                //

                                                                Tittle_cat(TITLE_PasswordSent);

                                                                //
                                                                //        Send email
                                                                //
                                                                {
                                                                        char subj[1000], bdy[10000];
                                                                                                                                                
                                                                        sprintf(subj, MAILACKN_LOSTPASS_SUBJECT, st);
                                                                        
                                                                        sprintf(bdy, MAILACKN_LOSTPASS_BODY, st, ui.password);
                                                                        //print2log("will send message now %s", bdy);
                                                                        wcSendMail(Fui.Email, subj, bdy);
                                                                        
                                                                        print2log("Password was sent to %s", Fui.Email);
                                                                }

                                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                                PrintBoardError(MESSAGEMAIN_lostpassw_ok, MESSAGEMAIN_lostpassw_ok2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                                                PrintBottomLines();

                                                                free(ss);
                                                                free(st);
                                                                goto End_part;
                                                        }
                                                }
                                        }
                                }

                                Tittle_cat(TITLE_LostPassword);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_LOGIN, MAINPAGE_INDEX);
                                printf(DESIGN_LOSTPASSW_HEADER, MESSAGEMAIN_lostpassw_hretry);
                                PrintLostPasswordForm();
                                PrintBottomLines();

                                goto End_part;
                        }

                        if(strcmp(st, "logoff") == 0) {
                                free(st);

                                if(ULogin.LU.ID[0] == 0) {
                                        /* not logged yet */
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                        PrintBoardError(MESSAGEMAIN_logoff_not_logged_in, MESSAGEMAIN_logoff_not_logged_in2, 0);
                                        PrintBottomLines();
                                        goto End_part;
                                }

                                print2log("User '%s' was logged out (%s)", ULogin.pui->username, getenv(REMOTE_ADDR));

                                /* close sequence */
                                ULogin.CloseSession(ULogin.LU.ID);
                                
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                
                                PrintBoardError(MESSAGEMAIN_logoff_ok, MESSAGEMAIN_logoff_ok, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                
                                PrintBottomLines();
                                goto End_part;
                        }
                        free(st);
                }

                Tittle_cat(TITLE_Login);

                /******* print login form *******/
                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_LOGIN, MAINPAGE_INDEX);
                printf(DESIGN_MODERATOR_ENTER_HEADER, MESSAGEMAIN_login_login_header);
                PrintLoginForm();
                PrintBottomLines();
                /********************************/
                goto End_part;
        }
        
        if(strncmp(deal, "close", 5) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_CLOSE_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"close=", 16, '&')) != NULL) {
                        char *ss;
                        DWORD midx;
                        errno = 0;
                        DWORD tmp = strtol(st, &ss, 10);
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || ((midx = DB.TranslateMsgIndex(tmp)) == NO_MESSAGE_CODE) ) {
                                printnomessage(deal);
                        }
                        else {
                                /* Security check for own message or USERRIGHT_SUPERUSER */

                                /******** read message ********/
                                if(!ReadDBMessage(midx, &mes)) printhtmlerror();
                                /* closing by author allowed in main thread only ! */
                                if(ULogin.LU.ID[0] != 0 && ((mes.UniqUserID == ULogin.LU.UniqID && mes.Level == 0) || (ULogin.LU.right & USERRIGHT_SUPERUSER))) {

                                        Tittle_cat(TITLE_ClosingMessage);

                                        DB.DB_ChangeCloseThread(tmp, 1);
                                        print2log("Message %d (%s (by %s)) was closed by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmp);
                                        PrintBoardError(MESSAGEMAIN_threadwasclosed, MESSAGEMAIN_threadwasclosed2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                        PrintBottomLines();
                                }
                                else printaccessdenied(deal);
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }
        
        
        if(strncmp(deal, "hide", 4) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"hide=", 16, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || DB.TranslateMsgIndex(tmp) == NO_MESSAGE_CODE) {
                                printnomessage(deal);
                        }
                        else {
                                DB.DB_ChangeInvisibilityThreadFlag(tmp, 1);
                                if(!ReadDBMessage(DB.TranslateMsgIndex(tmp), &mes)) printhtmlerror();
                                print2log("Message %d (%s (by %s)) was hided by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);

                                Tittle_cat(TITLE_HidingMessage);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmp);
                                PrintBoardError(MESSAGEMAIN_threadchangehided, MESSAGEMAIN_threadchangehided2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                PrintBottomLines();
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }
        
        
        if(strncmp(deal, "unhide", 6) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"unhide=", 16, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || DB.TranslateMsgIndex(tmp) == NO_MESSAGE_CODE) {
                                printnomessage(deal);
                        }
                        else {
                                Tittle_cat(TITLE_HidingMessage);

                                DB.DB_ChangeInvisibilityThreadFlag(tmp, 0);
                                if(!ReadDBMessage(DB.TranslateMsgIndex(tmp), &mes)) printhtmlerror();
                                print2log("Message %d (%s (by %s)) was unhided by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmp);
                                PrintBoardError(MESSAGEMAIN_threadchangehided, MESSAGEMAIN_threadchangehided2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                PrintBottomLines();
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }
        
        if(strncmp(deal, "unclose", 7) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_OPEN_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"unclose=", 16, '&')) != NULL) {
                        char *ss;
                        DWORD midx;
                        errno = 0;
                        DWORD tmp = strtol(st, &ss, 10);
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || ((midx = DB.TranslateMsgIndex(tmp)) == NO_MESSAGE_CODE) ) {
                                printnomessage(deal);
                        }
                        else {
                                /* Security check for own message or USERRIGHT_SUPERUSER */

                                /******** read message ********/
                                if(!ReadDBMessage(midx, &mes)) printhtmlerror();
                                if(ULogin.LU.ID[0] != 0 && (mes.UniqUserID == ULogin.LU.UniqID || (ULogin.LU.right & USERRIGHT_SUPERUSER))) {

                                        Tittle_cat(TITLE_ClosingMessage);

                                        DB.DB_ChangeCloseThread(tmp, 0);
                                        if(!ReadDBMessage(DB.TranslateMsgIndex(tmp), &mes)) printhtmlerror();
                                        print2log("Message %d (%s (by %s)) was opened by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmp);
                                        PrintBoardError(MESSAGEMAIN_threadwasclosed, MESSAGEMAIN_threadwasclosed2,
                                                HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                        PrintBottomLines();
                                }
                                else printaccessdenied(deal);
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }

        if(strncmp(deal, "roll", 4) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"roll=", 16, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || DB.TranslateMsgIndex(tmp) == NO_MESSAGE_CODE) {
                                printnomessage(deal);
                        }
                        else {

                                Tittle_cat(TITLE_RollMessage);

                                DB.DB_ChangeRollThreadFlag(tmp);
                                if(!ReadDBMessage(DB.TranslateMsgIndex(tmp), &mes)) printhtmlerror();
                                print2log("Message %d (%s (by %s)) was (un)rolled by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                        tmp);
                                PrintBoardError(MESSAGEMAIN_threadrolled, MESSAGEMAIN_threadrolled2,
                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                PrintBottomLines();
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }
        
        if(strncmp(deal, "delmsg", 6) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"delmsg=", 16, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        
                        if((!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || DB.TranslateMsgIndex(tmp) == NO_MESSAGE_CODE) {
                                printnomessage(deal);
                        }
                        else {

                                Tittle_cat(TITLE_DeletingMessage);

                                if(!ReadDBMessage(DB.TranslateMsgIndex(tmp), &mes)) printhtmlerror();
                                print2log("Message %d (%s (by %s)) was deleted by %s", tmp, mes.MessageHeader, mes.AuthorName, ULogin.pui->username);
                                DB.DB_DeleteMessages(tmp);
                                
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, tmp);
                                PrintBoardError(MESSAGEMAIN_threaddeleted, MESSAGEMAIN_threaddeleted2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                PrintBottomLines();
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }
        
        if(strncmp(deal, "changemsg", 9) == 0) {
                // precheck security
                if((ULogin.LU.right & USERRIGHT_MODIFY_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"changemsg=", 16, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                        DWORD tmp = strtol(st, &ss, 10);
                        DWORD midx;
                        if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||
                                tmp < 1 || ((midx = DB.TranslateMsgIndex(tmp)) == NO_MESSAGE_CODE)) {
                                printnomessage(deal);
                        }
                        else {
                                //
                                //        read message
                                //
                                if(!ReadDBMessage(midx, &mes)) printhtmlerror();

                                //
                                //        security check
                                //
                                if(!( (ULogin.LU.right & USERRIGHT_SUPERUSER) || (        // admin ?
                                                ((mes.Flag & MESSAGE_IS_INVISIBLE) == 0) && // not hided
                                                ((mes.Flag & MESSAGE_IS_CLOSED) == 0) &&        // and not closed
                                                (ULogin.LU.right & USERRIGHT_MODIFY_MESSAGE) &&        // can modify?
                                                (mes.UniqUserID == ULogin.LU.UniqID) )        // message posted by this user
                                        ))
                                {
                                        Tittle_cat(TITLE_Error);
                                        printaccessdenied(deal);
                                }
                                else {
#if STABLE_TITLE == 0
                                        // set title - change title to change message
                                        char *aheader = FilterBiDi(mes.MessageHeader);
                                        ConfTitle = (char*)realloc(ConfTitle,
                                                            strlen(ConfTitle) + strlen(TITLE_divider)
                                                            + strlen(TITLE_ChangingMessage) + strlen(aheader) + 1);
                                        strcat(ConfTitle, TITLE_divider);
                                        strcat(ConfTitle, TITLE_ChangingMessage);
                                        strcat(ConfTitle, aheader);
                                        if (aheader)
                                                free(aheader);
#endif
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, tmp);

                                        PrintMessageThread(&DB, tmp, mes.Flag, mes.UniqUserID);

                                        char *mesb = (char*)malloc(mes.msize + 1);
                                        mesb[0] = 0;

                                        //
                                        //        Read message body
                                        //
                                        if(!ReadDBMessageBody(mesb, mes.MIndex, mes.msize))
                                                printhtmlerrorat(LOG_UNABLETOLOCATEFILE, F_MSGBODY);

                                        PrintMessageForm(&mes, mesb, tmp, ACTION_BUTTON_EDIT);

                                        free(mesb);

                                        PrintBottomLines();
                                }
                        }
                        free(st);
                }
                else goto End_URLerror;
                goto End_part;
        }

        if(strncmp(deal, "uinfo", 5) == 0) {
                char *name;
                if((name = strget(deal,"uinfo=", PROFILES_MAX_USERNAME_LENGTH, '&')) != NULL) {
#if STABLE_TITLE == 0

                        char *nickname;
                        nickname = FilterHTMLTags(name, 1000);
                        
                        ConfTitle = (char*)realloc(ConfTitle, strlen(ConfTitle) + 2*strlen(TITLE_divider) + strlen(TITLE_ProfileInfo) + strlen(nickname) + 1);
                        strcat(ConfTitle, TITLE_divider);
                        strcat(ConfTitle, TITLE_ProfileInfo);
                        strcat(ConfTitle, TITLE_divider);
                        strcat(ConfTitle, nickname);

                        free(nickname);
#endif

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintAboutUserInfo(name);
                        
                        PrintBottomLines();
                        free(name);
                }
                else goto End_URLerror;
                goto End_part;
        }

        if(strncmp(deal, "searchword", 10) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                char *ss;
                DWORD start = 0;
                if((ss = strget(deal,"searchword=", 255 - 1, '&')) != NULL) {
                        if((st = strget(deal,"start=", 60, '&')) != NULL) {
                                errno = 0;
                                char *ss;
                                start = strtol(st, &ss, 10);
                                if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE || tmp == 0) {
                                        start = 1;
                                }
                                free(st);
                        }
                        else start = 1;

                        if(strlen(ss) > 0) {
                                Tittle_cat(TITLE_Search);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_SEARCH, MAINPAGE_INDEX | HEADERSTRING_NO_CACHE_THIS);
                                PrintSearchForm(ss, &DB);
                                if(strlen(ss) >= SEARCHER_MIN_WORD) {
                                        CMessageSearcher *ms = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
                                        if(ms->errnum == SEARCHER_RETURN_ALLOK) {
                                                DWORD c;
                                                DWORD *vmsg = ms->SearchMessagesByPattern(ss, &c);
                                                printf(DESIGN_SEARCH_SEARCH_STR_WAS, MESSAGEMAIN_search_search_str, ms->srch_str);
                                                if(c != 0) {
                                                        // print count of found messages
                                                        printf(DESIGN_SEARCH_RESULT, MESSAGEMAIN_search_result1, c, MESSAGEMAIN_search_result2);
                                                }
                                                else {
                                                        // Nothing have been found
                                                        printf(DESIGN_SEARCH_NO_RESULT, MESSAGEMAIN_search_result1, MESSAGEMAIN_search_result_nothing);
                                                }

                                                //        Check and adjust start
                                                if(c <= (start-1)*SEARCH_MES_PER_PAGE_COUNT) {
                                                        start = c/SEARCH_MES_PER_PAGE_COUNT + 1;
                                                }

                                                DWORD oldc = c;
                                                if(c > SEARCH_MES_PER_PAGE_COUNT) {
                                                        char *wrd = CodeHttpString(ss, 0);
                                                        if(wrd) {
                                                                printf("<CENTER>" MESSAGEMAIN_search_result_pages);
                                                                DWORD max = (c/SEARCH_MES_PER_PAGE_COUNT) + 
                                                                        (((c % SEARCH_MES_PER_PAGE_COUNT) == 0)? 0: 1);
                                                                for(DWORD i = 0; i < max; i++) {
                                                                        if(i > 0 && (i % 20) == 0) printf("<BR>");
                                                                        if(i != start - 1) printf("&nbsp;<A HREF=\"?searchword=%s&amp;start=%lu\">%lu</A>&nbsp;", wrd, i+1, i+1);
                                                                        else printf("<B>&nbsp;%lu&nbsp;</B>", i+1);
                                                                }
                                                                printf("</CENTER>");
                                                        }
                                                }

                                                if(c - (start-1)*SEARCH_MES_PER_PAGE_COUNT > SEARCH_MES_PER_PAGE_COUNT) c = SEARCH_MES_PER_PAGE_COUNT;
                                                else c = c - (start-1)*SEARCH_MES_PER_PAGE_COUNT;
                                                DB.PrintHtmlMessageBufferByVI(vmsg + (start-1)*SEARCH_MES_PER_PAGE_COUNT, c);
                                                free(vmsg);

                                                c = oldc;
                                                if(c > SEARCH_MES_PER_PAGE_COUNT) {
                                                        char *wrd = CodeHttpString(ss, 0);
                                                        if(wrd) {
                                                                printf("<BR><CENTER>" MESSAGEMAIN_search_result_pages);
                                                                DWORD max = (c/SEARCH_MES_PER_PAGE_COUNT) + 
                                                                        (((c % SEARCH_MES_PER_PAGE_COUNT) == 0)? 0: 1);
                                                                for(DWORD i = 0; i < max; i++) {
                                                                        if(i > 0 && (i % 20 == 0)) printf("<BR>");
                                                                        if(i != start - 1) printf("&nbsp;<A HREF=\"?searchword=%s&amp;start=%lu\">%lu</A>&nbsp;", wrd, i+1, i+1);
                                                                        else printf("<B>&nbsp;%lu&nbsp;</B>", i+1);
                                                                }
                                                                printf("</CENTER>");
                                                        }
                                                }
                                        }
                                        else {
                                                //        Write that searcher have not been configured properly
                                        }
                                        delete ms;
                                }
                                PrintBottomLines();
                                free(ss);
                                goto End_part;
                        }
                }

                Tittle_cat(TITLE_Search);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_SEARCH, MAINPAGE_INDEX);
                
                PrintSearchForm("", &DB, 1);
                
                PrintBottomLines();

                goto End_part;
        }

        if(strncmp(deal, "search", 6) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }

                if((st = strget(deal,"search=", 60, '&')) != NULL) {
                        if(strcmp(st, "action") == 0) {
                                free(st);
                                
                                /* get "method post" parameters */
                                par = GetParams(MAX_PARAMETERS_STRING);
                                if(par != NULL) {
                                        char *ss;
                                        /* read search pattern */
                                        ss = strget(par, "find=", 255 - 1, '&');
                                        if(ss == NULL) {
                                                ss = (char*)malloc(1);
                                                ss[0] = 0;
                                        }

                                        Tittle_cat(TITLE_Search);

#if ENABLE_LOG == 2
                                        print2log("Search from %s, query=%s", getenv(REMOTE_ADDR), ss);
#endif
                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_SEARCH, MAINPAGE_INDEX);
                                        PrintSearchForm(ss, &DB);
                                        if(strlen(ss) >= SEARCHER_MIN_WORD) {
                                                CMessageSearcher *ms = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
                                                if(ms->errnum == SEARCHER_RETURN_ALLOK) {
                                                        DWORD c;
                                                        DWORD *vmsg = ms->SearchMessagesByPattern(ss, &c);
                                                        printf(DESIGN_SEARCH_SEARCH_STR_WAS, MESSAGEMAIN_search_search_str, ms->srch_str);
                                                        if(c != 0) {
                                                                // print count of found messages
                                                                printf(DESIGN_SEARCH_RESULT, MESSAGEMAIN_search_result1, c, MESSAGEMAIN_search_result2);
                                                        }
                                                        else {
                                                                // Nothing have been found
                                                                printf(DESIGN_SEARCH_NO_RESULT, MESSAGEMAIN_search_result1, MESSAGEMAIN_search_result_nothing);
                                                        }
                                                        if(c > SEARCH_MES_PER_PAGE_COUNT) {
                                                                char *wrd = CodeHttpString(ss, 0);
                                                                if(wrd) {
                                                                        printf("<CENTER>" MESSAGEMAIN_search_result_pages);
                                                                        int max = (c/SEARCH_MES_PER_PAGE_COUNT) + 
                                                                                (((c % SEARCH_MES_PER_PAGE_COUNT) == 0)? 0: 1);
                                                                        for(int i = 0; i < max; i++) {
                                                                                if(i > 0 && (i % 20 == 0)) printf("<BR>");
                                                                                if(i != 0) printf("&nbsp;<A HREF=\"?searchword=%s&amp;start=%d\">%d</A>&nbsp;", wrd, i+1, i+1);
                                                                                else printf("&nbsp;<B>%d</B>&nbsp;", i+1);
                                                                        }
                                                                        printf("</CENTER>");
                                                                }
                                                        }
                                                        DWORD oldc = c;
                                                        if(c > 0) {
                                                                if( c > SEARCH_MES_PER_PAGE_COUNT) c = SEARCH_MES_PER_PAGE_COUNT;
                                                                DB.PrintHtmlMessageBufferByVI(vmsg, c);
                                                                free(vmsg);
                                                        }
                                                        c = oldc;
                                                        if(c > SEARCH_MES_PER_PAGE_COUNT) {
                                                                char *wrd = CodeHttpString(ss, 0);
                                                                if(wrd) {
                                                                        printf("<BR><CENTER>" MESSAGEMAIN_search_result_pages);
                                                                        int max = (c/SEARCH_MES_PER_PAGE_COUNT) + 
                                                                                (((c % SEARCH_MES_PER_PAGE_COUNT) == 0)? 0: 1);
                                                                        for(int i = 0; i < max; i++) {
                                                                                if(i > 0 && (i % 20 == 0)) printf("<BR>");
                                                                                if(i != 0) printf("&nbsp;<A HREF=\"?searchword=%s&amp;start=%d\">%d</A>&nbsp;", wrd, i+1, i+1);
                                                                                else printf("<B>&nbsp;%d&nbsp;</B>", i+1);
                                                                        }
                                                                        printf("</CENTER>");
                                                                }
                                                        }
                                                }
                                                else {
                                                        //        Write that searcher have not been configured properly
                                                }
                                                delete ms;
                                        }
                                        PrintBottomLines();
                                        free(ss);
                                        goto End_part;
                                }
                                else goto End_URLerror;
                        }
                        free(st);
                }

                Tittle_cat(TITLE_Search);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_SEARCH, MAINPAGE_INDEX);
                
                PrintSearchForm("", &DB, 1);
                
                PrintBottomLines();

                goto End_part;
        }

        if(strncmp(deal, "changeusr=", 8) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }
                if((st = strget(deal,"changeusr=", 30, '&')) != NULL) {
                        if(strcmp(st, "action") == 0) {
                                free(st);

                                //        here we do it :)
                                //        get parameters
                                par = GetParams(MAX_PARAMETERS_STRING);
                                if(par !=NULL) {
                                        BYTE ustat;
                                        DWORD right = 0;
                                        char *name;

                                        /* name */
                                        name = strget(par, "name=", PROFILES_MAX_USERNAME_LENGTH - 1, '&');

                                        /* ustat */
                                        if((st = strget(par, "ustat=", 10, '&')) != NULL) {
                                                errno = 0;
                                                char *ss;
                                                ustat = (BYTE)strtol(st, &ss, 10);
                                                if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE || ustat >= USER_STATUS_COUNT) {
                                                        ustat = 0;
                                                }
                                                free(st);
                                        }
                                        else ustat = 0;

                                        // read the right
                                        {
                                                DWORD i;
                                                char *ss;
                                                right = 0;
                                                for(i = 0; i < USERRIGHT_COUNT; i++)
                                                {
                                                        char st[30];
                                                        sprintf(st, "right%lu=", i);
                                                        if((ss = strget(par, st,  4, '&')) != NULL)
                                                        {
                                                                if(strcmp(ss, "on") == 0)
                                                                {
                                                                        right |= (1<<i);
                                                                }
                                                                free(ss);
                                                        }
                                                }
                                        }

                                        //
                                        //        Update the user
                                        //
                                        int updated = 0;
                                        if(name) {
                                                CProfiles *uprof;
                                                SProfile_UserInfo ui;
                                                SProfile_FullUserInfo fui;
                                                DWORD err = 0;
                                                DWORD idx;

                                                uprof = new CProfiles();
                                                err = uprof->GetUserByName(name, &ui, &fui, &idx);
                                                if(err == PROFILE_RETURN_ALLOK) {
                                                        int altnupd = 0;
                                                        // delete alt name if required
                                                        if(((ui.right & USERRIGHT_SUPERUSER) != 0 ||
                                                                (ui.right & USERRIGHT_ALT_DISPLAY_NAME) != 0) &&
                                                           ((right & USERRIGHT_ALT_DISPLAY_NAME) == 0 &&
                                                                (right & USERRIGHT_SUPERUSER) == 0)
                                                          )
                                                        {
                                                                ui.Flags &= (~PROFILES_FLAG_ALT_DISPLAY_NAME);
                                                                altnupd = 1;
                                                        }

                                                        ui.Status = ustat;
                                                        ui.right = right;

                                                        if(uprof->SetUInfo(idx, &ui)) {
                                                                if(altnupd) AltNames.DeleteAltName(ui.UniqID);
                                                                updated = 1;
                                                        }
                                                }
                                                if(uprof) delete uprof;
                                        }

                                        //
                                        //        Write complete message
                                        //
                                        if(updated) {
                                                Tittle_cat(TITLE_Registration);

                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                                        MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_register_edit_ex, MESSAGEMAIN_register_edit_ex2,
                                                        HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                                PrintBottomLines();

                                                // log this task
                                                print2log("User %s was updated by %s", name, ULogin.pui->username);
                                        }
                                        else {
                                                Tittle_cat(TITLE_Error);

                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_register_edit_err, MESSAGEMAIN_register_edit_err2, 0);
                                                PrintBottomLines();
                                        }

                                        free(par);
                                        goto End_part;
                                }
                                else goto End_URLerror;
                        }
                        free(st);
                }

                goto End_URLerror;
        }

        if(strncmp(deal, "register", 8) == 0) {
                /* security check */
                if((ULogin.LU.right & USERRIGHT_PROFILE_CREATE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }
                // We do not need to check security there, this action due to be done lately in DoCheckAndCreateProfile()
                if((st = strget(deal,"register=", 30, '&')) != NULL) {
                        if(strcmp(st, "action") == 0) {
                                free(st);
                                SProfile_FullUserInfo fui;
                                SProfile_UserInfo ui;

                                /****** set default user creation parameters ******/
                                ui.Flags = 0; // don't have picture or signature
                                ui.right = DEFAULT_USER_RIGHT;
                                /**************************************************/

                                /* get parameters */
                                par = GetParams(MAX_PARAMETERS_STRING);
                                if(par !=NULL) {
                                        char *ss, *passwdconfirm, *oldpasswd, *act;

                                        /* what we should do: edit, delete or create */
                                        act = strget(par, "register=", 255, '&');
                                        if(act == NULL) {
                                                // default action - register
                                                act = (char*)malloc(100);
                                                strcpy(act, MESSAGEMAIN_register_register);
                                        }

                                        /* check bot */
                                        ss = strget(par, "email=", PROFILES_FULL_USERINFO_MAX_EMAIL - 1, '&');
                                        if (ss && ss[0]) {
                                                print2log("register: A bot is trapped: email=%s", ss);
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_register_already_exit, MESSAGEMAIN_register_already_exit2, 0);
                                                PrintBottomLines();
                                                exit(0);
                                        }
                                        free(ss);
                                        
                                        /* read login name (username) and load user profile if update */
                                        ss = strget(par, "login=", PROFILES_MAX_USERNAME_LENGTH - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(ui.username, ss);
                                                free(ss);
                                        }
                                        else {
                                                if(ULogin.LU.ID[0] != 0)
                                                        strcpy(ui.username, ULogin.pui->username);
                                                else ui.username[0] = 0;
                                        }
                                        /* if edit - load current settings */
                                        if(strcmp(act, MESSAGEMAIN_register_edit) == 0 && ui.username[0] != 0) {
                                                CProfiles *cp = new CProfiles();
                                                cp->GetUserByName(ui.username, &ui, &fui, NULL);
                                                delete cp;
                                        }

                                        //        Read alternative display name for user
                                        ss = strget(par, "dispnm=", PROFILES_MAX_ALT_DISPLAY_NAME - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(ui.altdisplayname, ss);
                                                free(ss);
                                        }
                                        else ui.altdisplayname[0] = 0;

                                        /* read password 1 */
                                        ss = strget(par, "pswd1=", PROFILES_MAX_PASSWORD_LENGTH - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(ui.password, ss);
                                                free(ss);
                                        }
                                        else ui.password[0] = 0;
                                        
                                        /* read password 2 */
                                        passwdconfirm = strget(par, "pswd2=", PROFILES_MAX_PASSWORD_LENGTH - 1, '&');
                                        if(!passwdconfirm)
                                        {
                                                passwdconfirm = (char*)malloc(2);
                                                passwdconfirm[0] = 0;
                                        }
                                        
                                        /* read old password */
                                        oldpasswd = strget(par, "opswd=", PROFILES_MAX_PASSWORD_LENGTH - 1, '&');
                                        if(!oldpasswd)
                                        {
                                                oldpasswd = (char*)malloc(2);
                                                oldpasswd[0] = 0;
                                        }

                                        /* read full name */
                                        ss = strget(par, "name=", PROFILES_FULL_USERINFO_MAX_NAME - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(fui.FullName, ss);
                                                free(ss);
                                        }
                                        else fui.FullName[0] = 0;

                                        /* read email */
                                        ss = strget(par, "emchk=", PROFILES_FULL_USERINFO_MAX_EMAIL - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(fui.Email, ss);
                                                free(ss);
                                        }
                                        else fui.Email[0] = 0;
                                        
                                        /* read email */
                                        ss = strget(par, "icq=", PROFILES_MAX_ICQ_LEN - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(ui.icqnumber, ss);
                                                free(ss);
                                        }
                                        else ui.icqnumber[0] = 0;

                                        /* read homepage address */
                                        ss = strget(par, "hpage=", PROFILES_FULL_USERINFO_MAX_HOMEPAGE - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(fui.HomePage, ss);
                                                free(ss);
                                        }
                                        else fui.HomePage[0] = 0;
                                        
                                        /* read about */
                                        fui.AboutUser = strget(par, "about=", MAX_PARAMETERS_STRING - 1, '&');

                                        /* read signature */
                                        ss = strget(par, "sign=", PROFILES_MAX_SIGNATURE_LENGTH - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(fui.Signature, ss);
                                                free(ss);
                                        }
                                        else fui.Signature[0] = 0;

                                        //        read selected users
                                        ss = strget(par, "susr=", PROFILES_FULL_USERINFO_MAX_SELECTEDUSR - 1, '&');
                                        if(ss != NULL) {
                                                strcpy(fui.SelectedUsers, ss);
                                                free(ss);
                                        }
                                        else fui.SelectedUsers[0] = 0;

                                        /* invisible profile ? */
                                        ss = strget(par, "vprf=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags & (~PROFILES_FLAG_INVISIBLE);
                                        }
                                        else ui.Flags = ui.Flags | PROFILES_FLAG_INVISIBLE;
                                        if(ss != NULL) free(ss);

                                        /* always email ackn. for every post ? */
                                        ss = strget(par, "apem=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags  | PROFILES_FLAG_ALWAYS_EMAIL_ACKN;
                                        }
                                        else ui.Flags = ui.Flags & (~PROFILES_FLAG_ALWAYS_EMAIL_ACKN);
                                        if(ss != NULL) free(ss);

                                        /* disabled private messages ? */
                                        ss = strget(par, "pdis=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags | PROFILES_FLAG_PERSMSGDISABLED;
                                        }
                                        else ui.Flags = ui.Flags & (~PROFILES_FLAG_PERSMSGDISABLED);
                                        if(ss != NULL) free(ss);

                                        /* private message mail ackn ? */
                                        ss = strget(par, "peml=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags | PROFILES_FLAG_PERSMSGTOEMAIL;
                                        }
                                        else ui.Flags = ui.Flags & (~PROFILES_FLAG_PERSMSGTOEMAIL);
                                        if(ss != NULL) free(ss);

                                        /* public email ? */
                                        ss = strget(par, "pem=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags | PROFILES_FLAG_VISIBLE_EMAIL;
                                        }
                                        else ui.Flags = ui.Flags & (~PROFILES_FLAG_VISIBLE_EMAIL);
                                        if(ss != NULL) free(ss);
                                
                                        /* save view settings to profile */
                                        ss = strget(par, "vprs=", 10, '&');
                                        if(ss != NULL && strcmp(ss, "1") == 0) {
                                                ui.Flags = ui.Flags | PROFILES_FLAG_VIEW_SETTINGS;
                                        }
                                        else ui.Flags = ui.Flags & (~PROFILES_FLAG_VIEW_SETTINGS);
                                        if(ss != NULL) free(ss);
                                                                                                                                                                                                        

                                        
                                        if(act != NULL && strcmp(act, MESSAGEMAIN_register_register) == 0) {

                                                Tittle_cat(TITLE_Registration);

                                                DoCheckAndCreateProfile(&ui, &fui, passwdconfirm, oldpasswd, 1, deal);
                                        }
                                        else if(act != NULL && strcmp(act, MESSAGEMAIN_register_edit) == 0) {

                                                Tittle_cat(TITLE_Registration);

                                                DoCheckAndCreateProfile(&ui, &fui, passwdconfirm, oldpasswd, 2, deal);
                                        }
                                        else
                                        if(act != NULL && strcmp(act, MESSAGEMAIN_register_delete) == 0) {

                                                char* delete_confirmed = strget(par, CONFIRM_DELETE_CHECKBOX_TEXT "=", 255, '&');
                                                if(!delete_confirmed || !strlen(delete_confirmed))
                                                        goto End_URLerror;
                                                Tittle_cat(TITLE_Registration);

                                                DoCheckAndCreateProfile(&ui, &fui, passwdconfirm, oldpasswd, 3, deal);                                                
                                        }
                                        else {
                                                if(act != NULL) free(act);
                                                goto End_URLerror;
                                        }
                                        if(act != NULL) free(act);
                                        free(passwdconfirm);
                                        free(oldpasswd);
                                        goto End_part;
                                }
                                else goto End_URLerror;
                        }
                        free(st);
                }

                SProfile_FullUserInfo fui;
                SProfile_UserInfo ui;
                memset(&fui, 0, sizeof(fui));
                fui.AboutUser = (char*)malloc(1);
                fui.AboutUser[0] = 0;
                memset(&ui, 0, sizeof(ui));
                ui.Flags = USER_DEFAULT_PROFILE_CREATION_FLAGS;

                DWORD x = 0;
                if(ULogin.LU.ID[0] != 0 && (ULogin.LU.right & USERRIGHT_SUPERUSER)) x = 7;
                else if(ULogin.LU.ID[0] == 0) x = 1;
                else if(ULogin.LU.ID[0] != 0) x = 6;

                if(x & 0x02) {
                        ULogin.uprof->GetUserByName(ULogin.pui->username, &ui, &fui, NULL);
                }
                else {
                        strcpy(fui.HomePage, "http://");
                }

                Tittle_cat(TITLE_Registration);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_REGISTER, MAINPAGE_INDEX);
                
                PrintEditProfileForm(&ui, &fui, x);

                free(fui.AboutUser);

                PrintBottomLines();
                goto End_part;
        }

#if        TOPICS_SYSTEM_SUPPORT
        if(strncmp(deal, "ChangeTopic", 11) == 0) {
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) {
                char *sn;
                DWORD MsgNum = 0, Topic;
                if((sn = strget(deal, "ChangeTopic=", 255 - 1, '&')) != NULL) {
                        errno = 0;
                        int errok;
                        char *ss;
                        MsgNum = strtol(sn, &ss, 10);
                        if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE || MsgNum == 0 ||
                                (MsgNum = DB.TranslateMsgIndex(MsgNum)) == NO_MESSAGE_CODE) {
                                errok = 0;
                        }
                        else errok = 1;
                        free(sn);
                        if(errok && (st = strget(deal,"topic=", 60, '&')) != NULL) {
                                errno = 0;
                                char *ss;
                                Topic = strtol(st, &ss, 10);
                                if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE || Topic > TOPICS_COUNT) {
                                        errok = 0;
                                }
                                else errok = 1;
                                free(st);

                                //        Do real job (change the topic)
                                SMessage mes;
                                if(ReadDBMessage(MsgNum, &mes)) {
                                        DWORD oldTopic = mes.Topics;
                                        mes.Topics = Topic;
                                        if(WriteDBMessage(MsgNum, &mes)) {
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE,
                                                                mes.ViIndex, mes.ViIndex);
                                                PrintBoardError(MESSAGEMAIN_messagechanged, MESSAGEMAIN_messagechanged2,
                                                                HEADERSTRING_REFRESH_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_THREAD, mes.ViIndex);
                                                PrintBottomLines();
                                                print2log("Topic of message %d (%s (by %s)) was changed from [%s] to [%s] by %s", tmp, mes.MessageHeader, mes.AuthorName,
                                                        Topics_List[oldTopic], Topics_List[Topic], ULogin.pui->username);
                                                goto End_part;
                                        }
                                }
                        }
                }

                //        request error
                else goto End_URLerror;
                }
        }
#endif

        if(strncmp(deal, "userlist", 8) == 0) {
                char *sn;
                DWORD code = 1; // by name
                if((sn = strget(deal, "userlist=", 255 - 1, '&')) != NULL) {
                        errno = 0;
                        DWORD retval;
                        char *ss;
                        retval = strtol(sn, &ss, 10);
                        if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE || retval == 0 || retval > 6) {

                        }
                        else code = retval;
                }

                // security check
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }
                if(code == 2 || code == 3 || code == 4 || code == 5 || code == 6) {
                        // security check
                        if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                                printaccessdenied(deal);
                                goto End_part;
                        }
                }

                Tittle_cat(TITLE_UserList);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                PrintUserList(&DB, code);

                PrintBottomLines();

                goto End_part;
        }

        if(strncmp(deal, "persmsgform", 11) == 0) {
                if(ULogin.LU.UniqID != 0) {
                        // personal messages
                        char *sn = strget(deal, "persmsgform=", 255 - 1, '&');
                        if(sn) {
                                char * f = FilterHTMLTags(sn, 255-1);
                                free(sn);
                                sn = f;
                        }
                        if(!sn) {
                                sn = (char*)malloc(1000);
                                strcpy(sn, "");
                        }

                        Tittle_cat(TITLE_AddPrivateMsg);

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                        PrintPrivateMessageForm(sn, "");

                        PrintBottomLines();
                        free(sn);
                }
                else {
                        Tittle_cat(TITLE_Error);

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_privatemsg_denyunreg, MESSAGEMAIN_privatemsg_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }

        if(strncmp(deal, "persmsgpost", 11) == 0) {
                if(ULogin.LU.UniqID != 0) {
                // personal messages post or preview
                int bodyok = 0, nameok = 0, tolong = 0, allowpmsg = 1;
                
                /* get parameters */
                par = GetParams(MAX_PARAMETERS_STRING);
                if(par != NULL) {
                        char *name, *body, *fbody, *todo;
                        CProfiles prof;
                        SProfile_UserInfo ui;
                        SProfile_FullUserInfo fui;
                        int preview = 0;

                        todo = strget(par, "Post=", MAX_PARAMETERS_STRING - 1, '&');                        

                        if(todo != NULL && strcmp(todo, MESSAGEMAIN_privatemsg_prev_msg_btn) == 0) {
                                preview = 1;
                        }
                        else if(todo != NULL && strcmp(todo, MESSAGEMAIN_privatemsg_send_msg_btn) == 0) {
                                preview = 0;
                        }
                        else {
                                free(par);
                                printbadurl(deal);
                                goto End_part;
                        }

                        free(todo);

                        name = strget(par, "name=", PROFILES_MAX_USERNAME_LENGTH - 1, '&');
                        body = strget(par, "body=", MAX_PARAMETERS_STRING - 1, '&');

                        if(name && prof.GetUserByName(name, &ui, &fui, NULL) == PROFILE_RETURN_ALLOK) {
                                if( (ULogin.LU.right & USERRIGHT_SUPERUSER) == 0 && (ui.Flags & PROFILES_FLAG_PERSMSGDISABLED)) allowpmsg = 0;
                                nameok = 1;
                        }
                        if(body && strlen(body) > 0) {
                                DWORD retflg;
                                DWORD enabled_smiles = 0;
                                if((currentdsm & CONFIGURE_dsm) == 0)
                                        enabled_smiles = MESSAGE_ENABLED_SMILES;
                                if(FilterBoardTags(body, &fbody, 0, MAX_PARAMETERS_STRING - 1,
                                        enabled_smiles | MESSAGE_ENABLED_TAGS | BOARDTAGS_TAG_PREPARSE, &retflg) == 0) {
                                        /* if to long - ignore tags */
                                        
                                }
                                else {
                                        free(body);
                                        if(strcmp(fbody, " ") == 0) *fbody = 0;
                                        body = fbody;
                                }
                                if(strcmp(body, "") != 0) bodyok = 1;
                                if(strlen(body) > PROFILE_PERSONAL_MESSAGE_LENGHT - 1) tolong = 1;
                        }

                        if(bodyok && nameok && (!tolong) && allowpmsg) {
                                if(preview) {
                                        char tostr[2000];
                                        char *ss;

                                        Tittle_cat(TITLE_AddPrivateMsg);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                        printf(DESIGN_PREVIEW_PREVIEWMESSAGE "<BR>", MESSAGEHEAD_preview_preview_message);
                                        DB.Profile_UserName(name, tostr, 1);
                                        ss = ConvertFullTime(time(NULL));

                                        char *st = FilterHTMLTags(body, MAX_PARAMETERS_STRING - 1);
                                        char *st1 = NULL;
                                        char *st1_f;
                                        DWORD retflg;
                                        DWORD enabled_smiles = 0;
                                        if((currentdsm & CONFIGURE_dsm) == 0)
                                                enabled_smiles = MESSAGE_ENABLED_SMILES;
                                        if(FilterBoardTags(st, &st1, 0, MAX_PARAMETERS_STRING - 1, 
                                                enabled_smiles | MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE |
                                                BOARDTAGS_EXPAND_ENTER, &retflg) == 0)
                                        {
                                                st1 = st;
                                                st = NULL;
                                        }
                                        
                                        st1_f = FilterBiDi(st1);

                                        // print message text
                                        printf(DESIGN_PRIVATEMSG_FRAME, ss, MESSAGEMAIN_privatemsg_touser, tostr, CodeHttpString(name), 
                                               MESSAGEMAIN_privatemsg_write, DESIGN_PRIVATEMSG_FRAME_OUT, st1_f);
                                        
                                        PrintPrivateMessageForm(name, body);

                                        if(st) free(st);
                                        if(st1) free(st1);
                                        if (st1_f)
                                                free(st1_f);
                                }
                                else {
                                if(prof.PostPersonalMessage(name, 0, body, ULogin.pui->username, ULogin.LU.UniqID) == PROFILE_RETURN_ALLOK) {

                                        Tittle_cat(TITLE_PrivateMsgWasPosted);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                        PrintBoardError(MESSAGEMAIN_privatemsg_msgwassent, MESSAGEMAIN_privatemsg_msgwassent2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);

                                        // Send ackn. to recipient
                                        if((ui.Flags & PROFILES_FLAG_PERSMSGTOEMAIL)) {
                                                char *pb2;
                                                char subj[1000];
                                                char bdy[100000];

                                                sprintf(subj, MAILACKN_PRIVATEMSG_SUBJECT, ULogin.pui->username);

                                                if(!PrepareTextForPrint(body, &pb2, 0, MESSAGE_ENABLED_TAGS | BOARDTAGS_EXPAND_ENTER | BOARDTAGS_PURL_ENABLE)) {
                                                        pb2 = (char*)malloc(strlen(body) + 1);
                                                        strcpy(pb2, body);
                                                }

                                                sprintf(bdy, MAILACKN_PRIVATEMSG_BODY, name, ULogin.pui->username, pb2,
                                                        GetBoardUrl(), ULogin.pui->username, GetBoardUrl());

                                                wcSendMail(fui.Email, subj, bdy);
                                                print2log("Private message mailackn was sent to %s (%s->%s)", fui.Email, ULogin.pui->username, name);

                                                free(pb2);
                                        }
                                }
                                else {
                                        Tittle_cat(TITLE_Error);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                        PrintBoardError(MESSAGEMAIN_privatemsg_msgcantsend, MESSAGEMAIN_privatemsg_msgcantsend2, 0);
                                }
                                }

                                free(body);
                                free(name);
                                if(nameok) free(fui.AboutUser);

                                PrintBottomLines();
                                goto End_part;
                        }
                        else {
                                Tittle_cat(TITLE_AddPrivateMsg);

                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        }
                        if(nameok) free(fui.AboutUser);
                        // print form and errors
                        if(!allowpmsg) {
                                printf("<P><CENTER><LI> <FONT COLOR=RED><B>" MESSAGEMAIN_privatemsg_disable_pmsg "</B></FONT></CENTER>");
                        }
                        else {
                                if(!nameok)
                                        printf("<P><CENTER><LI> <FONT COLOR=RED>" MESSAGEMAIN_privatemsg_invalid_user "</FONT></CENTER>");
                                if(!bodyok)
                                        printf("<P><CENTER><LI> <FONT COLOR=RED>" MESSAGEMAIN_privatemsg_invalid_body "</FONT></CENTER>");
                                if(tolong)
                                        printf("<P><CENTER><LI> <FONT COLOR=RED>" MESSAGEMAIN_privatemsg_tolong_body "</FONT></CENTER>");
                        }

                        if(!name) {
                                name = (char*)malloc(10);
                                *name = 0;
                        }
                        if(!name) {
                                body = (char*)malloc(10);
                                *body = 0;
                        }
                        PrintPrivateMessageForm(name, body);

                        PrintBottomLines();
                        free(body);
                        free(name);
                }
                }
                else {
                        Tittle_cat(TITLE_Error);

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_privatemsg_denyunreg, MESSAGEMAIN_privatemsg_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }

        if(strncmp(deal, "persmsg", 7) == 0) {
                if(ULogin.LU.UniqID != 0) {
                // personal messages
                char *sn;
                DWORD type = 0;
                if((sn = strget(deal, "persmsg=", 255 - 1, '&')) != NULL) {
                        if(strcmp(sn, "all") == 0) {
                                type = 1;
                        }
                        free(sn);
                }

                CProfiles prof;
                SPersonalMessage *msg, *frommsg;
                DWORD *tt, *ft;
                if(type) {
                        tt = NULL;
                        ft = NULL;
                }
                else {
                        tt = (DWORD*)malloc(sizeof(DWORD));
                        *tt = 10;
                        ft = (DWORD*)malloc(sizeof(DWORD));
                        *ft = 0;
                }
                // let's read to messages (maybe from too)
                if(prof.ReadPersonalMessages(NULL, ULogin.LU.SIndex, &msg, tt, &frommsg, ft) != PROFILE_RETURN_ALLOK)
                        printhtmlerror();

                // let's get received message count
                DWORD cnt = 0, postedcnt = 0;
                if(msg) {
                        while(msg[cnt].Prev != 0xffffffff) cnt++;
                        cnt++;
                }

                if(ft) {
                        if(cnt) {
                                SPersonalMessage *msg1;
                                time_t ld = msg[cnt-1].Date;

                                if(prof.ReadPersonalMessagesByDate(NULL, ULogin.LU.SIndex, &msg1, 0, &frommsg, ld) != PROFILE_RETURN_ALLOK)
                                        printhtmlerror();

                                // let's get posted message count
                                if(frommsg) {
                                        while(frommsg[postedcnt].Prev != 0xffffffff) postedcnt++;
                                        postedcnt++;
                                }
                        }
                        else {
                                *ft = 10;
                                // let's read to messages (maybe from too)
                                if(prof.ReadPersonalMessages(NULL, ULogin.LU.SIndex, &msg, NULL, &frommsg, ft) != PROFILE_RETURN_ALLOK)
                                        printhtmlerror();
                                // let's get posted message count
                                if(frommsg) {
                                        while(frommsg[postedcnt].Prev != 0xffffffff) postedcnt++;
                                        postedcnt++;
                                }
                        }
                }
                else {
                        // let's get posted message count
                        if(frommsg) {
                                while(frommsg[postedcnt].Prev != 0xffffffff) postedcnt++;
                                postedcnt++;
                        }
                }

                Tittle_cat(TITLE_PrivateMsg);

                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_PRIVATEMSG, MAINPAGE_INDEX);

                char uuname[1000];
                DB.Profile_UserName(ULogin.pui->username, uuname, 1);

                printf("<BR><BR><CENTER><B>" MESSAGEMAIN_privatemsg_header " %s</B><BR>", uuname);
                if((ULogin.pui->Flags & PROFILES_FLAG_PERSMSGDISABLED))
                        printf("<U>" MESSAGEMAIN_privatemsg_disabled "</U><BR>");

                if(cnt) printf(MESSAGEMAIN_privatemsg_newmsgcnt " %d, ",
                        ULogin.pui->persmescnt - ULogin.pui->readpersmescnt);
                else printf(MESSAGEMAIN_privatemsg_nonewmsg ", ");
                printf(MESSAGEMAIN_privatemsg_allmsgcnt " %d, " MESSAGEMAIN_privatemsg_allmsgcnt1 " %d<BR>"
		       "<A HREF=\"" MY_CGI_URL "?persmsgform\" STYLE=\"text-decoration:underline;\">" MESSAGEMAIN_privatemsg_writenewmsg "</A><BR>"
		       "<A HREF=\"" MY_CGI_URL "?persmsg=all\" STYLE=\"text-decoration:underline;\">" MESSAGEMAIN_privatemsg_showall  "</A>"
		       "</CENTER><P><P>", ULogin.pui->persmescnt, ULogin.pui->postedmescnt);

                char tostr[1000], newm[100];
                char *ss;
                SPersonalMessage *pmsg;
                DWORD i = 0;
                DWORD j = 0;
                int received = 0;        // posted or received
                for(;;) {
                        // check exit expression
                        if(i == cnt && j == postedcnt) break;
                        if(i == cnt) {
                                pmsg = &(frommsg[j]);
                                j++;
                                received = 0;
                        } else {
                                if(j == postedcnt) {
                                        pmsg = &(msg[i]);
                                        i++;
                                        received = 1;
                                }
                                else {
                                        if(frommsg[j].Date > msg[i].Date) {
                                                pmsg = &(frommsg[j]);
                                                j++;
                                                received = 0;
                                        }
                                        else {
                                                pmsg = &(msg[i]);
                                                i++;
                                                received = 1;
                                        }
                                } 
                        }

                        if(!received) {
                                DB.Profile_UserName(pmsg->NameTo, tostr, 1);
                        }
                        else {
                                DB.Profile_UserName(pmsg->NameFrom, tostr, 1);
                        }

                        ss = ConvertFullTime(pmsg->Date);

                        if(received && i + ULogin.pui->persmescnt <= ULogin.pui->readpersmescnt)
                                strcpy(newm, MESSAGEMAIN_privatemsg_newmark);
                        else strcpy(newm, "");


                        char *st = FilterHTMLTags(pmsg->Msg, MAX_PARAMETERS_STRING - 1);
                        char *st1 = NULL;
                        char *st1_f;
                        DWORD retflg;
                        DWORD enabled_smiles = 0;
                        if((currentdsm & CONFIGURE_dsm) == 0)
                                enabled_smiles = MESSAGE_ENABLED_SMILES;
                        if(FilterBoardTags(st, &st1, 0, MAX_PARAMETERS_STRING - 1, 
                                enabled_smiles | MESSAGE_ENABLED_TAGS | BOARDTAGS_PURL_ENABLE |
                                BOARDTAGS_EXPAND_ENTER, &retflg) == 0)
                        {
                                st1 = st;
                                st = NULL;
                        }
                        
                        st1_f = FilterBiDi(st1);
                        
                        if(!received) {        
                                printf(DESIGN_PRIVATEMSG_FRAME, ss, MESSAGEMAIN_privatemsg_touser, tostr, CodeHttpString(pmsg->NameTo, 0),
                                        MESSAGEMAIN_privatemsg_write, DESIGN_PRIVATEMSG_FRAME_OUT, st1_f);
                        }else{
                                printf(DESIGN_PRIVATEMSG_FRAME, ss, MESSAGEMAIN_privatemsg_fromuser, tostr, CodeHttpString(pmsg->NameFrom, 0),
                                        MESSAGEMAIN_privatemsg_answer, DESIGN_PRIVATEMSG_FRAME_IN, st1_f);
                        }                

                        if(st) free(st);
                        if(st1) free(st1);
                        if (st1_f)
                                free(st1_f);
                }

                ULogin.pui->readpersmescnt = ULogin.pui->persmescnt;
                prof.SetUInfo(ULogin.LU.SIndex, ULogin.pui);

                PrintBottomLines();
                if(msg) free(msg);
                if(frommsg) free(frommsg);
                }
                else {
                        Tittle_cat(TITLE_Error);

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_privatemsg_denyunreg, MESSAGEMAIN_privatemsg_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }

        if(strncmp(deal, "globann", 7) == 0) {
                if((ULogin.LU.right & USERRIGHT_POST_GLOBAL_ANNOUNCE) != 0) {
                        // post global announce or global announce form
                        char *sn;
                        DWORD type = 0;
                        if((sn = strget(deal, "globann=", 255 - 1, '&')) != NULL) {
                                if(strcmp(sn, "post") == 0) {
                                        type = 1;
                                        free(sn);
                                }
                        }
                        if(!type) {
                                Tittle_cat(TITLE_PostGlobalAnnounce);

                                char *ss, body[GLOBAL_ANNOUNCE_MAXSIZE];
                                DWORD cgann_num;
                                cgann_num = strtoul(sn, &ss, 10);
                                if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE) {
                                        cgann_num = 0;
                                }
                                free(sn);

                                body[0] = 0;
                                if(cgann_num != 0) {
                                        SGlobalAnnounce *ga;
                                        DWORD cnt, i;
                                        if(ReadGlobalAnnounces(&ga, &cnt) != ANNOUNCES_RETURN_OK) printhtmlerror();
                                        for(i = 0; i < cnt; i++) {
                                                if(ga[i].Number == cgann_num) {
                                                        strcpy(body, ga[i].Announce);
                                                }
                                        }
                                }

                                // print form
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                PrintAnnounceForm(body, cgann_num);

                                PrintBottomLines();
                                goto End_part;
                        }
                        else {
                                char *body = NULL;
                                int cgann_num = 0, preview = 0, refid = 0;

                                // post
                                /* get parameters */
                                par = GetParams(MAX_PARAMETERS_STRING);
                                if(par) {
                                        char *todo = strget(par, "Post=", MAX_PARAMETERS_STRING - 1, '&');

                                        // preview or post ?
                                        if(todo != NULL && strcmp(todo, MESSAGEMAIN_globann_prev_ann_btn) == 0) {
                                                preview = 1;
                                        }
                                        else if(todo != NULL && strcmp(todo, MESSAGEMAIN_globann_send_ann_btn) == 0) {
                                                preview = 0;
                                        }
                                        else {
                                                free(par);
                                                printbadurl(deal);
                                                goto End_part;
                                        }
                                        free(todo);

                                        char *sn = strget(par, "cgann=", 255 - 1, '&');
                                        char *refids = strget(par, "refid=", 100, '&');
                                        if(refids) {
                                                if(strcmp(refids, "1") == 0) refid = 1;
                                                free(refids);
                                        }
                                        body = strget(par, "body=", MAX_PARAMETERS_STRING - 1, '&');
                                        // translate to numeric format
                                        char *ss;
                                        if(sn) {
                                                cgann_num = strtol(sn, &ss, 10);
                                                if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE) {
                                                        cgann_num = 0;
                                                }
                                                free(sn);
                                        }
                                        free(par);
                                }
                                if(body && strlen(body) > 5) {
                                        if(strlen(body) >= GLOBAL_ANNOUNCE_MAXSIZE - 1) {

                                                Tittle_cat(TITLE_PostGlobalAnnounce);

                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                                printf("<P><CENTER><LI> <FONT COLOR=RED>" MESSAGEMAIN_globann_tolong "</FONT></CENTER>");

                                                PrintAnnounceForm(body, cgann_num);

                                                PrintBottomLines();
                                        }
                                        else {
                                                if(preview) {
                                                        char uname[1000];
                                                        char *st;
                                                        char *date;
                                                        DWORD enabled_smiles = 0;
                                                        if((currentdsm & CONFIGURE_dsm) == 0)
                                                                enabled_smiles = MESSAGE_ENABLED_SMILES;

                                                        DB.Profile_UserName(ULogin.pui->username, uname, 1);                                                                                                         PrepareTextForPrint(body, &st, 0, enabled_smiles | MESSAGE_ENABLED_TAGS |
                                                                BOARDTAGS_PURL_ENABLE | BOARDTAGS_EXPAND_ENTER);
                                                        date = ConvertTime(time(NULL));

                                                        Tittle_cat(TITLE_PostGlobalAnnounce);

                                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                                        printf("<BR><BR><CENTER><B>%s</B></CENTER>", MESSAGEMAIN_globann_preview_hdr);

                                                        printf(DESIGN_GLOBALANN_FRAME, st, MESSAGEMAIN_globann_postedby,
                                                                uname, date, "");

                                                        PrintAnnounceForm(body, cgann_num);

                                                        PrintBottomLines();

                                                        if(st) free(st);
                                                }
                                                else {
                                                        if( (cgann_num && (!refid)) ? UpdateGlobalAnnounce(cgann_num, ULogin.pui->username,
                                                                ULogin.pui->UniqID, body, 0, 0,
                                                                ANNOUNCES_UPDATE_OPT_USER | ANNOUNCES_UPDATE_OPT_TIME |
                                                                ANNOUNCES_UPDATE_OPT_TTL | ANNOUNCES_UPDATE_OPT_FLAGS) !=
                                                                ANNOUNCES_RETURN_OK
                                                                : PostGlobalAnnounce(ULogin.pui->username, ULogin.pui->UniqID, body,
                                                                0, 0) != ANNOUNCES_RETURN_OK )
                                                        {
                                                                Tittle_cat(TITLE_Error);

                                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                                PrintBoardError(MESSAGEMAIN_globann_anncantsend, MESSAGEMAIN_globann_anncantsend2, 0);
                                                                PrintBottomLines();
                                                        }
                                                        else {
                                                                Tittle_cat(TITLE_GlobalAnnWasPosed);

                                                                if(cgann_num && refid) {
                                                                        DeleteGlobalAnnounce(cgann_num, 0);
                                                                        print2log("Announce %d was deleted during update", cgann_num);
                                                                }

                                                                if(!cgann_num) {
                                                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                                        PrintBoardError(MESSAGEMAIN_globann_annwassent, MESSAGEMAIN_globann_annwassent2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                                                        PrintBottomLines();
                                                                }
                                                                else {
                                                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                                        PrintBoardError(MESSAGEMAIN_globann_annwasupdated, MESSAGEMAIN_globann_annwasupdated2, HEADERSTRING_REFRESH_TO_MAIN_PAGE);
                                                                        PrintBottomLines();
                                                                }

                                                                print2log("Global announce (%s) was posted by %s", body, ULogin.pui->username);
                                                        }
                                                }
                                        }
                                }
                                else if(body) {
                                        Tittle_cat(TITLE_PostGlobalAnnounce);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                                        printf("<P><CENTER><LI> <FONT COLOR=RED>" MESSAGEMAIN_globann_toshort "</FONT></CENTER>");

                                        if(body) PrintAnnounceForm(body, cgann_num);
                                        else PrintAnnounceForm("", cgann_num);

                                        PrintBottomLines();
                                }
                                else {
                                        // invalid request
                                        printbadurl(deal);
                                }
                                goto End_part;
                        }
                }
                else {
                        printaccessdenied(deal);
                        goto End_part;
                }
        }

        if(strncmp(deal, "ganndel", 7) == 0) {
                char *sn;
                if((ULogin.LU.right & USERRIGHT_POST_GLOBAL_ANNOUNCE) != 0) {
                        DWORD MsgNum = 0;
                        if((sn = strget(deal, "ganndel=", 255 - 1, '&')) != NULL) {
                                errno = 0;
                                int errok;
                                char *ss;
                                MsgNum = strtol(sn, &ss, 10);
                                if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE) {
                                        errok = 0;
                                }
                                else errok = 1;
                                free(sn);

                                if(errok) {
                                        if(DeleteGlobalAnnounce(MsgNum, ((ULogin.LU.right & USERRIGHT_SUPERUSER) != 0) ? 0 : ULogin.LU.UniqID) == ANNOUNCES_RETURN_OK) {
                                                Tittle_cat(TITLE_GlobalAnnWasDeleted);

                                                print2log("Global Announce %d was deleted by user %s", MsgNum, ULogin.pui->username);
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_globann_wasdeleted, MESSAGEMAIN_globann_wasdeleted2, 0);
                                                PrintBottomLines();
                                        }
                                        else {
                                                Tittle_cat(TITLE_Error);

                                                print2log("Global Announce %d cannot be deleted by user %s", MsgNum, ULogin.pui->username);
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_globann_cannotdel, MESSAGEMAIN_globann_cannotdel2, 0);
                                                PrintBottomLines();
                                        }
                                }
                                else {
                                        Tittle_cat(TITLE_Error);

                                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                        PrintBoardError(MESSAGEMAIN_globann_invalidnum, MESSAGEMAIN_globann_invalidnum2, 0);
                                        PrintBottomLines();
                                }
                        }
                        goto End_part;
                }
        }

        if(strncmp(deal, "rann", 4) == 0) {
                char *sn;
                if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) != 0) {
                        DWORD MsgNum = 0;
                        if((sn = strget(deal, "rann=", 255 - 1, '&')) != NULL) {
                                errno = 0;
                                int errok;
                                char *ss;
                                MsgNum = strtol(sn, &ss, 10);
                                if( (!(*sn != '\0' && *ss == '\0')) || errno == ERANGE) {
                                        errok = 0;
                                }
                                else errok = 1;
                                free(sn);

                                if(!errok) goto End_URLerror;

                                currentlann = MsgNum;
                                // to main page at once
                                PrintHTMLHeader(HEADERSTRING_REDIRECT_NOW | HEADERSTRING_NO_CACHE_THIS, MAINPAGE_INDEX);
                                goto End_part;
                        }
                }
        }

#ifdef USER_FAVOURITES_SUPPORT
        if(strncmp(deal, "favs", 4) == 0) {
                int num;
                if(ULogin.LU.UniqID != 0) {
                        CProfiles prof;
                        //favourites
                        // security check
                        if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                                printaccessdenied(deal);
                                goto End_part;
                        }
                        Tittle_cat(TITLE_FavouritesPage);
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_DISABLE_FAVOURITES, MAINPAGE_INDEX);
                        printf("<P><CENTER><P><B>%s</B><BR></CENTER>", MESSAGEHEAD_favourites);

                        int updated;
                        if( (num = DB.PrintandCheckMessageFavsExistandInv(ULogin.pui,
                                ULogin.LU.right & USERRIGHT_SUPERUSER, &updated)) == 0)
                                printf("<P><CENTER><B>" MESSAGEMAIN_favourites_listclear "</B></CENTER><P>");
                        if(updated) prof.SetUInfo(ULogin.LU.SIndex, ULogin.pui);
                        PrintBottomLines();
                        goto End_part;
                }
                else {
                        Tittle_cat(TITLE_Error);
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_favourites_denyunreg, MESSAGEMAIN_favourites_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }
        
        if(strncmp(deal, "favadd", 6) == 0) {
                if(ULogin.LU.UniqID != 0){
                        if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                                printaccessdenied(deal);
                                goto End_part;
                        }
                        if((st = strget(deal, "favadd=", 255 - 1, '&')) != NULL) {
                                errno = 0;
                                DWORD addmsg;
                                char *ss;
                                addmsg = strtol(st, &ss, 10);
                                if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE  || addmsg < 1){
                                        free (st);
                                         goto End_URLerror;
                                }
                                free (st);
                                DWORD msg;
                                if( ( msg=DB.TranslateMsgIndex(addmsg)) == NO_MESSAGE_CODE){
                                        printnomessage(deal);
                                        goto End_part;
                                }
                                if(!ReadDBMessage(msg, &mes)) printhtmlerror();
                                /* allow read invisible message only to SUPERUSER */
                                if((mes.Flag & MESSAGE_IS_INVISIBLE) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0)) {
                                        printnomessage(deal);
                                        goto End_part;
                                }
#if USER_FAVOURITES_SUPPORT == 2
                                if( mes.ParentThread != 0){
                                                Tittle_cat(TITLE_FavouritesPageAdd);
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_favourites_addno, MESSAGEMAIN_favourites_addnoparent, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                }
#endif
                                CProfiles prof;
                                int result = prof.CheckandAddFavsList(ULogin.LU.SIndex, addmsg, 1);
                                switch(result) {
                                        case PROFILE_RETURN_ALLOK:
                                                Tittle_cat(TITLE_FavouritesPageAdd);
                                                PrintHTMLHeader(HEADERSTRING_DISABLE_ALL, 0);
                                                PrintBoardError(MESSAGEMAIN_favourites_added, MESSAGEMAIN_favourites_added2, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                        case PROFILE_RETURN_ALREADY_EXIST:
                                                Tittle_cat(TITLE_FavouritesPageAdd);
                                                PrintHTMLHeader(HEADERSTRING_DISABLE_ALL, 0);
                                                PrintBoardError(MESSAGEMAIN_favourites_addno, MESSAGEMAIN_favourites_addexist, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                        case PROFILE_RETURN_UNKNOWN_ERROR:
                                                Tittle_cat(TITLE_FavouritesPageAdd);
                                                PrintHTMLHeader(HEADERSTRING_DISABLE_ALL, 0);
                                                PrintBoardError(MESSAGEMAIN_favourites_addno, MESSAGEMAIN_favourites_addnoplace, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                        default:
                                                printhtmlerror();
                                                goto End_part;
                                        
                                }
                        }
                }
                else {
                        Tittle_cat(TITLE_Error);
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_favourites_denyunreg, MESSAGEMAIN_favourites_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }

        if(strncmp(deal, "favdel", 6) == 0) {
                if(ULogin.LU.UniqID != 0){
                        if((ULogin.LU.right & USERRIGHT_VIEW_MESSAGE) == 0) {
                                printaccessdenied(deal);
                                goto End_part;
                        }
                        if((st = strget(deal, "favdel=", 255 - 1, '&')) != NULL) {
                                errno = 0;
                                DWORD delmsg;
                                char *ss;
                                delmsg = strtol(st, &ss, 10);
                                if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE  || delmsg == 0){
                                        free (st);
                                         goto End_URLerror;
                                }
                                free (st);
                                DWORD msg;
                                if( (msg = DB.TranslateMsgIndex(delmsg)) == NO_MESSAGE_CODE){
                                        printnomessage(deal);
                                        goto End_part;
                                }
                                if(!ReadDBMessage(msg, &mes)) printhtmlerror();
                                /* allow read invisible message only to SUPERUSER */
                                if((mes.Flag & MESSAGE_IS_INVISIBLE) && ((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0)) {
                                        printnomessage(deal);
                                        goto End_part;
                                }
                                CProfiles prof;
                                int result = prof.DelFavsList(ULogin.LU.SIndex, delmsg);
                                switch(result) {
                                        case PROFILE_RETURN_ALLOK:
                                                Tittle_cat(TITLE_FavouritesPageDel);
                                                PrintHTMLHeader(HEADERSTRING_DISABLE_ALL, 0);
                                                PrintBoardError(MESSAGEMAIN_favourites_deleted, MESSAGEMAIN_favourites_deleted2, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                        case PROFILE_RETURN_UNKNOWN_ERROR:
                                                Tittle_cat(TITLE_FavouritesPageDel);
                                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                                                PrintBoardError(MESSAGEMAIN_favourites_delno,  MESSAGEMAIN_favourites_delnoexist, 0);
                                                PrintBottomLines();
                                                goto End_part;
                                        default:
                                                printhtmlerror();
                                                goto End_part;
                                        
                                }
                        }
                }
                else {
                        Tittle_cat(TITLE_Error);
                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);
                        PrintBoardError(MESSAGEMAIN_favourites_denyunreg, MESSAGEMAIN_favourites_denyunreg2, 0);
                        PrintBottomLines();
                }
                goto End_part;
        }
#endif

#ifdef CLEANUP_IDLE_USERS

#define DAY (60*60*24)
#define YEAR (DAY*365)
        if(strncmp(deal, "cluserlist", 10) == 0) {
                bool fDelete = false;
                if(!(ULogin.LU.right & USERRIGHT_SUPERUSER)) {
                        printaccessdenied(deal);
                        goto End_part;
                }
                if((st = strget(deal,"cluserlist=", 14, '&')) != NULL) {
                        if(strcmp(st, "yes") == 0) {
                                free(st);
                                fDelete = true;
                        }
                }
                time_t tn = time(NULL);
                DWORD i = 0, ii=0;
                char **buf = NULL;
                char name[1000];
                DWORD uc = 0;
                CUserLogin ULogin;
                CProfiles uprof;
                Tittle_cat(TITLE_UserList);
                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, MAINPAGE_INDEX);

                printf("<BR><BR><center>");
                printf("<font color=\"red\">%s</font><br>\n",
                       fDelete ?
                       "Следующие пользователи были успешно удалены:" : 
                       "Список пользователей подлежащих удалению:"
                       );

                if(!uprof.GenerateUserList(&buf, &uc)) {
                        printf("error generating list");
                } else {
                        qsort((void*)buf, uc, sizeof(char*), cmp_name);
                        for(i = 0; i < uc; i++) {
                                DWORD PostCount = *((DWORD*)(buf[i] + 4));
                                DWORD LoginDate = *((DWORD*)(buf[i] + 8));
                                DWORD RefreshCount = *((DWORD*)(buf[i] + 12));
                                DWORD activity = PostCount + RefreshCount;
                                char *username = buf[i] + 20;
                                int idletime = tn - LoginDate;
                                bool fAged1 = idletime > 3 * YEAR;
                                bool fAged2 = idletime > 2 * YEAR && activity < 1000;
                                bool fAged3 = idletime > YEAR && activity < 100;
                                bool fAged4 = idletime > YEAR / 2 && activity < 10;
                                bool fAged5 = idletime > DAY && activity == 0;
                                if (fAged1 || fAged2 || fAged3 || fAged4 || fAged5) {        
                                        if((ii % 10) != 0) printf(" | ");
                                        if(((ii % 10) == 0) && ii != 0) printf("<br>");
                                        DB.Profile_UserName(buf[i] + 20, name, 1, 1);
                                        printf("%s", name);
                                        if(fDelete) {
                                                DWORD err = uprof.DeleteUser(username);
                                                if(err == PROFILE_RETURN_ALLOK) {
                                                        printf("!");
                                                } else { 
                                                        printf("Невозможно удалить '%s' !!!", name);
                                                        goto End_part;
                                                }
                                        }
                                        ii++;
                                }
                                free(buf[i]);
                        }
                        
                        if(fDelete)
                                printf("<br /><b>удалено %lu из %lu пользователей</b> \n", ii, uc);
                        else
                                printf("<br /><b>Будет удалено <fonc color=red>%lu</font> из %lu пользователей</b>"
                                       "<br /><a href=\"" MY_CGI_URL "?cluserlist=yes\"><font color=\"red\">Продолжить ?</font></a>", ii, uc);
                }
                PrintBottomLines();
                goto End_part;
        }
#endif

        if(strncmp(deal, "banlist", 7) == 0) {
                
                /* security check */
                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0) {
                        printaccessdenied(deal);
                        goto End_part;
                }
                if((st = strget(deal,"banlist=", 30, '&')) != NULL) {
                        if(strcmp(st, "save") == 0) {
                                // read ban list
                                par = GetParams(MAX_PARAMETERS_STRING);
                                char *ban_list;
                                ban_list = strget(par,"ban_list=", MAX_PARAMETERS_STRING, '&');
                                
                                // check ban_list is empty
                                if(ban_list == NULL || *ban_list == 0) {
                                        Tittle_cat(TITLE_BanSave);
                                        PrintHTMLHeader(HEADERSTRING_DISABLE_ALL, 0);
                                        PrintBoardError(MESSAGEMAIN_ban_no_save, MESSAGEMAIN_ban_empty, 0);
                                        PrintBottomLines();
                                        goto End_part;
                        
                                }
                                
                                WCFILE *BAN_FILE;
                                if ((BAN_FILE = wcfopen(F_BANNEDIP, FILE_ACCESS_MODES_RW)) == NULL) printhtmlerror();
                                lock_file(BAN_FILE);
                                
                                if ( !fCheckedWrite(ban_list, strlen(ban_list), BAN_FILE)  )  {
                                        unlock_file(BAN_FILE);
                                        printhtmlerror();
                                }

                                if (truncate(F_BANNEDIP, strlen(ban_list)))
                                        printhtmlerror();

                                unlock_file(BAN_FILE);
                                wcfclose(BAN_FILE);
                                
                                Tittle_cat(TITLE_BanSave);
                                PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE | HEADERSTRING_REFRESH_TO_MAIN_PAGE, 0);
                                PrintBoardError(MESSAGEMAIN_ban_save, MESSAGEMAIN_ban_save2, 0);
                                PrintBottomLines();

                                print2log("Banlist update by %s from %s", ULogin.pui->username, Cip);
                        
                        }        
                }
                else{

                        PrintHTMLHeader(HEADERSTRING_RETURN_TO_MAIN_PAGE, 0);
                        PrintBanList();
                        PrintBottomLines();

                        print2log("Banlist view by %s from %s", ULogin.pui->username, Cip);
                }
                goto End_part;
        }
        
        
        
        if(strncmp(deal, "clsession1", 9) == 0) {
                if(ULogin.LU.UniqID != 0) {
                DWORD closeseq[2];
                if((st= strget(deal, "clsession1=", 255 - 1, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                         closeseq[0] = strtol(st, &ss, 10);
                        if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||  closeseq[0] == 0 ) {
                                free(st);
                                goto End_URLerror;
                        }
                }
                free(st);
                if((st= strget(deal, "clsession2=", 255 - 1, '&')) != NULL) {
                        errno = 0;
                        char *ss;
                         closeseq[1] = strtol(st, &ss, 10);
                        if( (!(*st != '\0' && *ss == '\0')) || errno == ERANGE ||  closeseq[1] == 0 ) {
                                free(st);
                                goto End_URLerror;
                        }
                }
                free(st);

                // checking session exists and user have suff. rights
                // superuser knows if session does not exits
                // user can receive only access deny message (session bf aware)

                if((ULogin.LU.right & USERRIGHT_SUPERUSER) == 0){
                        if( (ULogin.LU.right & USERRIGHT_PROFILE_MODIFY) == 0 || ULogin.CheckSession(closeseq, 0, ULogin.LU.UniqID) != 1) {
                                printaccessdenied(deal);
                                goto End_part;
                        }
                }
                else {
                        if( ULogin.CheckSession(closeseq, 0, 0) != 1) {
                                Tittle_cat(TITLE_ClSession);
                                PrintHTMLHeader(HEADERSTRING_DISABLE_ALL | HEADERSTRING_REFRESH_TO_MAIN_PAGE, 0);
                                PrintBoardError(MESSAGEMAIN_session_closed_no, MESSAGEMAIN_session_check_failed, 0);
                                PrintBottomLines();
                                goto End_part;
                        }
                }

                // force closing session
                if(ULogin.ForceCloseSessionBySeq(closeseq)){
                        Tittle_cat(TITLE_ClSession);
                        PrintHTMLHeader(HEADERSTRING_DISABLE_ALL | HEADERSTRING_REFRESH_TO_MAIN_PAGE, 0);
                        PrintBoardError(MESSAGEMAIN_session_closed_ok, MESSAGEMAIN_session_closed_ok2, 0);
                        PrintBottomLines();
                        goto End_part;
                }
                else {
                        Tittle_cat(TITLE_ClSession);
                        PrintHTMLHeader(HEADERSTRING_DISABLE_ALL | HEADERSTRING_REFRESH_TO_MAIN_PAGE, 0);
                        PrintBoardError(MESSAGEMAIN_session_closed_no, MESSAGEMAIN_session_close_failed, 0);
                        PrintBottomLines();
                        goto End_part;
                }
                }
                goto End_URLerror;
                
        }
        

End_URLerror:
        printbadurl(deal);

End_part:

#if _DEBUG_ == 1
        //print2log("Exit success");
#endif // _DEBUG_

        return EXIT_SUCCESS;
}
