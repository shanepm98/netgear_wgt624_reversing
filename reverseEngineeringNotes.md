
## Patching VxWorks Inflation algorithm and making a library 
I found what appears to be a legit copy of VxWorks' decompression algorithm source code. 

It includes a `vxCpu.h` header which I could not find the source for, but it doesnt appear to do a whole lot with this header. I commented this line out. This program rolls its own `memcpy()`, so I just used emacs to rename every occurrence of `memcpy` with `memcpy_vx` so that the compiler wouldnt complain. 

After that I basically just reformatted some lines that the compiler didn't like, such as multi-line defines. 


There's no `main` function, so I'll compile this as a library that I can include in a harness file.


Compiling as library:
```bash
 gcc -o compresslib.o -c potential_inflate_code.c
 ar rcs libvxworkscompress.a compresslib.o 
```

## Writing a decompression harness
Okay. I got a library built from the modified zlib functions. Now I need to write a harness program to apply the functions to the firmware. 

Need to do:
- extract the compressed firmware from the image
- load the extracted compressed data into the harness
- call the inflate function on the firmware

### Extracting the compressed firwmare
The compressed data appears to start @ 0x6880 and end at 0x3fa20
(233,888 bytes)

I used radare2 to dump this region to a binfile as follows:
```
pr 233888 @ 0x6880 > compressed_fw.bin
```

(`pr` = "print raw")


### Loading the extracted compressed data into the harness
The `inflate()` function expects a source pointer to the compressed data, a destination pointer to the buffer for decompressed data, and a size. I will just malloc two big blocks of memory. One for the the compressed data and one for the decompressed data that is 10x the size. 


- get size of file 
- allocate buffer of that size + one that is that size x10
- call inflate() with source, dest, and size args
- write decompressed buffer to file

==This same tool will be necessary for packaging my own modifications==


### Compiling
```bash
gcc decompress_poc.c -o test -L$(pwd) -lvxworkscompress
```

### SUCCESS!
It looks like the fucking thing actually worked. It produced valid and coherant mips code. 



# Notes on the compression format
As the vxworks source code and publicly-available documentation explains, vxworks's compression algorithm is a modified version of zlib.


Note the following line from the source code:
```c
  /*
   * Validate the compression stream.
   * The first byte should be Z_DEFLATED, the last two a valid checksum.
   */
  if (*src != Z_DEFLATED)
    {
      DBG_PUT ("inflate error: *src = %d. not Z_DEFLATED datan", *src);
      return (-1);
    }
```

where
```c
#define Z_DEFLATED   8
```

As we can see, `8` is kind of a magic number for the compression format. The first byte of the compressed stream will be `8` (else decompression function will abort).
Additionally, the last two bytes should be a valid checksum.



Question: does the `cksum` function actually care about the number of bytes? Yes, it does. 



I manually enabled checksum verification and debug messages. The variable is `inflateCksum`. Change it back to 0 if needed


# Getting a shell
Using bus pirate with UART macro 1 (transparent bridge)

Baud rate: 9600

- record the bus pirate config
- take picture of board connections 
- tee minicom to file
```
Welcome to minicom 2.10

OPTIONS: I18n 
Compiled on Feb 24 2025, 22:01:05.
Port /dev/ttyUSB0, 23:40:04 [U]

Press CTRL-A Z for help on special keys


HiZ>m
1. HiZ
2. 1-WIRE
3. UART
4. I2C
5. SPI
6. 2WIRE
7. 3WIRE
8. LCD
9. DIO
x. exit(without change)

(1)>3
Set serial port speed: (bps)
 1. 300
 2. 1200
 3. 2400
 4. 4800
 5. 9600
 6. 19200
 7. 38400
 8. 57600
 9. 115200
10. BRG raw value

(1)>5
Data bits and parity:
 1. 8, NONE *default 
 2. 8, EVEN 
 3. 8, ODD 
 4. 9, NONE
(1)>
Stop bits:
 5. 1 *default
 6. 2
(1)>
Receive polarity:
 7. Idle 1 *default
 8. Idle 0
(1)>
Select output type:
 9. Open drain (H=Hi-Z, L=GND)
 10. Normal (H=3.3V, L=GND)

(1)>2
Ready
UART>(0)
 0.Macro menu
 1.Transparent bridge
 2. Live monitor
 3.Bridge with flow control
UART>(1)
UART bridge
Reset to exit
Are you sure? y

ar531xPlus rev 0x00000087 firmware startup...

boardData checksum failed!


Atheros AR5001AP default version 4.0.0.167


 0
auto-booting...

Attaching to TFFS... done.
Loading /fl/APIMG1...
Decompressing... /  OK.
Starting at 0x80010000...

/fl/  - Volume is OK                                
Reading Configuration File "/fl/apcfg".
Configuration file checksum: 583ab is good
Failed to attach to device aeAttaching interface lo0...done

Adding 7521 symbols for standalone.
call usrAcosPPTPClientInit!
acosPPTPClientLibInit(): PPTP-v1.1.0, INIT OK!
0x80fffdf0 (): task deadCan't attach unknown device ae (unit 0).
tTxQueueTask: task started.
-> wireless access point starting...
wlan0 Ready
Ready
DNS redirect INIT
in abRegisterInputHook inputPktHook: 800df2e8
Calling dnsRedirect_hookAdd
PPPoE: Ambit MuxHook 1.0
PPP: DevName=ppp, unit=0
*** Enable PPPoE Dial-on-demand ***
IPCP dial on demand
pppDemandHook recv from unit 0
PPP: Recved an IP datagram. I am going to dial now
*** Enable PPPoE Dial-on-demand ***
IPCP dial on demand
!Nat_PPTP_Initialize is called
Info: No FWPT default policies.
DHCPS: init dhcps: devname=mirror0
DHCPS:Set the default Lease Time to 86400
Set option ACOS_DHCP_OPT_ROUTER to 10.0.0.1:
Set option ACOS_DHCP_OPT_DNS_SERVER to 10.0.0.1:
Login: POT has reached the max value.

Login: Gearguy
Password: Geardog
U12H05500> help

Commands are:

bridge         ddns           exit           ftpc           ip             
lan            nat            passwd         pot            reboot         
save           show           sntp           time           uptime         
version        wan            web            windsh         wla            

 '..'    return to previous directory

U12H05500> windsh
->
```






The login creds I used were `Gearguy:Geardog`

- get the symbol file and addrs
- dump flash layout
- disassemble functions
- the mRegs command might let me do real debugging easily...
- Fuzz for functions?
- look up windshell functions
- with `m` command, can modify memory contents (easily write in a debugger and fuzzer)
- See this link: https://daq00.triumf.ca/~daqweb/doc/vxworks/tornado2/docs/vxworks/ref/usrLib.html. This lists a bunch of wind shell functions. See which ones are implemented. Want to be able to see the filesystem and flash memory


OOH!
```
C80216aec shell          +1d4: shell (802b9560, 3, 80c74d24, ffffffff)
80216d14 shell          +3fc: execute (802b9568, 80c84a40, 80, &yyact)
80216e84 execute        +c4 : yyparse (80c84a40, ffffffff, ffffffff, 1)
8024621c yyparse        +8a8: 80244544 (eeeeeeee, eeeeeeee, eeeeee, eeeeeeee)
8024472c yystart        +8bc: ld (0, 1, 1, 0)
8021e888 ld             +68 : loadModule (2ba89c, 3, 0, 1fe)
80214f40 loadModule     +10 : loadModuleAt (0, 0, 8042d378, fffffffe)
80214f6c loadModuleAt   +18 : loadModuleAtSym (80c848d8, &yy_yyv, 0, &yyval)
80214fc8 loadModuleAtSym+48 : loadElfInit (0, 8, 0, 0)
80214740 loadElfInit    +19e8: 80212ef4 (eeeeeeee, eeeeeeee, 80c847b7, 80fcfc90)
80212f20 loadElfInit    +1c8: fioRead (80215060, eeeeeeee, 1, 802bb2a1)
80201470 fioRead        +34 : read (0, 80c84698, 8020454c, 80205c44)
8020450c read           +8  : iosRead (0, 80205d2c, 80c749e0, 3)
80205adc iosRead        +c8 : tyRead (8020ed9c, 80c749e0, 80204dd8, 3)
8020f468 tyRead         +50 : semTake (80fcfca4, ffffffff, 3, 3)
```

Looks like there ARE some file I/O functions built in, even though the ioHelp function is not implemented. I guess that obviously makes sense, it has to be able to load files.

The ones Im seeing are
`iosRead`, `read`, `fioRead`, `tyRead`

There are also other lib ref pages. Go through those as well:
https://daq00.triumf.ca/~daqweb/doc/vxworks/tornado2/docs/vxworks/ref/

`tffsConfig` and `tffsDrv` contain useful *IMPLEMENTED*  true-ffs (true flash filesystem) interface commands:
```
-> tffsShowAll
TFFS Version 2.2
0: socket=RFA: type=0x0, unitSize=0x10000, mediaSize=0x1d0000
```

`opendir` and `readdir` ARE implemented, and I can open `/fl/`, which is probably the tffs flash dev. `readdir` returns a pointer to a `dirent` instead of raw text, so I need to find a way to dump the dirent from memory without ls.


Read the `dirLib` documentation, thats my lifeline
using `stat()` on `"/fl/"` or `"/fl/apcfg"` crashes the system with a data bound error, which I assume is a seg fault.

Ah! Thats because I was using it wrong. Need to pass it a pointer that was already allocated, which I didnt do.


GOT IT!
I was able to get the contents of the `/fl/` directory using `opendir`, `readdir`, and examining memory at the address:
```
-> opendir("/fl/")
value = -2134422784 = 0x80c74b00
-> readdir(0x80c74b00)
value = -2134422776 = 0x80c74b08

-> d 0x80c74b08,10
80c74b00:                      6170 6366 6700 0000   *        apcfg...*
80c74b10:  0000 0000 0000 0000 0000 0000             *................*
```

Now we have to call `readdir()` repeatedly until it returns null, as each call only reads one entry.

How do we script this?
Not quite a script, but this makes it a 1-liner:
```
-> d readdir(0x80c74b00),10
80c74b00:                      6170 696d 6731 0020   *        apimg1. *
80c74b10:  2000 0000 0000 0000 0000 0000             * ...............*
```

Files in `/fl/` obtained by opendir/readdir/m method:
- apcfg
- NVRAM
- apimg1
- apcfg.bak



Well, I managed to CREATE a file:
```
creat("/fl/test")
```

How do I allocate a pStat struct to run statfs?
```
-> mybuffer = malloc(100)
```

See `# taskHookShow` and similar for hooking exception handler


Alright. Now how do I open and read files?
```
-> buff = malloc(400)
new symbol "buff" added to symbol table.
buff = 0x80c74720: value = -2134423744 = 0x80c74740 = buff + 0x20

-> open("/fl/apcfg", 0, 0644)
value = 13 = 0xd

-> read(0xd,buff,400)
value = 400 = 0x190

-> d buff,400
80c74740:  2320 436f 7079 7269 6768 7420 2863 2920   *# Copyright (c) *
80c74750:  3230 3032 2041 7468 6572 6f73 2043 6f6d   *2002 Atheros Com*
80c74760:  6d75 6e69 6361 7469 6f6e 732c 2049 6e63   *munications, Inc*
80c74770:  2e2c 2041 6c6c 2052 6967 6874 7320 5265   *., All Rights Re*
80c74780:  7365 7276 6564 0a23 2044 4f20 4e4f 5420   *served.# DO NOT *
80c74790:  4544 4954 202d 2d20 5468 6973 2063 6f6e   *EDIT -- This con*
80c747a0:  6669 6775 7261 7469 6f6e 2066 696c 6520   *figuration file *
80c747b0:  6973 2061 7574 6f6d 6174 6963 616c 6c79   *is automatically*
80c747c0:  2067 656e 6572 6174 6564 0a6d 6167 6963   * generated.magic*
80c747d0:  2041 7235 3278 7841 500a 6677 633a 2031   * Ar52xxAP.fwc: 1*
80c747e0:  340a 6c6f 6769 6e20 4164 6d69 6e0a 6e61   *4.login Admin.na*
80c747f0:  6d65 6164 6472 200a 646f 6d61 696e 7375   *meaddr .domainsu*
80c74800:  6666 6978 200a 5241 4449 5553 6164 6472   *ffix .RADIUSaddr*
80c74810:  200a 5241 4449 5553 706f 7274 2031 3831   * .RADIUSport 181*
80c74820:  320a 5241 4449 5553 7365 6372 6574 200a   *2.RADIUSsecret .*
80c74830:  7061 7373 776f 7264 2035 7570 0a76 6c61   *password 5up.vla*
80c74840:  6e20 4469 7361 626c 650a 696e 7465 7256   *n Disable.interV*
80c74850:  4620 456e 6162 6c65 0a77 6c61 6e30 3020   *F Enable.wlan00 *
80c74860:  696e 7472 6156 4620 456e 6162 6c65 0a77   *intraVF Enable.w*
80c74870:  6c61 6e31 3020 696e 7472 6156 4620 456e   *lan10 intraVF En*
80c74880:  6162 6c65 0a77 6c61 6e30 3020 7661 7073   *able.wlan00 vaps*
80c74890:  2031 0a77 6c61 6e31 3020 7661 7073 2031   * 1.wlan10 vaps 1*
80c748a0:  0a77 6c61 6e30 3020 7573 7270 2030 0a77   *.wlan00 usrp 0.w*
80c748b0:  6c61 6e31 3020 7573 7270 2030 0a77 6c61   *lan10 usrp 0.wla*
80c748c0:  6e30 3020 7076 6964 2031 0a77 6c61 6e31   *n00 pvid 1.wlan1*
80c748d0:  80c7 4730 0000 0020 eeee eeee eeee eeee   *..G0... ........*
80c748e0:  80c7 4900 eeee eeee eeee eeee eeee eeee   *..I.............*
80c748f0:  80c7 48d0 0000 0080 eeee eeee eeee eeee   *..H.............*
80c74900:  2f66 6c00 eeee eeee eeee eeee eeee eeee   */fl.............*
80c74910:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74920:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74930:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74940:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74950:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74960:  eeee eeee eeee eeee eeee eeee eeee eeee   *................*
80c74970:  80c7 48f0 0000 0020 eeee eeee eeee eeee   *..H.... ........*
80c74980:  2f66 6c2f 6170 6366 6700 eeee eeee eeee   */fl/apcfg.......*
80c74990:  80c7 4970 0000 0020 eeee eeee eeee eeee   *..Ip... ........*
80c749a0:  2f66 6c2f 6170 6366 6700 eeee eeee eeee   */fl/apcfg.......*
80c749b0:  80c7 4990 0000 0020 eeee eeee eeee eeee   *..I.... ........*
80c749c0:  2f66 6c2f 6170 6366 6700 eeee eeee eeee   */fl/apcfg.......*
80c749d0:  80c7 49b0 0000 0080 eeee eeee eeee eeee   *..I.............*
80c749e0:  0000 000c 0000 0121 7465 7374 002e 6261   *.......!test..ba*
80c749f0:  6b00 0000 0000 0000 0000 0000 0000 0000   *k...............*
80c74a00:  0000 0000 0000 0000 0000 0000 0000 0000   *................*
80c74a10:  0000 0000 0000 0000 0000 0000 0000 0000   *................*
80c74a20:  0000 0000 0000 0000 0000 0000 0000 0000   *................*
80c74a30:  0000 0000 0000 0000 0000 0000 0000 0000   *................*
80c74a40:  0000 0000 0000 0000 0000 0000 eeee eeee   *................*
80c74a50:  80c7 49d0 0000 0020 eeee eeee eeee eeee   *..I.... ........*
value = 21 = 0x15
```

I fucking got it. Hell yeah

So basically, you just allocate memory with malloc.



I might wrap it up for the night and pick it up again after finals. I think at this point I can be sure that all the tools I *NEED* to run my own code on here are in the wind shell, I just might have to do everything at a low level, like I did here. But it appears that I can write my own files to the file system. Ill have to see if the changes persist or if theyre just RAM. 

I may not have a nice polished user-friendly debug program in the shell, but I have access to all the OS API low-level functions that the debug code would be made of.


Also:
check out telnet
check out httpLib



# Disassembling functions
Use `i` to get a `ps aux` type output, then disassemble target function using 
`l(<target func name>, nInstructions)`



# Next up 
print the symbol table. Do this ASAP so I can load it into Ghidra