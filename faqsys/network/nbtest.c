/*
 * minimal NetBIOS test program
 * Copyright (C) 1987 Micro-Matic Research
 */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "nbtest.h"

#define FALSE           0
#define TRUE            1

#define SENDNAME        "NBTEST-S"
#define RECEIVENAME     "NBTEST-R"
#define MAXBLKSIZE      16384
#define MAXNCBS         20

NCB ncb;
char sbuffer[MAXBLKSIZE+1];
char rbuffer[MAXBLKSIZE+1];

int nrblocks = 200;
int blksize = 1024;
int bflag = 0;
int iflag = FALSE;
int vflag = FALSE;
int wflag = FALSE;

/* Ascii error code table */

char *nb_err_tab[] =
    {
    "GOOD RETURN",
    "ILLEGAL BUFFER LENGTH",
    "UNDEFINED",
    "INVALID COMMAND CODE",
    "UNDEFINED",
    "COMMAND TIMED OUT",
    "MESSAGE INCOMPLETE (NOT ERR)",
    "UNDEFINED",
    "ILLEGAL LOCAL SESSION NUMBER",
    "NO RESOURCE AVAILABLE",
    "SESSION CLOSED",
    "COMMAND CANCELLED",
    "UNDEFINED",
    "DUPLICATE NAME IN LOCAL NAME TABLE",
    "NAME TABLE FULL",
    "NAME HAS ACTIVE SESSION",
    "UNDEFINED",
    "LOCAL SESSION TABLE FULL",
    "SESSION OPEN REJECTED",
    "ILLEGAL NAMES NUMBER",
    "COULDN'T FIND CALLED NAME",
    "NAME NOT FOUND OR INVALID",
    "NAME IN USE",
    "NAME DELETED",
    "SESSION ENDED ABNORMALLY",
    "NAME CONFLICT DETECTED",
    "INCOMPATIBLE REMOTE DEVICE",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "INTERFACE BUSY",
    "TOO MANY COMMANDS OUTSTANDING",
    "INVALID LANA_NUM VALUE",
    "COMMAND NOT CANCELLED",
    "RESERVED NAME SPECIFIED",
    "COMMAND NOT VALID TO CANCEL"
    };

void receive (void), send (void);
void err (int);
int nbexec (NCB *), nbsubmit (NCB *);

main (argc, argv)
int argc;
char *argv[];
    {
    int errcode;
    int i;

    for (i = 1; i < argc; i++)
        {
        if (*argv[i] != '-')
            goto usage;

        switch (*++argv[i])
            {
            case 'b':
                /* increment nr of simultaneous NCBs in background for TX/RX */
                if (++i >= argc)
                    goto usage;
                bflag = atoi (argv[i]);
                break;
            case 'n':
                /* specify number of blocks to send */
                if (++i >= argc)
                    goto usage;
                nrblocks = atoi (argv[i]);
                break;
            case 's':
                /* specify size of blocks to send */
                if (++i >= argc)
                    goto usage;
                blksize = atoi (argv[i]);
                break;
            case 'i':
                /* information about bytes transferred displayed */
                iflag = TRUE;
                break;
            case 'w':
                /* wait for key before sending */
                wflag = iflag = TRUE;
                break;
            case 'v':
                /* verify received data */
                vflag = TRUE;
                break;
            default:
                goto usage;
            }
        }

    if (nrblocks < 1 || nrblocks > 10000 ||
        blksize < 1 || blksize > MAXBLKSIZE ||
        bflag < 0 || bflag > MAXNCBS)
        goto usage;

    /* prime send buffer with cyclic bytes 0..255 */
    for (i = 0; i < blksize; i++)
        sbuffer[i] = i;

    printf ("\nNo-Wait option: %s\n", bflag ? "ON" : "OFF");
    if (bflag)
        printf ("%d NCB%s issued simultaneously\n", bflag,
                (bflag == 1) ? " is" : "s are");

    /* try to act as the receiver first */

    printf ("\nTrying to register name '%s'\n", RECEIVENAME);
    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_ADDNAME;
    strncpy (ncb.ncb_name, RECEIVENAME, 16);

    if ((errcode = nbexec (&ncb)) == 0)
        {
        receive();
        exit (0);
        }

    if (errcode == 0x16)
        printf ("--> NAME IN USE\n");
    else
        err (errcode);

    /* if we cannot be receiver we'll have to be sender */

    printf ("\nTrying to register name '%s'\n", SENDNAME);
    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_ADDNAME;
    strncpy (ncb.ncb_name, SENDNAME, 16);

    if ((errcode = nbexec (&ncb)) == 0)
        {
        send();
        exit (0);
        }

    err (errcode);
    exit (0);

usage:
    printf ("Usage:\n");
    printf ("nbtest [-i] [-b oc] [-n nr_blocks] [-s size_blocks]\n");
    printf ("    -i         interactive; updates bytes exchanged\n");
    printf ("    -w         wait for key before sending each packet\n");
    printf ("    -v         verify received data length and data\n");
    printf ("    -b nnn     number of commands sent in NO-WAIT mode (0..%d)\n",
            MAXNCBS);
    printf ("    -n nnn     number of blocks exchanged (1..10000)\n");
    printf ("    -s nnn     size of blocks (1..16384)\n",
            MAXBLKSIZE);
    printf ("Default: -b 0 -n 200 -s 1024\n");
    exit (1);
    }


void send (void)
    {
    int errcode;
    long starttime, endtime;
    unsigned long byte = 0;
    unsigned long kbyte = 0;
    int etime;
    int i;
    BYTE lsn;

call:
    printf ("\nCalling '%s'\n", RECEIVENAME);
    fflush (stdout);

    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_CALL;
    strncpy (ncb.ncb_name,     SENDNAME,    16);
    strncpy (ncb.ncb_callname, RECEIVENAME, 16);
    ncb.ncb_rto = 20;
    ncb.ncb_sto = 20;

    if ((errcode = nbexec (&ncb)) != 0)
        {
        err (errcode);
        goto deregister;
        }

    printf ("-> Session established with '%s' as #%d\n",
            RECEIVENAME, ncb.ncb_lsn);
    fflush (stdout);

    lsn = ncb.ncb_lsn;

transfer:
    printf ("\nStarting transfer of %d blocks of %d bytes\n",
            nrblocks, blksize);
    printf ("-> KB transferred:       ");
    fflush (stdout);

    time (&starttime);


    for (i = 0; i < nrblocks; i += (bflag ? bflag : 1))
        {
        if (wflag)
            getch ();

        memset (&ncb, 0, sizeof (ncb));

        ncb.ncb_command = N_SEND;
        ncb.ncb_lsn = lsn;
        ncb.ncb_buffer = (BYTE _far *) sbuffer;
        ncb.ncb_length = blksize;

        if ((errcode = nbsubmit (&ncb)) != 0)
            break;

        byte += bflag ? (long)blksize*bflag : blksize;

        if (byte > kbyte * 1024L)
            {
            while (byte > kbyte * 1024L)
                kbyte++;

            if (iflag)
                {
                printf ("\b\b\b\b\b\b%-6ld", kbyte);
                fflush (stdout);
                }
            }
        }

    printf ("\b\b\b\b\b\b%-6ld\n", kbyte);
    fflush (stdout);

    time (&endtime);
    etime = endtime - starttime;

    if (errcode != 0)
        err (errcode);

    printf ("-> Data transfer completed.\n");
    printf ("-> Time elapsed: %d seconds\n", etime);
    printf ("-> Blocks transferred: %d\n", i);

    if (etime >= 3 && kbyte > 0 && (kbyte / etime) > 0)
        printf ("-> Transfer rate: %d KB/sec\n", (int)(kbyte / etime));
    else
        printf ("-> Transfer rate not determined\n");

hangup:
    printf ("\nHanging up\n");
    fflush (stdout);

    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_HANGUP;
    ncb.ncb_lsn = lsn;
    if ((errcode = nbexec (&ncb)) != 0 && errcode != E_SES_CLOSE)
        {
        err (errcode);
        goto deregister;
        }

    printf ("-> Session with '%s' closed\n", RECEIVENAME);

deregister:
    printf ("\nDeregistering name '%s'\n", SENDNAME);
    fflush (stdout);

    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_DELETENAME;
    strncpy (ncb.ncb_name, SENDNAME, 16);
    if ((errcode = nbexec (&ncb)) != 0)
        {
        err (errcode);
        return;
        }

    printf ("-> Name deregistered\n");
    return;
    }


void receive (void)
    {
    int errcode;
    long starttime, endtime;
    unsigned long kbyte = 0;
    unsigned long byte = 0;
    int etime;
    int i;
    BYTE lsn;

listen:
    printf ("\nWaiting for a call from '%s'\n", SENDNAME);
    fflush (stdout);

    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_LISTEN;
    strncpy (ncb.ncb_name,     RECEIVENAME, 16);
    strncpy (ncb.ncb_callname, SENDNAME,    16);
    ncb.ncb_rto = 20;
    ncb.ncb_sto = 20;

    if ((errcode = nbexec (&ncb)) != 0)
        {
        err (errcode);
        goto deregister;
        }

    printf ("-> Session established with '%s' as #%d\n",
            SENDNAME, ncb.ncb_lsn);
    fflush (stdout);

    lsn = ncb.ncb_lsn;

transfer:
    printf ("\nReception of %d blocks of %d bytes in progress\n",
            nrblocks, blksize);
    printf ("-> KB transferred:       ");
    fflush (stdout);

    time (&starttime);


    for (i = 0; i < nrblocks; i += (bflag ? bflag : 1))
        {
        memset (&ncb, 0, sizeof (ncb));

        ncb.ncb_command = N_RECEIVE;
        ncb.ncb_lsn = lsn;
        ncb.ncb_buffer = (BYTE _far *) rbuffer;
        ncb.ncb_length = blksize;

        if ((errcode = nbsubmit (&ncb)) != 0)
            break;

        if (vflag)
            {
            if (ncb.ncb_length != blksize)
                {
                printf ("\n-> Received length mismatch; expected=%d actual=%d\n",
                    blksize, ncb.ncb_length);
                fflush (stdout);
                break;
                }
            if (memcmp (sbuffer, rbuffer, blksize) != 0)
                {
                printf ("\n-> Received data miscompare\n");
                fflush (stdout);
                break;
                }
            }

        byte += bflag ? (long)blksize*bflag : blksize;

        if (byte > kbyte * 1024L)
            {
            while (byte > kbyte * 1024L)
                kbyte++;

            if (iflag)
                {
                printf ("\b\b\b\b\b\b%-6ld", kbyte);
                fflush (stdout);
                }
            }
        }

    printf ("\b\b\b\b\b\b%-6ld\n", kbyte);
    fflush (stdout);

    time (&endtime);
    etime = endtime - starttime;

    if (errcode != 0)
        err (errcode);

    printf ("-> Data transfer completed.\n");
    printf ("-> Time elapsed: %d seconds\n", etime);
    printf ("-> Blocks transferred: %d\n", i);

    if (etime >= 3 && kbyte > 0 && (kbyte / etime) > 0)
        printf ("-> Transfer rate: %d KB/sec\n", (int)(kbyte / etime));
    else
        printf ("-> Transfer rate not determined\n");

hangup:
    printf ("\nHanging up\n");
    fflush (stdout);

    memset (&ncb, 0, sizeof (ncb));
    ncb.ncb_command = N_HANGUP;
    ncb.ncb_lsn = lsn;
    if ((errcode = nbexec (&ncb)) != 0 && errcode != E_SES_CLOSE)
        {
        err (errcode);
        goto deregister;
        }

    printf ("-> Session with '%s' closed\n", RECEIVENAME);

deregister:
    printf ("\nDeregistering name '%s'\n", RECEIVENAME);
    fflush (stdout);

    ncb.ncb_command = N_DELETENAME;
    strncpy (ncb.ncb_name, RECEIVENAME, 16);
    if ((errcode = nbexec (&ncb)) != 0)
        {
        err (errcode);
        return;
        }

    printf ("-> Name deregistered\n");

    return;
    }


void err (code)
int code;
    {
    printf ("-> ERROR: code = 0x%x", code);
    if (code < (sizeof (nb_err_tab) / sizeof (*nb_err_tab)))
        printf (" (%s)", nb_err_tab[code]);
    printf ("\n");
    }

static volatile int bg_nberr;
static volatile int bg_num;
static NCB bg_ncbs[MAXNCBS+1];

#pragma check_stack-

void _interrupt _far nbpost (es, ds, di, si, bp, sp, bx)
unsigned short es, ds, di, si, bp, sp, bx;
    {
    NCB _far *pncb;

    FP_OFF (pncb) = bx;
    FP_SEG (pncb) = es;

    --bg_num;
    bg_nberr = pncb->ncb_cmd_cplt;
    }

#pragma check_stack+

int nbexec (NCB *pncb)
    {
    union REGS regs;
    struct SREGS sregs;
    char far *fptr = (char far *) pncb;

    regs.x.bx = FP_OFF (fptr);
    sregs.es = FP_SEG (fptr);
    int86x (0x5C, &regs, &regs, &sregs);
    return (pncb->ncb_retcode);
    }

/* execute the ncb in foreground or background */

int nbsubmit (pncb)
NCB *pncb;
    {
    int errcode;
    int i;

    /* execute directly if wait mode */
    if (bflag == 0)
        {
        pncb->ncb_post = NULL;
        return (nbexec (pncb));
        }

    bg_nberr = -1;
    bg_num = bflag;

    pncb->ncb_command |= N_NOWAIT;
    pncb->ncb_post = nbpost;

    /* issue bflag copies of the NCB in nowait mode */
    for (i = 0; i < bflag; ++i)
        {
        bg_ncbs[i] = *pncb;
        if ((errcode = nbexec (&bg_ncbs[i])) != 0 && errcode != 0xFF)
            return (errcode);
        }

    /* wait till all completed */
    while (bg_num > 0)
        if (bg_nberr != -1 && bg_nberr != 0)
            return (bg_nberr);

    return (0);
    }


//*********************** NBTEST.H ****************************

/* C include file for NetBios applications */

/****************************************************************************
 *                              CONSTANTS                                   *
 ****************************************************************************/

/* Values for ncb_command */

#define N_RESET                 0x32    /* RESET LOCAL ATTACHMENT       */
#define N_STATUS                0x33    /* RECEIVE STATUS OF SESSIONS   */
#define N_CANCEL                0x35    /* CANCEL INDICATED REQUEST     */
#define N_ADDNAME               0x30    /* ADD UNIQUE NAME              */
#define N_ADDGROUPNAME          0x36    /* ADD NON UNIQUE NAME          */
#define N_DELETENAME            0x31    /* DELETE NAME                  */
#define N_CALL                  0x10    /* OPEN A SESSION               */
#define N_LISTEN                0x11    /* LISTEN FOR A CALL            */
#define N_HANGUP                0x12    /* END A SESSION                */
#define N_SEND                  0x14    /* SEND                         */
#define N_SENDMULTIPLE          0x17    /* SEND                         */
#define N_RECEIVE               0x15    /* RECEIVE                      */
#define N_RECEIVEANY            0x16    /* RECEIVE FROM ANY             */
#define N_SESSIONSTATUS         0x34    /* SESSION STATUS               */
#define N_SENDDATAGRAM          0x20    /* SEND A DATAGRAM              */
#define N_RECEIVEDATAGRAM       0x21    /* RECEIVE A DATAGRAM           */
#define N_SENDBROADCAST         0x22    /* SEND A BROADCAST DATAGRAM    */
#define N_RECEIVEBROADCAST      0x23    /* RECEIVE A BROADCAST DATAGRAM */

#define N_NOWAIT                0x80    /* NO-WAIT FLAG                 */

/* Values for ncb_retcode */

#define E_GOOD_RET              0x00    /* GOOD RETURN                  */
#define E_BAD_BUFLEN            0x01    /* ILLEGAL BUFFER LENGTH        */
#define E_BAD_CMD               0x03    /* ILLEGAL COMMAND              */
#define E_TIME_OUT              0x05    /* COMMAND TIMED OUT            */
#define E_BUF_FULL              0x06    /* MESSAGE INCOMPLETE (NOT ERR  */
#define E_SESNUM_ERR            0x08    /* SESSION NUMBER OUT OF RANGE  */
#define E_NO_RESOURCE           0x09    /* NO RESOURCE AVAILABLE        */
#define E_SES_CLOSE             0x0A    /* SESSION CLOSED               */
#define E_CMD_ABORT             0x0B    /* COMMAND CANCELLED            */
#define E_DUP_NAME              0x0D    /* DUPLICATE NAME               */
#define E_TBL_FULL              0x0E    /* NAME TABLE FULL              */
#define E_NO_DEL                0x0F    /* NAME HAS ACTIVE SESSION      */
#define E_SESTBL_FUL            0x11    /* LOCAL SESSION TABLE FULL     */
#define E_ALNUM_ERR             0x13    /* ILLEGAL NAMES NUMBER         */
#define E_NOFND_CALLNAME        0x14    /* COULDN'T FIND CALLED NAME    */
#define E_BAD_NAME              0x15    /* CAN'T PUT * IN NCB_NAME      */
#define E_NAME_USED             0x16    /* NAME IN USE                  */
#define E_EQL_NAMES             0x17    /* NAME DELETED                 */
#define E_SES_BAD               0x18    /* SESSION ENDED ABNORMALLY     */
#define E_MUL_NAME              0x19    /* NAME CONFLICT DETECTED       */
#define E_INCOM_REM             0x1A    /* INCOMPATIBLE REMOTE DEVICE   */
#define E_LANA_LOCKED           0x21    /* INTERFACE BUSY               */
#define E_MAX_CMD               0x22    /* TOO MANY COMMANDS            */
#define E_LANA_NUM              0x23    /* INVALID LANA_NUM VALUE       */
#define E_CMD_CNL               0x24    /* COMMAND CMPLTD WHILE CANCEL  */
#define E_NO_CNL                0x26    /* COMMAND NO VALID TO CANCEL   */
#define E_YS_ERR                0x40    /* SYSTEM ERROR                 */
#define E_CON_CARRIER_NOT_ME    0x41    /* CONTINUOUS CARRIER (I did)   */
#define E_CON_CARRIER_ME        0x42    /* CONTINUOUS CARRIER (I didn't)*/
#define E_NO_CARRIER            0x43    /* NO CARRIER DETECTED          */

/****************************************************************************
 *                          TYPE DEFINITIONS                                *
 ****************************************************************************/

/* define BYTE/WORD/DWORD as unsigned char/int/long */
typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;

/* NCB structure */
typedef struct ncb_tag
    {
    BYTE        ncb_command;        /* ncb COMMAND FIELD */
    BYTE        ncb_retcode;        /* ncb RETURN CODE */
    BYTE        ncb_lsn;            /* ncb LOCAL SESSION NUMBER */
    BYTE        ncb_num;            /* ncb ALIAS NUMBER */
    BYTE _far * ncb_buffer;         /* ncb MESSAGE BUFFER */
    WORD        ncb_length;         /* ncb LENGTH (IN BYTES) */
    BYTE        ncb_callname[16];   /* ncb NAME ON REMOTE ATTACHMENT */
    BYTE        ncb_name[16];       /* ncb ALIAS NAME */
    BYTE        ncb_rto;            /* ncb RECEIVE TIMEOUT */
    BYTE        ncb_sto;            /* ncb SEND TIMEOUT */
    void        (_interrupt _far *ncb_post)();
                                    /* ncb POINTER TO POST ROUTINE */
    BYTE        ncb_lana_num;       /* ncb ADAPTER NR (0 or 1) */
    BYTE        ncb_cmd_cplt;       /* ncb COMMAND PENDING INDICATION */
    BYTE        ncb_reserved[14];   /* ncb RESERVED AREA */
    } NCB;

/* name table entry */
typedef struct name_entry_tag
    {
    BYTE    name[16];           /* name */
    BYTE    name_nr;            /* # name */
    BYTE    name_stat;          /* name status */
    } NAME_ENTRY;

/* adapter status structure */
typedef struct adapter_stat
    {
    BYTE    stat_uin[6];        /* unit identification number */
    BYTE    stat_jumper;        /* external jumper */
    BYTE    stat_selftest;      /* results of last self-test */
    BYTE    stat_version[2];    /* software version high/low */
    WORD    stat_period;        /* duration of reporting period in minutes */
    WORD    stat_crc;           /* # CRC errors */
    WORD    stat_align;         /* # alignment errors */
    WORD    stat_collision;     /* # collisions */
    WORD    stat_abort;         /* # aborted transmissions */
    DWORD   stat_txpacket;      /* # ok transmitted packets */
    DWORD   stat_rxpacket;      /* # ok received packets */
    WORD    stat_retx;          /* # retransmissions */
    WORD    stat_rxexhaust;     /* # receive exhausted */
    BYTE    stat_rsv1[8];
    WORD    stat_freencb;       /* # free command buffers */
    WORD    stat_cfgncb;        /* max # NCB's */
    WORD    stat_maxncb;        /* max # command buffers */
    BYTE    stat_rsv2[4];
    WORD    stat_pendsess;      /* # pending sessions */
    WORD    stat_cfgsess;       /* max pending sessions */
    WORD    stat_maxsess;       /* max # sessions */
    WORD    stat_psize;         /* maximum packet size */
    WORD    stat_nrnames;       /* # names in local name table */

    NAME_ENTRY
            stat_localname[16]; /* local name table */
    } ADAPTER_STATUS;

