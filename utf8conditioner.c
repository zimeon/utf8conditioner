/* UTF-8 checker and conditioner
 * Simeon Warner - 10July2001 
 *
 * Designed for use with Open Archive Initiative (OAI, 
 * see: http://www.openarchives.org/) harvesting software. Aims to 
 * `fix' bad characters in UTF-8 encode XML so that XML parsers will
 * be able to parse it (albeit with some corruption introduced by
 * substitution of dummy characters in place of invalid ones).
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
 * - doesn't handle UTF-8 encoding of UTF-16/UCS-4. 
 *
 * [CVS: $Id: utf8conditioner.c,v 1.3 2001/07/19 16:53:40 simeon Exp $]
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* for strtoul() */
#include "getopt.h" /* for getopt(), could use unistd on Unix */ 

#define MAX_BAD_CHAR 100
  
extern const char *optarg;
extern int getopt (int argc, char *const *argv, const char *shortopts);

int validXMLChar(unsigned int ch);

int main (int argc, char* argv[]) {
  int j;
  int ch;
  char error[200];               /* place to build error string */
  char buf[50];                  /* tmp used when building error string */ 

  int byte[6];                   /* bytes of UTF-8 char */
  int contBytes;                 /* number of continuation bytes (0-5) */
  unsigned long int bytenum=0;   /* count of bytes read */
  unsigned long int charnum=0;   /* count of characters read */
  unsigned long int linenum=1;   /* count of lines */
  unsigned int unicode;          /* Unicode character represented by UTF-8 */
 
  int maxErrors=100;             /* max number of error messages to print */
  int numErrors=0;               /* count of errors */   
  int quiet=0;                   /* quiet option */
  int substituteChar = '?';      /* substitute for bad characters */
  int checkXMLChars=1;           /* XML checks option */

  int badChar=0;                 /* variables for bad characters option */ 
  unsigned int badChars[MAX_BAD_CHAR];
  badChars[badChar] = '\0'; /* terminator */

  /*
   * Read any options
   */
  while ((j=getopt(argc,argv,"hqb:e:xs:"))!=EOF) {
    switch (j) {
      case '?':
      case 'h': 
        fprintf(stderr,"usage: %s [-q] [-b char] [-x] [-s char] [-h]\n"
          "  -q   quiet, no output messages\n"   
          "  -b   add Unicode character char to list of bad characters (decimal, 0octal, 0xhex)\n"  
          "  -e   number of error messages to print (currently %d, 0 for unlimited)\n"
          "  -x   do NOT remove characters not allowed in UTF-8 XML streams\n"
          "  -s   change character substituted for bad characters (currently '%c')\n"
          "  -h   this help\n\n", argv[0], maxErrors, substituteChar);  
        exit(1);
      case 'q':
        quiet=1;
        break;
      case 'b':
        if (badChar>=(MAX_BAD_CHAR-1)) {
          fprintf(stderr,"Too many bad characters specified, limit %d.\n", 
                         MAX_BAD_CHAR);
          exit(1); 
        } 
        badChars[badChar++]=(int)strtoul(optarg,NULL,0);
        badChars[badChar]='\0';
        break;
      case 'e':
        maxErrors=(int)strtoul(optarg,NULL,0);
        break;
      case 'x':
        checkXMLChars=0;
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
      if (!quiet) { sprintf(error,"illegal byte: 0x%02x", ch); }
      contBytes=0;
    }
    byte[0]=ch;
    
    unicode=(ch&0x7F);
    for (j=1; j<=contBytes; j++) {
      if ((ch=getc(stdin))!=EOF) {
        bytenum++;
        if ((ch&0xA0)!=0x80) {
          /* doesn't match 10xxxxxx */
	  sprintf(buf,"byte %d isn't continuation: 0x%02x", (j+1), ch);
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
      if (checkXMLChars && !validXMLChar(unicode)) {
        sprintf(error,"character not allowed in XML: 0x%04x",unicode);
      } else { 
        while (badChars[++j]!=0) {
          if (unicode==badChars[j]) {
            sprintf(error,"bad character: 0x%04x", unicode);
            break;
          }
        }
      } 
    }

    if (error[0]!='\0') {
      numErrors++;
      if (!quiet && (numErrors<=maxErrors || maxErrors==0)) {
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

  if (!quiet && (numErrors>maxErrors) && (maxErrors!=0)) {
    fprintf(stderr,"%d additional errors not reported.\n",
                   (numErrors-maxErrors));
  }
  exit(0);
}



/* From http://www.w3.org/TR/2000/REC-xml-20001006 
 * (sec 2.2, extracted 16July2001)
 *
 * Legal characters are tab, carriage return, line feed, and the legal 
 * characters of Unicode and ISO/IEC 10646. The versions of these standards 
 * cited in A.1 Normative References were current at the time this document 
 * was prepared. New characters may be added to these standards by amendments 
 * or new editions. Consequently, XML processors must accept any character 
 * in the range specified for Char. The use of "compatibility characters", 
 * as defined in section 6.8 of [Unicode] (see also D21 in section 3.6 of 
 * [Unicode3]), is discouraged.]
 *
 * Character Range
 * Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] |
 *                            [#x10000-#x10FFFF]
 * [end excerpt]
 *
 * We are not here concerned with the range #x10000-#x10FFFF since these
 * are used only for UTF-16 endcoding of UCS-4 (64bit characters). This
 * code assumes UTF-8 encoding of UCS-2 (32 bit characters) and would need
 * modification to handle UTF-8 encoding of UTF-16.
 */
int validXMLChar(unsigned int ch) {
  return(ch==0x09 || ch==0x0A || ch==0x0D ||
         (ch>=0x20 && ch<=0xD7FF) ||
         (ch>=0xE000 && ch<=0xFFFD)); 
} 


/***end***/
