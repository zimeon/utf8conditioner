/* UTF-8 checker and conditioner
 * Simeon Warner - 10July2001 
 * [CVS: $Id: utf8conditioner.c,v 1.1 2001/07/10 18:46:14 simeon Exp $]
 */

#include <stdio.h>

char* checkContinuation(int ch) {
  if ((ch&0xA0)==0x80) {
    /* okay, matches 10xxxxxx */
    return((char*)NULL);
  }
  return("Bad continuation byte");
}

main (int argc, char* argv[]) {
  unsigned int ch;
  unsigned int chr[5];
  unsigned int *chp;
  unsigned long int numbytes=0;  /* count of bytes read */
  unsigned long int numchars=0;  /* cound of characters read */
  char *error;

  while (ch=getc(stdin)) {
    numbytes++; numchars++;
    chr[0]=0;
    error=(char*)NULL;
    if (!(ch&0x80)) {
      /* on byte char    0000 0000-0000 007F   0xxxxxxx */
      /* only legal way to specify null */
    } else if ((ch&0xE0)==0xC0) {
      /* 0000 0080-0000 07FF   110xxxxx 10xxxxxx */
    } else if ((ch&0xF0)==0xE0) {
      /* 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx */
    } else if ((ch&0xF8)==0xF0) {
      /* 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    } else if ((ch&0xFC)==0xF8) {
      /* 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    } else if ((ch&0xFE)==0xFC) {
      /* 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx */
    } else {
      error="Illegal byte ( )";
    }

    if (error) {
      fprintf(stderr,error);
      ch='?';
    }
    putc(ch,stdout);
    chp=chr;
    while (*chp!=0) {
      putc(*chp++,stdout);
    }   
  }
  fprintf(stderr,"Done.\n");
}
