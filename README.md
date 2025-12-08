# Reverse Engineering the Netgear WGT624

This file will serve as my journal for work done on the project, both work completed and work left to be done.


# TODO
- make a header file for the vxworks compress functions
- optimize the decompress program to make output file only as big as needed, not 10X the input size ( I was using 10x the input as that is the expected maximum compression rate)
- hook up to the UART console again and get as much information as I can from it, especially about memory addresses and entry points
- figure out the address in memory at which the decompressed image is loaded into, and the entrypoint
- After determining load/entry address, get the firmware loaded into Ghidra and get Ghidra cooperating
- After getting Ghidra reading the firmware correctly, determine symbols and rename functions
- Patch whatever I need to patch to be able to upload new instructions via UART console
- modify the exception handler routine to provide fuzzing feedback
- write an in-memory fuzzer

# Updates
## 11/27/2025
Fixed the seg fault problem by rewriting the `zcalloc` and `zcfree` functions to use standard malloc calls instead of the highly-constrained
static buffer it used originally. It makes sense to do it their way when the code has to run on a resource-constrained embedded system, but since
my patched version will only ever run on a desktop or laptop realistically, I found it easier to scrap theirs than to try and massage it into
cooperating. After modifying the code to resolve the seg fault, the 288KB compressed block inflates to over 500KB, and running `strings` on it
verifies that it is valid code. Awesome.

I found an old Black Hat talk on hacking a VxWorks device thats full of awesome information, and going forward I will try to loosely follow the
roadmap that their research provided. (==LINK IT HERE==). In following their research, my next steps will be to figure out where the decompressed
image is loaded in memory so that the relative addresses work as intended. Then get it loaded into Ghidra to begin a more human-friendly analysis.


## 11/25/2025
I tested the patched decompression program on my extracted source code. The checksum is valid which means I successfully extracted the entire
piece of compressed data. The program DOES decompress a portion of the code, but then seg faults after about 100KB of output. I isolated the
source of the seg fault to a macro that attempts to dereference a pointer as an int, then cast that int as a pointer. This wouldn't be a problem
on the target 32-bit architecture in which pointers and ints are the same number of bytes, but on a 64 bit system it causes you to try to access
invalid memory. Further work needed to work around this problem.

## 11/23/2025
I modified what I believe to be the legit VxWorks decompression algorithm to not depend on the vxCpu.h header and to be able to compile
with no special libraries. It really didn't take much effort, just had to reformat some multi-line `#define`s that the compiler didnt like.
Makes sense though, all its doing is decompressing. It isnt calling on any specific device hardware.

I have yet to actually test the code on my firmware. But if it works, Im going to publish the modified/compilable inflate source code here,
the library file I made from it, and a standalone tool for decompressing vxworks firwmare images. I also plan on contributing a model to
binwalk to recognize vxworks magic numbers and decompress the firwmare image.



