/* UTF-8 checker and conditioner
 * Simeon Warner - 10July2001 
 *
 * I assume that the most likely cause of errors is inclusion of non-UTF8
 * bytes from various 8-bit character sets. Thus, any attempt to read a
 * sequence of continuation bytes that finds an invalid byte will result
 * in the termination of the attempt to read the multi-byte character. The
 * hope is that this will avoid an error having run-on effects which might
 * interfere with XML markup.
 *
 * Bugs:
 * - no protection against overflow of byte, character and line counters
 *   (unsigned long int). Will result in incorrect output messages.
 * - doesn't check for multibyte character 0 representation
 *
 * [CVS: $Id: utf8conditioner.c,v 1.2 2001/07/17 01:58:02 simeon Exp $]
 */

#include <stdio.h>
#include <stdlib.h> /* for strtoul() */
#include <string.h>
#include <unistd.h> /* for getopt() */ 

#define MAX_BAD_CHAR 100
  
extern char *optarg;

int main (int argc, char* argv[]) {
  int ch;
  int byte[5];
  int contBytes;
  unsigned long int bytenum=0;   /* count of bytes read */
  unsigned long int charnum=0;   /* count of characters read */
  unsigned long int linenum=1;   /* count of lines */
  unsigned int unicode;
  char error[200];
  char buf[50];
  int j;
  int substituteChar = '?';
  int quiet=0;
  
  int badChar=0;
  unsigned int badChars[MAX_BAD_CHAR];
  badChars[badChar++] = '\f';
  badChars[badChar++] = '\t';
  badChars[badChar++] = 6;
  badChars[badChar]   = '\0'; /* terminator */

  /*
   * Read any options
   */
  while ((j=getopt(argc,argv,"hqb:xs:"))!=EOF) {
    switch (j) {
      case '?':
      case 'h': 
        fprintf(stderr,"usage: %s [-q] [-b char] [-x] [-s char] [-h]\n"
          "  -q   quiet, no output messages\n"   
          "  -b   add Unicode character char to list of bad characters (decimal, 0octal, 0xhex)\n"  
          "  -x   remove characters not allowed in UTF-8 XML streams\n"
          "  -s   change charecter substituted for bad characters (currently '%c')\n"
          "  -h   this help\n\n", argv[0], substituteChar);  
        exit(1);
      case 'q':
        quiet=1;
        break;
      case 'b':
        badChars[badChar++]=(int)strtoul(optarg,NULL,0);
        badChars[badChar]='\0';
        break;
      case 's':
        substituteChar = optarg[0];
        break;
    }
  }

  /*
   * Got through input character by character, check for correct use of
   * UTF-8 continuation bytes, check for unicode character validity
   */
  while ((ch=getc(stdin))!=EOF) {
    bytenum++; charnum++;
    if (ch=='\n') { linenum++; }
    error[0]='\0'; /* clear error string */
    if (!(ch&0x80)) {
      /* one byte char    0000 0000-0000 007F   0xxxxxxx */
      /* one byte is the only legal way to specify null */
      contBytes=0;
    } else if ((ch&0xE0)==0xC0) {
      /* 0000 0080-0000 07FF   110xxxxx 10xxxxxx */
      contBytes=1;
    } else if ((ch&0xF0)==0xE0) {
      /* 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx */
      contBytes=2;
    } else if ((ch&0xF8)==0xF0) {
      /* 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
      contBytes=3;
    } else if ((ch&0xFC)==0xF8) {
      /* 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
      contBytes=4;
    } else if ((ch&0xFE)==0xFC) {
      /* 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx */
      contBytes=5;
    } else {
      if (!quiet) { sprintf(error,"illegal byte: #%02x", ch); }
      contBytes=0;
    }
    byte[0]=ch;
    
    unicode=(ch&0x7F);
    for (j=1; j<=contBytes; j++) {
      if ((ch=getc(stdin))!=EOF) {
        bytenum++;
        if ((ch&0xA0)!=0x80) {
          /* doesn't match 10xxxxxx */
	  sprintf(buf,"byte %d isn't continuation: #%02x", (j+1), ch);
          strcat(error, buf);
	  ungetc(ch,stdin);
	  bytenum--;
	  break;
        }
	byte[j]=ch;
        unicode = (unicode << 6) + (ch&0x3F);
      } else {
        sprintf(buf,"premature EOF at byte %ld, should be byte %d of character",
	        bytenum, j);
        strcat(error,buf);
      }
    }

    if (error[0]=='\0') {
      j=-1;
      while (badChars[++j]!=0) {
        if (badChars[j]==unicode) {
          sprintf(error,"bad character #%04x", unicode);
          break;
        }
      } 
    }

    if (error[0]!='\0') {
      if (!quiet) {
        fprintf(stderr,"Line %ld, byte %ld, char %ld: %s\n",
                linenum,bytenum,charnum,error);
      }
      putc(substituteChar,stdout);
    } else {
      for (j=0; j<=contBytes; j++) {
        putc(byte[j],stdout);
      }
    }   
  }
  exit(0);
}
