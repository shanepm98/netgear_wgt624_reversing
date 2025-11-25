CC=gcc

all: vxdecompress

vxdecompress: libvxworkscompress.a vxdecompress.c
	$(CC) vxdecompress.c -o vxdecompress -L$(CURDIR) -lvxworkscompress

compresslib.o: patched_vxworks_inflate.c
	$(CC) -c -o compresslib.o patched_vxworks_inflate.c

libvxworkscompress.a: compresslib.o
	ar rcs libvxworkscompress.a compresslib.o

clean:
	rm compresslib.o libvxworkscompress.a vxdecompress
