#include <string.h>
#include <stdio.h>
#include "version.h"
#include <unistd.h>

unsigned char version_opt=0;
unsigned char help_opt=0;
unsigned char p_opt=0;

#define GZIP_BUFFER 21000

#include <zlib.h>

#ifdef __CC65__
#include <stdlib.h>
#include <cc65.h>
#endif

#ifndef __CC65__
/*
** Emulate inflatemem() if using original zlib.
** As you can see, this program is quite portable.
*/
unsigned inflatemem(char* dest, const char* source)
{
        z_stream stream;

        stream.next_in = (Bytef*) source;
        stream.avail_in = 65535;

        stream.next_out = dest;
        stream.avail_out = 65535;

        stream.zalloc = (alloc_func) 0;
        stream.zfree = (free_func) 0;

        inflateInit2(&stream, -MAX_WBITS);
        inflate(&stream, Z_FINISH);
        inflateEnd(&stream);

        return stream.total_out;
}
#endif /* __CC65__ */

/*
** Structure of a GZIP file:
**
** 1. GZIP header:
**    Offset 0: Signature (2 bytes: 0x1f, 0x8b)
**    Offset 2: Compression method (1 byte: 8 == "deflate")
**    Offset 3: Flags (1 byte: see below)
**    Offset 4: File date and time (4 bytes)
**    Offset 8: Extra flags (1 byte)
**    Offset 9: Target OS (1 byte: DOS, Amiga, Unix, etc.)
**    if (flags & FEXTRA) { 2 bytes of length, then length bytes }
**    if (flags & FNAME) { ASCIIZ filename }
**    if (flags & FCOMMENT) { ASCIIZ comment }
**    if (flags & FHCRC) { 2 bytes of CRC }
**
** 2. Deflate compressed data.
**
** 3. GZIP trailer:
**    Offset 0: CRC-32 (4 bytes)
**    Offset 4: uncompressed file length (4 bytes)
*/

/* Flags in the GZIP header. */
#define FTEXT     1     /* Extra text */
#define FHCRC     2     /* Header CRC */
#define FEXTRA    4     /* Extra field */
#define FNAME     8     /* File name */
#define FCOMMENT 16     /* File comment */

/*
** We read whole GZIP file into this buffer.
** Then we use this buffer for the decompressed data.
*/
static unsigned char buffer[GZIP_BUFFER];

/*
** Get a 16-bit little-endian unsigned number, using unsigned char* p.
** On many machines this could be (*(unsigned short*) p),
** but I really like portability. :-)
*/
#define GET_WORD(p) (*(p) + ((unsigned) (p)[1] << 8))

/* Likewise, for a 32-bit number. */
#define GET_LONG(p) (GET_WORD(p) + ((unsigned long) GET_WORD(p + 2) << 16))

/*
** Uncompress a GZIP file.
** On entry, buffer[] should contain the whole GZIP file contents,
** and the argument complen should be equal to the length of the GZIP file.
** On return, buffer[] contains the uncompressed data, and the returned
** value is the length of the uncompressed data.
*/
unsigned uncompress_buffer(unsigned complen)
{
        unsigned char* ptr;
        unsigned long crc;
        unsigned long unclen;
        void* ptr2;
        unsigned unclen2;

        /* check GZIP signature */
        if (buffer[0] != 0x1f || buffer[1] != 0x8b) {
                printf("Not GZIP format");
                return 0;
        }

        /* check compression method (it is always (?) "deflate") */
        if (buffer[2] != 8) {
                printf("Unsupported compression method");
                return 0;
        }

        /* get CRC from GZIP trailer */
        crc = GET_LONG(buffer + complen - 8);

        /* get uncompressed length from GZIP trailer */
        unclen = GET_LONG(buffer + complen - 4);
		
        if (unclen > sizeof(buffer)) {
                printf("Uncompressed size too big. Requested %lu bytes in memory, but buffer is %u bytes.\n",unclen,sizeof(buffer));
                return 0;
        }
	

        /* skip extra field, file name, comment and crc */
        ptr = buffer + 10;
        if (buffer[3] & FEXTRA)
                ptr = buffer + 12 + GET_WORD(buffer + 10);
        if (buffer[3] & FNAME)
                while (*ptr++ != 0);
        if (buffer[3] & FCOMMENT)
                while (*ptr++ != 0);
        if (buffer[3] & FHCRC)
                ptr += 2;

        /*
        ** calculate length of raw "deflate" data
        ** (without the GZIP header and 8-byte trailer)
        */
        complen -= (ptr - buffer) + 8;

        /*
        ** We will move the compressed data to the end of buffer[].
        ** Thus the compressed data and the decompressed data (written from
        ** the beginning of buffer[]) may overlap, as long as the decompressed
        ** data doesn't go further than unread compressed data.
        ** ptr2 points to the beginning of compressed data at the end
        ** of buffer[].
        */
        ptr2 = buffer + sizeof(buffer) - complen;
        /* move the compressed data to end of buffer[] */
        memmove(ptr2, ptr, complen);

        /* uncompress */
        printf("Inflating...\n");
        unclen2 = inflatemem(buffer, ptr2);

        /* verify uncompressed length */
        if (unclen2 != (unsigned) unclen) {
                printf("Uncompressed size does not match in the header : %d bytes but when deflating %d bytes\n",unclen,unclen2);
                return 0;
        }

        /* verify CRC */
        printf("Calculating CRC...\n");
        if (crc32(crc32(0L, Z_NULL, 0), buffer, unclen2) != crc) {
                printf("CRC mismatch");
                return 0;
        }

        /* return number of uncompressed bytes */
        return unclen2;
}

/*
** Get a filename from standard input.
*/
char* get_fname(void)
{
        
        static char filename[100];
        unsigned len;
        return NULL;
        fgets(filename, sizeof(filename), stdin);
        len = strlen(filename);
        if (len >= 1 && filename[len - 1] == '\n')
                filename[len - 1] = '\0';
        return filename;
}


void version()
{
  printf("gunzip %s\n",VERSION);
}

void usage()
{
  printf("usage:\n");
  printf("gunzip filein\n");
  return;
}

unsigned char getopts(char *arg)
{
  // 2: arg is not an option
  if (arg[0]!='-') return 2;
  if (strcmp(arg,"--version")==0 || strcmp(arg,"-v")==0) 
  {
    version_opt=1;
    return 0;
  }
  
  if (strcmp(arg,"--help")==0 || strcmp(arg,"-h")==0) 
  {
    help_opt=1;
    return 0;
  }  

  return 1;
  
}

    
int main(int argc,char *argv[])
{
    
  FILE* fp;
  unsigned int length;
  unsigned int nb_write;
  unsigned char i,ret,found_a_folder_in_arg_found=0,start=1;
  static unsigned char destfilename[9];

  if (argc==2 || argc==3)
  {
    for (i=1;i<argc;i++)
    {
      ret=getopts(argv[i]);
      if (ret==1) 
      {
        //this is a parameter but not recognized
        usage();
        return 1;
      }
      if (ret==2) 
      {
        //theses are to stop if we have 2 folders on commands line, in the future it will bepossible
        if (found_a_folder_in_arg_found==0) 
            found_a_folder_in_arg_found=1;
        else
        {
          // here we found 2 folders on the command line
          usage();
          return 1;
        }
      }
    }
  }
    else
    {
     usage();
     return 1;
    }

  if (version_opt==1)
  {
    version();
    return 0;
  }
  
  if (help_opt==1)
  {
    usage();
    return 0;
  }  
  
 //   unpack();


#ifdef __CC65__
        /* allow user to read exit messages */
        if (doesclrscrafterexit()) {
                atexit((void (*)) getchar);
        }
#endif /* __CC65__ */

        printf("This version can unzip only file smaller than %d bytes.",GZIP_BUFFER);
        printf("File : %s\n",argv[1]);
      
        for (i=0;i<strlen(argv[1]);i++)
        {
          //printf("%d X%c\n",i,argv[1][i]);
          if (argv[1][i]=='\0') break;
          if (argv[1][i]=='.') break;
          
          destfilename[i]=argv[1][i];
        }
       // i++;
        destfilename[i]='\0';
        printf("Output filename :  %s\n",destfilename);
      
        fp = fopen(argv[1], "r");
        if (fp==NULL) {
                printf("Can't open GZIP file\n");
                return 1;
        }
        length = fread(buffer, GZIP_BUFFER, 1, fp);
        printf("open %d bytes read\n",length);
        fclose(fp);
        if (length == sizeof(buffer)) {
                printf("File is too long\n");
                return 1;
        }

        /* decompress */
        length = uncompress_buffer(length);
        if (length == 0)
                return 1;

              
        /* write uncompressed file */
        //printf("Uncompressed file name:\n");
        fp = fopen(destfilename, "wb");
        if (!fp) {
                printf("Can't create output file\n");
                return 1;
        }
        nb_write=fwrite(buffer, 1, length, fp);
        if (nb_write != length) {
               printf("Error while writing output file writing %u bytes, but fwrite returns %u bytes\n",length,nb_write);
                return 1;
        }
        fclose(fp);

        printf("Ok.\n");

  
  return(0);
}

