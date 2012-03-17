/***************************************************************************
                    profman.cpp  -  profile and database management tool
                             -------------------
    begin                : Wed May 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include <stdlib.h>
#include "basetypes.h"
#include "profiles.h"

#define DIR_CREATION_MASK    511
#define FILES_CREATION_MASK  511

int HPrinted = 0;

void printusage(char *iam)
{
        printf("Usage %s <arg>\n"
               "Possible arguments:"
               " -n              - create/renew database (create new messages and profiles database)\n"
               " -nu             - create/renew ONLY profiles database (delete all profiles)\n"
               " -nm             - create/renew ONLY messages database (delete all forum messages)\n"
               " -v user         - view user information for user 'username'\n"
               " -vp username    - view all private messages for user 'username'\n"
               " -aa user passwd - create admin (spec. username and password)\n"
               " -au user passwd - create user (spec. username and password)\n"
               " -d user         - delete user\n"
               " -np             - renew(delete) private messages database (in profiles)\n"
               " -r              - zero refresh count for all users\n"
               " -vs             - set default view settings for all users\n",
               iam);
}

void printuserrigth(DWORD r)
{
        printf("User right: ");
        int smthprinted = 0;
        if(r & USERRIGHT_SUPERUSER) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("SuperUser");
        }
        if(r & USERRIGHT_VIEW_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("View message");
        }
        if(r & USERRIGHT_MODIFY_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Modify own message");
        }
        if(r & USERRIGHT_CLOSE_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Close own message thread");
        }
        if(r & USERRIGHT_OPEN_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Open own message thread");
        }
        if(r & USERRIGHT_CREATE_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Reply on message");
        }
        if(r & USERRIGHT_CREATE_MESSAGE_THREAD) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Create new message thread");
        }
        if(r & USERRIGHT_ALLOW_HTML) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Allow HTML");
        }
        if(r & USERRIGHT_PROFILE_MODIFY) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Own Profile modification");
        }
        if(r & USERRIGHT_PROFILE_CREATE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("New profile creation");
        }
        if(r & USERRIGHT_ROLL_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Roll own messages");
        }
        if(r & USERRIGHT_UNROLL_MESSAGE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Unroll own messages");
        }
        if(r & USERRIGHT_POST_GLOBAL_ANNOUNCE) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Post global announce");
        }
        if(r & USERRIGHT_ALT_DISPLAY_NAME) {
                if(smthprinted) printf(", ");
                else smthprinted = 1;
                printf("Alternative name");
        }
        printf("\n");
}

void printerror(DWORD er)
{
        printf("Result: ");
        switch(er) {
        case PROFILE_RETURN_ALLOK:
                printf("All ok\n");
        case PROFILE_RETURN_ALREADY_EXIST:
                printf("User already exits\n");
                break;
        case PROFILE_RETURN_DB_ERROR:
                printf("Database error\n");
                break;
        case PROFILE_RETURN_INVALID_FORMAT:
                printf("Invalid file format\n");
                break;
        case PROFILE_RETURN_INVALID_LOGIN:
                printf("Invalid login name\n");
                break;
        case PROFILE_RETURN_INVALID_PASSWORD:
                printf("Invalid password\n");
                break;
        case PROFILE_RETURN_PASSWORD_SHORT:
                printf("Password too short\n");
                break;
        case PROFILE_RETURN_UNKNOWN_ERROR:
                printf("Unknown database error\n");
                break;
        }
}

int CreateProfilesDatabase()
{
        FILE *f;
        if((f = fopen(F_PROF_NINDEX, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating Database file : %s\n" \
                        "Please make sure you have necessary directory structure\n",
                        F_PROF_NINDEX);
                return 0;
        }
        else {
                DWORD x = 0;
                fwrite(&x, 1, sizeof(x), f);
                fwrite(&x, 1, sizeof(x), f);
                fclose(f);
        }
        if((f = fopen(F_PROF_FREENIDX, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_PROF_FREENIDX);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_PROF_BODY, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_PROF_BODY);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_PROF_FREEBODY, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_PROF_FREEBODY);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_PROF_PERSMSG, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_PROF_PERSMSG);
                return 0;
        }
        else fclose(f);

        remove(F_PROF_INDEX);
        if(AddStringToHashedIndex("aaa", 1) != HASHINDEX_ER_OK) {
                printf("Error creating profiles database file : %s\n", F_PROF_INDEX);
                return 0;
        }
        if(DeleteStringFromHashedIndex("aaa") != HASHINDEX_ER_OK) {
                printf("Error creating profiles database file : %s\n", F_PROF_INDEX);
                return 0;
        }
        return 1;
}

int CreateMessagesDatabase()
{
        FILE *f;
        if((f = fopen(F_MSGINDEX, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_MSGINDEX);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_MSGBODY, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_MSGBODY);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_INDEX, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_INDEX);
                return 0;
        }
        else fclose(f);
        if((f = fopen(F_VINDEX, FILE_ACCESS_MODES_CW)) == NULL) {
                printf("Error creating profiles database file : %s\n", F_VINDEX);
                return 0;
        }
        else fclose(f);
        return 1;
}

int CheckAndCreateFolder(const char *d, const char *s)
{
        if(chdir(d) != 0) {
           if(mkdir(d, DIR_CREATION_MASK) != 0) {
                        printf("Cannot create directory %s\n", d);
                        return 0;
                }
                else printf("%s created\n", d);
        }
        else {
                // return to our folder
                if(chdir(s) != 0) {
                        printf("Cannot change directory to %s\n", s);
                        return 0;
                }
        }
        return 1;
}

int CreateFullDatabase()
{
        // alloc mem and save our current folder
        char *s = (char*)malloc(2560);
        if(getcwd(s, 2559) == NULL) {
                printf("Cannot get current directory\n");
                return 0;
        }

        if(!CheckAndCreateFolder(DIR_MAINDATA, s))
                return 0;

        if(!CheckAndCreateFolder(DIR_MESSAGES, s))
                return 0;

        if(!CheckAndCreateFolder(DIR_PROFILES, s))
                return 0;

        if(!CheckAndCreateFolder(DIR_PROF_PIC, s))
                return 0;

        if(!CheckAndCreateFolder(DIR_SETTINGS, s))
                return 0;

        if(!CheckAndCreateFolder(DIR_INTERNALS, s))
                return 0;
        free(s);

        if(!CreateProfilesDatabase())
                return 0;
        if(!CreateMessagesDatabase())
                return 0;

        return 1;
}

int main(int argc, char *argv[])
{
        CProfiles *ul;
        DWORD errcode;

        SProfile_UserInfo ui;
        WCFILE *fw;

        ul = new CProfiles();
        if(ul->errnum != PROFILE_RETURN_ALLOK) {
        }
        
        printf("WWWConf console management tool\n");

#if USE_LOCALE
        if(setlocale(LC_ALL, LANGUAGE_LOCALE) == NULL) {
                printf("Setting locale [%s] failed !\n", LANGUAGE_LOCALE);
                printf("Bailing out to default locale \"C\"");
                setlocale(LC_ALL, "C");
        }
#endif
        
        if(argc <= 1) {
                goto go_end;
        }
        if((strcmp(argv[1], "-au") == 0 || strcmp(argv[1], "-aa") == 0) && argc == 4) {
                // create user
                SProfile_UserInfo ui;
                SProfile_FullUserInfo fui;
                
                strcpy(fui.Email, "");
                strcpy(fui.FullName, argv[2]);
                strcpy(fui.Signature, "");
                strcpy(fui.HomePage, "");
                strcpy(fui.SelectedUsers, "");
                strcpy(ui.username, argv[2]);
                strcpy(ui.password, argv[3]);
                strcpy(ui.icqnumber, "");
                strcpy(ui.altdisplayname, "");
                fui.AboutUser = NULL;

                ui.lastIP = 0;
                ui.Flags = 0; // don't have picture or signature
                if(strcmp(argv[1], "-aa") != 0)
                        ui.right = DEFAULT_USER_RIGHT;
                else
                        ui.right = DEFAULT_ADMIN_RIGHT;

                if((errcode = ul->AddNewUser(&ui, &fui, NULL)) == PROFILE_RETURN_ALLOK)
                        printf("User successfully created\n");
                else printerror(errcode);
                goto go_stop;
        }

        if(strcmp(argv[1], "-d") == 0 && argc == 3) {
                if((errcode = ul->DeleteUser(argv[2])) == PROFILE_RETURN_ALLOK) {
                        printf("User successfully deleted\n");
                }
                else printerror(errcode);
                goto go_stop;
        }

        if(strcmp(argv[1], "-v") == 0 && argc == 3) {
                SProfile_FullUserInfo fui;
                DWORD ind;
                if(ul->errnum != PROFILE_RETURN_ALLOK) {
                        printf("Fatal: profiles database could not be accessed\n");
                        goto go_stop;
                }
                if((errcode = ul->GetUserByName(argv[2], &ui, &fui, &ind)) == PROFILE_RETURN_ALLOK) {
                        printf("User Information\n");
                        printf("Login name : %s  Password : %s\n", ui.username, ui.password);
                        fui.SelectedUsers[PROFILES_FULL_USERINFO_MAX_SELECTEDUSR - 1] = 0;
                        printf("AltName : %s\n, Full name : %s, E-mail : %s\nSelected users: %s\nAbout : %s\nSignature: %s\n"
                                   "Flags: 0x%08lx\nRefresh count : %lu\n",
                                ui.altdisplayname, fui.FullName, fui.Email, fui.SelectedUsers, fui.AboutUser,
                                fui.Signature, ui.Flags, ui.RefreshCount);
                        printuserrigth(ui.right);
                        
                        char *s;
                        s = ctime(&fui.CreateDate);
                        printf("Creation date: %s\n", s);

                        int nf = 0;

                        if(ui.LoginDate != 0) s = ctime(&ui.LoginDate);
                        else {
                                nf = 1;
                                s = (char*)malloc(100);
                                strcpy(s, "Never logged in");
                        }        
                        printf("Last access date: %s\n", s);
                        if(nf) free(s);
/*                        ui.LoginDate -= 36000*7;
                        ul->SetUInfo(ind, &ui);*/
                }
                else printerror(errcode);
                goto go_stop;
        }

        if(argc == 2 && strcmp(argv[1], "-np") == 0) {
                printf("Renewing private messages database...\n");
                if((fw = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_CW)) != NULL) {
                        wcfclose(fw);
                }
                else {
                        printf("Fatal: profiles database could not be accessed !!!\n");
                        return 0;
                }
                if((fw = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) != NULL) {
                        int i = 0;
                        lock_file(fw);
                        wcfseek(fw, 8, SEEK_SET);
                        while(!wcfeof(fw)) {
                                DWORD pos = wcftell(fw);
                                if(wcfread(&ui, 1, sizeof(SProfile_UserInfo), fw) < sizeof(SProfile_UserInfo))
                                        break;
                                ui.persmescnt = 0;
                                ui.persmsg = 0xffffffff;
                                ui.readpersmescnt = 0;
                                ui.postedpersmsg = 0xffffffff;
                                ui.postedmescnt = 0;
                                wcfseek(fw, pos, SEEK_SET);
                                wcfwrite(&ui, 1, sizeof(SProfile_UserInfo), fw);
                                wcfseek(fw, wcftell(fw), SEEK_SET);
                                i++;
                        }
                        printf("%d users processed\n", i);
                        unlock_file(fw);
                        wcfclose(fw);
                        printf("Done!\n");
                }
                else printf("Fatal: profiles database could not be accessed !!!\n");
                goto go_stop;
        }


           if(strcmp(argv[1], "-cp") == 0 && argc == 3) {
                SProfile_FullUserInfo fui;
                DWORD ind;
                if(ul->errnum != PROFILE_RETURN_ALLOK) {
                        printf("Fatal: profiles database could not be accessed\n");
                        goto go_stop;
                }
                if((errcode = ul->GetUserByName(argv[2], &ui, &fui, &ind)) == PROFILE_RETURN_ALLOK) {
                        printf("Clean Privates\n");
                
                        printf("Login name : %s  persmescnt : %d\n", ui.username, ui.persmescnt);
                        
                        ui.persmescnt = 0;
                        ui.persmsg = 0xffffffff;
                        ui.readpersmescnt = 0;
                        ui.postedpersmsg = 0xffffffff;
                        ui.postedmescnt = 0;


                        ul->SetUInfo(ind, &ui);
                }
                else printerror(errcode);
                goto go_stop;
        }


        if(argc == 2 && strcmp(argv[1], "-r") == 0) {
                printf("Clearing refresh count...\n");
                if((fw = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) != NULL) {
                        int i = 0;
                        lock_file(fw);
                        wcfseek(fw, 8, SEEK_SET);
                        while(!wcfeof(fw)) {
                                DWORD pos = wcftell(fw);
                                if(wcfread(&ui, 1, sizeof(SProfile_UserInfo), fw) < sizeof(SProfile_UserInfo))
                                        break;
                                ui.RefreshCount = 0;
                                wcfseek(fw, pos, SEEK_SET);
                                wcfwrite(&ui, 1, sizeof(SProfile_UserInfo), fw);
                                wcfseek(fw, wcftell(fw), SEEK_SET);
                                i++;
                        }
                        printf("%d users processed\n", i);
                        unlock_file(fw);
                        wcfclose(fw);
                        printf("Done!\n");
                }
                else printf("Fatal: profiles database couldn't be accessed !!!\n");
                goto go_stop;
        }

        if(argc == 2 && strcmp(argv[1], "-vs") == 0) {
                printf("Clearing view settings...\n");
                if((fw = wcfopen(F_PROF_NINDEX, FILE_ACCESS_MODES_RW)) != NULL) {
                        int i = 0;
                        lock_file(fw);
                        wcfseek(fw, 8, SEEK_SET);
                        while(!wcfeof(fw)) {
                                DWORD pos = wcftell(fw);
                                if(wcfread(&ui, 1, sizeof(SProfile_UserInfo), fw) < sizeof(SProfile_UserInfo))
                                        break;
                                
                        /*        ui.vs.dsm = CONFIGURE_SETTING_DEFAULT_dsm;
                                ui.vs.topics = CONFIGURE_SETTING_DEFAULT_topics;
                                ui.vs.tv = CONFIGURE_SETTING_DEFAULT_tv;
                                ui.vs.tc = CONFIGURE_SETTING_DEFAULT_tc;
                                ui.vs.ss = CONFIGURE_SETTING_DEFAULT_ss;
                                ui.vs.lsel = CONFIGURE_SETTING_DEFAULT_lsel;
                                ui.vs.tt = CONFIGURE_SETTING_DEFAULT_tt;
                                ui.icqnumber[0] = 0;
                                ui.Flags |= PROFILES_FLAG_VIEW_SETTINGS; 
                                ui.vs.tz = DATETIME_DEFAULT_TIMEZONE;
                                */
                                ui.vs.topics  |= (1<<17);        
                
                                wcfseek(fw, pos, SEEK_SET);
                                wcfwrite(&ui, 1, sizeof(SProfile_UserInfo), fw);
                                wcfseek(fw, wcftell(fw), SEEK_SET);
                                i++;
                        }
                        printf("%d users processed\n", i);
                        unlock_file(fw);
                        wcfclose(fw);
                        printf("Done!\n");
                }
                else printf("Fatal: profiles database couldn't be accessed !!!\n");
                goto go_stop;
        }

        if(argc == 3 && strcmp(argv[1], "-vp") == 0) {
                SPersonalMessage *topm, *frompm;
                CProfiles prof;
                int code;
                if((code = prof.GetUserByName(argv[2], &ui, NULL, NULL)) != PROFILE_RETURN_ALLOK) {
                        printf("User not found!\n");
                        exit(0);
                }
                if((code = prof.ReadPersonalMessages(argv[2], 0, &topm, NULL, &frompm, NULL)) == PROFILE_RETURN_ALLOK) {
                        char tostr[1000], newm[100];
                        char *ss;
                        SPersonalMessage *pmsg;
                        int i = 0;
                        int j = 0;
                        int received = 0;        // posted or received
                        int cnt = 0, postedcnt = 0;
                        if(topm) {
                                while(topm[cnt].Prev != 0xffffffff) cnt++;
                                cnt++;
                        }
                        if(frompm) {
                                while(frompm[postedcnt].Prev != 0xffffffff) postedcnt++;
                                postedcnt++;
                        }
                        printf("        Messages for user %s\n", argv[2]);
                        for(;;) {
                                // check exit expression
                                if(i == cnt && j == postedcnt) break;
                                if(i == cnt) {
                                        pmsg = &(frompm[j]);
                                        j++;
                                        received = 0;
                                } else {
                                        if(j == postedcnt) {
                                                pmsg = &(topm[i]);
                                                i++;
                                                received = 1;
                                        }
                                        else {
                                                if(frompm[j].Date > topm[i].Date) {
                                                        pmsg = &(frompm[j]);
                                                        j++;
                                                        received = 0;
                                                }
                                                else {
                                                        pmsg = &(topm[i]);
                                                        i++;
                                                        received = 1;
                                                }
                                        } 
                                }
        
                                if(!received) {
                                        strcpy(tostr, pmsg->NameTo);
                                }
                                else {
                                        strcpy(tostr, pmsg->NameFrom);
                                }

                                ss = ctime(&pmsg->Date);

                                if(received && i <= (ui.persmescnt - ui.readpersmescnt))
                                        strcpy(newm, "Not readed");
                                else strcpy(newm, "");

                                if(!received) {
                                        // posted message
                                        printf("To user: %s, Date: %s\t\t\t%s", tostr, ss, newm);
                                }
                                else {
                                        // received message
                                        printf("From user: %s, Date: %s\t\t\t%s", tostr, ss, newm);
                                }

                                printf("%s\n\n", pmsg->Msg);
                        }
                }
                else printf("ReadPersonalMessages() returned error code: %x (%s)\n", code, (code == PROFILE_RETURN_INVALID_LOGIN) ? "unknown user" : "database error");
                goto go_stop;
        }

        if(argc == 2 && strcmp(argv[1], "-nu") == 0) {
                printf("Creating/Renewing profiles database...\n");
                if(!CreateProfilesDatabase())
                        goto go_stop;
                printf("Operation completed successfully\n");
                goto go_stop;
        }

        if(argc == 2 && strcmp(argv[1], "-n") == 0) {
                printf("Creating/Renewing profiles and messages databases...\n");
                if(!CreateFullDatabase())
                        goto go_stop;
                printf("Operation completed successfully\n");
                goto go_stop;
        }

        if(argc == 2 && strcmp(argv[1], "-nm") == 0) {
                printf("Creating/Renewing messages database...\n");
                if(!CreateMessagesDatabase())
                        goto go_stop;
                printf("Operation completed successfully\n");
                goto go_stop;
        }

go_end:
        printf("Fatal: Invalid option was specified: %s\n\n", argv[1] ? argv[1] : "");
        printusage(argv[0]);
go_stop:
        delete ul;
        return 0;
}
