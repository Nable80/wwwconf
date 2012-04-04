 /***************************************************************************
                          messages.h  -  Все сообщения конференции
                             -------------------
    begin                : Fri Mar 23 2001
    перевод                 : Александр Гляков <vektor@3ka.mipt.ru>
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED

#include "basetypes.h"

/* константы связанные со временем
 * 0,1,2,3 -> час день неделя месяц
 */
extern const char *MESSAGEHEAD_timetypes[4];

#define MESSAGEHEAD_preview_preview_message        "Предварительный просмотр"
#define MESSAGEHEAD_preview_change_message        ""

#define MESSAGEHEAD_configure                                 "Настройки"
#define MESSAGEHEAD_resetnew                                "Сбросить +"
#define MESSAGEHEAD_post_new_message                 "Новое сообщение"
#define MESSAGEHEAD_return_to_main_page                "На главную страницу"
#define MESSAGEHEAD_logoff                                        "Выход"
#define MESSAGEHEAD_help_showhelp                        "Правила"
#define MESSAGEHEAD_register                                "Регистрация"
#define MESSAGEHEAD_registerprf                                "Ваш профайл"
#define MESSAGEHEAD_search                                        "Поиск"
#define MESSAGEHEAD_login                                        "Вход"
#define MESSAGEHEAD_userlist                                "Пользователи"
#define MESSAGEHEAD_personalmsg                                "Приват"
#define MESSAGEHEAD_makeannounce                        "Добавить анонс"
#define MESSAGEHEAD_favourites                                "Избранное"
#define MESSAGEHEAD_banlist                                "Банлист"

#define MESSAGEMAIN_ban_save                        "Спасибо. Список сохранен успешно."
#define MESSAGEMAIN_ban_save2                        "Через несколько секунд вы вернетесь к списку сообщений"

#define MESSAGEMAIN_ban_no_save                        "Внимание! Список не сохранен!"
#define MESSAGEMAIN_ban_empty                        "Отправленный список пуст. "


#define MESSAGEMAIN_add_banned                        "Сообщение не может быть добавлено"
#define MESSAGEMAIN_add_banned2                 "Отправка сообщений с вашего компьютера была запрещена Администратором"
#define MESSAGEMAIN_add_closed                        "Сообщение не может быть добавлено"
#define MESSAGEMAIN_add_closed2                 "Эта ветвь закрыта Администратором, новые сообщения не принимаются"
#define MESSAGEMAIN_add_invalid_reply        "Сообщение не может быть добавлено"
#define MESSAGEMAIN_add_invalid_reply2        "Сообщение на которое Вы хотите ответить не существует!"
#define MESSAGEMAIN_edit_denied                 "Сообщение не может быть отредактировано"
#define MESSAGEMAIN_edit_denied2                "Возможно, у Вас нет прав на редактирование сообщений"
#define MESSAGEMAN_invisible    "Сообщение не может быть добавлено или изменено"
#define MESSAGEMAN_invisible2   "Сообщение было скрыто Администратором"

#define MESSAGEMAIN_add_ok                                "Сообщение добавлено"
#define MESSAGEMAIN_add_ok_login                "Сообщение добавлено, Вы вошли в конференцию "
#define MESSAGEMAIN_add_ok2                                "Спасибо за участие. "

#define MESSAGEMAIN_add_spelling                "Сообщение не может быть добавлено "
#define MESSAGEMAIN_add_spelling2                "Ваше сообщение содержит запрещенные слова, попробуйте написать его без них"

#define MESSAGEMAIN_add_no_name                        "В вашем сообщении нет имени"
#define MESSAGEMAIN_add_no_name2                 "Чтобы отправить сообщение, Вы должны указать по крайней мере Тему и ваше Имя"

#define MESSAGEMAIN_add_no_subject                "В вашем сообщении нет темы"
#define MESSAGEMAIN_add_no_subject2         "Чтобы отправить сообщение, Вы должны указать по крайней мере Тему и ваше Имя"

// TODO :.....
#define MESSAGEMAIN_add_no_right_thrd        "Создание нового потока было запрещено вам модераторами"
#define MESSAGEMAIN_add_no_right_thrd2        "Если Вы считаете, что это ошибка свяжитесь с модераторами форума"

#define MESSAGEMAIN_add_emptymsg_java        "Напишите, пожалуйста, что-нибудь и укажите по возможности топик Вашего сообщения! Спасибо!"
#define MESSAGEMAIN_add_tolong_java                "Размер Вашего сообщения превышает максимум на"
#define MESSAGEMAIN_add_tolong_java2        "символов! Пожалуйста, уменьшите его, разбив на несколько частей! Спасибо!"

#define MESSAGEMAIN_add_no_body                        "В вашем сообщении нет тела"
#define MESSAGEMAIN_add_no_body2                 "Чтобы отправить сообщение, Вы должны написать тело сообщения"

#define MESSAGEMAIN_add_flood                        "Вы пытаетесь повторно отправить сообщение или время между вашими сообщениями слишком мало"
#define MESSAGEMAIN_add_flood2                         "Если вы пытаетесь отправить новое сообщение, попробуйте сделать это через некоторое время"

#define MESSAGEMAIN_unknownerr                        "Произошла неизвестная ошибка"
#define MESSAGEMAIN_unknownerr2                 "Сообщите пожалуйста Администратору об этой ошибке"

#define MESSAGEMAIN_requesterror                 "Ошибочный запрос на действие"
#define MESSAGEMAIN_requesterror2                 "Вы или ваш броузер передали неправильную ссылку, и скрипт не может ее обработать."

#define MESSAGEMAIN_nonexistingmsg                 "Это сообщение не существует или введено неправильно"
#define MESSAGEMAIN_nonexistingmsg2         "Вы или ваш броузер передали неправильный номер сообщения, перезагрузите пожалуйста страницу или введите правильный номер"

#define MESSAGEMAIN_threadwasclosed                "Состояние этой ветви было успешно изменено"
#define MESSAGEMAIN_threadwasclosed2        "Ветвь могла стать закрытой или открытой"

#define MESSAGEMAIN_threadchangehided        "Видимость этой ветви была успешно изменена"
#define MESSAGEMAIN_threadchangehided2        "Ветвь могла стать видимой или невидимой"

#define MESSAGEMAIN_threaddeleted                "Ветвь была успешно удалена"
#define MESSAGEMAIN_threaddeleted2                ""

#define MESSAGEMAIN_threadrolled                "Ветвь была успешно свернута или развернута"
#define MESSAGEMAIN_threadrolled2                ""

#define MESSAGEMAIN_messagechanged                "Сообщение было изменено"
#define MESSAGEMAIN_messagechanged2                "Спасибо за участие"

#define MESSAGEMAIN_incorrectpwd                 "Указанное имя уже существует и введен неверный пароль"
#define MESSAGEMAIN_incorrectpwd2                 "Проверьте правильность написания пароля и имени<BR>Если Вы хотите отправить сообщение, не регистрируясь в конференции, попробуйте указать другое не занятое имя"

#define MESSAGEMAIN_robotmessage                 "Использование указанного имени запрещено"
#define MESSAGEMAIN_robotmessage2                 ""

#define MESSAGEMAIN_session_end                 "Вы пытаетесь получить доступ используя неавторизированное подключение"
#define MESSAGEMAIN_session_end2                 "Скорее всего время вашего подключения истекло и Вы должны ввести имя и пароль заново"

#define MESSAGEMAIN_logoff_not_logged_in        "Вы не вошли но пытаетесь выйти"
#define MESSAGEMAIN_logoff_not_logged_in2         "Вы уже вышли, Вам не нужно делать это еще раз"

#define MESSAGEMAIN_logoff_ok                        "Вы успешно вышли"
#define MESSAGEMAIN_logoff_ok2                         ""

#define MESSAGEMAIN_login_ok                        "%s, %s, Вы успешно вошли в конференцию"
#define MESSAGEMAIN_login_ok2                        ""

#define MESSAGEMAIN_login_helloday                "Добрый день"
#define MESSAGEMAIN_login_helloevn                "Добрый вечер"
#define MESSAGEMAIN_login_hellonight        "Доброй ночи"
#define MESSAGEMAIN_login_hellomor                "Доброе утро"

#define MESSAGEMAIN_lostpassw_ok                "Ваш пароль был отправлен по email"
#define MESSAGEMAIN_lostpassw_ok2                "Спасибо за пользование системой напоминания пароля"

#define MESSAGEMAIN_access_denied                "Доступ к этой функции для Вас запрещен"
#define MESSAGEMAIN_access_denied2                 "Проверьте, вошли ли вы в конференцию<BR>Если Вы считаете, что это ошибка - обратитесь к модераторам конференции"

#define MESSAGEMAIN_spamtry                                "Блокировка попытки спама или защита от повторной отправки"
#define MESSAGEMAIN_spamtry2                         "Если Вы пытаетесь отправить сообщение, то это значит, что оно уже отправлено и вам незачем делать это еще раз"

#define MESSAGEMAIN_BANNED_REASON                "Причина запрета добавления сообщений для Вас была :"

#define MESSAGEMAIN_browser_return                "<P ALIGN=CENTER>Ваш броузер должен вернуть вас к списку сообщений через несколько секунд</P>"

#define MESSAGEMAIN_browser_to_thread                "<P><P ALIGN=CENTER><B><A STYLE=\"text-decoration:underline;\" HREF=\"" MY_CGI_URL "?read=%lu\">Вернуться</A> к своему сообщению</B></P></P><P></P>"

#define MESSAGEMAIN_admin_contact                 "Если Вы считаете что это неправильно, обратитесь к <A HREF=\"mailto:" ADMIN_MAIL "\">Администратору</A> конференции"

/***************** Форма отправки сообщений ******************/
#define        MESSAGEMAIN_post_newmessage                 "Новое сообщение"
#define        MESSAGEMAIN_post_replymessage                 "Ответить"
#define        MESSAGEMAIN_post_editmessage                 "Изменить сообщение"
#define        MESSAGEMAIN_post_bot  "Оставьте поля в первой строке пустыми!<br>Leave fields in the first row blank!"

#define MESSAGEMAIN_post_you_name                        "<b>Имя:</b>"
#define MESSAGEMAIN_post_your_password                "<b>Пароль:</b>"
#define MESSAGEMAIN_post_login_me                        "<b>Автологин</b>"
#define MESSAGEMAIN_post_hostname                        "<b>Host:</b>"
#define MESSAGEMAIN_post_message_subject        "<b>Тема:</b>"
#define MESSAGEMAIN_post_message_body                 "<b>Сообщение:</b> "

#define MESSAGEMAIN_post_disable_wwwconf_tags        "Отключить распознавание Тегов конференции"
#define MESSAGEMAIN_post_disable_smile_tags                "Отключить распознавание  смайл-кодов"
#define MESSAGEMAIN_post_reply_acknl                         "Получать уведомления об ответах по почте"

#define MESSAGEMAIN_post_preview_message        "Предварительный просмотр"
#define MESSAGEMAIN_post_post_message                 "Отправить"
#define MESSAGEMAIN_post_edit_message                "Изменить"

/****************** Регистрационная форма ******************/
#define MESSAGEMAIN_register_intro                        "Регистрация"
#define MESSAGEMAIN_register_chg_prof_intro "Изменение профайла"
#define MESSAGEMAIN_register_login                        "Имя (*):"
#define MESSAGEMAIN_register_displayname        "Отображаемое имя :"
#define MESSAGEMAIN_register_oldpass_req        "Для изменения профайла Вы <U>должны</U> указать Ваш пароль !"
#define MESSAGEMAIN_register_oldpassword        "Старый пароль (*):"
#define MESSAGEMAIN_register_if_want_change        "Если вы хотите изменить пароль введите тут новый пароль, иначе оставьте эти поля пустыми"
#define MESSAGEMAIN_register_password1                "Новый пароль (*):"
#define MESSAGEMAIN_register_password2                "Новый пароль еще раз (*):"
#define MESSAGEMAIN_register_full_name                "Ваше полное имя:"
#define MESSAGEMAIN_register_validemail_req        "Вы должны указать действующий E-Mail адрес <BR>(он будет использоваться для получения пароля и сводок по конференции)"
#define MESSAGEMAIN_register_bot "Внимание!<br>Если вы видите два поля, то введите e-mail только<br>во второе, оставьте первое пустым!<br>Attention!<br>If you see two fields, fill with e-mail only<br>the second one, leave the first one blank!"
#define MESSAGEMAIN_register_email  "E-Mail адрес (*):"        
#define MESSAGEMAIN_register_email_pub                "Публичный доступ к Вашему E-Mail"
#define MESSAGEMAIN_register_homepage                "Адрес Вашей домашней страницы:"
#define MESSAGEMAIN_register_icq                        "ICQ :"
#define MESSAGEMAIN_register_signature                "Если Вы хотите собственную подпись (будет появляться в" \
                " новых сообщениях по умолчанию,<BR>и не может быть длиннее 255 символов), введите ее здесь," \
                " или оставьте поле пустым"
#define MESSAGEMAIN_register_selectedusers        "Выбраные пользователи, сообщения которых будут подсвечиваться<BR>" \
                "(каждое имя пишется на новой строке, максимально можно ввести 184 символа)"
#define MESSAGEMAIN_register_about                        "Введите что-нибудь о Ваших интересах, хобби и т.д."
#define MESSAGEMAIN_register_private_prof        "Публичный доступ к Вашему профайлу"
#define MESSAGEMAIN_register_always_emlackn        "Всегда отсылать ответы на Ваши сообщения на E-mail"
#define MESSAGEMAIN_register_pmsg_disable        "Запретить сервис приватных сообщений"
#define MESSAGEMAIN_register_pmsg_email                "Присылать оповещение о приходе нового приватного сообщения по почте"
#define MESSAGEMAIN_register_req_fields                "Все поля отмеченные * являются обязательными !"
#define MESSAGEMAIN_register_register                "Зарегистрировать"
#define MESSAGEMAIN_register_edit                        "Изменить"
#define MESSAGEMAIN_register_delete                        "Удалить"
#define MESSAGEMAIN_register_confirm_delete        "подтвердить удаление"
#define MESSAGEMAIN_register_view_saving    "Использовать профайл для хранения настроек внешнего вида"
        
#define CONFIRM_DELETE_CHECKBOX_TEXT                "confirm_delete"

/**************** Регистрационные сообщения ***************/
/**************** registration mnssages ***************/
#define MESSAGEMAIN_register_create_ex                "Профиль создан"
#define MESSAGEMAIN_register_create_ex2                "и Вы автоматически вошли"

#define MESSAGEMAIN_register_edit_ex                "Пользователь был создан / изменен"
#define MESSAGEMAIN_register_edit_ex2                ""

#define MESSAGEMAIN_register_edit_err                "Произошла ошибка при обновлении профайла"
#define MESSAGEMAIN_register_edit_err2                "Возможно, стоит попробовать еще раз"

#define MESSAGEMAIN_register_delete_ex                "Пользователь удален"
#define MESSAGEMAIN_register_delete_ex2                ""

#define MESSAGEMAIN_register_delete_logoff        "Ваш аккаунт был удален"
#define MESSAGEMAIN_register_delete_logoff2        ""

#define MESSAGEMAIN_register_already_exit        "Это имя уже занято"
#define MESSAGEMAIN_register_already_exit2        "Попробуйте придумать другое имя"

#define MESSAGEMAIN_register_invalid_psw        "Имя или пароль введены неправильно"
#define MESSAGEMAIN_register_invalid_psw2        "Вы должны ввести правильное имя и пароль или если вы изменяете" \
                                                                                        " информацию о себе вы должны перед этим войти в систему"

#define MESSAGEMAIN_register_invalid_n_psw        "Подтверждение пароля не совпадает или пароль слишком короткий"
#define MESSAGEMAIN_register_invalid_n_psw2        "Для подтверждения пароля вам необходимо ввести его верно" \
                                                                                        " дважды и длина пароля должна быть не менее 3 символов"

#define MESSAGEMAIN_register_invalid_email        "Введенный E-Mail адрес некорректен"
#define MESSAGEMAIN_register_invalid_email2        "Чтобы зарегистрироваться в конференции Вы должны ввести действующий" \
                                                                                        " E-Mail иначе Вы не сможете пользоваться некоторыми функциями конференции"

#define MESSAGEMAIN_register_cannot_delete        "Пользователь %s не может быть удален"
#define MESSAGEMAIN_register_cannot_delete2        "Вы должны ввести правильное имя и пароль"

#define MESSAGEMAIN_register_invalid_lg_spell        "Нет такого имени пользователя или оно неправильное"
#define MESSAGEMAIN_register_invalid_lg_spell2        "Вы должны ввести правильное имя и пароль(длина имени должна быть не меньше 3 символов)"


/***************** profile information *****************/
#define MESSAGEMAIN_profview_intro                        "Информация о пользователе:"
#define MESSAGEMAIN_profview_login                        "Имя: "
#define MESSAGEMAIN_profview_postpersmsg        "отправить приватное сообщение"
#define MESSAGEMAIN_profview_editinfo                "изменить"
#define MESSAGEMAIN_profview_altname                "Альтернативное имя: "
#define MESSAGEMAIN_profview_fullname                "Полное имя: "
#define MESSAGEMAIN_profview_homepage                "Домашняя страница: "
#define MESSAGEMAIN_profview_email                        "Email: "
#define MESSAGEMAIN_profview_dogsubstitute        " at "
#define MESSAGEMAIN_profview_user_icq                "ICQ: "
#define MESSAGEMAIN_profview_user_status        "Статус: "
#define MESSAGEMAIN_profview_u_moderator        "<FONT COLOR=#FF0000>Модератор</FONT>"
#define MESSAGEMAIN_profview_u_user                        "Обычный пользователь"
#define MESSAGEMAIN_profview_postcount                "Число сообщений: "
#define MESSAGEMAIN_profview_reg_date                "Дата регистрации: "
#define MESSAGEMAIN_profview_login_date                "Время последнего доступа: "
#define MESSAGEMAIN_profview_about_user                "О пользователе: "
#define MESSAGEMAIN_profview_refreshcnt                "Число обновлений: "
#define MESSAGEMAIN_profview_lastip                        "Последний вход с: "
#define MESSAGEMAIN_profview_persmsgcnt                "Всего персональных сообщений(прочитано): "

#define MESSAGEMAIN_profview_no_user                "<P></P><CENTER><span>Пользователь <span class=\"nick\">%s</span> не " \
        "зарегистрирован в этой конференции</span></CENTER>"

#define MESSAGEMAIN_profview_privacy_prof        "Дальнейшая информация о пользователе недоступна"
#define MESSAGEMAIN_profview_privacy_inf        "Личная информация"

#define MESSAGEMAIN_profview_sechdr                        "Код безопастности для темы: "
#define MESSAGEMAIN_profview_secbdy                        "Код безопастности для тела: "

//        user statuses
#define USER_STATUS_COUNT 5
extern const char *UserStatus_List[USER_STATUS_COUNT];

extern const char *UserRight_List[USERRIGHT_COUNT];

/******************* session information *******************/
#define MESSAGEMAIN_session_intro                "Зарегистрированные сессии "
#define MESSAGEMAIN_session_ip                        "IP адрес входа:"
#define MESSAGEMAIN_session_date                        "Время последнего доступа:"
#define MESSAGEMAIN_session_state                        "Состояние сессии:"
#define MESSAGEMAIN_session_state_active        "активная"
#define MESSAGEMAIN_session_state_toclose        "закрыть"
#define MESSAGEMAIN_session_state_closed        "закрыта"
#define MESSAGEMAIN_session_no                        "нет сессий"
#define MESSAGEMAIN_session_closed_ok                "Сессия закрыта<BR>"
#define MESSAGEMAIN_session_closed_ok2        "Спасибо за участие!"
#define MESSAGEMAIN_session_closed_no                "Сессия не закрыта<BR>"
#define MESSAGEMAIN_session_check_failed        "Сессия не найдена<BR>Возможно, она была только что закрыта"
#define MESSAGEMAIN_session_close_failed        "Ошибка при закрытии сессии<BR>"
#define MESSAGEMAIN_session_ip_nocheck                "(без проверки)"
#define MESSAGEMAIN_session_ip_check                ""

/************* private(personal) messages **************/
#define MESSAGEMAIN_privatemsg_send_message "Отправить сообщение"
#define MESSAGEMAIN_privatemsg_send_msg_btn "Отправить"
#define MESSAGEMAIN_privatemsg_prev_msg_btn "Предпросмотр"
#define MESSAGEMAIN_privatemsg_send_msg_hdr "Отправить личное сообщение"
#define MESSAGEMAIN_privatemsg_send_msg_usr        "Пользователю: "
#define MESSAGEMAIN_privatemsg_send_msg_bdy "Сообщение (не может быть больше 384 символов)"

#define MESSAGEMAIN_privatemsg_msgwassent        "Сообщение было отправлено пользователю"
#define MESSAGEMAIN_privatemsg_msgwassent2        "Спасибо за участие"

#define MESSAGEMAIN_privatemsg_msgcantsend        "Сообщение не может быть отправлено !"
#define MESSAGEMAIN_privatemsg_msgcantsend2        "Попробуйте позже !"

#define MESSAGEMAIN_privatemsg_denyunreg        "Данная возможность недоступна незарегистрированным пользователям !<BR>"
#define MESSAGEMAIN_privatemsg_denyunreg2        "Для того чтоб иметь возможность отправлять и получать персональное сообщение Вы должны зарегистрироваться в конференции !<BR><BR>"

#define MESSAGEMAIN_privatemsg_invalid_user        "Вы должны указать существующий ник пользователя"
#define MESSAGEMAIN_privatemsg_invalid_body "Сообщение не может быть пустое (иначе какой смысл его посылать ?)"
#define MESSAGEMAIN_privatemsg_tolong_body        "Сообщение слишком длинное, уменьшите его длину"
#define MESSAGEMAIN_privatemsg_disable_pmsg "Пользователь запретил получение персональных сообщений,<BR>отправить ему персональное сообщение невозможно"

#define MESSAGEMAIN_privatemsg_header                "Персональные сообщения для пользователя"

#define MESSAGEMAIN_privatemsg_disabled                "Получение персональных соообщений запрещено!"

#define MESSAGEMAIN_privatemsg_newmsgcnt        "Количество новых сообщений:"
#define MESSAGEMAIN_privatemsg_nonewmsg                "Новых сообщений нет"

#define        MESSAGEMAIN_privatemsg_allmsgcnt        "Сообщений всего, получено:"
#define        MESSAGEMAIN_privatemsg_allmsgcnt1        "отправлено:"

#define MESSAGEMAIN_privatemsg_writenewmsg        "Написать новое персональное сообщение"

#define MESSAGEMAIN_privatemsg_newmsgann        "Есть"
#define MESSAGEMAIN_privatemsg_newmsgann1        "непрочитанных личных сообщений для Вас!"

#define MESSAGEMAIN_privatemsg_newmark                "<font color=red>[ Новое !]</font>"

#define MESSAGEMAIN_privatemsg_answer                "[Ответить]"
#define MESSAGEMAIN_privatemsg_write                "[Написать]"


#define MESSAGEMAIN_privatemsg_fromuser                "От пользователя:"
#define MESSAGEMAIN_privatemsg_touser                "Пользователю:"

#define MESSAGEMAIN_privatemsg_showall     "Просмотреть все сообщения"

/***************** favourites system *****************/
#define MESSAGEMAIN_favourites_denyunreg        "Данная возможность недоступна незарегистрированным пользователям !<BR>"
#define MESSAGEMAIN_favourites_denyunreg2        "Для того, чтобы иметь возможность хранить ссылки на избранные сообщения, Вы должны зарегистрироваться и войти в конференцию !<BR><BR>"

#define MESSAGEMAIN_favourites_listclear        "Ваш список избранного пуст"

#if USER_FAVOURITES_SUPPORT == 1
#define MESSAGEMAIN_favourites_added                "Сообщение занесено в Избранное!<BR>"
#define MESSAGEMAIN_favourites_added2                "Спасибо за участие !<BR>Теперь ссылку на это сообщение можно найти в списке<BR>"
#define MESSAGEMAIN_favourites_addno                "Сообщение не занесено в Избранное!<BR>"
#define MESSAGEMAIN_favourites_addexist                "Сообщение уже присутствует в списке!<BR>Возможно, Вы только что его добавили и пытаетесь сделать это еще раз<BR>"
#define MESSAGEMAIN_favourites_addnoplace        "У Вас уже сохранено 20 сообщений!<BR>Для того, чтобы добавить это сообщение, необходимо удалить хотя бы одно из уже сохраненных<BR>"
#define MESSAGEMAIN_favourites_deleted                "Сообщение удалено из Избранного!<BR>"
#define MESSAGEMAIN_favourites_deleted2                "Спасибо за участие !<BR><BR>"
#define MESSAGEMAIN_favourites_delno                "Сообщение не удалено из Избранного!<BR>"
#define MESSAGEMAIN_favourites_delnoexist        "Сообщения нет в списке !<BR> Возможно, Вы только что его удалили<BR>"
#else
#define MESSAGEMAIN_favourites_added                "Поток занесен в Избранное!<BR>"
#define MESSAGEMAIN_favourites_added2                "Спасибо за участие !<BR>Теперь ссылку на этот поток можно найти в списке<BR>"
#define MESSAGEMAIN_favourites_addno                "Поток не занесен в Избранное!<BR>"
#define MESSAGEMAIN_favourites_addexist                "Поток уже присутствует в списке!<BR>Возможно, Вы только что его добавили<BR>"
#define MESSAGEMAIN_favourites_addnoplace        "У Вас уже сохранено 20 потоков!<BR>Для того, чтобы добавить этот тред, необходимо удалить хотя бы один из уже сохраненных<BR>"
#define MESSAGEMAIN_favourites_deleted                "Поток удален из Избранного!<BR>"
#define MESSAGEMAIN_favourites_deleted2                "Спасибо за участие !<BR><BR>"
#define MESSAGEMAIN_favourites_delno                "Поток не удален из Избранного!<BR>"
#define MESSAGEMAIN_favourites_delnoexist        "Потока нет в списке !<BR> Возможно, Вы только что его удалили<BR>"
#define MESSAGEMAIN_favourites_addnoparent        "Сообщение не является первым для данного потока!<BR>Добавлять можно только первые сообщения в треде"
#endif

/****************** global announces *****************/
#define MESSAGEMAIN_globann_send_ann_hdr        "Разместить новый глобальный анонс"
#define MESSAGEMAIN_globann_upd_ann_hdr                "Изменить глобальный анонс"
#define MESSAGEMAIN_globann_send_ann_body        "Текст анонса (не может быть больше 512 символов)"
#define MESSAGEMAIN_globann_upd_ann_id                "Обновить номер анонса (снова всплывет)"
#define MESSAGEMAIN_globann_send_ann_btn        "Отправить"
#define MESSAGEMAIN_globann_prev_ann_btn        "Предпросмотр"

#define MESSAGEMAIN_globann_preview_hdr                "Предварительный просмотр анонса"

#define MESSAGEMAIN_globann_anncantsend                "Анонс не может быть добавлен !"
#define MESSAGEMAIN_globann_anncantsend2        "Может быть Вам стоит попробовать позже..."

#define MESSAGEMAIN_globann_annwassent                "Анонс был добавлен"
#define MESSAGEMAIN_globann_annwassent2                "Спасибо за участие"

#define MESSAGEMAIN_globann_annwasupdated        "Анонс был обновлен"
#define MESSAGEMAIN_globann_annwasupdated2        "Спасибо за участие"

#define MESSAGEMAIN_globann_tolong                        "Анонс слишком длинный, уменьшите его и повторите попытку"
#define MESSAGEMAIN_globann_toshort                        "Анонс слишком короткий, зачем его вообще писать ;) ?"

#define MESSAGEMAIN_globann_wasdeleted                "Анонс был успешно удален"
#define MESSAGEMAIN_globann_wasdeleted2                "<BR>"

#define MESSAGEMAIN_globann_cannotdel                "Анонс не может быть удален!"
#define MESSAGEMAIN_globann_cannotdel2                "Возможно, у Вас недостаточно прав для удаления"

#define MESSAGEMAIN_globann_invalidnum                "Указан не существующий идентификатор анонса!"
#define MESSAGEMAIN_globann_invalidnum2                "Проверьте и обновите ссылку по которой Вы попали сюда"

#define MESSAGEMAIN_globann_delannounce                "Удалить этот анонс"
#define MESSAGEMAIN_globann_updannounce                "Обновить этот анонс"
#define MESSAGEMAIN_globann_postedby                "Отправлен"

#define MESSAGEMAIN_globann_showall                        "Показать все активные анонсы"
#define MESSAGEMAIN_globann_hidenewann                "Скрыть текущие анонсы"

/******************  moderation form  ******************/
#define MESSAGEMAIN_moderate_hide_thread         "Скрыть эту ветвь"
#define MESSAGEMAIN_moderate_close_thread        "Закрыть эту ветвь"
#define MESSAGEMAIN_moderate_unhide_thread         "Показать эту ветвь"
#define MESSAGEMAIN_moderate_unclose_thread        "Открыть эту ветвь"
#define MESSAGEMAIN_moderate_delete_thread        "Удалить эту ветвь"
#define MESSAGEMAIN_moderate_change_message        "Изменить это сообщение"
#define MESSAGEMAIN_moderate_roll                        "Свернуть эту ветвь"
#define MESSAGEMAIN_moderate_unroll                        "Развернуть эту ветвь"

/********************* Форма входа *********************/
#define MESSAGEMAIN_login_login_header        "Введите Ваше имя и пароль"
#define MESSAGEMAIN_login_loginname                " Имя : "
#define MESSAGEMAIN_login_password                " Пароль : "
#define        MESSAGEMAIN_login_ipcheckshort                "Не проверять IP"
#define MESSAGEMAIN_login_ipcheck                "Не проверять IP адрес для сессии"

#define MESSAGEMAIN_login_lostpassw                "<CENTER>Если Вы забыли Ваш пароль <A HREF=\"" MY_CGI_URL "?login=lostpasswform\">кликните тут</A></CENTER><BR><BR>"

/**************** Форма "забыли пароль" ****************/
#define MESSAGEMAIN_lostpassw_header        "<BIG>Система напоминания пароля</BIG><BR><BR>Введите Ваш ник и адрес электронной почты<BR>" \
                                                                                "и Вы получите письмо с вашим паролем"
#define MESSAGEMAIN_lostpassw_hretry        "<BOLD><FONT COLOR=RED>Вы указали неверный ник и/или адрес электронной почты</FONT></BOLD><BR>" \
                                                                                "Пожалуйста, проверьте Ваш ник и email и повторите попытку еще раз"
#define MESSAGEMAIN_lostpassw_loginname        " Ник : "
#define MESSAGEMAIN_lostpassw_email                " Email : "
#define MESSAGEMAIN_lostpassw_getpassw        "Получить пароль"

/******************* message view form ******************/
#define MESSAGEMAIN_viewthread_sent                "Сообщение было послано:"
#define MESSAGEMAIN_viewthread_ipview        ", IP: "
#define MESSAGEMAIN_viewthread_date                "Дата:"
#define MESSAGEMAIN_viewthread_mdate        "Изменен:"
#define MESSAGEMAIN_viewthread_sigdisabled        "<U><I><SMALL>Пользователь имеет подпись, но отображение подписей запрещено в настройках.</SMALL></I></U>"
#define        MESSAGEMAIN_in_this_thread                "<CENTER><BIG>Сообщения в этом потоке</BIG></CENTER>"

/********************** userlist ***********************/
#define MESSAGEMAIN_total_user_count                "Всего пользователей: "
#define MESSAGEMAIN_userlist_sortby                        "Сортировать по: "
#define MESSAGEMAIN_userlist_sortbyname                "нику"
#define MESSAGEMAIN_userlist_sortbydate                "дате последнего доступа"
#define MESSAGEMAIN_userlist_sortbypcnt                "кол-ву сообщений"
#define MESSAGEMAIN_userlist_sortbyhost                "хосту/IP адресу"
#define MESSAGEMAIN_userlist_sortbyrefresh        "по количеству обновлений"
#define MESSAGEMAIN_userlist_sortbyright        "по правам пользователей"

/******************  configure form   ******************/
#define MESSAGEHEAD_to_message                                "Перейти к ответам"
#define MESSAGEHEAD_configure_showmsgs                "Показать ветви"
#define MESSAGEHEAD_configure_msgslast                " за последние "
#define MESSAGEHEAD_configure_lastnum                " последние "
#define MESSAGEHEAD_configure_showstyle                "Стиль отображения : "
#define MESSAGEHEAD_configure_disablesmiles "Глобально отменить смайл-коды"
#define MESSAGEHEAD_configure_disableuppic        "Отключить заголовок конференции"
#define MESSAGEHEAD_configure_disable2links        "Отключить вторую строку ссылок"
#define MESSAGEHEAD_configure_ownpostshighlight        "Отключить выделение собственных сообщений"
#define MESSAGEHEAD_configure_showhostnames        "Отключить отображение хостов в индексе"
#define MESSAGEHEAD_configure_showaltnames        "Отключить отображение альтернативных имен"
#define MESSAGEHEAD_configure_showsign                "Отключить отображение подписей пользователей"
#define MESSAGEHEAD_configure_plus_is_href        "\" " TAG_NEW_MSG_MARK "\" является ссылкой на следующее новое сообщение"
#define MESSAGEHEAD_configure_disablecolor        "Отключить отображение цветов в индексе"
#define MESSAGEHEAD_configure_disablebot  "Отключить защиту против ботов"
#define MESSAGEHEAD_configure_showreplyform        "Не отображать форму ответа"
#define MESSAGEHEAD_configure_applysettings        "Сохранить настройки"


#define MESSAGEHEAD_configure_saving_to_profile "Настройки сохраняются в профайле и будут доступны с любого браузера"
#define MESSAGEHEAD_configure_saving_to_browser "Настройки сохраняются в браузере и доступны только с данного компьютера"
#define MESSAGEHEAD_configure_view_saving    "Изменить способ хранения настроек можно на <a href=\""MY_CGI_URL"?register=form\" style=\"text-decoration:underline;\"  >странице редактирования профайла</a>"

// coded as "1"
//#define MESSAGEHEAD_configure_showhronforward        "По потокам в хронологическом виде, по возрастанию"
// coded as "2"
#define MESSAGEHEAD_configure_showhronbackward                "По потокам в хронологическом виде, по убыванию"
// coded as "3"
#define MESSAGEHEAD_configure_showhronwothreads                "По сообщениям в хронологическом виде, по возрастанию"
// coded as "4"
#define MESSAGEHEAD_configure_showhrononlyheaders        "По сообщениям в хронологическом виде, только заголовки"


#define WWWCONF_FULL_NAME "свободное™ общение"

// for STABLE_TITLE = 1
#if STABLE_TITLE == 1
#define TITLE_StaticTitle                 "not used " VERSION
#endif

/******************* topic support *********************/
#if TOPICS_SYSTEM_SUPPORT
#define TOPICS_COUNT        19
#define TOPICS_DEFAULT_SELECTED        0
// this variable should be defined in main.cpp
extern const char *Topics_List[TOPICS_COUNT];
extern DWORD Topics_List_map[TOPICS_COUNT];
#endif

/**************************** title messages *****************************/
// for STABLE_TITLE = 0
// board name prefix
#define TITLE_WWWConfBegining                "свободное™ общение"
#define TITLE_divider                                " : "
// forms name
#define TITLE_WWWConfIndex                        " "
#define TITLE_Form                                        "Новое сообщение"
#define TITLE_WriteReply                        "Написать ответ"
#define TITLE_Configure                                "Настройка"
#define TITLE_Error                                        "Ошибка !"
#define TITLE_Spamtry                                "Блокирована попытка спама!"
#define TITLE_Login                                        "Вход в конференцию"
#define TITLE_Logoff                                "Выход из конференции"
#define TITLE_LostPassword                        "Забыли пароль?"
#define TITLE_PasswordSent                        "Пароль отправлен"
#define TITLE_IncorrectPassword                "Неправильный пароль!"
#define TITLE_ClosingMessage                "Закрыть сообщения"
#define TITLE_HidingMessage                        "Скрыть сообщения"
#define TITLE_DeletingMessage                "Удалить сообщения"
#define TITLE_ChangingMessage                "Изменить сообщение - "
#define TITLE_RollMessage                        "Развернуть сообщение"
#define TITLE_Registration                        "Регистрация / изменить профиль"
#define TITLE_ProfileInfo                        "Информация о пользователе"
#define TITLE_UserList                                "Список юзеров"
#define TITLE_PrivateMsg                        "Приватные сообщения"
#define TITLE_AddPrivateMsg                        "Написать приватное сообщения"
#define TITLE_PrivateMsgWasPosted        "Приватное сообщение отправлено"
#define TITLE_PostGlobalAnnounce        "Добавить глобальный анонс"
#define TITLE_GlobalAnnWasPosed                "Глобальный анонс добавлен"
#define TITLE_GlobalAnnWasDeleted        "Глобальный анонс удален"
#define TITLE_FavouritesPage                "Избранное"
#define TITLE_FavouritesPageAdd                "Добавление темы в Избранное"
#define TITLE_FavouritesPageDel                "Удаление темы из Избранного"
#define TITLE_BanSave                "Сохранение списка забаненных айпи"
#define TITLE_ClSession                        "Закрытие сессии"

/***************************** welcome messages **************************/
#define WELCOME_CONFERENCE_HEADER        "<B>Вход:</B> "

#define MESSAGEMAIN_ACTIVITY_STATVIEW        "Активность: <B><font title=\"хосты\" color=\"blue\">%ld</font> "\
        "<font title=\"хиты\" color=\"#229911\">%ld</font></B> за последние 10 минут"

#define MESSAGEMAIN_WELCOME_LOGGEDSTART        "<div class=\"info\"><div class=\"left_info\">" \
        WELCOME_CONFERENCE_HEADER MESSAGEMAIN_WELCOME_HELLOREG "%s%s<BR>%s%s</div>"\
        "<div class=\"right_info\">%s<BR>" MESSAGEMAIN_WELCOME_SELECTTOPIC " %s</div>"\
        "<div style=\"clear: both; height: 0px; overflow: hidden;\"><br/></div></div>"

#define MESSAGEMAIN_WELCOME_START        "<div class=\"info\"><div class=\"left_info\">"\
        "<form method=post action=\""MY_CGI_URL "?login=action\">"\
        WELCOME_CONFERENCE_HEADER MESSAGEMAIN_login_loginname \
        "<INPUT TYPE=TEXT NAME=\"mname\" SIZE=15 " \
        "MAXLENGTH=%d VALUE=\"%s\">&nbsp;&nbsp;&nbsp;" MESSAGEMAIN_login_password \
        "<INPUT TYPE=PASSWORD NAME=\"mpswd\" SIZE=15 MAXLENGTH=%d>&nbsp;"\
        "<INPUT TYPE=CHECKBOX NAME=\"ipoff\" VALUE=1>" MESSAGEMAIN_login_ipcheckshort \
        "&nbsp;<INPUT TYPE=SUBMIT VALUE=\"Enter\"></form>%s%s</div>" \
        "<div class=\"right_info\">%s<BR>" MESSAGEMAIN_WELCOME_SELECTTOPIC " %s</div>"\
        "<div style=\"clear: both; height: 0px; overflow: hidden;\"><br/></div></div>"



                                                                                                                                                                                                                
#define MESSAGEMAIN_WELCOME_HELLOREG                "Вы вошли в конференцию как "

#if TOPICS_SYSTEM_SUPPORT
#define MESSAGEMAIN_WELCOME_SELECTTOPIC  "Выберите тему:"
#else
#define MESSAGEMAIN_WELCOME_SELECTTOPIC  ""
#endif

#define MESSAGEMAIN_WELCOME_NEWTHREADS_TEXT                "новых потоков(сообщений): <a href=\"#n0\" "\
        "style=\"color: #f00;font-weight:bold;\">+</a> %lu(%lu) из %lu"
#define MESSAGEMAIN_WELCOME_NEWTHREADS "<span id=\"new_count\">" MESSAGEMAIN_WELCOME_NEWTHREADS_TEXT "</span>"
                                
#define MESSAGEMAIN_WELCOME_NONEWTHREADS_TEXT        "новых сообщений нет, всего %ld"
#define MESSAGEMAIN_WELCOME_NONEWTHREADS "<span id=\"new_count\">"\
                        MESSAGEMAIN_WELCOME_NONEWTHREADS_TEXT"</span>"

#define MESSAGEMAIN_WELCOME_DISPLAYTIME                ", показаны сообщения за %lu %s"
#define MESSAGEMAIN_WELCOME_DISPLAYTHREADS        ", показаны последние %lu потоков"

#define MESSAGEMAIN_WELCOME_YOURSETTINGS        "выбранные темы"
#define MESSAGEMAIN_WELCOME_ALLTOPICS                "все темы"

/**************************** date/time messages *************************/
#define MESSAGEMAIN_DATETIME_JAN                        "Январь"
#define MESSAGEMAIN_DATETIME_FEB                        "Февраль"
#define MESSAGEMAIN_DATETIME_MAR                        "Март"
#define MESSAGEMAIN_DATETIME_APR                        "Апрель"
#define MESSAGEMAIN_DATETIME_MAY                        "Май"
#define MESSAGEMAIN_DATETIME_JUN                        "Июнь"
#define MESSAGEMAIN_DATETIME_JUL                        "Июль"
#define MESSAGEMAIN_DATETIME_AUG                        "Август"
#define MESSAGEMAIN_DATETIME_SEP                        "Сентябрь"
#define MESSAGEMAIN_DATETIME_OCT                        "Октябрь"
#define MESSAGEMAIN_DATETIME_NOV                        "Ноябрь"
#define MESSAGEMAIN_DATETIME_DEC                        "Декабрь"

#define MESSAGEMAIN_DATETIME_DAY_SUN                "Воскресенье"
#define MESSAGEMAIN_DATETIME_DAY_MON                "Понедельник"
#define MESSAGEMAIN_DATETIME_DAY_TEU                "Вторник"
#define MESSAGEMAIN_DATETIME_DAY_WED                "Среда"
#define MESSAGEMAIN_DATETIME_DAY_THU                "Четверг"
#define MESSAGEMAIN_DATETIME_DAY_FRI                "Пятница"
#define MESSAGEMAIN_DATETIME_DAY_SAT                "Суббота"

/**************************** mailing messeges ***************************/

#define MAILACKN_HEADER "Content-type: text/html; charset=\"windows-1251\"\r\n\r\n"


#define MAIL_SEND_GREETING        "<HTML>Здравствуйте, <bold><font color=#009000>"\
                "<strong>%s</strong></font></bold>.<br />"
                
#define MAIL_SEND_SIGNING        "Это письмо послано автоматически форумом "WWWCONF_FULL_NAME \
                ".<br />Вам <b>не</b> следует отвечать на него.<br /></HTML>"

//        reply notification
#define MAILACKN_REPLY_SUBJECT                        WWWCONF_FULL_NAME ": %s"
#define MAILACKN_REPLY_BODY                MAIL_SEND_GREETING \
"%s ответил на ваше сообщение <strong>%s</strong>.<br /><br />"\
"Тема ответа:<strong> %s</strong><br />"\
"----------- Сообщение ---------<br />%s<br />-------------------------------<br /><br />"\
"Весь тред можно увидеть <a href=\"%s?read=%lu\">здесь</A><br /><br />"

//        password recovery
#define MAILACKN_LOSTPASS_SUBJECT          WWWCONF_FULL_NAME ": пароль для профиля %s"
#define MAILACKN_LOSTPASS_BODY        MAIL_SEND_GREETING \
"Текущий пароль для вашего профиля: %s<br /><br /><br />"


//        private message
#define MAILACKN_PRIVATEMSG_SUBJECT        WWWCONF_FULL_NAME ": новое личное сообщение от %s"
#define MAILACKN_PRIVATEMSG_BODY        MAIL_SEND_GREETING \
"Для Вас получено новое личное сообщение от <strong>%s</strong>.<br /><br />"\
"----------- Сообщение ---------<br />%s<br />-------------------------------<br /><br />"\
"Ответить на него можно <a href=\"%s?persmsgform=%s\">здесь</a><br />" \
"Посмотреть остальные личные сообщения Вы можете <a href=\"%s?persmsg\">здесь</A><br /><br />"



/**************************** mailing messeges ***************************/
/*#define MAILACKN_SUBJECT                        WWWCONF_FULL_NAME ": %s"
#define MAILACKN_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"Здравствуйте, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR> %s ответил на ваше сообщение <STRONG>%s</STRONG>." \
"<BR><BR>Тема ответа:<STRONG> %s</STRONG>\n<BR>----------- Сообщение ---------<BR>%s<BR>-------------------------------" \
"<BR><BR>Весь тред можно увидеть <A HREF=" WC_EMAILACN_READURL "?read=%d>здесь</A>" \
"<BR><BR>This mail was generated automatically by mailing engine" \
" of " WWWCONF_FULL_NAME ".<BR>You <B>SHOULD NOT REPLY</B> to this message.<BR></HTML>"
*/
//        password recovery
/*#define MAILLOSTPASSW_SUBJECT                WWWCONF_FULL_NAME ": пароль для профиля %s"
#define MAILLOSTPASSW_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"Здравствуйте, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR>Текущий пароль для вашего профиля: %s<BR>" \
"<BR><BR>This mail was generated automatically by mailing engine" \
" of " WWWCONF_FULL_NAME ".<BR>You <B>SHOULD NOT REPLY</B> to this message.<BR></HTML>"

#define MAILACKN_PRIVATEMSG_SUBJECT                        WWWCONF_FULL_NAME ": новое личное сообщение от %s"
#define MAILACKN_PRIVATEMSG_MAINBODY_FORMAT        "Content-type: text/html; charset=\"windows-1251\"\n<HTML>" \
"Здравствуйте, <BOLD><FONT COLOR=#009000><STRONG>%s</STRONG></FONT></BOLD>.<BR> для Вас получено новое личное сообщение от <STRONG>%s</STRONG>." \
"<BR><BR>----------- Сообщение ---------<BR>%s<BR>-------------------------------" \
"<BR><BR>Ответить на него можно <A HREF=" WC_EMAILACN_READURL "?persmsgform=%s>здесь</A><BR>" \
"Посмотреть остальные личные сообщения Вы можете <A HREF=" WC_EMAILACN_READURL "?persmsg>здесь</A>" \
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

// from design.h
/*=====================================================================*/
#define TAG_MSG_HAVE_BODY               "+"
#define TAG_MSG_HAVE_NO_BODY            "-"
#define TAG_MSG_HAVE_PIC                "pic"
#define TAG_MSG_HAVE_URL                "url"
#define TAG_MSG_HAVE_TEX                "tex"
#define TAG_MSG_HAVE_TUB                "tub"
#define TAG_MSG_CLOSED_THREAD        "<span class=\"close\" id=c%ld>(закрыто) </span>"
#define TAG_MSG_ROLLED_THREAD "<br><span id=r%ld><span class=\"roll1\">свернуто: </span><span class=roll2>(ответов: <b>%lu</b>)</span><br></span>"
#define TAG_MSG_ROLLED_THREAD_MARKNEW        "<br><span id=r%ld><span class=\"roll1\">свернуто: </span><span class=roll2>" \
                                        "(ответов: <b>%lu</b>, новых: <b>%d</b>, последний ответ: <A HREF=%s?read=%ld>от %s, %s</A>)</span><br></span>"
#define TAG_REPLY_PREFIX                "Re: "
#define TAG_IP_NOT_DETECTED                "X.X.X.X"
#define BAD_WORD_SYMBOL                        '#'
#define TAG_NEW_MSG_MARK_HREF        "<span class=e id=p%ld><a name=n%ld href=\"#n%ld\">+</a> </span>"
#define TAG_NEW_MSG_MARK                "<span class=e>+ </span>"

#define DESIGN_NOWRAP_START  "<span class=\"nw\">"
#define DESIGN_NOWRAP_END    "</span>"
#define DESIGN_WRAP "<span class=\"yw\"> </span>"

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
extern char DESIGN_threads_divider[500];

#define DESIGN_OP_DL                                "<DL><DD>"
#define DESIGN_CL_DL                                "</DL>"
#define DESIGN_OP_DIV                                "<DIV>"
#define DESIGN_OP_DIV_grey                        "<DIV class=g>"
#define DESIGN_OP_DIV_white                        "<DIV class=w>"
#define DESIGN_CL_DIV                                "</DIV>"
extern char DESIGN_open_dl[10];
extern char DESIGN_open_dl_grey[20];
extern char DESIGN_open_dl_white[20];
extern char DESIGN_close_dl[10];

#define DESIGN_BR                "<BR>"
#define DESIGN_DD                "<DD>"
extern char DESIGN_break[10];

#define DESIGN_BUTTONS_DIVIDER                        "&nbsp;&nbsp;"

#define DESIGN_GLOBAL_BOARD_MESSAGE                "<P ALIGN=CENTER><BIG><STRONG>%s</STRONG></BIG><BR><BR>%s"

#define DESING_INDEX_MSG_HEADER  "<span class=\"subject\">%s</span>"

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

#define DESIGN_CONFIGURE_CHECKALL "Все темы  <input type=checkbox checked onClick=\"selectall(checked)\">" \
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
#define DESIGN_BAN_FORM        "<center><form action=\""MY_CGI_URL"?banlist=save\" method=\"post\">"\
                                        "<textarea name=\"ban_list\" rows=\"30\" cols=\"70\" wrap=\"soft\">"

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
	DESIGN_STYLE_BUTTONS_ADD("0", "55px", "smile", "smile", "таблица смайлов (alt+0)", "show", "")

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

#define DESIGN_FORM_MESSAGE_BUTTON        "<INPUT TYPE=SUBMIT NAME=\"%s\" onClick=\"onSubmit(this)\" VALUE=\"%s\" class=\"fb\" tabindex=\"%d\">"

#define SCRIPT_FORM_MESSAGE_QEDIT "<script language=\"JavaScript\" type=\"text/javascript\" "\
"src=\"qecode2.js\"></script>"

#define SCRIPT_FORM_MESSAGE_BUTTON "<script language=\"JavaScript\" type=\"text/javascript\" "\
"src=\"postcheck.js\"></script>"

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
        "Конференция основана на движке "                               \
        "<A STYLE=\"text-decoration:underline;\" href=\"http://code.google.com/p/wwwconf/\" TARGET=\"_blank\">wwwconf</A>.</div>"

#endif
