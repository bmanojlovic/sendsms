#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef _AIX
 #include <getopt.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#ifndef MINGW32
 #include <netdb.h>
 #include <sys/socket.h>
 #include <sys/select.h>
#else
 #include <windows.h>
 #define O_BLOCK 0
 #define O_NONBLOCK FIONBIO
 #define F_GETFL 3
 #define F_SETFL 4
#endif

void
usage (char *msg)
{
  printf
    ("Usage:\n %s -s <SERVER IP ADDRESS> -p <PORT NUMBER> -b <GSM NUMBER TO> -t \"<TEXT TO BE SENT>\" -c [sending check]\n",
     msg);
}


char *
replace (char *string, char *oldpiece, char *newpiece)
{

  int str_index, newstr_index, oldpiece_index, end, new_len, old_len, cpy_len;
  char *c;
  static char newstring[2000000];

  if ((c = (char *) strstr (string, oldpiece)) == NULL)

    return string;

  new_len = strlen (newpiece);
  old_len = strlen (oldpiece);
  end = strlen (string) - old_len;
  oldpiece_index = c - string;


  newstr_index = 0;
  str_index = 0;
  while (str_index <= end && c != NULL)
    {
      cpy_len = oldpiece_index - str_index;
      strncpy (newstring + newstr_index, string + str_index, cpy_len);
      newstr_index += cpy_len;
      str_index += cpy_len;
      strcpy (newstring + newstr_index, newpiece);
      newstr_index += new_len;
      str_index += old_len;
      if ((c = (char *) strstr (string + str_index, oldpiece)) != NULL)
        oldpiece_index = c - string;
    }
  strcpy (newstring + newstr_index, string + str_index);

  return newstring;
}

int
sendsms (struct hostent *host, char *gsmnumber, char *gsmtext, int checksent,
         int port)
{
  const char urlconfirm[] =
    "GET /sendsms.php?sent=cli&to=%s&fromform=true&confirmation=on&text=%s HTTP/1.1\r\n"
    "Host:%s\r\n" "Connection: close\r\n\r\n";
  const char urlnoconfirm[] =
    "GET /sendsms.php?sent=cli&to=%s&fromform=true&text=%s HTTP/1.1\r\n"
    "Host:%s\r\n" "Connection: close\r\n\r\n";
  const char urlcontinuecheck[] =
    "GET /sendsms.php?sent=cli&confirmation=on&msgid=%s HTTP/1.1\r\n"
    "Host:%s\r\n" "Connection: close\r\n\r\n";
  char *buffer, *smsreq, *urltemplate, *out, *smsid = NULL, *smsstatus;
  int sd, flags = 0, i;


  struct sockaddr_in addr;


  memset (&addr, 0x0, sizeof (addr));
  smsid = (char *) malloc (50);
  memset (smsid, 0x0, 14);
  for (i = 0; i < 10; i++)
    {
      //printf("smsid=%s\n",smsid);
      sd = socket (PF_INET, SOCK_STREAM, 0);
      if (sd == -1)
        {
          fprintf (stderr, "Cannot open socket\n");
          exit (1);
        }
#ifndef MINGW32
#define SHUTDOWN(fd)    { shutdown((fd),0); close((fd)); }
      flags = fcntl (sd, F_GETFL, 0);
#else
#define fcntl(fd,b,c) { u_long arg=1;ioctlsocket(fd, c, &arg); }
#define SHUTDOWN(fd) { shutdown((fd),0); closesocket((fd)); }
#define sleep(x) Sleep((x)*1000)
      flags = 0;
#endif

      fcntl (sd, F_SETFL, flags);
      addr.sin_family = AF_INET;
      addr.sin_port = htons (port);
      addr.sin_addr.s_addr = *((long *) host->h_addr_list[0]);
      connect (sd, (struct sockaddr *) &addr, sizeof (struct sockaddr));
      buffer = (char *) malloc (1023);
      memset (buffer, 0x0, 1023);
      if (!(strncmp (smsid, "send_", 5) == 0))
        {
          if (checksent == 1)
            {
              urltemplate = (char *) malloc (strlen (urlconfirm));
              memset (urltemplate, 0x0, strlen (urlconfirm));
              strcpy (urltemplate, urlconfirm);
            }
          else
            {
              urltemplate = (char *) malloc (strlen (urlnoconfirm));
              memset (urltemplate, 0x0, strlen (urlnoconfirm));
              strcpy (urltemplate, urlnoconfirm);
            }
          smsreq = (char *) malloc (strlen (urltemplate) + 1023);
          memset (smsreq, 0x0, strlen (urltemplate) + 1023);
          sprintf (smsreq, urltemplate, gsmnumber,
                   replace (gsmtext, " ", "+"), host);
        }
      else
        {
          urltemplate = (char *) malloc (strlen (urlcontinuecheck));
          memset (urltemplate, 0x0, strlen (urlcontinuecheck));
          strcpy (urltemplate, urlcontinuecheck);
          smsreq = (char *) malloc (strlen (urltemplate) + 1023);
          memset (smsreq, 0x0, strlen (urltemplate) + 1023);

          sprintf (smsreq, urltemplate, smsid, host);
        }
      if (send (sd, smsreq, strlen (smsreq), 0) != strlen (smsreq))
        {
          printf ("SMSSTATUS: FAILED in send()\n");
          exit (-1);
        }
      out = (char *) malloc (1024);
      memset (out, 0x0, 1024);
      recv (sd, out, 1024, 0);
      SHUTDOWN (sd);            // close socket...
      smsstatus = strstr (out, "SMSSTATUS: ");
      if (strstr (out, "HTTP/1.1 200") == NULL)
        {
          printf ("SMSSTATUS: FAILED SERVER NOT CONFIGURED\n");
          exit (-1);
        }
      char *tmp;
      if ((tmp = (char *) strchr (smsstatus, '#')) != NULL)
        {
          smsid = strdup (tmp + 1);
        }
      free (buffer);
      free (urltemplate);
      free (smsreq);
      free (out);
      if (!checksent)
        exit (0);
      if (strstr (smsstatus, "SENT"))
        {
          printf ("%s\n", smsstatus);
          exit (0);
        }
      if (strstr (smsstatus, "FAILED"))
        {
          printf ("%s\n", smsstatus);
          exit (-1);
        }
      sleep (5);
    }
  printf ("SMSSTATUS: POSSIBLE FAIL (TIMEOUT)\n");
  return 1;
}


int
main (int argc, char *argv[])
{
  int err = 0;
  int c, port, checksent = 0;
  char *serveraddr, *gsmnumber, *gsmtext, *errstr;
  struct hostent *host;

  serveraddr = (char *) malloc (40);
  memset (serveraddr, 0x0, 40);
  gsmnumber = (char *) malloc (40);
  memset (gsmnumber, 0x0, 40);
  gsmtext = (char *) malloc (40);
  memset (gsmtext, 0x0, 40);
  errstr = (char *) malloc (1000);
  memset (errstr, 0x0, 1000);
  if (argc < 4)
    {
      usage (argv[0]);
      exit (-1);
    }

  while ((c = getopt (argc, argv, "chs:b:t:p:")) != -1)
    switch (c)
      {
      case 'c':
        checksent = 1;
        break;
      case 's':
        serveraddr = optarg;
        break;
      case 'b':
        gsmnumber = optarg;
        break;
      case 't':
        gsmtext = optarg;
        break;
      case 'p':
        port = atoi (optarg);
        break;
      case '?':
        if (optopt == 's' || optopt == 'b' || optopt == 't')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;
      case 'h':
        usage (argv[0]);
        exit (0);
        break;
      default:
        abort ();
      }

  if (strlen (serveraddr) == 0)
    {
      strcat (errstr, "Server address must be specified\n");
      err = 1;
    }
  if (strlen (gsmnumber) == 0)
    {
      strcat (errstr, "Number to send must be specified\n");
      err = 1;
    }
  if (strlen (gsmtext) == 0)
    {
      strcat (errstr, "Text to send must be specified\n");
      err = 1;
    }

  if (err == 1)
    {
      printf ("Errors detected:\n%s\nType \"%s -h\" for help\n", errstr,
              argv[0]);
      exit(-1);
    }

  if (checksent == 1)
    {
      //printf ("Checking if sent....(timeout ~ 15 sec)\n");
    }
#ifdef MINGW32
  WSADATA wsaData;
  int iResult = WSAStartup (MAKEWORD (2, 2), &wsaData);
  if (iResult != NO_ERROR)
    printf ("Error at WSAStartup()\n");
#endif
  host = gethostbyname (serveraddr);
  if (h_errno == HOST_NOT_FOUND)
    {
      fprintf (stderr, "Server host not found\n");
      exit (-1);
    }


  sendsms (host, gsmnumber, gsmtext, checksent, port);

#ifdef MINGW32
  WSACleanup ();
#endif
  return 0;
}
