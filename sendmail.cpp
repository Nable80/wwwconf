/***************************************************************************
                          sendmail.cpp  -  mail sending support
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2002 by Alexander Bilichenko
    email                : pricer@mail.ru
 ***************************************************************************/

#include "sendmail.h"
#include "messages.h"
#include "error.h"


static unsigned char dtable[512];

void buffer_new(struct buffer_st *b)
{
  b->length = 512;
  b->data = (char *)malloc(sizeof(char)*(b->length));
  b->data[0] = 0;
  b->ptr = b->data;
  b->offset = 0;
}

void buffer_add(struct buffer_st *b, char c)
{
  *(b->ptr++) = c;
  b->offset++;
  if (b->offset == b->length) {
    b->length += 512;
    b->data = (char *)realloc(b->data, b->length);
    b->ptr = b->data + b->offset;
  }
}

void buffer_end(struct buffer_st *b)
{
    b->data[b->offset++]= 0;
    //b->data = (char *)realloc(b->data, b->offset++);
  
}

void buffer_delete(struct buffer_st *b)
{
  free(b->data);
  b->length = 0;
  b->offset = 0;
  b->ptr = NULL;
  b->data = NULL;
}




void base64_encode(struct buffer_st *b, const char *source, int length)
{
  int i, hiteof = 0;
  int offset = 0;
  int olen;
  
  olen = 0;
  
  buffer_new(b);
  
  /*	Fill dtable with character encodings.  */
  
  for (i = 0; i < 26; i++) {
    dtable[i] = 'A' + i;
    dtable[26 + i] = 'a' + i;
  }
  for (i = 0; i < 10; i++) {
    dtable[52 + i] = '0' + i;
  }
  dtable[62] = '+';
  dtable[63] = '/';
  
  while (!hiteof) {
    unsigned char igroup[3], ogroup[4];
    int c, n;
    
    igroup[0] = igroup[1] = igroup[2] = 0;
    for (n = 0; n < 3; n++) {
      c = *(source++);
      offset++;
      if (offset > length) {
	hiteof = 1;
	break;
      }
      igroup[n] = (unsigned char) c;
    }
    if (n > 0) {
      ogroup[0] = dtable[igroup[0] >> 2];
      ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
      ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
      ogroup[3] = dtable[igroup[2] & 0x3F];
      
      /* Replace characters in output stream with "=" pad
	 characters if fewer than three characters were
	 read from the end of the input stream. */
      
      if (n < 3) {
	ogroup[3] = '=';
	if (n < 2) {
	  ogroup[2] = '=';
	}
      }
      for (i = 0; i < 4; i++) {
	buffer_add(b, ogroup[i]);
	//if (!(b->offset % 72)) {
	//  buffer_add(b, '\r');
	//  buffer_add(b, '\n');
	//}
      }
    }
  }
   //buffer_add(b, '\r');
  //buffer_add(b, '\n');
 buffer_end(b);
}



#if  MA_TYPE == 1

/* Store the address of HOSTNAME, internet-style, to WHERE.  First
   check for it in the host list, and (if not found), use
   ngethostbyname to get it.
   Return 1 on successful finding of the hostname, 0 otherwise.  */
int store_hostaddress(unsigned char *where, const char *hostname)
{
	unsigned long addr;
	struct hostent *hptr;
	struct in_addr in;

	/* If the address is of the form d.d.d.d, there will be no trouble
	   with it.  */
	addr = (unsigned long)inet_addr(hostname);
	/* If we have the numeric address, just store it.  */
	if((int)addr != -1)
	{
		/* This works on both little and big endian architecture, as
		   inet_addr returns the address in the proper order.  It
		   appears to work on 64-bit machines too.  */
		memcpy (where, &addr, 4);
		return 1;
	}
	/* Since all else has failed, let's try gethostbyname().  Note that
	   we use gethostbyname() rather than ngethostbyname(), because we
	   *know* the address is not numerical. */
	hptr = gethostbyname (hostname);
	if (!hptr)
		return 0;
	/* Copy the address of the host to socket description.  */
	memcpy(where, hptr->h_addr_list[0], hptr->h_length);
	/* Now that we're here, we could as well cache the hostname for
	   future use, as in realhost().  First, we have to look for it by
	   address to know if it's already in the cache by another name.  */

	/* Originally, we copied to in.s_addr, but it appears to be missing
	   on some systems.  */
	memcpy(&in, *hptr->h_addr_list, sizeof (in));
	return 1;
}

/* Create an internet connection to HOSTNAME on PORT.  The created
   socket will be stored to *SOCK.  */
int make_connection(int *sock, const char *hostname, unsigned short port)
{
	struct sockaddr_in sock_name;
	/* struct hostent *hptr; */

	/* Get internet address of the host.  We can do it either by calling
	   ngethostbyname, or by calling store_hostaddress, from host.c.
	   storehostaddress is better since it caches calls to
	   gethostbyname. */
	   
#if 1
	if(!store_hostaddress ((unsigned char *)&sock_name.sin_addr, hostname))
		return 0;
#else  /* never */
	if(!(hptr = ngethostbyname (hostname)))
		return 0;
	// Copy the address of the host to socket description.
	memcpy(&sock_name.sin_addr, hptr->h_addr, hptr->h_length);
#endif /* never */

	/* Set port and protocol */
	sock_name.sin_family = AF_INET;
	sock_name.sin_port = htons (port);

	/* Make an internet socket, stream type.  */
	if ((*sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
		return 0;

	/* Connect the socket to the remote host. */
	if(connect(*sock, (struct sockaddr *) &sock_name, sizeof (sock_name)))
	{
		return 0;
    }

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	if(setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(timeval)) == -1)
		return 0;
	if(setsockopt(*sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(timeval)) == -1)
		return 0;

	return 1;
}




int wcSendMail(char *to, char *subj, char *body)
{
	int ssock;
	int rc;
	char sbuf[100000];
	char rbuf[10000];

	if(!make_connection(&ssock, MA_SENDER, 25))
		return 0;
	sprintf(sbuf, MAIL_SEND_HELO, MA_FROM, to);
	
	if(send(ssock, sbuf, strlen(sbuf), 0) != (int)strlen(sbuf))
		return 0;

	sleep(1);
	rc = recv(ssock, rbuf, 9999, 0);

	
	buffer_st subj_base64;
	base64_encode(&subj_base64, subj ,strlen(subj) );
	
	//print2log("body %s to %s", body, to);
	sprintf(sbuf, MAIL_SEND_DATA, MA_FROM, to, subj_base64.data, body);
	
	
	buffer_delete(&subj_base64);
	
	if(send(ssock, sbuf, strlen(sbuf), 0) != (int)strlen(sbuf))
		return 0;
	rc = recv(ssock, rbuf, 9999, 0);
	
	sleep(1);
	shutdown(ssock, 2);
	return 1;
}

#else

int wcSendMail(char *to, char *subj, char *body)
{
    
	struct buffer_st subj_base64;
	base64_encode(&subj_base64, subj, strlen(subj));
	
	FILE* MA_SEND;
	MA_SEND=popen(MA_SENDER, FILE_ACCESS_MODES_RW);
	fprintf(MA_SEND, MAIL_SEND_DATA, MA_FROM, to, subj_base64.data, body);
	pclose(MA_SEND);
 	
	buffer_delete(&subj_base64);
	
	return 1;
}


#endif
