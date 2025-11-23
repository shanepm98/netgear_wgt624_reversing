# Utilities

## patched_vxworks_inflate.c
This is a patched version of (what I believe to be) the authentic VxWorks decompression algorithm, found here:
https://en.verysource.com/code/2857996_1/inflateLib.c.html

I tweaked it so that you can compile it without the vxCpu.h header.

Note that this is just a collection of functions. Once I write a utility program to use these functions to decompress a firmware image I will publish it.


## libvxworksinflate.a
This is a library file I compiled from the inflate source code. 
