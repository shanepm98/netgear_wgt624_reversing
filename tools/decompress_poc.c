/*

  Shane Mullin
  11/22/2025

  PoC to see if the algorithm actually decompresses the firmware image correctly.

 */

// includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


// defs


// prototypes
int inflate(unsigned char*, unsigned char*, int);


//----------------------------
int main (int argc, char** argv){

  // check usage
  if(argc != 3){
    printf("Usage: %s <filename of compressed input> <filename of decompressed output>\n", argv[0]);
    exit(1);
  }
  
  // open compressed file specified by CLI arg
  FILE* fp;
  fp = fopen(argv[1], "r");

  // get file size
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET); 

  // allocate buffer for the compressed file as well as the decompressed file
  unsigned char* src, *dst;
  src = (unsigned char*)malloc(size);
  dst = (unsigned char*)malloc((size*10) + 1000); // add 1000 bytes for "just in case"

  // read file into buffer
  for(int i = 0; i < size; i++){
    src[i] = (char)fgetc(fp);
  }
  
  // *fingers crossed* run the decompression algorithm
  inflate(src, dst, size);

  // write output to file
  fclose(fp);
  fp = fopen(argv[2], "a+");
  for(int i = 0; i < (size * 10)+1000 ; i++){
    putc(dst[i], fp);
  }
  
  // clean up
  free(src);
  free(dst);
  fclose(fp);

  // exit
  return EXIT_SUCCESS;
}

