/*
catbin.c
Concatenate two or more binary files into one
By Damian Yerrick


Copyright 2001 Damian Yerrick

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


/* Rationale

What is catbin?

  catbin is a program to take multiple binary files and produce the
  concatenation of those files.

Why not just use DOS copy?

  MinGW make doesn't seem to be able to call copy /b properly.

Why not just use cat?

  Neither MinGW nor the minimal Cygwin distribution included with
  devkit advance includes a port of GNU fileutils to win32.

DJGPP has fileutils.  Why not use DJGPP's cat?

  DJGPP and Windows 2000 have issues with each other.

Why not require the full Cygwin distribution?

  I prefer MinGW to Cygwin because MinGW is a smaller download.  I
  don't expect all users of my software to have upwards of $200,000
  to move their families to an area where broadband Internet access
  is affordable.

*/

#include <stdio.h>
#include <stdlib.h>

const char syntax_help[] =
"catbin 0.1 by Damian Yerrick: concatenates binary files\n"
"usage: catbin INFILE [INFILE...] OUTFILE\n";

int main(int argc, char **argv)
{
  char buf[1024];
  size_t n_got;
  int arg;
  FILE *infile, *outfile;

  if(argc < 3)
    {
      fputs(syntax_help, stderr);
      return 1;
    }

  outfile = fopen(argv[argc - 1], "wb");
  if(!outfile)
    {
      fputs("catbin could not open output file ", stderr);
      perror(argv[argc - 1]);
      return 1;
    }

  for(arg = 1; arg < argc - 1; arg++)
    {
      infile = fopen(argv[arg], "rb");
      if(!infile)
	{
	  fclose(outfile);
	  fputs("catbin could not open input file ", stderr);
	  perror(argv[arg]);
	  return 1;
	}
      while((n_got = fread(buf, sizeof(char), sizeof(buf), infile)) > 0)
	{
	  size_t n_written;

	  n_written = fwrite(buf, sizeof(char), n_got, outfile);
	  if(n_written < n_got)
	    {
	      fclose(infile);
	      fclose(outfile);
	      fprintf(stderr, "%lu bytes read but %lu bytes written\n",
		      (unsigned long)n_got, (unsigned long)n_written);
	      fputs("catbin could not write to input file ", stderr);
	      perror(argv[argc - 1]);
	      return 1;
	    }
	}
    }

  fclose(outfile);
  return 0;
}
