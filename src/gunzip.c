#include <string.h>
#include <stdio.h>
#include "version.h"
#include <unistd.h>
#include <conio.h>
#include <stddef.h>

extern unsigned __fastcall__ inflatememfromfptofp (unsigned char* dest, const unsigned char* source, FILE *fp_input , FILE *fp_output);

extern void orix_system();

// $D0D3 : Offset qui charge l'octet en mémoire

// bs $3C25

// 3C7E au chargement $FA

// bsm $3C81 
// 3C81 au runtime, cela écrit $02

/*
00000000: 0100 6f72 6902 0017 0400 0000 b220 0008  ..ori........ ..
00000010: b128 0008 ba8e 7228 209b 2820 9424 20bf  .(....r( .( .$ .
00000020: 1920 f919 ae72 289a a219 bd74 2895 b0ca  . ...r(....t(...
00000030: 10f8 6020 fa21 a010 2030 23a2 00ad 8f28  ..` .!.. 0#....(
00000040: c91f 20ca 20f0 034c 4e08 a200 ad90 28c9  .. . ..LN.....(.
*/

unsigned char version_opt = 0;
unsigned char help_opt = 0;
unsigned char p_opt = 0;


#define READ_BUFFER    2000
#define DEFLATE_BUFFER 2000

#include <zlib.h>

#ifdef __CC65__
#include <stdlib.h>
#include <cc65.h>
#endif

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
unsigned char buffer[DEFLATE_BUFFER];

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
unsigned uncompress_buffer(unsigned complen, FILE *fp_input, FILE *fp_output)
{
    unsigned char* ptr;
    unsigned long crc;
    unsigned long unclen;
    void* ptr2;
    unsigned unclen2;
    int offset;

    /* check GZIP signature */
    if (buffer[0] != 0x1f || buffer[1] != 0x8b) {
        printf("Not GZIP format\n");
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

    /* skip extra field, file name, comment and crc */
    ptr = buffer + 10;
    offset = 10;
    if (buffer[3] & FEXTRA) {
        ptr = buffer + 12 + GET_WORD(buffer + 10);
        offset = 12 + GET_WORD(buffer + 10);
    }

    // If the file name is present, skip it.
    if (buffer[3] & FNAME) {
        while (*ptr++ != 0)
            offset++;
        offset ++; // to skip null byte
    }

    // If the comment is present, skip it.
    if (buffer[3] & FCOMMENT)
        while (*ptr++ != 0)
            offset++;

    if (buffer[3] & FHCRC) {
        ptr += 2;
        offset += 2;
    }

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
    //memmove(ptr2, ptr, complen);
    /* uncompress */


    printf("Inflating... %d %d %x %x\n", complen, offset, buffer, ptr2);
    printf("L'adresse de buffer est : %p de ptr2 %p\n", (void *)&buffer, (void *)ptr);

    // Seek to reach end of the header
    fseek( fp_input, offset , SEEK_SET);
    // FIXME : ptr2 is not used, but we need to pass it to the function
    // to be compatible with the original zlib function.
    unclen2 = inflatememfromfptofp(buffer, ptr2, fp_input, fp_output);
    /* verify uncompressed length */
    if (unclen2 != (unsigned) unclen) {
        printf("!WARNING! Uncompressed size does not match in the header : %d bytes but when deflating %d bytes\n", unclen, unclen2);
        //return 0;
    }
    /* verify CRC */
    puts("Calculating CRC...\n");
    if (crc32(crc32(0L, Z_NULL, 0), buffer, unclen2) != crc) {
        puts("Warning CRC mismatch");
        return 0;
    }
    /* return number of uncompressed bytes */
    return unclen2;
}

/*
** Get a filename from standard input.
*/
char* get_fname(void) {
    static char filename[100];
    unsigned len;
    return NULL;
    fgets(filename, sizeof(filename), stdin);
    len = strlen(filename);
    if (len >= 1 && filename[len - 1] == '\n')
            filename[len - 1] = '\0';
    return filename;
}


void version() {
    printf("gunzip %s\nFrom cc65 (Fuzik), gunzip deflate\n Orix port : Jede\n", VERSION);
}

void usage() {
    puts("usage:\n");
    puts("gunzip filein\n");
    return;
}

unsigned char getopts(char *arg) {
    // 2: arg is not an option
    if (arg[0] != '-') return 2;
    if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
        version_opt = 1;
        return 0;
    }

    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") ==0) {
        help_opt = 1;
        return 0;
    }

    return 1;

}

unsigned int length;

int main(int argc, char *argv[]) {

    FILE *fp;
    FILE *fp_input;
    FILE *fp_output;


    unsigned char i, ret, found_a_folder_in_arg_found = 0,start = 1;
    char destfilename[9];



    // bs $4A1
    // 4AE : 

    if (argc == 2 || argc == 3) {
        for (i=1; i < argc; i++) {
            ret = getopts(argv[i]);

            if (ret == 1) {
            //this is a parameter but not recognized
                usage();
                return 1;
            }

            if (ret == 2) {
                //theses are to stop if we have 2 folders on commands line, in the future it will bepossible
                if (found_a_folder_in_arg_found == 0)
                    found_a_folder_in_arg_found = 1;
                else {
                    // here we found 2 folders on the command line
                    usage();
                    return 1;
                }
            }
        }
    }
    else {
        usage();
        return 1;
    }

    if (version_opt == 1) {
        version();
        return 0;
    }

    if (help_opt == 1) {
        usage();
        return 0;
    }


#ifdef __CC65__
    /* allow user to read exit messages */
    if (doesclrscrafterexit()) {
        atexit((void (*)) getchar);
    }
#endif /* __CC65__ */


    printf("File : %s\n", argv[1]);

    for (i=0; i < strlen(argv[1]); i++) {
        if (argv[1][i] == '\0') break;
        if (argv[1][i] == '.') break;
        destfilename[i] = argv[1][i];
    }
    // Is it tgz ? if yes, add .tar extension
    // Check if it's a tgz ? If yes, in that case, the output file will be .tar
    if (argv[1][i + 1] == 't' && argv[1][i + 2] == 'g' && argv[1][i + 3] == 'z') {
        destfilename[i] = '.';
        i++;
        destfilename[i] = 't';
        i++;
        destfilename[i] = 'a';
        i++;
        destfilename[i] = 'r';
        i++;
    }
    //printf("%c%c%c",argv[1][i],argv[1][i + 1],argv[1][i + 2]);
    destfilename[i] = '\0';
    fp = fopen(destfilename, "r");
    if (fp) {
        printf("destination file %s exists. Overwrite [y/n] (default : n) ?\n", destfilename);
        ret = cgetc();
        if (ret != 'y') {
            puts("Aborted\n");
            return 1;
        }
        unlink(destfilename);
    }
    fclose(fp);

    // open GZIP file
    fp_input = fopen(argv[1], "r");
    if (fp_input == NULL) {
        printf("Can't open GZIP file %s\n", argv[1]);
        return 1;
    }

    // Get file size
    length = fread(buffer, 1, READ_BUFFER, fp_input);
    printf("open %d bytes read\n", length);
    fclose(fp_input);

    // Reset fp
    fp_input = fopen(argv[1], "r");

    // if (length == sizeof(buffer)) {
    //     printf("File is too big\n");
    //     return 1;
    // }

    fp_output = fopen(destfilename, "wb");
    if (!fp_output) {
        puts("Can't create output file\n");
        return 1;
    }

    /* decompress */
    length = uncompress_buffer(length, fp_input, fp_output);
    if (length == 0)
        return 1;
    /* write uncompressed file */

    // $23AF $2399
    // nb_write = fwrite(buffer, 1, length, fp_output);
    // if (nb_write != length) {
    //        printf("Error while writing output file writing %u bytes, but fwrite returns %u bytes\n", length, nb_write);
    //         return 1;
    // }
    fclose(fp_output);

    puts("Ok.\n");
    return 0;
}

