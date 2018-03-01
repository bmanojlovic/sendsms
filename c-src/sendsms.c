/*!
 * sendsms
 * https://github.com/bmanojlovic/sendsms/
 *
 * Copyright (c) 2015 Boris Manojlovic
 * http://steki.net
 *
 * Licensed under http://opensource.org/licenses/MIT
 */

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
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #include <windows.h>
 #define O_BLOCK 0
 #define O_NONBLOCK FIONBIO
 #define F_GETFL 3
 #define F_SETFL 4
#endif

void usage (char *msg)
{
    printf
        ("Usage:\n %s -s <SERVER NAME OR ADDRESS> -p <PORT NUMBER> -b <GSM NUMBER TO> -t \"<TEXT TO BE SENT>\" -c [sending check]\n",
         msg);
}


char *replace (char *string, char *oldpiece, char *newpiece)
{

    int str_index, newstr_index, oldpiece_index, end, new_len, old_len,
        cpy_len;

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
    while (str_index <= end && c != NULL) {
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

int sendsms (char *host, char *gsmnumber, char *gsmtext, int checksent,
             char *port)
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

    int sd, s, flags = 0, i;


    struct addrinfo hints;

    struct addrinfo *result, *rp;

    smsid = (char *) malloc (50);
    memset (smsid, 0x0, 14);

    memset (&hints, 0x0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    s = getaddrinfo (host, port, &hints, &result);
    if (s != 0) {
        printf ("SMSTATUS: GETADDRINFO FAILED - %s\n", gai_strerror (s));
        return -1;
    }

    for (i = 0; i < 10; i++) {
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sd == -1)
                continue;
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
            if (connect (sd, rp->ai_addr, rp->ai_addrlen) != -1)
                break;
            close (sd);
        }
        if (rp == NULL) {
            printf ("SMSSTATUS: FAILED CONNECT TO SERVER\n");
            return -1;
        }

        buffer = (char *) malloc (1023);
        memset (buffer, 0x0, 1023);
        if (!(strncmp (smsid, "send_", 5) == 0)) {
            if (checksent == 1) {
                urltemplate = (char *) malloc (strlen (urlconfirm));
                memset (urltemplate, 0x0, strlen (urlconfirm));
                strcpy (urltemplate, urlconfirm);
            }
            else {
                urltemplate = (char *) malloc (strlen (urlnoconfirm));
                memset (urltemplate, 0x0, strlen (urlnoconfirm));
                strcpy (urltemplate, urlnoconfirm);
            }
            smsreq = (char *) malloc (strlen (urltemplate) + 1023);
            memset (smsreq, 0x0, strlen (urltemplate) + 1023);
            sprintf (smsreq, urltemplate, gsmnumber,
                     replace (gsmtext, " ", "+"), host);
        }
        else {
            urltemplate = (char *) malloc (strlen (urlcontinuecheck));
            memset (urltemplate, 0x0, strlen (urlcontinuecheck));
            strcpy (urltemplate, urlcontinuecheck);
            smsreq = (char *) malloc (strlen (urltemplate) + 1023);
            memset (smsreq, 0x0, strlen (urltemplate) + 1023);

            sprintf (smsreq, urltemplate, smsid, host);
        }
        if (send (sd, smsreq, strlen (smsreq), 0) != strlen (smsreq)) {
            printf ("SMSSTATUS: FAILED in send()\n");
            return -1;
        }
        out = (char *) malloc (1024);
        memset (out, 0x0, 1024);
        recv (sd, out, 1024, 0);
        SHUTDOWN (sd);          /* close socket... */
        smsstatus = strstr (out, "SMSSTATUS: ");
        if (strstr (out, "HTTP/1.1 200") == NULL) {
            printf ("SMSSTATUS: FAILED SERVER NOT CONFIGURED\n");
            return (-1);
        }
        char *tmp;

        if ((tmp = (char *) strchr (smsstatus, '#')) != NULL) {
            smsid = strdup (tmp + 1);
        }
        free (buffer);
        free (urltemplate);
        free (smsreq);
        free (out);
        if (!checksent)
            return 0;
        if (strstr (smsstatus, "SENT")) {
            printf ("%s\n", smsstatus);
            return 0;
        }
        if (strstr (smsstatus, "FAILED")) {
            printf ("%s\n", smsstatus);
            return -1;
        }
        sleep (5);
    }

    printf ("SMSSTATUS: POSSIBLE FAIL (TIMEOUT)\n");
    return 1;
}


int main (int argc, char *argv[])
{
    int err = 0;

    int retval, c, checksent = 0;

    char *serveraddr, *port, *gsmnumber, *gsmtext, *errstr;


    serveraddr = (char *) malloc (40);
    memset (serveraddr, 0x0, 40);
    gsmnumber = (char *) malloc (40);
    memset (gsmnumber, 0x0, 40);
    gsmtext = (char *) malloc (40);
    memset (gsmtext, 0x0, 40);
    errstr = (char *) malloc (1000);
    memset (errstr, 0x0, 1000);
    if (argc < 4) {
        usage (argv[0]);
        return -1;
    }

    while ((c = getopt (argc, argv, "chs:b:t:p:")) != -1)
        switch (c) {
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
            port = optarg;
            break;
        case '?':
            if (optopt == 's' || optopt == 'b' || optopt == 't')
                fprintf (stderr, "Option -%c requires an argument.\n",
                         optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        case 'h':
            usage (argv[0]);
            return 0;
            break;
        default:
            return -1;
        }

    if (strlen (serveraddr) == 0) {
        strcat (errstr, "Server address/name must be specified\n");
        err = 1;
    }
    if (strlen (gsmnumber) == 0) {
        strcat (errstr, "Number to send must be specified\n");
        err = 1;
    }
    if (strlen (gsmtext) == 0) {
        strcat (errstr, "Text to send must be specified\n");
        err = 1;
    }

    if (err == 1) {
        printf ("Errors detected:\n%s\nType \"%s -h\" for help\n", errstr,
                argv[0]);
        return -1;
    }

    if (checksent == 1) {
        /* printf ("Checking if sent....(timeout ~ 15 sec)\n"); */
    }
#ifdef MINGW32
    WSADATA wsaData;

    int iResult = WSAStartup (MAKEWORD (2, 2), &wsaData);

    if (iResult != NO_ERROR)
        printf ("SMSSTATUS: Error at WSAStartup()\n");
#endif
    retval = sendsms (serveraddr, gsmnumber, gsmtext, checksent, port);
#ifdef MINGW32
    WSACleanup ();
#endif
    return retval;
}
