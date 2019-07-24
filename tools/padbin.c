/* padbin.c
   pad a binary to a given size boundary

Copyright (C) 2002  Damian Yerrick

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/


#include <stdio.h>
#include <stdlib.h>

char zeroz[1024];

int main(int argc, char **argv)
{
  FILE *fp;
  unsigned long overage;
  unsigned long factor;

  /* clear to 0xff for faster flash writing */
  for(factor = 0; factor < 1024; factor++)
    zeroz[factor] = 0xff;

  if(argc != 3)
  {
    fputs("pads a binary file to an integer multiple of a given number of bytes\n"
          "syntax: padbin FACTOR FILE\n"
          "FACTOR can be decimal (e.g. 256), octal (e.g. 0400), or hex (e.g. 0x100)\n", stderr);
    return 1;
  }

  factor = strtoul(argv[1], NULL, 0);
  if(factor < 2)
  {
    fputs("error: FACTOR must be greater than or equal to 2\n", stderr);
    return 1;
  }

  fp = fopen(argv[2], "rb+");
  if(!fp)
  {
    fputs("could not open", stderr);
    perror(argv[2]);
    return 1;
  }

  /* find the amount the file has over the limit */
  fseek(fp, 0, SEEK_END);
  overage = ftell(fp) % factor;
  if(overage != 0)
  {
    unsigned long extension = factor - overage;

    while(extension >= 1024)
    {
      fwrite(zeroz, 1, 1024, fp);
      extension -= 1024;
    }
    if(extension >= 0)
    {
      fwrite(zeroz, 1, extension, fp);
    }
  }
  fclose(fp);
  return 0;
}
