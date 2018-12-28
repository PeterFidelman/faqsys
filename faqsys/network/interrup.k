--------N-7A---------------------------------
INT 7A - Novell NetWare - LOW-LEVEL API - Notes
Note:   this interrupt is used for IPX/SPX access in NetWare versions through
          2.0a; in later versions, you should use INT 2F/AX=7A00h to get an
          entry point even though INT 7A still exists.  For both INT 7A and
          the FAR entry point, BX contains the function number; IPX is
          sometimes called internally with BX bit 15 set, which causes the
          handler to bypass some initial checks and an optional call to the
          IPX Windows support handler set with INT 2F/AX=7AFFh/BX=0000h
          (see #1514)
SeeAlso: INT 2F/AX=7A00h,INT 64"Novell",INT 7A/BX=0000h
--------N-7A----BX0000-----------------------
INT 7A - Novell NetWare - IPX Driver - OPEN SOCKET
        BX = 0000h
        AL = socket longevity
            00h open until close or terminate
            FFh open until close
        DX = socket number (high byte in DL)
            0000h dynamic allocation
            else  socket to open (see #2212)
Return: AL = return code
            00h success
                DX = socket number
            FEh socket table full
            FFh socket already open
Notes:  TSRs which need to use sockets should set AL to FFh, non-resident
          programs should normally use AL=00h
        IPX can be configured to support up to 150 open sockets on a
          workstation, and defaults to 20
        this function is supported by Advanced NetWare 1.02+
SeeAlso: INT 7A/BX=0001h,INT 7A/BX=0004h,INT 7A/BX=0023h

(Table 2212)
Values for IPX socket number:
 0451h  File Service (NetWare Core Protocol)
 0452h  Service Advertising Protocol
 0453h  Routing Information Packet
 0455h  NetBIOS Packet
 0456h  diagnostics
 0457h  server serial numbers (labeled "Copy Protection" by Lanalyzer)
 4000h-7FFFh used for dynamic allocation
 4444h  Brightwork Development's SiteLock server
 5555h  Brightwork Development's SiteLock client (workstation)
 8000h-FFFFh assigned by Novell
Note:   SiteLock is an application metering product using IPX to communicate
          between the application and the license server
--------N-7A----BX0001-----------------------
INT 7A - Novell NetWare - IPX Driver - CLOSE SOCKET
        BX = 0001h
        DX = socket number (high byte in DL)
Notes:  also cancels events set by any Event Control Blocks for the socket
        the program must close all open sockets before terminating
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0000h
--------N-7A----BX0002-----------------------
INT 7A - Novell NetWare - IPX Driver - GET LOCAL TARGET
        BX = 0002h
        ES:SI -> target internetwork address (see INT 7A/BX=000Bh)
        ES:DI -> 6-byte buffer for local target
Return: AL = return code
            00h success
               CX = expected one-way transfer time (clock ticks) for a 576-byte
                      packet
               ES:DI -> local target
            FAh unsuccessful (no path to destination)
Notes:  the internetwork address consists of a 4-byte network address followed
          by a 6-byte node address.  The local target is only a 6-byte node
          address.  If the target is in the same network, the local target is
          just the node address of target; otherwise, the local target is the
          node address of the bridge that leads to the target.
        this function may be called from inside IPX and AES Event Service
          Routines, but not from other interrupt handlers
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0009h
--------N-7A----BX0003-----------------------
INT 7A - Novell NetWare - IPX Driver - SEND PACKET
        BX = 0003h
        ES:SI -> Event Control Block (see #2213,#2214)
Notes:  returns immediately; IPX attempts to send the packet in the background
        this function is supported by Advanced NetWare 1.02+
        this function is nearly identical to BX=000Fh, except that it always
          copies the source address into the IPX header assumed to be at the
          beginning of the first fragment
SeeAlso: BX=0004h,BX=000Fh,INT 21/AH=EEh"Novell"

Format of IPX Event Control Block:
Offset  Size    Description     (Table 2213)
 00h    DWORD   Link
 04h    DWORD   -> Event Service Routine (00000000h if none)
 08h    BYTE    in use flag (see #2215)
 09h    BYTE    completion code (see #2216)
 0Ah    WORD    (big-endian) socket number (see INT 7A/BX=0000h)
 0Ch  4 BYTEs   IPX workspace
 10h 12 BYTEs   driver workspace
 1Ch  6 BYTEs   immediate local node address
 22h    WORD    fragment count
 24h    var     fragment descriptors
                Offset  Size    Description
                 00h    DWORD   -> fragment data
                 04h    WORD    size of fragment in bytes.
Notes:  ESR is a far procedure that is called when the ECB has been handled.
          On call, the in use flag is zero if the ECB has been handled,
          non-zero otherwise. If the flag is zero, the completion code holds
          the result of the event.
        the first fragment should start with an IPX header
        all fragments are concatenated and sent in one piece
        node address FFh FFh FFh FFh FFh FFh broadcasts to all nodes

Format of AES-ECB:
Offset  Size    Description     (Table 2214)
 00h    DWORD   Link
 04h    DWORD   ESR address
 08h    BYTE    in use flag (see #2215)
 09h  5 BYTEs   AES workspace

(Table 2215)
Values for ECB in use flag:
 00h    available
 E0h    AES temporary
 F6h    \ special IPX/SPX processing for v3.02+
 F7h    /
 F8h    IPX in critical section
 F9h    SPX listening
 FAh    processing
 FBh    holding
 FCh    AES waiting
 FDh    AES couting down delay time
 FEh    awaiting packet reception
 FFh    sending packet

(Table 2216)
Values for ECB completion code:
 00h    success
 ECh    remote terminated connection without acknowledging packet
 EDh    abnormal connection termination
 EEh    invalid connection ID
 EFh    SPX connection table full
 F9h    event should not be cancelled
 FAh    cannot establish connection with specified destination
 FCh    cancelled
 FDh    malformed packet
 FEh    packet undeliverable
 FFh    physical error

(Table 2217)
Values event Service Routine is called with:
        AL = caller's identity (00h = AES, FFh = IPX)
        ES:SI -> event control block
        interrupts disabled

Format of IPX header:
Offset  Size    Description     (Table 2218)
 00h    WORD    (big-endian) checksum
 02h    WORD    (big-endian) length in bytes of total packet
 04h    BYTE    transport control
 05h    BYTE    packet type (see #2219)
 06h 10 BYTEs   destination internetwork address
 10h    WORD    (big-endian) destination socket
 12h 10 BYTEs   source internetwork address
 1Ch    WORD    (big-endian) source socket

(Table 2219)
Values for IPX packet type:
 00h    unknown packet type
 01h    routing information packet
 02h    echo packet
 03h    error packet
 04h    packet exchange packet (always use this one)
 05h    SPX packet
 11h    NetWare Core Protocol
 14h    Propagated Packet (for NetWare), NetBIOS name packet
 15h-1Eh experimental protocols
Note:   undocumented packet type 14h will cross up to 16 networks deep in
          all directions; as Aaron Martin of Origin Systems discovered, the
          first 64 bytes of the IPX data in such packets should be considered
          reserved, as IPX places the traversed server nodes there.

Format of Service Advertising Protocol Service Query Packet:
Offset  Size    Description     (Table 2220)
 00h 30 BYTEs   IPX header
 1Eh    WORD    (big-endian) query type
                0001h general find service
                0003h find nearest server
 20h    WORD    (big-endian) server type (see INT 21/AH=E3h"NetWare")

Format of Service Advertising Protocol Server Identification Packet:
Offset  Size    Description     (Table 2221)
 00h 30 BYTEs   IPX header
 1Eh    WORD    (big-endian) response type
                0002h general service
                0004h nearest service
 20h 64N BYTEs  server entries (1-7) (see #2222)

Format of SAP server entry:
Offset  Size    Description     (Table 2222)
 00h    WORD    (big-endian) server type (see INT 21/AH=E3h"NetWare")
 02h 48 BYTEs   ASCIZ server name
 32h  2 WORDs   (big-endian) network number
 34h  3 WORDs   (big-endian) node number
 3Ch    WORD    (big-endian) socket number
 3Eh    WORD    (big-endian) number of hops between caller and server

Format of IPX Routing Information packet:
Offset  Size    Description     (Table 2223)
 00h 30 BYTEs   IPX header
 1Eh    WORD    operation (0001h request, 0002h response)
 20h 8N BYTEs   network entries (1-50) (see #2224)

Format of RIP network entry:
Offset  Size    Description     (Table 2224)
 00h    DWORD   network number (FFFFFFFFh = general request)
 04h    WORD    (response) number of hops
 06h    WORD    (response) number of clock ticks to reach destination
--------N-7A----BX0004-----------------------
INT 7A - Novell NetWare - IPX Driver - LISTEN FOR PACKET
        BX = 0004h
        ES:SI -> Event Control Block (see BX=0003h)
Return: AL = status
            00h successful
            FFh no listening socket for packet
Desc:   this function provides IPX with an ECB for receiving an IPX packet, but
          does not wait for a packet to arrive
Notes:  the application must open a socket and initialize the ECB's ESR
          address, socket number, fragment count, and fragment descriptor
          fields before invoking this function
        there is no limit on the number of ECBs which may simultaneously be
          listening on a socket
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0000h,BX=0003h
--------N-7A----BX0005-----------------------
INT 7A - Novell NetWare - IPX Driver - SCHEDULE IPX EVENT
        BX = 0005h
        AX = delay time in clock ticks
        ES:SI -> Event Control Block (see BX=0003h)
Note:   this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0006h,BX=0007h,BX=0008h
--------N-7A----BX0006-----------------------
INT 7A - Novell NetWare - IPX Driver - CANCEL EVENT
        BX = 0006h
        ES:SI -> Event Control Block (see BX=0003h)
Return: AL = return code (see #2225)
Notes:  cannot cancel packets which the node's driver has already sent
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0005h

(Table 2225)
Values for IPX return code:
 00h    success
 F9h    event in use
 FCh    event cancelled
 FFh    unsuccessful, event not in use, or unrecognized ECB flag
--------N-7A----BX0007-----------------------
INT 7A - Novell NetWare - IPX Driver - SCHEDULE SPECIAL EVENT
        BX = 0007h
        AX = delay time
        ES:SI -> Event Control Block (see BX=0003h)
Note:   this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0006h
--------N-7A----BX0008-----------------------
INT 7A - Novell NetWare - IPX Driver - GET INTERVAL MARKER
        BX = 0008h
Return: AX = interval marker in clock ticks
Notes:  may be used to measure the time elapsed between two events, up to one
          hour
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0005h
--------N-7A----BX0009-----------------------
INT 7A - Novell NetWare - IPX Driver - GET INTERNETWORK ADDRESS
        BX = 0009h
        ES:SI -> buffer for own internetwork address (see #2226)
Return: ES:SI buffer filled
        SI destroyed
Note:   this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0002h,BX=000Bh

Format of IPX internetwork address:
Offset  Size    Description     (Table 2226)
 00h  4 BYTEs   (big-endian) network number
 04h  6 BYTEs   (big-endian) node number within network
--------N-7A----BX000A-----------------------
INT 7A - Novell NetWare - IPX Driver - RELINQUISH CONTROL
        BX = 000Ah
Desc:   this call indicates that the application is idle and permits the IPX
          driver to do some work
Note:   this function is supported by Advanced NetWare 1.02+
SeeAlso: INT 15/AX=1000h,INT 21/AH=89h,INT 2F/AX=1680h
--------N-7A----BX000B-----------------------
INT 7A - Novell NetWare - IPX Driver - DISCONNECT FROM TARGET
        BX = 000Bh
        ES:SI -> internetwork address (see #2227)
Notes:  this function permits the network software on the remote machine to
          remove any virtual connection with the calling machine
        only use in point-to-point networks
        should never be called from within an Event Service Routine
        this function is supported by Advanced NetWare 1.02+
SeeAlso: BX=0002h,BX=0009h

Format of IPX internetwork address:
Offset  Size    Description     (Table 2227)
 00h  4 BYTEs   (big-endian) destination network
 04h  6 BYTEs   (big-endian) destination node
 0Ah  2 BYTEs   (big-endian) destination socket
--------N-7A----BX000C-----------------------
INT 7A U - Novell NetWare - IPX Driver - internal - INITIALIZE NETWORK ADDRESS
        BX = 000Ch
        CX:DX = global network address (see INT 7A/BX=0002h)
        ES:DI -> "OSINCRITICALSECTION" flag
        DS:SI -> current mode for socket
Note:   the address cannot be changed once it has been initialized
SeeAlso: INT 7A/BX=0024h
--------N-7A----BX000D-----------------------
INT 7A U - Novell NetWare - IPX Driver - internal - IPX GET PACKET SIZE
        BX = 000Dh
Return: AX = maximum packet size
        CX = retry count
SeeAlso: BX=001Ah
--------N-7A----BX000E-----------------------
INT 7A U - Novell NetWare - IPX Driver - internal - TERMINATE SOCKETS
        BX = 000Eh
Return: nothing
Notes:  this function terminates all sockets opened with the current mode; this
          may be intended for future enhancements as the socket mode never
          changes in v2.15
        called by the NetWare shell if a program terminates
--------N-7A----BX000F-----------------------
INT 7A - Novell NetWare - IPX Driver - INTERNAL - SEND PACKET
        BX = 000Fh
        ES:SI -> Event Control Block (see BX=0003h)
Note:   nearly identical to function 0003h, but does not copy address into
          the first fragment, and bypasses normal error checking
SeeAlso: BX=0003h
--------N-7A----BX0010-----------------------
INT 7A - Novell NetWare - SPX Driver - INSTALLATION CHECK
        BX = 0010h
        AL = 00h
Return: AL = FFh if SPX loaded
            BH = SPX major version
            BL = SPX minor version
            CX = maximum SPX connections
            DX = SPX connections available
Notes:  this function is supported by Advanced NetWare 2.1+
        this interrupt is used for IPX/SPX access in NetWare versions through
          2.0a; in later versions, you should use INT 2F/AX=7A00h to get an
          entry point even though INT 7A still exists.  For both INT 7A and
          the FAR entry point, BX contains the function number
        IPX is sometimes called internally with BX bit 15 set, which causes the
          entry point handler to bypass some checks and an optional call to
          the IPX Windows support handler set with INT 2F/AX=7AFFh/BX=0000h
          (see #1514)
SeeAlso: BX=0015h
--------N-7A----BX0011-----------------------
INT 7A - Novell NetWare - SPX Driver - ESTABLISH SPX CONNECTION
        BX = 0011h
        AL = retry count
        AH = watchdog flag
        ES:SI -> Event Control Block (see BX=0003h)
Return: AL = status (see #2228)
        DX = assigned connection ID number
Desc:   attempt to establish a connection with a listening socket
Notes:  there should always be at least two SPX ECB's listening to a socket, so
          that NetWare can perform its internal packet exchanges
        the first fragment should start with a SPX header.  Fill in all
          destination addresses.
        this function is supported by Advanced NetWare 2.1+
SeeAlso: BX=0000h,BX=0012h,BX=0013h,BX=0014h,BX=0015h

(Table 2228)
Values for SPX function status:
 00h    attempting to contact destination socket
 EFh    local connection table full
 FDh    buffer size not 42 or fragment count not 1
 FFh    sending socket not open

Format of SPX header:
Offset  Size    Description     (Table 2229)
 00h    WORD    (big-endian) checksum
 02h    WORD    (big-endian) length in bytes of total packet
 04h    BYTE    transport control
 05h    BYTE    packet type (see INT 7A/BX=0003h)
 06h 10 BYTEs   destination internet address
 10h    WORD    (big-endian) destination socket
 12h 10 BYTEs   source internet address
 1Ch    WORD    (big-endian) source socket
 1Eh    BYTE    connection control (see #2230)
 1Fh    BYTE    datastream type
                FEh terminate connection request packet
                FFh terminate connection acknowledgement packet
                other user-defined, ignored by SPX
 20h    WORD    (big-endian) source connection ID
 22h    WORD    (big-endian) destination connection ID
 24h    WORD    (big-endian) sequence number
 26h    WORD    (big-endian) acknowledge number
 28h    WORD    (big-endian) allocation number

Bitfields for SPX connection control:
Bit(s)  Description     (Table 2230)
 3-0    unused???
 4      end of message
 5      reserved
 6      acknowledgement required
 7      system packet
--------N-7A----BX0012-----------------------
INT 7A - Novell NetWare - SPX Driver - LISTEN FOR SPX CONNECTION
        BX = 0012h
        AH = watchdog flag (00h disabled, 01h enabled)
        AL = retry count (00h = default)
        ES:SI -> Event Control Block (see BX=0003h)
Notes:  there should always be at least two SPX ECB's listening to a socket, so
          that NetWare can perform its internal packet exchanges
        this function is supported by Advanced NetWare 2.1+
SeeAlso: BX=0011h,BX=0013h,BX=0014h
--------N-7A----BX0013-----------------------
INT 7A - Novell NetWare - SPX Driver - TERMINATE SPX CONNECTION
        BX = 0013h
        DX = connection ID to terminate
        ES:SI -> Event Control Block (see BX=0003h)
Note:   this function is supported by Advanced NetWare 2.1+
SeeAlso: BX=0011h,BX=0012h,BX=0014h
--------N-7A----BX0014-----------------------
INT 7A - Novell NetWare - SPX Driver - ABORT SPX CONNECTION
        BX = 0014h
        DX = connection ID to terminate
Notes:  this function is supported by Advanced NetWare 2.1+
        this function does not tell the other side that the connection has been
          terminated
        also aborts any outstanding Establish Connection, Terminate Connection,
          and Send Sequenced Packet commands
SeeAlso: BX=0011h,BX=0013h
--------N-7A----BX0015-----------------------
INT 7A - Novell NetWare - SPX Driver - GET SPX CONNECTION STATUS
        BX = 0015h
        DX = connection ID
        ES:SI -> status buffer (see #2231)
Return: AL = return code
            00h connection still valid
                ES:SI -> status buffer filled
            EEh no such connection
Note:   this function is supported by Advanced NetWare 2.1+
SeeAlso: BX=0010h,BX=0011h

Format of SPX status buffer:
Offset  Size    Description     (Table 2231)
 00h    BYTE    connection state
                01h waiting to establish connection
                02h starting (attempting to create connection)
                03h connection established
                04h terminating
 01h    BYTE    watchdog flag
                bit 0: used internally by SPX
                bit 1: SPX watchdog is monitoring connection
                bits 2-7 used internally by SPX
 02h    WORD    (big-endian) source connection ID
 04h    WORD    (big-endian) destination connection ID
 06h    WORD    (big-endian) sequence number of next packet sent
 08h    WORD    (big-endian) acknowledge number, expected sequence number of
                  next received packet
 0Ah    WORD    (big-endian) maximum sequence number remote SPX may send
                  without ACK from local SPX
 0Ch    WORD    (big-endian) remote acknowledge number, next sequence number
                  remote SPX expects to receive
 0Eh    WORD    (big-endian) remote allocation number, maximum sequence number
                  local SPX may send
 10h    WORD    (big-endian) connection socket
 12h  6 BYTEs   immediate node address--bridge on local network to destination
 18h 10 BYTEs   destination internetwork address (see INT 7A/BX=000Bh)
 22h    WORD    (big-endian) retransmit count
 24h    WORD    (big-endian) estimated roundtrip delay
 26h    WORD    (big-endian) retransmitted packets
 28h    WORD    (big-endian) suppressed packets
 2Ah 12 BYTEs   ??? (v2.15)
--------N-7A----BX0016-----------------------
INT 7A - Novell NetWare - SPX Driver - SEND SPX PACKET
        BX = 0016h
        DX = connection ID
        ES:SI -> Event Control Block (see BX=0003h)
Notes:  this function is supported by Advanced NetWare 2.1+
        CX may need to be 0001h ???
SeeAlso: BX=0011h,BX=0017h
--------N-7A----BX0017-----------------------
INT 7A - Novell NetWare - SPX Driver - LISTEN FOR SPX PACKET
        BX = 0017h
        DX = connection ID (unused in v2.15)
        ES:SI -> Event Control Block (see BX=0003h)
Notes:  this function is supported by Advanced NetWare 2.1+
        CX may need to be 0001h ???
SeeAlso: BX=0011h,BX=0016h
--------N-7A----BX0018-----------------------
INT 7A U - Novell NetWare - IPX Driver - internal - ADD DIAGNOSTIC ELEMENT
        BX = 0018h
        ES:SI -> diagnostic element (see #2232) to be added to Diagnostic Queue
Note:   this function is supported on file servers only under v2.15; v3.02 also
          supports it on workstations
SeeAlso: BX=0019h

Format of IPX diagnostic element:
Offset  Size    Description     (Table 2232)
 00h    DWORD   pointer to next diagnostic element
 04h    DWORD   pointer to function for ???
 08h    DWORD   pointer to function for ???
--------N-7A----BX0019-----------------------
INT 7A U - Novell NetWare - IPX Driver - internal - CANCEL DIAGNOSTIC ELEMENT
        BX = 0019h
        ES:SI -> diagnostic element (see BX=0018h) to be removed
Note:   this function is supported on file servers only under v2.15; v3.02 also
          supports it on workstations
SeeAlso: BX=0018h
--------N-7A----BX001A-----------------------
INT 7A - Novell NetWare - IPX Driver - internal - GET DRIVER PACKT SIZE LIMIT
        BX = 001Ah
Return: AX = packet size with preamble
        CX = IPX retry count
Note:   this function is has existed since November 1989; it is documented in
          Novell document FYI.A.3709, 03May91
SeeAlso: BX=000Dh
--------N-7A----BX001B-----------------------
INT 7A U - Novell NetWare - IPX Driver - INTERNAL
        BX = 001Bh
        ???
Return: ???
Notes:  this function is supported on file servers only under v2.15; v3.02 also
          supports it on workstations
        used by NetWare Access Server
--------N-7A----BX001C-----------------------
INT 7A U - Novell NetWare - NetWare Access Server - ???
        BX = 001Ch to 001Eh
        ???
Return: ???
        BX corrupted
Notes:  these functions are NOPs for standard IPX drivers such as IPXODI v2.12
SeeAlso: INT 2F/AX=7AF1h
--------N-7A----BX001F-----------------------
INT 7A - Novell NetWare - IPXODI v2.12+ - GET IPX FLAGS
        BX = 001Fh
        DX = 0000h
Return: AX = ???
        BX:CX -> ??? entry point (see #2233)
        DX = flags
            bit 0: IPXODI rather than dedicated IPX
            bit 1: checksumming functions 0020h-0022h supported
        ES:SI -> array of words containing used socket number (undocumented);
                0000h ends array
Note:   IPXODI v2.12 is distributed as part of the Personal NetWare system
          bundled with Novell DOS 7
SeeAlso: BX=0020h,INT 2F/AX=7A2Fh

(Table 2233)
Call IPXODI entry point with:
        ES:SI -> ??? data (offset 24h is far pointer to ??? data)
--------N-7A----BX0020-----------------------
INT 7A - Novell NetWare - IPXODI v2.12+ - SEND WITH CHECKSUM
        BX = 0020h
        ES:SI -> ECB data (see #2234)
Return: DS preserved
        BX,BP corrupted
SeeAlso: BX=001Fh,BX=0021h,BX=0022h,INT 2F/AX=7A2Fh

Format of ECB data:
Offset  Size    Description     (Table 2234)
 00h  8 BYTEs   ???
 08h    BYTE    (return) ???
 09h    BYTE    (return) ???
 0Ah    WORD    ???
 0Ch 16 BYTEs   ???
 1Ch  3 WORDs   ???
 22h    WORD    number of records at offset 28h
 24h    DWORD   -> ???
 28h 6N BYTEs   ??? records
--------N-7A----BX0021-----------------------
INT 7A - Novell NetWare - IPXODI v2.12+ - IPX GENERATE CHECKSUM
        BX = 0021h
        ES:SI -> ECB data (see #2234)
Return: ES,DS,SI preserved
        BX,BP corrupted
Notes:  the checksum and TransportControl fields of the IPX packet are updated
        this function enables interrupts and is fully reentrant
SeeAlso: BX=001Fh,BX=0020h,BX=0022h,INT 2F/AX=7A2Fh
--------N-7A----BX0022-----------------------
INT 7A - Novell NetWare - IPXODI v2.12+ - IPX VERIFY CHECKSUM
        BX = 0022h
        ES:SI -> ECB data (see #2234)
Return: AX = status (0000h checksum matches)
        DS,ES,SI preserved
        BX,BP corrupted
Note:   this function enables interrupts and is fully reentrant
SeeAlso: BX=001Fh,BX=0020h,BX=0021h,INT 2F/AX=7A2Fh
--------N-7A----BX0023-----------------------
INT 7A - Novell NetWare - IPXODI v2.12+ - OPEN LOOK-AHEAD SOCKET
        BX = 0023h
        AL = ???
        DX = socket number
        ES:SI -> Look Ahead handler (see #2235)
Return: AL = 00h if successful
        BX corrupted
Notes:  the socket will always be long-lived, and must thus be explicitly
          closed with INT 7A/BX=0001h before the Look Ahead handler code is
          removed from memory (i.e. the program terminates)
        this function is only supported if INT 2F/AX=7A00h returns ES:BX
          pointing at an IPX version greater than 3.30
SeeAlso: INT 7A/BX=0000h,INT 7A/BX=0001h

(Table 2235)
Call IPX Look-Ahead handler with:
        AX = socket
        DS:SI -> look-ahead structure (see #1583 at INT 2F/AX=C000h"LSL")
        DF clear
        interrupt disabled (must remain disabled)
Return: AX = packet use
            0000h application want packet
                ES:SI -> ODI ECB (see #1584 at INT 2F/AX=C000h"LSL")
            8001h application does not want packet
        ZF set if AX=0000h
        DS,DI,BP,SS,SP preserved
--------N-7A----BX0024-----------------------
INT 7A U - Novell NetWare - IPXODI v2.20+ - SET INTERNETWORK ADDRESS
        BX = 0024h
        ES:SI -> buffer containing internetwork address (see #2227)
Return: BX,CX,SI,DI,ES destroyed
Note:   this function differs from INT 7A/BX=000Ch in that it unconditionally
          sets the address
SeeAlso: INT 7A/BX=000Ch
