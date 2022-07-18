 /***************************************************************************
                          messages.h  -  ��� ��������� �����������
                             -------------------
    begin                : Fri Mar 23 2001
    �������                 : ��������� ������ <vektor@3ka.mipt.ru>
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED

#include "basetypes.h"

/* ��������� ��������� �� ��������
 * 0,1,2,3 -> ��� ���� ������ �����
 */
extern const char *MESSAGEHEAD_timetypes[4];

#define MESSAGEHEAD_preview_preview_message        "��������������� ��������"
#define MESSAGEHEAD_preview_change_message        ""

#define MESSAGEHEAD_configure                                 "���������"
#define MESSAGEHEAD_resetnew                                "�������� +"
#define MESSAGEHEAD_post_new_message                 "����� ���������"
#define MESSAGEHEAD_return_to_main_page                "�� ������� ��������"
#define MESSAGEHEAD_logoff                                        "�����"
#define MESSAGEHEAD_help_showhelp                        "�������"
#define MESSAGEHEAD_register                                "�����������"
#define MESSAGEHEAD_registerprf                                "��� �������"
#define MESSAGEHEAD_search                                        "�����"
#define MESSAGEHEAD_login                                        "����"
#define MESSAGEHEAD_userlist                                "������������"
#define MESSAGEHEAD_personalmsg                                "������"
#define MESSAGEHEAD_makeannounce                        "�������� �����"
#define MESSAGEHEAD_favourites                                "���������"
#define MESSAGEHEAD_banlist                                "�������"

#define MESSAGEMAIN_ban_save                        "�������. ������ �������� �������."
#define MESSAGEMAIN_ban_save2                        "����� ��������� ������ �� ��������� � ������ ���������"

#define MESSAGEMAIN_ban_no_save                        "��������! ������ �� ��������!"
#define MESSAGEMAIN_ban_empty                        "������������ ������ ����. "


#define MESSAGEMAIN_add_banned                        "��������� �� ����� ���� ���������"
#define MESSAGEMAIN_add_banned2                 "�������� ��������� � ������ ���������� ���� ��������� ���������������"
#define MESSAGEMAIN_add_closed                        "��������� �� ����� ���� ���������"
#define MESSAGEMAIN_add_closed2                 "��� ����� ������� ���������������, ����� ��������� �� �����������"
#define MESSAGEMAIN_add_invalid_reply        "��������� �� ����� ���� ���������"
#define MESSAGEMAIN_add_invalid_reply2        "��������� �� ������� �� ������ �������� �� ����������!"
#define MESSAGEMAIN_edit_denied                 "��������� �� ����� ���� ���������������"
#define MESSAGEMAIN_edit_denied2                "��������, � ��� ��� ���� �� �������������� ���������"
#define MESSAGEMAN_invisible    "��������� �� ����� ���� ��������� ��� ��������"
#define MESSAGEMAN_invisible2   "��������� ���� ������ ���������������"

#define MESSAGEMAIN_add_ok                                "��������� ���������"
#define MESSAGEMAIN_add_ok_login                "��������� ���������, �� ����� � ����������� "
#define MESSAGEMAIN_add_ok2                                "������� �� �������. "

#define MESSAGEMAIN_add_spelling                "��������� �� ����� ���� ��������� "
#define MESSAGEMAIN_add_spelling2                "���� ��������� �������� ����������� �����, ���������� �������� ��� ��� ���"

#define MESSAGEMAIN_add_no_name                        "� ����� ��������� ��� �����"
#define MESSAGEMAIN_add_no_name2                 "����� ��������� ���������, �� ������ ������� �� ������� ���� ���� � ���� ���"

#define MESSAGEMAIN_add_no_subject                "� ����� ��������� ��� ����"
#define MESSAGEMAIN_add_no_subject2         "����� ��������� ���������, �� ������ ������� �� ������� ���� ���� � ���� ���"

// TODO :.....
#define MESSAGEMAIN_add_no_right_thrd        "�������� ������ ������ ���� ��������� ��� ������������"
#define MESSAGEMAIN_add_no_right_thrd2        "���� �� ��������, ��� ��� ������ ��������� � ������������ ������"

#define MESSAGEMAIN_add_emptymsg_java        "��������, ����������, ���-������ � ������� �� ����������� ����� ������ ���������! �������!"
#define MESSAGEMAIN_add_tolong_java                "������ ������ ��������� ��������� �������� ��"
#define MESSAGEMAIN_add_tolong_java2        "��������! ����������, ��������� ���, ������ �� ��������� ������! �������!"

#define MESSAGEMAIN_add_no_body                        "� ����� ��������� ��� ����"
#define MESSAGEMAIN_add_no_body2                 "����� ��������� ���������, �� ������ �������� ���� ���������"

#define MESSAGEMAIN_add_flood                        "�� ��������� �������� ��������� ��������� ��� ����� ����� ������ ����������� ������� ����"
#define MESSAGEMAIN_add_flood2                         "���� �� ��������� ��������� ����� ���������, ���������� ������� ��� ����� ��������� �����"

#define MESSAGEMAIN_unknownerr                        "��������� ����������� ������"
#define MESSAGEMAIN_unknownerr2                 "�������� ���������� �������������� �� ���� ������"

#define MESSAGEMAIN_requesterror                 "��������� ������ �� ��������"
#define MESSAGEMAIN_requesterror2                 "�� ��� ��� ������� �������� ������������ ������, � ������ �� ����� �� ����������."

#define MESSAGEMAIN_nonexistingmsg                 "��� ��������� �� ���������� ��� ������� �����������"
#define MESSAGEMAIN_nonexistingmsg2         "�� ��� ��� ������� �������� ������������ ����� ���������, ������������� ���������� �������� ��� ������� ���������� �����"

#define MESSAGEMAIN_threadwasclosed                "��������� ���� ����� ���� ������� ��������"
#define MESSAGEMAIN_threadwasclosed2        "����� ����� ����� �������� ��� ��������"

#define MESSAGEMAIN_threadchangehided        "��������� ���� ����� ���� ������� ��������"
#define MESSAGEMAIN_threadchangehided2        "����� ����� ����� ������� ��� ���������"

#define MESSAGEMAIN_threaddeleted                "����� ���� ������� �������"
#define MESSAGEMAIN_threaddeleted2                ""

#define MESSAGEMAIN_threadrolled                "����� ���� ������� �������� ��� ����������"
#define MESSAGEMAIN_threadrolled2                ""

#define MESSAGEMAIN_messagechanged                "��������� ���� ��������"
#define MESSAGEMAIN_messagechanged2                "������� �� �������"

#define MESSAGEMAIN_incorrectpwd                 "��������� ��� ��� ���������� � ������ �������� ������"
#define MESSAGEMAIN_incorrectpwd2                 "��������� ������������ ��������� ������ � �����<BR>���� �� ������ ��������� ���������, �� ������������� � �����������, ���������� ������� ������ �� ������� ���"

#define MESSAGEMAIN_robotmessage                 "������������� ���������� ����� ���������"
#define MESSAGEMAIN_robotmessage2                 ""

#define MESSAGEMAIN_session_end                 "�� ��������� �������� ������ ��������� ������������������ �����������"
#define MESSAGEMAIN_session_end2                 "������ ����� ����� ������ ����������� ������� � �� ������ ������ ��� � ������ ������"

#define MESSAGEMAIN_logoff_not_logged_in        "�� �� ����� �� ��������� �����"
#define MESSAGEMAIN_logoff_not_logged_in2         "�� ��� �����, ��� �� ����� ������ ��� ��� ���"

#define MESSAGEMAIN_logoff_ok                        "�� ������� �����"
#define MESSAGEMAIN_logoff_ok2                         ""

#define MESSAGEMAIN_login_ok                        "%s, %s, �� ������� ����� � �����������"
#define MESSAGEMAIN_login_ok2                        ""

#define MESSAGEMAIN_login_helloday                "������ ����"
#define MESSAGEMAIN_login_helloevn                "������ �����"
#define MESSAGEMAIN_login_hellonight        "������ ����"
#define MESSAGEMAIN_login_hellomor                "������ ����"

#define MESSAGEMAIN_lostpassw_ok                "��� ������ ��� ��������� �� email"
#define MESSAGEMAIN_lostpassw_ok2                "������� �� ����������� �������� ����������� ������"

#define MESSAGEMAIN_access_denied                "������ � ���� ������� ��� ��� ��������"
#define MESSAGEMAIN_access_denied2                 "���������, ����� �� �� � �����������<BR>���� �� ��������, ��� ��� ������ - ���������� � ����������� �����������"

#define MESSAGEMAIN_spamtry                                "���������� ������� ����� ��� ������ �� ��������� ��������"
#define MESSAGEMAIN_spamtry2                         "���� �� ��������� ��������� ���������, �� ��� ������, ��� ��� ��� ���������� � ��� ������� ������ ��� ��� ���"

#define MESSAGEMAIN_BANNED_REASON                "������� ������� ���������� ��������� ��� ��� ���� :"

#define MESSAGEMAIN_browser_return                "<P ALIGN=CENTER>��� ������� ������ ������� ��� � ������ ��������� ����� ��������� ������</P>"

#define MESSAGEMAIN_browser_to_thread                "<P><P ALIGN=CENTER><B><A STYLE=\"text-decoration:underline;\" HREF=\"" MY_CGI_URL "?read=%lu\">���������</A> � ������ ���������</B></P></P><P></P>"

#define MESSAGEMAIN_admin_contact                 "���� �� �������� ��� ��� �����������, ���������� � <A HREF=\"mailto:" ADMIN_MAIL "\">��������������</A> �����������"

/***************** ����� �������� ��������� ******************/
#define        MESSAGEMAIN_post_newmessage                 "����� ���������"
#define        MESSAGEMAIN_post_replymessage                 "��������"
#define        MESSAGEMAIN_post_editmessage                 "�������� ���������"
#define        MESSAGEMAIN_post_bot  "�������� ���� � ������ ������ �������!<br>Leave fields in the first row blank!"

#define MESSAGEMAIN_post_you_name                        "<b>���:</b>"
#define MESSAGEMAIN_post_your_password                "<b>������:</b>"
#define MESSAGEMAIN_post_login_me                        "<b>���������</b>"
#define MESSAGEMAIN_post_hostname                        "<b>Host:</b>"
#define MESSAGEMAIN_post_message_subject        "<b>����:</b>"
#define MESSAGEMAIN_post_message_body                 "<b>���������:</b> "

#define MESSAGEMAIN_post_disable_wwwconf_tags        "��������� ������������� ����� �����������"
#define MESSAGEMAIN_post_disable_smile_tags                "��������� �������������  �����-�����"
#define MESSAGEMAIN_post_reply_acknl                         "�������� ����������� �� ������� �� �����"

#define MESSAGEMAIN_post_preview_message        "��������������� ��������"
#define MESSAGEMAIN_post_post_message                 "���������"
#define MESSAGEMAIN_post_edit_message                "��������"

/****************** ��������������� ����� ******************/
#define MESSAGEMAIN_register_intro                        "�����������"
#define MESSAGEMAIN_register_chg_prof_intro "��������� ��������"
#define MESSAGEMAIN_register_login                        "��� (*):"
#define MESSAGEMAIN_register_displayname        "������������ ��� :"
#define MESSAGEMAIN_register_oldpass_req        "��� ��������� �������� �� <U>������</U> ������� ��� ������ !"
#define MESSAGEMAIN_register_oldpassword        "������ ������ (*):"
#define MESSAGEMAIN_register_if_want_change        "���� �� ������ �������� ������ ������� ��� ����� ������, ����� �������� ��� ���� �������"
#define MESSAGEMAIN_register_password1                "����� ������ (*):"
#define MESSAGEMAIN_register_password2                "����� ������ ��� ��� (*):"
#define MESSAGEMAIN_register_full_name                "���� ������ ���:"
#define MESSAGEMAIN_register_validemail_req        "�� ������ ������� ����������� E-Mail ����� <BR>(�� ����� �������������� ��� ��������� ������ � ������ �� �����������)"
#define MESSAGEMAIN_register_bot "��������!<br>���� �� ������ ��� ����, �� ������� e-mail ������<br>�� ������, �������� ������ ������!<br>Attention!<br>If you see two fields, fill with e-mail only<br>the second one, leave the first one blank!"
#define MESSAGEMAIN_register_email  "E-Mail ����� (*):"
#define MESSAGEMAIN_register_email_pub                "��������� ������ � ������ E-Mail"
#define MESSAGEMAIN_register_homepage                "����� ����� �������� ��������:"
#define MESSAGEMAIN_register_icq                        "ICQ :"
#define MESSAGEMAIN_register_signature                "���� �� ������ ����������� ������� (����� ���������� �" \
                " ����� ���������� �� ���������,<BR>� �� ����� ���� ������� 254 ��������), ������� �� �����," \
                " ��� �������� ���� ������"
#define MESSAGEMAIN_register_selectedusers        "�������� ������������, ��������� ������� ����� ��������������<BR>" \
                "(������ ��� ������� �� ����� ������, ����������� ����� ������ 184 �������)"
#define MESSAGEMAIN_register_about                        "������� ���-������ � ����� ���������, ����� � �.�."
#define MESSAGEMAIN_register_private_prof        "��������� ������ � ������ ��������"
#define MESSAGEMAIN_register_always_emlackn        "������ �������� ������ �� ���� ��������� �� E-mail"
#define MESSAGEMAIN_register_pmsg_disable        "��������� ������ ��������� ���������"
#define MESSAGEMAIN_register_pmsg_email                "��������� ���������� � ������� ������ ���������� ��������� �� �����"
#define MESSAGEMAIN_register_req_fields                "��� ���� ���������� * �������� ������������� !"
#define MESSAGEMAIN_register_register                "����������������"
#define MESSAGEMAIN_register_edit                        "��������"
#define MESSAGEMAIN_register_delete                        "�������"
#define MESSAGEMAIN_register_confirm_delete        "����������� ��������"
#define MESSAGEMAIN_register_view_saving    "������������ ������� ��� �������� �������� �������� ����"

#define CONFIRM_DELETE_CHECKBOX_TEXT                "confirm_delete"

/**************** ��������������� ��������� ***************/
/**************** registration mnssages ***************/
#define MESSAGEMAIN_register_create_ex                "������� ������"
#define MESSAGEMAIN_register_create_ex2                "� �� ������������� �����"

#define MESSAGEMAIN_register_edit_ex                "������������ ��� ������ / �������"
#define MESSAGEMAIN_register_edit_ex2                ""

#define MESSAGEMAIN_register_edit_err                "��������� ������ ��� ���������� ��������"
#define MESSAGEMAIN_register_edit_err2                "��������, ����� ����������� ��� ���"

#define MESSAGEMAIN_register_delete_ex                "������������ ������"
#define MESSAGEMAIN_register_delete_ex2                ""

#define MESSAGEMAIN_register_delete_logoff        "��� ������� ��� ������"
#define MESSAGEMAIN_register_delete_logoff2        ""

#define MESSAGEMAIN_register_already_exit        "��� ��� ��� ������"
#define MESSAGEMAIN_register_already_exit2        "���������� ��������� ������ ���"

#define MESSAGEMAIN_register_invalid_psw        "��� ��� ������ ������� �����������"
#define MESSAGEMAIN_register_invalid_psw2        "�� ������ ������ ���������� ��� � ������ ��� ���� �� ���������" \
                                                                                        " ���������� � ���� �� ������ ����� ���� ����� � �������"

#define MESSAGEMAIN_register_invalid_n_psw        "������������� ������ �� ��������� ��� ������ ������� ��������"
#define MESSAGEMAIN_register_invalid_n_psw2        "��� ������������� ������ ��� ���������� ������ ��� �����" \
                                                                                        " ������ � ����� ������ ������ ���� �� ����� 3 ��������"

#define MESSAGEMAIN_register_invalid_email        "��������� E-Mail ����� �����������"
#define MESSAGEMAIN_register_invalid_email2        "����� ������������������ � ����������� �� ������ ������ �����������" \
                                                                                        " E-Mail ����� �� �� ������� ������������ ���������� ��������� �����������"

#define MESSAGEMAIN_register_cannot_delete        "������������ %s �� ����� ���� ������"
#define MESSAGEMAIN_register_cannot_delete2        "�� ������ ������ ���������� ��� � ������"

#define MESSAGEMAIN_register_invalid_lg_spell        "��� ������ ����� ������������ ��� ��� ������������"
#define MESSAGEMAIN_register_invalid_lg_spell2        "�� ������ ������ ���������� ��� � ������(����� ����� ������ ���� �� ������ 3 ��������)"


/***************** profile information *****************/
#define MESSAGEMAIN_profview_intro                        "���������� � ������������:"
#define MESSAGEMAIN_profview_login                        "���: "
#define MESSAGEMAIN_profview_postpersmsg        "��������� ��������� ���������"
#define MESSAGEMAIN_profview_editinfo                "��������"
#define MESSAGEMAIN_profview_altname                "�������������� ���: "
#define MESSAGEMAIN_profview_fullname                "������ ���: "
#define MESSAGEMAIN_profview_homepage                "�������� ��������: "
#define MESSAGEMAIN_profview_email                        "Email: "
#define MESSAGEMAIN_profview_dogsubstitute        " at "
#define MESSAGEMAIN_profview_user_icq                "ICQ: "
#define MESSAGEMAIN_profview_user_status        "������: "
#define MESSAGEMAIN_profview_u_moderator        "<FONT COLOR=#FF0000>���������</FONT>"
#define MESSAGEMAIN_profview_u_user                        "������� ������������"
#define MESSAGEMAIN_profview_postcount                "����� ���������: "
#define MESSAGEMAIN_profview_reg_date                "���� �����������: "
#define MESSAGEMAIN_profview_login_date                "����� ���������� �������: "
#define MESSAGEMAIN_profview_about_user                "� ������������: "
#define MESSAGEMAIN_profview_refreshcnt                "����� ����������: "
#define MESSAGEMAIN_profview_lastip                        "��������� ���� �: "
#define MESSAGEMAIN_profview_persmsgcnt                "����� ������������ ���������(���������): "

#define MESSAGEMAIN_profview_no_user                "<P></P><CENTER><span>������������ <span class=\"nick\">%s</span> �� " \
        "��������������� � ���� �����������</span></CENTER>"

#define MESSAGEMAIN_profview_privacy_prof        "���������� ���������� � ������������ ����������"
#define MESSAGEMAIN_profview_privacy_inf        "������ ����������"

#define MESSAGEMAIN_profview_sechdr                        "��� ������������� ��� ����: "
#define MESSAGEMAIN_profview_secbdy                        "��� ������������� ��� ����: "

//        user statuses
#define USER_STATUS_COUNT 5
extern const char *UserStatus_List[USER_STATUS_COUNT];

extern const char *UserRight_List[USERRIGHT_COUNT];

/******************* session information *******************/
#define MESSAGEMAIN_session_intro                "������������������ ������ "
#define MESSAGEMAIN_session_ip                        "IP ����� �����:"
#define MESSAGEMAIN_session_date                        "����� ���������� �������:"
#define MESSAGEMAIN_session_state                        "��������� ������:"
#define MESSAGEMAIN_session_state_active        "��������"
#define MESSAGEMAIN_session_state_toclose        "�������"
#define MESSAGEMAIN_session_state_closed        "�������"
#define MESSAGEMAIN_session_no                        "��� ������"
#define MESSAGEMAIN_session_closed_ok                "������ �������<BR>"
#define MESSAGEMAIN_session_closed_ok2        "������� �� �������!"
#define MESSAGEMAIN_session_closed_no                "������ �� �������<BR>"
#define MESSAGEMAIN_session_check_failed        "������ �� �������<BR>��������, ��� ���� ������ ��� �������"
#define MESSAGEMAIN_session_close_failed        "������ ��� �������� ������<BR>"
#define MESSAGEMAIN_session_ip_nocheck                "(��� ��������)"
#define MESSAGEMAIN_session_ip_check                ""

/************* private(personal) messages **************/
#define MESSAGEMAIN_privatemsg_send_message "��������� ���������"
#define MESSAGEMAIN_privatemsg_send_msg_btn "���������"
#define MESSAGEMAIN_privatemsg_prev_msg_btn "������������"
#define MESSAGEMAIN_privatemsg_send_msg_hdr "��������� ������ ���������"
#define MESSAGEMAIN_privatemsg_send_msg_usr        "������������: "
#define MESSAGEMAIN_privatemsg_send_msg_bdy "��������� (�� ����� ���� ������ 384 ��������)"

#define MESSAGEMAIN_privatemsg_msgwassent        "��������� ���� ���������� ������������"
#define MESSAGEMAIN_privatemsg_msgwassent2        "������� �� �������"

#define MESSAGEMAIN_privatemsg_msgcantsend        "��������� �� ����� ���� ���������� !"
#define MESSAGEMAIN_privatemsg_msgcantsend2        "���������� ����� !"

#define MESSAGEMAIN_privatemsg_denyunreg        "������ ����������� ���������� �������������������� ������������� !<BR>"
#define MESSAGEMAIN_privatemsg_denyunreg2        "��� ���� ���� ����� ����������� ���������� � �������� ������������ ��������� �� ������ ������������������ � ����������� !<BR><BR>"

#define MESSAGEMAIN_privatemsg_invalid_user        "�� ������ ������� ������������ ��� ������������"
#define MESSAGEMAIN_privatemsg_invalid_body "��������� �� ����� ���� ������ (����� ����� ����� ��� �������� ?)"
#define MESSAGEMAIN_privatemsg_tolong_body        "��������� ������� �������, ��������� ��� �����"
#define MESSAGEMAIN_privatemsg_disable_pmsg "������������ �������� ��������� ������������ ���������,<BR>��������� ��� ������������ ��������� ����������"

#define MESSAGEMAIN_privatemsg_header                "������������ ��������� ��� ������������"

#define MESSAGEMAIN_privatemsg_disabled                "��������� ������������ ���������� ���������!"

#define MESSAGEMAIN_privatemsg_newmsgcnt        "���������� ����� ���������:"
#define MESSAGEMAIN_privatemsg_nonewmsg                "����� ��������� ���"

#define        MESSAGEMAIN_privatemsg_allmsgcnt        "��������� �����, ��������:"
#define        MESSAGEMAIN_privatemsg_allmsgcnt1        "����������:"

#define MESSAGEMAIN_privatemsg_writenewmsg        "�������� ����� ������������ ���������"

#define MESSAGEMAIN_privatemsg_newmsgann        "����"
#define MESSAGEMAIN_privatemsg_newmsgann1        "������������� ������ ��������� ��� ���!"

#define MESSAGEMAIN_privatemsg_newmark                "<font color=red>[ ����� !]</font>"

#define MESSAGEMAIN_privatemsg_answer                "[��������]"
#define MESSAGEMAIN_privatemsg_write                "[��������]"


#define MESSAGEMAIN_privatemsg_fromuser                "�� ������������:"
#define MESSAGEMAIN_privatemsg_touser                "������������:"

#define MESSAGEMAIN_privatemsg_showall     "����������� ��� ���������"

/***************** favourites system *****************/
#define MESSAGEMAIN_favourites_denyunreg        "������ ����������� ���������� �������������������� ������������� !<BR>"
#define MESSAGEMAIN_favourites_denyunreg2        "��� ����, ����� ����� ����������� ������� ������ �� ��������� ���������, �� ������ ������������������ � ����� � ����������� !<BR><BR>"

#define MESSAGEMAIN_favourites_listclear        "��� ������ ���������� ����"

#if USER_FAVOURITES_SUPPORT == 1
#define MESSAGEMAIN_favourites_added                "��������� �������� � ���������!<BR>"
#define MESSAGEMAIN_favourites_added2                "������� �� ������� !<BR>������ ������ �� ��� ��������� ����� ����� � ������<BR>"
#define MESSAGEMAIN_favourites_addno                "��������� �� �������� � ���������!<BR>"
#define MESSAGEMAIN_favourites_addexist                "��������� ��� ������������ � ������!<BR>��������, �� ������ ��� ��� �������� � ��������� ������� ��� ��� ���<BR>"
#define MESSAGEMAIN_favourites_addnoplace        "� ��� ��� ��������� 20 ���������!<BR>��� ����, ����� �������� ��� ���������, ���������� ������� ���� �� ���� �� ��� �����������<BR>"
#define MESSAGEMAIN_favourites_deleted                "��������� ������� �� ����������!<BR>"
#define MESSAGEMAIN_favourites_deleted2                "������� �� ������� !<BR><BR>"
#define MESSAGEMAIN_favourites_delno                "��������� �� ������� �� ����������!<BR>"
#define MESSAGEMAIN_favourites_delnoexist        "��������� ��� � ������ !<BR> ��������, �� ������ ��� ��� �������<BR>"
#else
#define MESSAGEMAIN_favourites_added                "����� ������� � ���������!<BR>"
#define MESSAGEMAIN_favourites_added2                "������� �� ������� !<BR>������ ������ �� ���� ����� ����� ����� � ������<BR>"
#define MESSAGEMAIN_favourites_addno                "����� �� ������� � ���������!<BR>"
#define MESSAGEMAIN_favourites_addexist                "����� ��� ������������ � ������!<BR>��������, �� ������ ��� ��� ��������<BR>"
#define MESSAGEMAIN_favourites_addnoplace        "� ��� ��� ��������� 20 �������!<BR>��� ����, ����� �������� ���� ����, ���������� ������� ���� �� ���� �� ��� �����������<BR>"
#define MESSAGEMAIN_favourites_deleted                "����� ������ �� ����������!<BR>"
#define MESSAGEMAIN_favourites_deleted2                "������� �� ������� !<BR><BR>"
#define MESSAGEMAIN_favourites_delno                "����� �� ������ �� ����������!<BR>"
#define MESSAGEMAIN_favourites_delnoexist        "������ ��� � ������ !<BR> ��������, �� ������ ��� ��� �������<BR>"
#define MESSAGEMAIN_favourites_addnoparent        "��������� �� �������� ������ ��� ������� ������!<BR>��������� ����� ������ ������ ��������� � �����"
#endif

/****************** global announces *****************/
#define MESSAGEMAIN_globann_send_ann_hdr        "���������� ����� ���������� �����"
#define MESSAGEMAIN_globann_upd_ann_hdr                "�������� ���������� �����"
#define MESSAGEMAIN_globann_send_ann_body        "����� ������ (�� ����� ���� ������ 512 ��������)"
#define MESSAGEMAIN_globann_upd_ann_id                "�������� ����� ������ (����� ��������)"
#define MESSAGEMAIN_globann_send_ann_btn        "���������"
#define MESSAGEMAIN_globann_prev_ann_btn        "������������"

#define MESSAGEMAIN_globann_preview_hdr                "��������������� �������� ������"

#define MESSAGEMAIN_globann_anncantsend                "����� �� ����� ���� �������� !"
#define MESSAGEMAIN_globann_anncantsend2        "����� ���� ��� ����� ����������� �����..."

#define MESSAGEMAIN_globann_annwassent                "����� ��� ��������"
#define MESSAGEMAIN_globann_annwassent2                "������� �� �������"

#define MESSAGEMAIN_globann_annwasupdated        "����� ��� ��������"
#define MESSAGEMAIN_globann_annwasupdated2        "������� �� �������"

#define MESSAGEMAIN_globann_tolong                        "����� ������� �������, ��������� ��� � ��������� �������"
#define MESSAGEMAIN_globann_toshort                        "����� ������� ��������, ����� ��� ������ ������ ;) ?"

#define MESSAGEMAIN_globann_wasdeleted                "����� ��� ������� ������"
#define MESSAGEMAIN_globann_wasdeleted2                "<BR>"

#define MESSAGEMAIN_globann_cannotdel                "����� �� ����� ���� ������!"
#define MESSAGEMAIN_globann_cannotdel2                "��������, � ��� ������������ ���� ��� ��������"

#define MESSAGEMAIN_globann_invalidnum                "������ �� ������������ ������������� ������!"
#define MESSAGEMAIN_globann_invalidnum2                "��������� � �������� ������ �� ������� �� ������ ����"

#define MESSAGEMAIN_globann_delannounce                "������� ���� �����"
#define MESSAGEMAIN_globann_updannounce                "�������� ���� �����"
#define MESSAGEMAIN_globann_postedby                "���������"

#define MESSAGEMAIN_globann_showall                        "�������� ��� �������� ������"
#define MESSAGEMAIN_globann_hidenewann                "������ ������� ������"

/******************  moderation form  ******************/
#define MESSAGEMAIN_moderate_hide_thread         "������ ��� �����"
#define MESSAGEMAIN_moderate_close_thread        "������� ��� �����"
#define MESSAGEMAIN_moderate_unhide_thread         "�������� ��� �����"
#define MESSAGEMAIN_moderate_unclose_thread        "������� ��� �����"
#define MESSAGEMAIN_moderate_delete_thread        "������� ��� �����"
#define MESSAGEMAIN_moderate_change_message        "�������� ��� ���������"
#define MESSAGEMAIN_moderate_roll                        "�������� ��� �����"
#define MESSAGEMAIN_moderate_unroll                        "���������� ��� �����"

/********************* ����� ����� *********************/
#define MESSAGEMAIN_login_login_header        "������� ���� ��� � ������"
#define MESSAGEMAIN_login_loginname                " ��� : "
#define MESSAGEMAIN_login_password                " ������ : "
#define        MESSAGEMAIN_login_ipcheckshort                "�� ��������� IP"
#define MESSAGEMAIN_login_ipcheck                "�� ��������� IP ����� ��� ������"

#define MESSAGEMAIN_login_lostpassw                "<CENTER>���� �� ������ ��� ������ <A HREF=\"" MY_CGI_URL "?login=lostpasswform\">�������� ���</A></CENTER><BR><BR>"

/**************** ����� "������ ������" ****************/
#define MESSAGEMAIN_lostpassw_header        "<BIG>������� ����������� ������</BIG><BR><BR>������� ��� ��� � ����� ����������� �����<BR>" \
                                                                                "� �� �������� ������ � ����� �������"
#define MESSAGEMAIN_lostpassw_hretry        "<BOLD><FONT COLOR=RED>�� ������� �������� ��� �/��� ����� ����������� �����</FONT></BOLD><BR>" \
                                                                                "����������, ��������� ��� ��� � email � ��������� ������� ��� ���"
#define MESSAGEMAIN_lostpassw_loginname        " ��� : "
#define MESSAGEMAIN_lostpassw_email                " Email : "
#define MESSAGEMAIN_lostpassw_getpassw        "�������� ������"

/******************* message view form ******************/
#define MESSAGEMAIN_viewthread_sent                "��������� ���� �������:"
#define MESSAGEMAIN_viewthread_ipview        ", IP: "
#define MESSAGEMAIN_viewthread_date                "����:"
#define MESSAGEMAIN_viewthread_mdate        "�������:"
#define MESSAGEMAIN_viewthread_sigdisabled        "<U><I><SMALL>������������ ����� �������, �� ����������� �������� ��������� � ����������.</SMALL></I></U>"
#define        MESSAGEMAIN_in_this_thread                "<CENTER><BIG>��������� � ���� ������</BIG></CENTER>"

/********************** userlist ***********************/
#define MESSAGEMAIN_total_user_count                "����� �������������: "
#define MESSAGEMAIN_userlist_sortby                        "����������� ��: "
#define MESSAGEMAIN_userlist_sortbyname                "����"
#define MESSAGEMAIN_userlist_sortbydate                "���� ���������� �������"
#define MESSAGEMAIN_userlist_sortbypcnt                "���-�� ���������"
#define MESSAGEMAIN_userlist_sortbyhost                "�����/IP ������"
#define MESSAGEMAIN_userlist_sortbyrefresh        "�� ���������� ����������"
#define MESSAGEMAIN_userlist_sortbyright        "�� ������ �������������"

/******************  configure form   ******************/
#define MESSAGEHEAD_to_message                                "������� � �������"
#define MESSAGEHEAD_configure_showmsgs                "�������� �����"
#define MESSAGEHEAD_configure_msgslast                " �� ��������� "
#define MESSAGEHEAD_configure_lastnum                " ��������� "
#define MESSAGEHEAD_configure_showstyle                "����� ����������� : "
#define MESSAGEHEAD_configure_disablesmiles "��������� �������� �����-����"
#define MESSAGEHEAD_configure_disableuppic        "��������� ��������� �����������"
#define MESSAGEHEAD_configure_disable2links        "��������� ������ ������ ������"
#define MESSAGEHEAD_configure_ownpostshighlight        "��������� ��������� ����������� ���������"
#define MESSAGEHEAD_configure_showhostnames        "��������� ����������� ������ � �������"
#define MESSAGEHEAD_configure_showaltnames        "��������� ����������� �������������� ����"
#define MESSAGEHEAD_configure_showsign                "��������� ����������� �������� �������������"
#define MESSAGEHEAD_configure_plus_is_href        "\" " TAG_NEW_MSG_MARK "\" �������� ������� �� ��������� ����� ���������"
#define MESSAGEHEAD_configure_disablecolor        "��������� ����������� ������ � �������"
#define MESSAGEHEAD_configure_disablebot  "��������� ������ ������ �����"
#define MESSAGEHEAD_configure_showreplyform        "�� ���������� ����� ������"
#define MESSAGEHEAD_configure_applysettings        "��������� ���������"


#define MESSAGEHEAD_configure_saving_to_profile "��������� ����������� � �������� � ����� �������� � ������ ��������"
#define MESSAGEHEAD_configure_saving_to_browser "��������� ����������� � �������� � �������� ������ � ������� ����������"
#define MESSAGEHEAD_configure_view_saving    "�������� ������ �������� �������� ����� �� <a href=\"" MY_CGI_URL "?register=form\" style=\"text-decoration:underline;\"  >�������� �������������� ��������</a>"

// coded as "1"
//#define MESSAGEHEAD_configure_showhronforward        "�� ������� � ��������������� ����, �� �����������"
// coded as "2"
#define MESSAGEHEAD_configure_showhronbackward                "�� ������� � ��������������� ����, �� ��������"
// coded as "3"
#define MESSAGEHEAD_configure_showhronwothreads                "�� ���������� � ��������������� ����, �� �����������"
// coded as "4"
#define MESSAGEHEAD_configure_showhrononlyheaders        "�� ���������� � ��������������� ����, ������ ���������"


#define WWWCONF_FULL_NAME "��������� �������"

// for STABLE_TITLE = 1
#if STABLE_TITLE == 1
#define TITLE_StaticTitle                 "not used " VERSION
#endif

/******************* topic support *********************/
#if TOPICS_SYSTEM_SUPPORT
#define TOPICS_NUM          20
#define TOPICS_COUNT        10
#define TOPICS_DEFAULT_SELECTED        0
// this variable should be defined in main.cpp
extern const char *Topics_List[TOPICS_NUM];
extern DWORD blanktopics;
extern DWORD Topics_List_map[TOPICS_COUNT];
#endif

/**************************** title messages *****************************/
// for STABLE_TITLE = 0
// board name prefix
#define TITLE_WWWConfBegining                "��������� �������"
#define TITLE_divider                                " : "
// forms name
#define TITLE_WWWConfIndex                        " "
#define TITLE_Form                                        "����� ���������"
#define TITLE_WriteReply                        "�������� �����"
#define TITLE_Configure                                "���������"
#define TITLE_Error                                        "������ !"
#define TITLE_Spamtry                                "����������� ������� �����!"
#define TITLE_Login                                        "���� � �����������"
#define TITLE_Logoff                                "����� �� �����������"
#define TITLE_LostPassword                        "������ ������?"
#define TITLE_PasswordSent                        "������ ���������"
#define TITLE_IncorrectPassword                "������������ ������!"
#define TITLE_ClosingMessage                "������� ���������"
#define TITLE_HidingMessage                        "������ ���������"
#define TITLE_DeletingMessage                "������� ���������"
#define TITLE_ChangingMessage                "�������� ��������� - "
#define TITLE_RollMessage                        "���������� ���������"
#define TITLE_Registration                        "����������� / �������� �������"
#define TITLE_ProfileInfo                        "���������� � ������������"
#define TITLE_UserList                                "������ ������"
#define TITLE_PrivateMsg                        "��������� ���������"
#define TITLE_AddPrivateMsg                        "�������� ��������� ���������"
#define TITLE_PrivateMsgWasPosted        "��������� ��������� ����������"
#define TITLE_PostGlobalAnnounce        "�������� ���������� �����"
#define TITLE_GlobalAnnWasPosed                "���������� ����� ��������"
#define TITLE_GlobalAnnWasDeleted        "���������� ����� ������"
#define TITLE_FavouritesPage                "���������"
#define TITLE_FavouritesPageAdd                "���������� ���� � ���������"
#define TITLE_FavouritesPageDel                "�������� ���� �� ����������"
#define TITLE_BanSave                "���������� ������ ���������� ����"
#define TITLE_ClSession                        "�������� ������"

/***************************** welcome messages **************************/
#define WELCOME_CONFERENCE_HEADER        "<B>����:</B> "

#define MESSAGEMAIN_ACTIVITY_STATVIEW        "����������: <B><font title=\"�����\" color=\"blue\">%ld</font> "\
        "<font title=\"����\" color=\"#229911\">%ld</font></B> �� ��������� 10 �����"

#define MESSAGEMAIN_WELCOME_LOGGEDSTART        "<div class=\"info\"><div class=\"left_info\">" \
        WELCOME_CONFERENCE_HEADER MESSAGEMAIN_WELCOME_HELLOREG "%s%s<BR>%s%s</div>"\
        "<div class=\"right_info\">%s<BR>" MESSAGEMAIN_WELCOME_SELECTTOPIC " %s</div>"\
        "<div style=\"clear: both; height: 0px; overflow: hidden;\"><br/></div></div>"

#define MESSAGEMAIN_WELCOME_START        "<div class=\"info\"><div class=\"left_info\">"\
        "<form method=post action=\"" MY_CGI_URL "?login=action\">"\
        WELCOME_CONFERENCE_HEADER MESSAGEMAIN_login_loginname \
        "<INPUT TYPE=TEXT NAME=\"mname\" SIZE=15 " \
        "MAXLENGTH=%d VALUE=\"%s\">&nbsp;&nbsp;&nbsp;" MESSAGEMAIN_login_password \
        "<INPUT TYPE=PASSWORD NAME=\"mpswd\" SIZE=15 MAXLENGTH=%d>&nbsp;"\
        "<INPUT TYPE=CHECKBOX NAME=\"ipoff\" VALUE=1>" MESSAGEMAIN_login_ipcheckshort \
        "&nbsp;<INPUT TYPE=SUBMIT VALUE=\"Enter\"></form>%s%s</div>" \
        "<div class=\"right_info\">%s<BR>" MESSAGEMAIN_WELCOME_SELECTTOPIC " %s</div>"\
        "<div style=\"clear: both; height: 0px; overflow: hidden;\"><br/></div></div>"




#define MESSAGEMAIN_WELCOME_HELLOREG                "�� ����� � ����������� ��� "

#if TOPICS_SYSTEM_SUPPORT
#define MESSAGEMAIN_WELCOME_SELECTTOPIC  "�������� ����:"
#else
#define MESSAGEMAIN_WELCOME_SELECTTOPIC  ""
#endif

#define MESSAGEMAIN_WELCOME_NEWTHREADS_TEXT                "����� �������(���������): <a href=\"#n0\" "\
        "style=\"color: #f00;font-weight:bold;\">+</a> %lu(%lu) �� %lu"
#define MESSAGEMAIN_WELCOME_NEWTHREADS "<span id=\"new_count\">" MESSAGEMAIN_WELCOME_NEWTHREADS_TEXT "</span>"

#define MESSAGEMAIN_WELCOME_NONEWTHREADS_TEXT        "����� ��������� ���, ����� %ld"
#define MESSAGEMAIN_WELCOME_NONEWTHREADS "<span id=\"new_count\">"\
                        MESSAGEMAIN_WELCOME_NONEWTHREADS_TEXT "</span>"

#define MESSAGEMAIN_WELCOME_DISPLAYTIME                ", �������� ��������� �� %lu %s"
#define MESSAGEMAIN_WELCOME_DISPLAYTHREADS        ", �������� ��������� %lu �������"

#define MESSAGEMAIN_WELCOME_YOURSETTINGS        "��������� ����"
#define MESSAGEMAIN_WELCOME_ALLTOPICS                "��� ����"

/**************************** date/time messages *************************/
#define MESSAGEMAIN_DATETIME_JAN                        "������"
#define MESSAGEMAIN_DATETIME_FEB                        "�������"
#define MESSAGEMAIN_DATETIME_MAR                        "����"
#define MESSAGEMAIN_DATETIME_APR                        "������"
#define MESSAGEMAIN_DATETIME_MAY                        "���"
#define MESSAGEMAIN_DATETIME_JUN                        "����"
#define MESSAGEMAIN_DATETIME_JUL                        "����"
#define MESSAGEMAIN_DATETIME_AUG                        "������"
#define MESSAGEMAIN_DATETIME_SEP                        "��������"
#define MESSAGEMAIN_DATETIME_OCT                        "�������"
#define MESSAGEMAIN_DATETIME_NOV                        "������"
#define MESSAGEMAIN_DATETIME_DEC                        "�������"

#define MESSAGEMAIN_DATETIME_DAY_SUN                "�����������"
#define MESSAGEMAIN_DATETIME_DAY_MON                "�����������"
#define MESSAGEMAIN_DATETIME_DAY_TEU                "�������"
#define MESSAGEMAIN_DATETIME_DAY_WED                "�����"
#define MESSAGEMAIN_DATETIME_DAY_THU                "�������"
#define MESSAGEMAIN_DATETIME_DAY_FRI                "�������"
#define MESSAGEMAIN_DATETIME_DAY_SAT                "�������"

/**************************** mailing messeges ***************************/

#define MAILACKN_HEADER "Content-type: text/html; charset=\"windows-1251\"\r\n\r\n"


#define MAIL_SEND_GREETING        "<HTML>������������, <bold><font color=#009000>"\
                "<strong>%s</strong></font></bold>.<br />"

#define MAIL_SEND_SIGNING        "��� ������ ������� ������������� ������� " WWWCONF_FULL_NAME \
                ".<br />��� <b>��</b> ������� �������� �� ����.<br /></HTML>"

//        reply notification
#define MAILACKN_REPLY_SUBJECT                        WWWCONF_FULL_NAME ": %s"
#define MAILACKN_REPLY_BODY                MAIL_SEND_GREETING \
"%s ������� �� ���� ��������� <strong>%s</strong>.<br /><br />"\
"���� ������:<strong> %s</strong><br />"\
"----------- ��������� ---------<br />%s<br />-------------------------------<br /><br />"\
"���� ���� ����� ������� <a href=\"%s?read=%lu\">�����</A><br /><br />"

//        password recovery
#define MAILACKN_LOSTPASS_SUBJECT          WWWCONF_FULL_NAME ": ������ ��� ������� %s"
#define MAILACKN_LOSTPASS_BODY        MAIL_SEND_GREETING \
"������� ������ ��� ������ �������: %s<br /><br /><br />"


//        private message
#define MAILACKN_PRIVATEMSG_SUBJECT        WWWCONF_FULL_NAME ": ����� ������ ��������� �� %s"
#define MAILACKN_PRIVATEMSG_BODY        MAIL_SEND_GREETING \
"��� ��� �������� ����� ������ ��������� �� <strong>%s</strong>.<br /><br />"\
"----------- ��������� ---------<br />%s<br />-------------------------------<br /><br />"\
"�������� �� ���� ����� <a href=\"%s?persmsgform=%s\">�����</a><br />" \
"���������� ��������� ������ ��������� �� ������ <a href=\"%s?persmsg\">�����</A><br /><br />"



/**************************** mailing messeges ***************************/
/*#define MAILACKN_SUBJECT                        WWWCONF_FULL_NAME ": %s"
#define MAILACKN_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"������������, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR> %s ������� �� ���� ��������� <STRONG>%s</STRONG>." \
"<BR><BR>���� ������:<STRONG> %s</STRONG>\n<BR>----------- ��������� ---------<BR>%s<BR>-------------------------------" \
"<BR><BR>���� ���� ����� ������� <A HREF=" WC_EMAILACN_READURL "?read=%d>�����</A>" \
"<BR><BR>This mail was generated automatically by mailing engine" \
" of " WWWCONF_FULL_NAME ".<BR>You <B>SHOULD NOT REPLY</B> to this message.<BR></HTML>"
*/
//        password recovery
/*#define MAILLOSTPASSW_SUBJECT                WWWCONF_FULL_NAME ": ������ ��� ������� %s"
#define MAILLOSTPASSW_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"������������, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR>������� ������ ��� ������ �������: %s<BR>" \
"<BR><BR>This mail was generated automatically by mailing engine" \
" of " WWWCONF_FULL_NAME ".<BR>You <B>SHOULD NOT REPLY</B> to this message.<BR></HTML>"

#define MAILACKN_PRIVATEMSG_SUBJECT                        WWWCONF_FULL_NAME ": ����� ������ ��������� �� %s"
#define MAILACKN_PRIVATEMSG_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"������������, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR> ��� ��� �������� ����� ������ ��������� �� <STRONG>%s</STRONG>." \
"<BR><BR>----------- ��������� ---------<BR>%s<BR>-------------------------------" \
"<BR><BR>�������� �� ���� ����� <A HREF=" WC_EMAILACN_READURL "?persmsgform=%s>�����</A><BR>" \
"���������� ��������� ������ ��������� �� ������ <A HREF=" WC_EMAILACN_READURL "?persmsg>�����</A>" \
"<BR><BR>This mail was generated automatically by mailing engine" \
" of " WWWCONF_FULL_NAME ".<BR>You <B>SHOULD NOT REPLY</B> to this message.<BR></HTML>"
*/

/**************************** log messages *******************************/
#define LOG_SPAM_TRY                                "Spam try from %s:, deal=%s"
#define LOG_UNKNOWN_URL                                "Unknown URL request from : %s, deal=%s"
#define LOG_UNHANDLED                                "UNHANDLED EXCEPTION at %s at line %d from : %s\n\tError: %s\t\tQUERY_STRING=%s"
#define LOG_UNHANDLED_HTML                        "UNHANDLED EXCEPTION at %s at line %lu from : %s<BR><table width=100%% cellpadding=1 cellspacing=1 border=1 bgcolor=#E1E1E1><TR><TD>%s</TD><TD>QUERY_STRING=%s</TD></TR></TABLE>"
#define LOG_ERRORTYPEUNKN                        "Unknown"
#define LOG_ACCESS_DENIED                        "Access denied from : %s, deal=%s"
#define LOG_PSWDERROR                                "Incorrect password from : %s, deal=%s"
#define LOG_FATAL_NOMEMORY                        "Unable to allocate memory block of size %d"
#define LOG_FILESIZETOOHIGH                        "Size of file %s is too high to be handled by this version"
#define LOG_GETFILESIZEFAILED                "Filesize() failed for file %s"
#define LOG_UNABLETOLOCATEFILE                "Unable to open/access file %s for R"
#define LOG_UNABLETOLOCATEFILERW        "Unable to open/access file %s for RW"
#define LOG_UNABLETOCREATEFILE                "Unable to open/create file %s"
#define LOG_WARN_UNABLETOOPENFILE        "WARNING: Unable to open/access file %s, feature connected to this file will not be used"

/*=====================================================================*/
#define TAG_MSG_HAVE_BODY              "+"
#define TAG_MSG_HAVE_NO_BODY           "-"

#define TAG_MSG_HAVE_PIC               "pic"
#define TAG_MSG_HAVE_URL               "url"
#define TAG_MSG_HAVE_TEX               "tex"
#define TAG_MSG_HAVE_TUB               "tub"

#define TAG_MSG_CLOSED_THREAD          "<span class=\"close\" id=\"c%ld\">(�������) </span>"

#define TAG_MSG_ROLLED_THREAD          "<div id=\"r%ld\"><span class=\"roll1\">��������: </span>" \
        "<span class=roll2>(�������: <b>%lu</b>)</span><br></div>"

#define TAG_MSG_ROLLED_THREAD_MARKNEW  "<div id=\"r%ld\"><span class=\"roll1\">��������: </span>" \
        "<span class=roll2>(�������: <b>%lu</b>, �����: <b>%d</b>, ��������� �����: " \
        "<a href=\"%s?read=%ld\">�� %s, %s</a>)</span><br></div>"

#define TAG_IP_NOT_DETECTED  "X.X.X.X"

#define BAD_WORD_SYMBOL  '#'

#define TAG_NEW_MSG_MARK_HREF  "<span class=e id=p%ld><a name=n%ld href=\"#n%ld\">+</a>&nbsp;</span>"
#define TAG_NEW_MSG_MARK       "<span class=e>+&nbsp;</span>"

#define DESIGN_NOBR_START  "<span class=\"nobr\">"
#define DESIGN_WBR_START   "<span class=\"wbr\">"
#define DESIGN_BR_END      "</span>"
#define DESIGN_WRAP        "<span> </span>"


/******************** Unicode BiDi ************************/
#define DESIGN_BIDI_LRO     "<span class=\"lro\">"
#define DESIGN_BIDI_LRE     "<span class=\"lre\">"
#define DESIGN_BIDI_RLE     "<span class=\"rle\">"
#define DESIGN_BIDI_RLO     "<span class=\"rlo\">"
#define DESIGN_BIDI_CLOSE   "</span>"
#define DESIGN_BIDI_MAXLEN  21  // max length of the tags

#define DESIGN_BIDI_LTR_GUARD  "<span class=\"lre\"></span>"
#define DESIGN_BIDI_RTL_GUARD  "<span class=\"lre\"></span>"

/*********************** Topics **************************/
#define DESIGN_TOPIC_TAG_OPEN                "["
#define DESIGN_TOPIC_TAG_CLOSE                "]"
#define DESIGN_TOPIC_DIVIDER                " &nbsp;"

/***************** nick name information *****************/
#define DESIGN_REGISTRED_NICK                "<span class=\"reg\">%s</span>"
#define DESIGN_REGISTRED_OWN_NICK        "<span class=\"own\">%s</span>"
#define DESIGN_SELECTEDUSER_NICK        "<span class=\"sel\">%s</span>"
#define DESIGN_UNREGISTRED_NICK                "<span class=\"unr\">%s</span>"

/********************** favorites ************************/
#define DESIGN_FAVORITES_DEL_THREAD "<FONT COLOR=RED SIZE=3>[X]</FONT>"
#define DESIGN_FAVORITES_ADD_THREAD "<FONT COLOR=GREEN SIZE=3>[+]</FONT>"

#define DESIGN_THREADS_DIVIDER_IMG        ""
#define DESIGN_THREADS_DIVIDER_HR        "\n<HR size=1>"
#define DESIGN_THREADS_DIVIDER_grey        "#E9E9E9"

extern char DESIGN_open_dl[10];
extern char DESIGN_open_dl_grey[20];
extern char DESIGN_open_dl_white[20];
extern char DESIGN_close_dl[10];
extern char DESIGN_break[10];

#define DESIGN_OP_UL        "<ul>"
#define DESIGN_OP_UL_grey   "<ul class=\"g\">"
#define DESIGN_OP_UL_white  "<ul class=\"w\">"
#define DESIGN_CL_UL        "</ul>"
#define DESIGN_BR           "<br>"

#define DESIGN_BUTTONS_DIVIDER                        "&nbsp;&nbsp;"

#define DESIGN_GLOBAL_BOARD_MESSAGE                "<P ALIGN=CENTER><BIG><STRONG>%s</STRONG></BIG><BR><BR>%s"

#define DESIGN_VIEW_THREAD_BODY                        "<br /><div class=\"body\">%s</div>"
#define DESIGN_VIEW_THREAD_SIGN                        "<br /><div class=\"sign\">%s</div>"
#define DESIGN_VIEW_THREAD_MSG_HEADER        "<BR><DIV ALIGN=CENTER>"
// meesage was sent by NIK
#define DESIGN_VIEW_THREAD_MSG_SENT                "<BR>%s %s "
#define DESIGN_VIEW_THREAD_MSG_SENT1        "&lt;<A HREF=\"mailto:%s\">%s</A>&gt; "
#define DESIGN_VIEW_THREAD_MSG_SENT2        "<small>(%s)</small>"
#define DESIGN_VIEW_THREAD_MSG_SENT3        "</DIV><BR>"
// topic when reading
#define DESIGN_VIEW_THREAD_TOPIC                "<BIG>[%s]</BIG>&nbsp;&nbsp;"
//date
#define DESIGN_VIEW_THREAD_DATE                        "<BR>%s %s"
#define DESIGN_VIEW_THREAD_MDATE                "<I>(%s %s)</I>"

#define DESIGN_POST_NEW_MESSAGE_TABLE        "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=3>"

#define DESIGN_PREVIEW_PREVIEWMESSAGE        "<P ALIGN=CENTER><STRONG>%s</STRONG>"
#define DESIGN_PREVIEW_CHANGEMESSAGE        "<P ALIGN=CENTER><STRONG>%s</STRONG>"

#define DESIGN_BAN_REASON_STYLE                        "<P ALIGN=CENTER> %s <BOLD><EM>%s</EM></BOLD>"

#define DESIGN_MODERATOR_ENTER_HEADER        "<P ALIGN=CENTER>%s"

#define DESIGN_LOSTPASSW_HEADER                        "<P ALIGN=CENTER>%s"

#define DESIGN_COMMAND_TABLE_BEGIN        "<div class=\"menu\">"
#define DESIGN_COMMAND_TABLE_END        "</div>"

#define DESIGN_BEGIN_LOGIN_OPEN                        "<CENTER><TABLE BORDER=0 CELLSPACING=0 CELLPADDING=3>"
#define DESIGN_END_LOGIN_CLOSE                        "</TABLE></CENTER>"

#define DESIGN_BEGIN_LOSTPASSW_OPEN                "<CENTER><TABLE BORDER=0 CELLSPACING=0 CELLPADDING=3>"
#define DESIGN_END_LOSTPASSW_CLOSE                "</TABLE></CENTER>"

#define DESIGN_BEGIN_REGISTER_OPEN                "<CENTER><TABLE BORDER=0 CELLSPACING=1 CELLPADDING=3>"
#define DESIGN_END_REGISTER_CLOSE                "</TABLE></CENTER>"
// nick in read_message
#define DESIGN_MESSAGE_REG    "<a href=\"" MY_CGI_URL "?uinfo=%s\" class=\"nn\" target=\"_blank\">%s</a>"
#define DESIGN_MESSAGE_UNREG                        "<span class=\"unreg\">%s</span>"

#define DESIGN_BEGIN_USERINFO_INTRO_OPEN        "<CENTER><TABLE BORDER=0 CELLSPACING=1 CELLPADDING=3>"
#define DESIGN_END_USERINFO_INTRO_CLOSE                "</TABLE></CENTER>"

/******************* private messages *******************/
#define DESIGN_PRIVATEMSG_FRAME                        "<div class=\"pr_info\">%s<br>%s %s <A HREF=\"" MY_CGI_URL "?persmsgform=%s\">"\
                                                                                "%s</A><div class=\"%s\">%s</div></div>"
#define DESIGN_PRIVATEMSG_FRAME_IN                "pr_from"
#define DESIGN_PRIVATEMSG_FRAME_OUT                "pr_to"

/********************** announces ***********************/
#define DESIGN_GLOBALANN_FRAME                "<div class=\"an\">%s<BR>%s %s (%s) %s</div>"

#define DESIGN_CONFIGURE_CHECKALL "��� ����  <input type=checkbox checked onClick=\"selectall(checked)\">" \
"\n<script language=\"JavaScript\" type=\"text/javascript\">\n" \
"function selectall(state){\n" \
"                        var item,i=0;\n" \
"                        while (1) {\n" \
"                                item=eval(\"document.configure.topic\"+i);\n" \
"                                if (item) {        item.checked = state;i++;\n" \
"                                } else { break;        }\n" \
"                        }\n" \
"}" \
"</script>\n"

#define DESIGN_WELCOME_QUICKNAV                "<SELECT CLASS=CL onchange=\"if (this.options[this.selectedIndex].value == '') this.selectedIndex=0;" \
"else window.open(this.options[this.selectedIndex].value,'_top')\"><OPTION VALUE=\"" MY_CGI_URL "?index=0\"%s>%s</OPTION>" \
"<OPTION VALUE=\"" MY_CGI_URL "?index=all\"%s>%s</OPTION>"


/************************* ban list *************************/
#define DESIGN_BAN_FORM        "<center><form action=\"" MY_CGI_URL "?banlist=save\" method=\"post\">"\
                                        "<textarea name=\"ban_list\" rows=\"30\" cols=\"70\" wrap=\"soft\" maxlength=\"%d\">"

#define DESIGN_BAN_FORM2        "</textarea><br /><input type=\"submit\" name=\"submit\" value=\"Save\">"\
                                        "</form></center>"

/******************* new msg **************************/

#define DESIGN_STYLE_BUTTONS_BEGIN()                            \
        printf("<tr><td colspan=\"2\" align=\"center\">"

#define DESIGN_STYLE_BUTTONS_END()              \
        "</td></tr>")

#define DESIGN_STYLE_BUTTONS_NEWLINE() "<br />"

#define DESIGN_STYLE_BUTTONS_ADD(accesskey, width, label, name, title, callback, argumentlist) \
        "<button type=\"button\" class=\"style\" name=\"" name "\" accesskey=\"" accesskey "\" style=\"width: " width "\" title=\"" title "\" " \
        "onclick=\"" callback "(" argumentlist ");\">" label "</button>&nbsp;"

#define DESIGN_STYLE_BUTTONS_ADD_WRAP(accesskey, width, label, title, opentag, closetag, allowsubj) \
	DESIGN_STYLE_BUTTONS_ADD(accesskey, width, label, "", title, "wrap", "'" opentag "', '" closetag "', " #allowsubj)

#define DESIGN_STYLE_BUTTONS_ADD_INSERT(accesskey, width, label, title, tag, allowsubj) \
	DESIGN_STYLE_BUTTONS_ADD(accesskey, width, label, "", title, "insert", "'" tag "', " #allowsubj)

#define DESIGN_STYLE_BUTTONS_ADD_SHOWSMILE()				\
	DESIGN_STYLE_BUTTONS_ADD("0", "55px", "smile", "smile", "������� ������� (alt+0)", "show", "")

#define DESIGN_STYLE_SMILES_BEGIN()                                     \
        "<div class=\"smiles\" id=\"smiles\"><table class=\"smiles\"><tr>"

#define DESIGN_STYLE_SMILES_END()               \
        "</tr></table></div>"

#define DESIGN_STYLE_SMILES_NEWLINE() "</tr><tr>"

#define DESIGN_STYLE_SMILES_ADD(pic, smile)                             \
	"<td><button type=\"button\" class=\"smile\" onclick=\"insert(' " smile " ', 0)\" title=\"" smile "\">" \
        "<img src=\"" BOARD_PIC_URL pic "\" /></button></td>"

#define DESIGN_FORM_MESSAGE_BODY        "<textarea cols=\"75\" rows=\"12\" name=\"body\" class=\"post\" wrap=\"virtual\" " \
"onfocus=\"last = document.postform.body;\" \">%s</textarea></td></tr>"

#define DESIGN_FORM_MESSAGE_CHECKBOX "<tr><td colspan=\"2\" align=\"right\" class=\"cl\">%s"\
"<input type=\"checkbox\" name=\"%s\" class=\"cl\" %s /></td></tr>"

#define DESIGN_FORM_MESSAGE_BUTTON  "<INPUT TYPE=SUBMIT NAME=\"%s\" VALUE=\"%s\" class=\"fb\" tabindex=\"%d\">"

#define XML_MES_STATUS_TEMPLATE   "<message id=\"%lu\"><status>%s</status></message>"
#define XML_MES_STATUS_BASELEN    (strlen(XML_MES_STATUS_TEMPLATE) - strlen("%lu%s"))
#define XML_MES_STATUS_DELETED    "deleted"
#define XML_MES_STATUS_CLOSED     "closed"
#define XML_MES_STATUS_NOTEXISTS  "not_exists"
#define XML_MES_STATUS_HIDED      "hided"

#define XML_BANNED   "<error><type>forbidden</type></error>"
#define XMLFP_BANNED "You are forbideen to read messages."
#define XMLFP_WRONG_BOUNDS  "Wrong bounds."

//****************************************************//
#define HTML_START "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" "\
        "\"http://www.w3.org/TR/html4/loose.dtd\">\n<html><head>"

#define HTML_END "</BODY></HTML>"

#define RSS_START "<?xml version=\"1.0\" encoding=\"windows-1251\"?>"\
                               "<rss version=\"2.0\">"\
                               "<channel>"\
                               "<title>BOARD RSS</title>"\
                               "<link>%s</link>"\
                               "<description>BOARD RSS</description>"\
                               "<language>ru</language>"

#define RSS_END "</channel></rss>"

#define XML_START  "Content-type: application/xml\n\n" \
        "<?xml version=\"1.0\" encoding=\"windows-1251\"?>"

#define PLAIN_START  "Content-type: text/plain; charset=windows-1251\n\n"

#define HTML_ENCODING_HEADER        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\" />"\
        "<link rel=\"shortcut icon\" href=\"favicon.ico\" />"\
        "<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" />"\
        "<link rel=\"alternate\" type=\"application/rss+xml\" title=\"RSS\" href=\"?index=rss\" />"\
        "<meta http-equiv=\"Page-Exit\" content=\"progid:DXImageTransform.Microsoft.Fade(Duration=0.2)\" />"

#define HTML_STYLE_HEADER "</head><body>\n"

#define HTML_BOTTOMBANNER                                               \
        "<div style=\"font-size: 7pt; text-align: center;margin:10px;\">" \
        "����������� �������� �� ������ "                               \
        "<A STYLE=\"text-decoration:underline;\" href=\"https://github.com/Nable80/wwwconf\" TARGET=\"_blank\">wwwconf</A>.</div>"

#endif
