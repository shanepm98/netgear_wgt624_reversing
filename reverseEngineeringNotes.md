
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