# Utilities
This directory contains code to build a tool to decompress blocks of vxworks-compressed firmware.

Just run `make` and it will build the decompress tool.

To use the tool, dump the compressed area of firmware to a file (for example, using `dd` or `radare2`).

To use the decompress tool:
```bash
./vxdecompress /path/to/compressed/input /path/to/decompressed/output
```


When I have time I will try to refine this tool further to extract and decompress compressed code blocks from the firwmare image automatically. For now though, you have to
manually extract and provide compressed blocks yourself.

## patched_vxworks_inflate.c
This is a patched version of (what I believe to be) the authentic VxWorks decompression algorithm, found here:
https://en.verysource.com/code/2857996_1/inflateLib.c.html

I tweaked it so that you can compile it without the vxCpu.h header.

Note that this is just a collection of functions. Once I write a utility program to use these functions to decompress a firmware image I will publish it.
