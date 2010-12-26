/***************************************************************************
                          reindex.cpp  -  message index build tool
                             -------------------
    begin                : Wed Apr 29 2001
    copyright            : (C) 2001 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "basetypes.h"
#include "searcher.h"

void printusage(char *iam)
{
        printf("Usage:\n" \
                   "          %s -r [max_index]      - index messages (up to max_index (optional))\n" \
                   "          %s -s word             - search for word\n", iam, iam);
        exit(0);
}

int main(int argc, char *argv[])
{
        FILE *f, *f1, *f2, *fl;
        DWORD StartMsg = 0, LastDate = 0, StopIndex = 0;
        CMessageSearcher *in;
        SMessage *sm;
        time_t now, start;

        printf("WWWConf search index builder\n");
#if USE_LOCALE
        if(setlocale(LC_ALL, LANGUAGE_LOCALE) == NULL) {
                printf("Setting locale [%s] failed !\n", LANGUAGE_LOCALE);
                printf("Bailing out to default locale \"C\"");
                setlocale(LC_ALL, "C");
        }
#endif
        if(argc < 2 || (argc >= 2 && strcmp(argv[1], "-r") != 0)) {
                if(argc == 3 && strcmp(argv[1], "-s") == 0) {
                        in = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
                        if(in->errnum != SEARCHER_RETURN_ALLOK) {
                                printf("Searcher init = failed\n");
                                exit(0);
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
                        exit(0);
                }
                else {
                        printusage(argv[0]);
                        exit(0);
                }
        }

        if(argc != 3 && argc != 2) {
                printusage(argv[0]);
                exit(0);
        }
        if(argc == 3) {
                char *st;
                StopIndex = strtol(argv[2], &st, 10);
                if(((!(*(argv[2]) != '\0' && *st == '\0')) || errno == ERANGE)) {
                        printusage(argv[0]);
                        exit(0);
                }
                else {
                        // correct value
                }
        }
        else StopIndex = 0xffffffff;

        fl = fopen("lastindex", FILE_ACCESS_MODES_RW);
        if(fl == NULL) {
        }
        else {
                fscanf(fl, "%d %d", &StartMsg, &LastDate);
                fclose(fl);
        }

        if(StartMsg == 0) {
                f = fopen(F_SEARCH_DB, FILE_ACCESS_MODES_CW);
                if(f == NULL) {
                        printf("Error opening DB file\n");
                        exit(0);
                }
                fclose(f);
                in = new CMessageSearcher(SEARCHER_INDEX_CREATE_NEW);
        }
        else in = new CMessageSearcher(SEARCHER_INDEX_CREATE_EXISTING);
        if(in->errnum != SEARCHER_RETURN_ALLOK) {
                printf("Searcher init = failed\n");
                exit(0);
        }

        sm = (SMessage*)malloc(sizeof(SMessage)*READ_MESSAGE_HEADER);
        in->indexed_word_count = 0;

        f = fopen(F_MSGINDEX, FILE_ACCESS_MODES_R);
        f1 = fopen(F_MSGBODY, FILE_ACCESS_MODES_R);
        f2 = fopen(F_VINDEX, FILE_ACCESS_MODES_R);

        if(f == NULL || f1 == NULL || f2 == NULL) {
                printf("Error opening database files\n");
                exit(0);
        }

        DWORD rr, idx;
        char *mbody;
        DWORD wc = 0;
        DWORD tm = time(NULL);
        printf("Indexing(starting from %ld), report format: NUM-SEC, NUM-processed mess, SEC-seconds last 100 mess were indexed", StartMsg);

        if(fseek(f2, (StartMsg+1)*sizeof(DWORD), SEEK_SET) != 0) {
                printf(" Invalid starting index\n");
                exit(0);
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
                rr = fread(sm, 1, sizeof(SMessage), f);
                if(rr != sizeof(SMessage) && rr != 0) {
                        printf("Error reading %s\n", F_MSGINDEX);
                        exit(0);
                }
                wc++;
                in->InsertMessageToIndex(sm->MessageHeader, sm->ViIndex);

        //        fflush(stdout);
                if((sm->Flag & MESSAGE_HAVE_BODY) != 0 && sm->msize > 2) {
                        DWORD readed;
                        mbody = (char*)malloc(sm->msize + 10);
                        if(fseek(f1, sm->MIndex, SEEK_SET) != 0) {
                                free(mbody);
                                continue;
                        }
                        if((readed = fread(mbody, 1, sm->msize + 2, f1)) < sm->msize) {
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
                                exit(0);
                        }
                        in->InsertMessageToIndex(mbody, sm->ViIndex);
                        free(mbody);
                }
                if((wc % 100) == 0) {
                        now = time(NULL);
                    printf("%ld-%d.", wc, start-now);
                        start = now;
                        fflush(stdout);
                }
        }
        fclose(f);
        fclose(f1);
        fclose(f2);
        printf("\nDone at %ld\nFlusing cache...", StartMsg);
        DWORD iww = in->indexed_word_count;
        delete in;

        fl = fopen("lastindex", FILE_ACCESS_MODES_CW);
        if(f != NULL) {
                fprintf(fl, "%d %d", StartMsg, LastDate);
                fclose(fl);
        }

        printf("\nFinished in %ld seconds, words indexed %ld\n", time(NULL) - tm, iww);
        free(sm);
        return 0;
}
