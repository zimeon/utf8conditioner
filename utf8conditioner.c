/* UTF-8 checker and conditioner
 * Copyright (C) 2001-2003 Simeon Warner - simeon@cs.cornell.edu
 *
 * utf8conditioner is supplied under the GNU Public License and comes 
 * with ABSOLUTELY NO WARRANTY; see COPYING for more details.
 *
 * utf8conditioner includes software developed by the University of
 * California, Berkeley and its contributors. (getopt)
 *
 * Designed for use with Open Archives Initiative (OAI, 
 * see: http://www.openarchives.org/) harvesting software. Aims to 
 * `fix' bad codes in UTF-8 encoded XML so that XML parsers will
 * be able to parse it (albeit with some corruption introduced by
 * substitution of dummy characters in place of illegal codes).
 *
 * I assume that the most likely cause of errors is inclusion of non-UTF-8
 * bytes from various 8-bit character sets. Thus, any attempt to read a
 * sequence of continuation bytes that finds an invalid byte will result
 * in the termination of the attempt to read the multi-byte character. Each
 * valid single byte will be written out, invalid single bytes will be 
 * replaced with a dummy character. The hope is that this will avoid an 
 * error having run-on effects which might interfere with XML markup.
 * (Option -m changes this behaviour to replace with invalid multi-byte
 * with a single dummy character.)
 *
 * Bugs:
 * - no protection against overflow of byte, character and line counters
 *   (unsigned long int). Will result in incorrect output messages.
 * - doesn't handle UTF-8 encoding of UTF-16/UCS-4. 
 *
 * [CVS: $Id: utf8conditioner.c,v 1.9 2003/01/14 19:31:09 simeon Exp $]
 */

#define PROGRAM_NOTICE "\
utf8conditioner version 14Jan2003. Copyright (C) 2001-2003 Simeon Warner\n\
\n\
utf8conditioner is supplied under the GNU Public License and comes\n\
with ABSOLUTELY NO WARRANTY; run with -L flag for more details.\n\
utf8conditioner includes software developed by the University of\n\
California, Berkeley and its contributors. (getopt)\n"

#define GNU_GPL_NOTICE1 "\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\n"

#define GNU_GPL_NOTICE2 "\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program (in the file COPYING); if not, visit\n\
http://www.gnu.org/licenses/gpl.html or write to the Free Software\n\
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA\n"

#include <stdio.h>
extern int snprintf(char *str, size_t size, const char *format, ...);
#include <string.h>
#include <stdlib.h> /* for strtoul() */
#include "getopt.h" /* for getopt(), could use unistd on Unix */ 

#define MAX_BAD_CHAR 100

extern const char *optarg;
extern const int optind;
extern int getopt (int argc, char *const *argv, const char *shortopts);

int validUnicodeChar(unsigned int ch);
int validXMLChar(unsigned int ch);
int validUTF8Char(unsigned int ch);

int main (int argc, char* argv[]) {
  int j,k;
  int ch;
  char error[200];                /* place to build error string */
  char buf[50];                   /* tmp used when building error string */ 

  int byte[6];                    /* bytes of UTF-8 char */
  int contBytes;                  /* number of continuation bytes (0-5) */
  unsigned long int bytenum=0;    /* count of bytes read */
  unsigned long int charnum=0;    /* count of characters read */
  unsigned long int linenum=1;    /* count of lines */
  unsigned int unicode;           /* Unicode character represented by UTF-8 */
 
  int maxErrors=1000;             /* max number of error messages to print */
  int numErrors=0;                /* count of errors */   
  int quiet=0;                    /* quiet option */
  int check=0;                    /* check only option */
  int substituteChar = '?';       /* substitute for bad characters */
  int checkXMLChars=0;            /* XML checks option */
  int checkOverlong=1;            /* Check for overlong character encodings */
  int badMultiByteToMultiChar=0;  /* -m option */

  int badChar=0;                  /* variables for bad characters option */ 
  unsigned int badChars[MAX_BAD_CHAR];
  int highestCharInNBytes[6];

  badChars[badChar] = 0;          /* terminator */

  highestCharInNBytes[0]=0x7F;
  highestCharInNBytes[1]=0x7FF;
  highestCharInNBytes[2]=0xFFFF;
  highestCharInNBytes[3]=0x1FFFFF;
  highestCharInNBytes[4]=0x3FFFFFF;
  highestCharInNBytes[5]=0x7FFFFFFF;

  /*
   * Read any options
   */
  while ((j=getopt(argc,argv,"hH?qce:b:s:xmlL"))!=EOF) {
    switch (j) {
      case 'h': 
      case 'H':
      case '?':
        /* string split to meet ISO C89 requirement of <=509 chars */
        fprintf(stderr, PROGRAM_NOTICE);
        fprintf(stderr,"\n\
usage: %s [-q] [-c] [-e num] [[-b char]] [-x] [-s char] [-h]\n\n\
Takes UTF-8 input from stdin, writes processed UTF-8 to stdout\n\
and errors/warnings to stderr.\n\n", argv[0]);
        fprintf(stderr,"\
  -c   just check, no output of XML to stdout\n\
  -q   quiet, don't output messages to stderr\n\
  -x   XML check, remove codes not allowed in UTF-8 XML streams\n\
       (valid codes are: #x9 | #xA | #xD and the ranges\n\
        [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF])\n\n");
        fprintf(stderr,"\
  -b   add Unicode character code to list of bad codes\n\
       (specified as decimal, 0octal or  0xhex),\n\
       use multiple times at add multiple characters\n\
  -e   maximum number of error messages to print\n\
       (currently %d, 0 for unlimited)\n\
  -l   lax - don't check for overlong encodings\n\
  -m   replace invalid multi-byte sequences with multiple dummy characters\n\
  -s   change character substituted for bad codes (currently '%c')\n\n\
  -L   display information about license\n\
  -h   this help\n\n", maxErrors, substituteChar);  
        exit(1);
      case 'q':
        quiet=1;
        break;
      case 'c':
        check=1;
        break; 
      case 'b':
        if (badChar>=(MAX_BAD_CHAR-1)) {
          fprintf(stderr,"Too many bad codes specified (limit %d), aborting!\n", MAX_BAD_CHAR);
          exit(1); 
        } 
        badChars[badChar++]=(int)strtoul(optarg,NULL,0);
        badChars[badChar]=0;
        break;
      case 'e':
        maxErrors=(int)strtoul(optarg,NULL,0);
        break;
      case 's':
        substituteChar = optarg[0];
        break;
      case 'm':
        badMultiByteToMultiChar=1;
        break;
      case 'l':
        checkOverlong=0;
        break;
      case 'x':
        checkXMLChars=1;
        break; 
      case 'L':
        fprintf(stderr,GNU_GPL_NOTICE1);
        fprintf(stderr,GNU_GPL_NOTICE2);
        exit(1);
    }
  }

  /*
   * Barf if anything on command line left unread (probably an attempt to 
   * specify a file name instead of using stdin)
   */
  if (argc>optind) {
    fprintf(stderr,"Unknown parameters specified on command line (-h for help), aborting!\n");
    exit(1); 
  }

  /*
   * Go through input code (character) by code and check for correct use 
   * of UTF-8 continuation bytes, check for unicode character validity
   */
  while ((ch=getc(stdin))!=EOF) {
    bytenum++; charnum++;
    if (ch=='\n') { linenum++; }
    error[0]='\0'; /* clear error string */
    unicode=ch;
    if ((ch&0x80)==0) {
      /* one byte char    0000 0000-0000 007F   0xxxxxxx */
      contBytes=0;
    } else if ((ch&0xE0)==0xC0) {
      /* 0000 0080-0000 07FF   110xxxxx 10xxxxxx */
      contBytes=1;
      unicode=(ch&0x1F);
    } else if ((ch&0xF0)==0xE0) {
      /* 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx */
      contBytes=2;
      unicode=(ch&0x0F);
    } else if ((ch&0xF8)==0xF0) {
      /* 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
      contBytes=3;
      unicode=(ch&0x07);
    } else if ((ch&0xFC)==0xF8) {
      /* 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
      contBytes=4;
      unicode=(ch&0x03);
    } else if ((ch&0xFE)==0xFC) {
      /* 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx */
      contBytes=5;
      unicode=(ch&0x01);
    } else {
      if (!quiet) { snprintf(error,sizeof(error),"illegal byte: 0x%02X", ch); }
      contBytes=0;
    }
    byte[0]=ch;
    
    for (j=1; j<=contBytes; j++) {
      if ((ch=getc(stdin))!=EOF) {
        bytenum++;
	byte[j]=ch;
        if ((ch&0xC0)!=0x80) {
          /* doesn't match 10xxxxxx */
	  snprintf(buf,sizeof(buf),"byte %d isn't continuation:",(j+1));
          strncat(error, buf, sizeof(error));
          for (k=0; k<=j; k++) {
            snprintf(buf,sizeof(buf)," 0x%02X", byte[k]);
            strncat(error, buf, sizeof(error));
          }
	  snprintf(buf,sizeof(buf),", restart at 0x%02X",ch);
          strncat(error, buf, sizeof(error));
	  ungetc(ch,stdin);
	  bytenum--;
	  break;
        }
        unicode = (unicode << 6) + (ch&0x3F);
      } else {
        snprintf(buf,sizeof(buf),"premature EOF at byte %ld, should be byte %d of code", bytenum, j);
        strncat(error,buf,sizeof(error));
        break;
      }
    }

    if (error[0]=='\0') {
      if (checkXMLChars && !validXMLChar(unicode)) {
        snprintf(error,sizeof(error),"code not allowed in XML: 0x%04X",unicode);
      } else { 
        for (k=0; badChars[k]!=0; k++) {
          if (unicode==badChars[k]) {
            snprintf(error,sizeof(error),"bad code: 0x%04X", unicode);
            break;
          }
        }
      } 
    }
  
    /* check for illegal Unicode chars */
    if ((error[0]=='\0') && !validUTF8Char(unicode)) {
      snprintf(buf,sizeof(buf),"illegal UTF-8 code: 0x%04X",unicode);
      strncat(error,buf,sizeof(error));
    }

    /* check for overlong encodings if no error already */
    if ((error[0]=='\0') && checkOverlong && contBytes>0 
                         && (unicode<=highestCharInNBytes[contBytes-1])) {
      snprintf(buf,sizeof(buf),"overlong encoding, not safe: 0x%04X",unicode);
      strncat(error,buf,sizeof(error));
    }
 

    if (error[0]!='\0') {
      numErrors++;
      if (badMultiByteToMultiChar && j>1) {
        /* now test individual bytes of bad multibyte char, will always
         * make substitution for at least the first char.
         */
        for (k=0; k<=j; k++) {
          if (byte[k]>0x7F || (checkXMLChars && !validXMLChar((unsigned int)byte[j]))) {
            byte[k]=substituteChar;
          }
        }
	snprintf(buf,sizeof(buf),", substituted ");
        strncat(error, buf, sizeof(error));
        for (k=0; k<=(j-1); k++) {
          snprintf(buf,sizeof(buf)," 0x%02X", byte[k]);
          strncat(error, buf, sizeof(error));
        }
      } else {
        /* substitute one char for all bytes of bad multibyte char
         */
        byte[0]=substituteChar;
        j=1;
	snprintf(buf,sizeof(buf),", substituted 0x%02X", byte[0]);
        strncat(error, buf, sizeof(error));
      }

      if (!quiet && (numErrors<=maxErrors || maxErrors==0)) {
        fprintf(stderr,"Line %ld, char %ld, byte %ld: %s\n",
                linenum,charnum,bytenum,error);
      }
      contBytes=j-1;
    }

    if (!check) { 
      for (k=0; k<=contBytes; k++) {
        putc(byte[k],stdout);
      }
    }
  }

  if (!quiet && (numErrors>maxErrors) && (maxErrors!=0)) {
    fprintf(stderr,"%d additional errors not reported.\n", (numErrors-maxErrors));
  }
  exit(0);
}


/* Returns true unless the character is one of a small set of codes
 * that do not represent legal characters in Unicode.
 *
 * From Unicode 3.2 (http://www.unicode.org/unicode/reports/tr28/#3_1_conformance) 
 * U+D800..U+DFFF ill-formed 
 *
 * Ux10FFFF is highest UTF-8 char
 */
int validUTF8Char(unsigned int ch) {
  return((ch<0xD800 || ch>0xDFFF) && ch<=0x10FFFF);
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
