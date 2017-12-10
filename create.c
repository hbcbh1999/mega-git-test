#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Note: This is not very cleaned up code, (I wrote it in 5 minutes),
// especially the the `filebuf` size computation vs `snprintf()`
// is a quick hack.

int main(int argc, char const *argv[])
{
  const int64_t numfiles = 9000000 / 10;
  const int64_t total_num_lines = 2000000000 / 10;
  const int64_t chars_per_line = 100;

  const int64_t lines_per_file = total_num_lines / numfiles;
  const int64_t chars_per_file = lines_per_file * chars_per_line;

  char filebuf[lines_per_file][chars_per_line];

  printf("Creating %ld files\n  %ld lines each\n  %ld total lines\n  %ld chars per line\n  %f total GB\n",
    numfiles,
    lines_per_file,
    lines_per_file * numfiles,
    chars_per_line,
    (lines_per_file * chars_per_line * numfiles) * 1e-9);

  for (int64_t i = 0; i < numfiles; i++)
  {
      for (int64_t l = 0; l < lines_per_file; l++)
      {
        sprintf(&filebuf[l][0], "file %010ld line %04ld - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt\n", i, l + 1);
        filebuf[l][chars_per_line-1] = '\n';
      }
      filebuf[lines_per_file][-1] = '\0';

      char filename[40];
      sprintf(filename, "repo/file-%010ld", i);
      FILE * f = fopen(filename, "w");
      if (!f)
      {
        perror(filename);
        exit(1);
      }
      if (fputs((const char *) filebuf, f) < 0) {
        perror("fputs");
        exit(1);
      }
      fclose(f);
  }
  return 0;
}
