# Reverse Engineering the Netgear WGT624


I modified what I believe to be the legit VxWorks decompression algorithm to not depend on the vxCpu.h header and to be able to compile
with no special libraries. It really didn't take much effort, just had to reformat some multi-line `#define`s that the compiler didnt like.
Makes sense though, all its doing is decompressing. It isnt calling on any specific device hardware.

I have yet to actually test the code on my firmware. But if it works, Im going to publish the modified/compilable inflate source code here,
the library file I made from it, and a standalone tool for decompressing vxworks firwmare images. I also plan on contributing a model to
binwalk to recognize vxworks magic numbers and decompress the firwmare image.

