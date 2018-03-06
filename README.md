# sendsms readme/faq

## what is this?

 Little program for sending sms from command line trough sms-tools server implementation
 program can be compiled for unix and windows operating systems (works on AIX, Linux, and probably any other semi decent posix compliant unixoid)

## Why

 Needed to send sms from different system in local network without need for
 multiple gsm modems per machine :)

## usage

    bmanojlovic@pc:~> sendsms --help
    sending sms from command line trough smstools3 server implementation

    Usage:
      sendsms [flags]

    Flags:
      -c, --checksentstatus         verbose output
      -h, --help                    help for sendsms
      -t, --messagetext string      
      -p, --port string             Port Number (default "80")
      -b, --receivernumber string   Receiver number
      -s, --servername string       Server hosting smstools

## Example

    sendsms -s 10.11.10.20 -p 80 -b 11111222333 -t "orao pao" -c       // this way if you need confirmation from modem (not network!!!)
    sendsms -s 10.11.10.20 -p 80 -b 11111222333 -t "orao pao"          // no confirmation

## Program output

    SMSSTATUS: FAILED                    // if somehting is wrong it will be printed on stdout
    SMSSTATUS: SENT                      // message is sent (from gsm modem perspective)
    SMSSTATUS: SENT (NO CHECK)!          // no checks on sending performed ( param -c is not used)
    SMSSTATUS: POSSIBLE FAIL (TIMEOUT)   // Loop (sleep(5) x 10 ) didn't had any good response it could be ok but who knows...

## Security

 basic idea is that smstools has /etc/smsd.white whitelisting support
 meaning only authorized users would be able to receive messages
 System logs from which IP address message was sent.

## Web Server setup:

 Put php/sendsms.php into server document root so it can be reached from sendsms binary with /sendsms.php url

## Sanity check if web server part is configured correctly is:

    $ curl -I http://10.10.10.10/sendsms.php
    HTTP/1.1 200 OK
    Date: Tue, 24 Mar 2015 08:33:55 GMT
    Server: Apache (Linux/SUSE)
    X-Powered-By: PHP
    Content-Type: text/html
