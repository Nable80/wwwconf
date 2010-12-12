/***************************************************************************
                    profman.cpp  -  profile and database management tool
                             -------------------
    begin                : Wed May 14 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "profiles.h"
#include "searcher.h"

#define DIR_CREATION_MASK	511
#define FILES_CREATION_MASK 511

int HPrinted = 0;

void printusage(char *iam)
{
	printf("Usage:\n\t %s -n              - create/renew database (create new messages and profiles database)\n" \
				   "\t %s -nu             - create/renew ONLY profiles database (delete all profiles)\n" \
				   "\t %s -nm             - create/renew ONLY messages database (delete all forum messages)\n" \
				   "\t %s -dcheck         - check database consistency and print database information\n" \
				   "\t %s -v user         - view user information for user 'username'\n" \
				   "\t %s -vp username    - view all private messages for user 'username'\n" \
				   "\t %s -aa user passwd - create admin (spec. username and password)\n" \
				   "\t %s -au user passwd - create user (spec. username and password)\n" \
				   "\t %s -d user         - delete user\n" \
				   "\t %s -np             - renew(delete) private messages database (in profiles)\n" \
				   "\t %s -r              - zero refresh count for all users\n" \
				   "\t %s -vs             - set default view settings for all users\n" \
				   "\t %s -sr [max_index] - create search index (up to max_index message, optional))\n" \
				   "\t %s -s [word]       - search for word using index\n",
				iam, iam, iam, iam, iam, iam, iam, iam, iam, iam, iam, iam, iam, iam);
	exit(0);
}

int mmkdir(const char *s)
{
	int r;
	char *ss;
	if((ss = strchr(s, '/')) || (ss = strchr(s, '\\'))) {
		char os[1000];
		if(strlen(ss) > 1) {
			strncpy(os, s, ss - s);
			os[ss - s] = 0;
#ifdef WIN32
			mkdir(os);
#else
			mkdir(os, DIR_CREATION_MASK);
#endif
		}
		if((r =
#ifdef WIN32
			mkdir(s)
#else
			mkdir(s, DIR_CREATION_MASK)
#endif
		) != 0)
			return r;
	}
	return r;
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
	if(r & USERRIGTH_ALLOW_HTML) {
		if(smthprinted) printf(", ");
		else smthprinted = 1;
		printf("Allow HTML");
	}
	if(r & USERRIGTH_PROFILE_MODIFY) {
		if(smthprinted) printf(", ");
		else smthprinted = 1;
		printf("Own Profile modification");
	}
	if(r & USERRIGTH_PROFILE_CREATE) {
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

int CheckAndCreateFolder(char *d, char *s)
{
	if(chdir(d) != 0) {
		if(mmkdir(d) != 0) {
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

	if(!CheckAndCreateFolder(DIR_SEARCHER, s))
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
		if(strcmp(argv[1], "-aa") != 0) {
			ui.right = DEFAULT_USER_RIGHT;
			ui.secur = DEFAULT_USER_SECURITY_BYTE;
			ui.secheader = DEFAULT_USER_HDR_SEC_BYTE;
		}
		else {
			ui.right = DEFAULT_ADMIN_RIGHT;
			ui.secur = DEFAULT_ADMIN_SECURITY_BYTE;
			ui.secheader = DEFAULT_ADMIN_HDR_SEC_BYTE;
		}

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
			int sr = ui.secheader;
			int srb = ui.secur;
			printf("Login name : %s  Password : %s\n", ui.username, ui.password);
			fui.SelectedUsers[PROFILES_FULL_USERINFO_MAX_SELECTEDUSR - 1] = 0;
			printf("AltName : %s\n, Full name : %s, E-mail : %s\nSelected users: %s\nAbout : %s\nSignature: %s\n"
				   "Flags: 0x%08x\nSecurity right byte (hdr) : %d\nSecurity right byte (body) : %d\nRefresh count : %d\n",
				ui.altdisplayname, fui.FullName, fui.Email, fui.SelectedUsers, fui.AboutUser,
				fui.Signature, ui.Flags, sr, srb, ui.RefreshCount);
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
/*			ui.LoginDate -= 36000*7;
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
		
			printf("Login name : %s  persmescnt : %ld\n", ui.username, ui.persmescnt);
			
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
				
			/*	ui.vs.dsm = CONFIGURE_SETTING_DEFAULT_dsm;
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
			int received = 0;	// posted or received
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

	if(argc == 2 && strcmp(argv[1], "-dcheck") == 0) {
		printf("Checking profiles database...\n");
#if 0
		printf("Operation completed successfully\n");
		printf("Checking messages database...\n");
		printf("Operation completed successfully\n");
#else
		printf("Function not yet implemented!\n");
#endif
		goto go_stop;
	}

	if(strcmp(argv[1], "-s") == 0) {
		if(argc == 3) {
			CMessageSearcher *in = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
			if(in->errnum != SEARCHER_RETURN_ALLOK) {
				printf("Searcher init = failed\nMake sure that search index was created\n");
				goto go_stop;
			}
			DWORD c;
			DWORD *buf = in->SearchMessagesByPattern(argv[2], &c);
			if(c > 0) {
				printf("Found %ld matches in messages : ", c);
				for(register DWORD i = 0; i < c; i++) {
					printf("%ld ", buf[i]);
				}
				free(buf);
			}
			else {
				printf("Found 0 matches");
			}
			printf("\n");
			delete in;
			goto go_stop;
		}
		else {
			printf("Invalid parameters count for -s\n");
			goto go_stop;
		}
	}

	if(strcmp(argv[1], "-sr") == 0) {
		DWORD StartMsg = 0, LastDate = 0, DBDirty = 0, StopIndex = 0;
		CMessageSearcher *in = NULL;
		SMessage *sm;
		time_t now, start;
		FILE *fl, *f, *f1, *f2;
		if(argc == 2) {
			StopIndex = 0xffffffff;
		}
		else {
			char *st;
			StopIndex = strtol(argv[2], &st, 10);
			if(((!(*(argv[2]) != '\0' && *st == '\0')) || errno == ERANGE)) {
				printf("Invalid parameter for -sr command\n");
				goto go_stop;
			}
		}
		
		fl = fopen(F_SEARCH_LASTINDEX, FILE_ACCESS_MODES_RW);
		if(fl != NULL) {
			fscanf(fl, "%d %d", &StartMsg, &LastDate, &DBDirty);
			fclose(fl);
			if(DBDirty) {
				printf("Database marked as dirty (another instance of indexer in progress?)\n", F_SEARCH_LASTINDEX);
				goto go_stop;
			}
		}

		// mark as dirty
		fl = fopen(F_SEARCH_LASTINDEX, FILE_ACCESS_MODES_CW);
		if(f != NULL) {
			fprintf(fl, "%d %d %d", StartMsg, LastDate, 1);
			fclose(fl);
		}
		else {
			printf("Couldn't create status file %s\n", F_SEARCH_LASTINDEX);
			goto go_stop;
		}

		if(StartMsg == 0) {
			f = fopen(F_SEARCH_DB, FILE_ACCESS_MODES_CW);
			if(f == NULL) {
				printf("Error creating searcher database file %s\n", F_SEARCH_DB);
				goto go_stop;
			}
			fclose(f);
			in = new CMessageSearcher(SEARCHER_INDEX_CREATE_NEW);
		}
		else in = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
		if(in->errnum != SEARCHER_RETURN_ALLOK) {
			printf("Searcher init = failed\n");
			goto go_stop;
		}

		sm = (SMessage*)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER);
		in->indexed_word_count = 0;

		f = fopen(F_MSGINDEX, FILE_ACCESS_MODES_R);
		f1 = fopen(F_MSGBODY, FILE_ACCESS_MODES_R);
		f2 = fopen(F_VINDEX, FILE_ACCESS_MODES_R);

		if(f == NULL || f1 == NULL || f2 == NULL) {
			free(sm);
			printf("Error opening database files\n");
			goto go_stop;
		}

		DWORD rr, idx;
		char *mbody;
		DWORD wc = 0;
		time_t tm = time(NULL);
		printf("Indexing(starting from %ld), report format: NUM-SEC, NUM-processed mess, SEC-seconds last 100 mess were indexed", StartMsg);

		if(fseek(f2, (StartMsg+1)*sizeof(DWORD), SEEK_SET) != 0) {
			free(sm);
			printf("Invalid starting index\n");
			goto go_stop;
		}

		start = time(NULL);
		while(!feof(f2)) {

			if(StartMsg >= StopIndex) {
				printf("Stopped at index %ld (StopIndex)", StartMsg);
				break;
			}

			if(fread(&idx, 1, sizeof(DWORD), f2) != sizeof(DWORD)) {
				break;
			}
			StartMsg++;

			if(idx == 0xFFFFFFFF) continue;

			if(fseek(f, idx, SEEK_SET) != 0) {
				continue;
			}
			rr = (DWORD)fread(sm, 1, sizeof(SMessage), f);
			if(rr != sizeof(SMessage) && rr != 0) {
				printf("Error reading %s\n", F_MSGINDEX);
				goto go_stop;
			}
			wc++;
			in->InsertMessageToIndex(sm->MessageHeader, sm->ViIndex);

			if((sm->Flag & MESSAGE_HAVE_BODY) != 0 && sm->msize > 2) {
				DWORD readed;
				mbody = (char*)malloc(sm->msize + 10);
				if(fseek(f1, sm->MIndex, SEEK_SET) != 0) {
					free(mbody);
					continue;
				}
				if((readed = (DWORD)fread(mbody, 1, sm->msize + 2, f1)) < sm->msize) {
					free(mbody);
					continue;
				}
				mbody[readed] = 0;
				if(sm->msize != 0 && *mbody == 0) {
					char *ss = mbody;
					ss++;
					while(*ss) {
						*(ss-1) = *(ss);
						ss++;
					}
					*(ss-1) = *ss;
				}
				if(strlen(mbody) > 65000) {
					printf("\nIncorrect DB format: Body of message %d is too long\n", sm->ViIndex);
					goto go_stop;
				}
				in->InsertMessageToIndex(mbody, sm->ViIndex);
				free(mbody);
			}
			if((wc % 100) == 0) {
				now = time(NULL);
				printf("%ld-%d.", wc, start - now);
				start = now;
				fflush(stdout);
			}
		}
		fclose(f);
		fclose(f1);
		fclose(f2);
		printf("\nDone at %ld\nFlusing cache...", StartMsg);
		DWORD iww = in->indexed_word_count;
		DWORD tcache = time(NULL);
		delete in;
		printf("done in %ld seconds\n", time(NULL) - tcache);

		fl = fopen(F_SEARCH_LASTINDEX, FILE_ACCESS_MODES_CW);
		if(f != NULL) {
			fprintf(fl, "%d %d %d", StartMsg, LastDate, 0);
			fclose(fl);
		}

		printf("\nFinished in %ld seconds, words indexed %ld\n", time(NULL) - tm, iww);
		free(sm);
		goto go_stop;
	}

go_end:
	printf("Fatal: Invalid option was specified: %s\n\n", argv[1] ? argv[1] : "");
	printusage(argv[0]);
go_stop:
	delete ul;
	return 0;
}
