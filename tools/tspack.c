// Illtyd Wynn, 7-27-2026, TSPACK

/*
  A simple tool to read and write packfiles.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ts/packfile/packfile.h>

void handle_packfile_open_error(
  const char* file,
  const TS_PACKFILE_OPEN_RESULT err
);

void print_toc_entry(PTS_PACKFILE_TOC_ENTRY e);

int main(int argc, char** argv) {
  int entry_index;
  int i;
  TS_PACKFILE_OPEN_RESULT open_result;
  TS_PACKFILE packfile;
  int result;
  TS_PACKFILE_TOC_ENTRY toc_entry;

  //
  // Intro and usage.
  //

  packfile_lib_init();
  printf("TSPACK\n");

  if (argc != 2) {
    fprintf(stderr, "usage: tspack PACKFILE_PATH\n");
    exit(-1);
  }

  open_result = packfile_open(argv[1], &packfile);
  if (open_result != TS_PACKFILE_OPEN_SUCCESS) {
    handle_packfile_open_error(argv[1], open_result);
  }

  printf("Packfile size: %lu bytes\n", packfile.toc_sz);
  printf("Num ToC entries: %u\n", packfile.num_toc_entries);

  //
  // Now attempt to read each ToC entry.
  //

  for (i = 0; i < packfile.num_toc_entries; i++) {
    result = packfile_read_toc_entry(
      &packfile,
      i,
      &toc_entry
    );

    if (result == 0) {
      fprintf(stderr, "failed to read %d entry of packfile\n", entry_index);
      close(packfile.fd);
      exit(-1);
    }

    printf("Entry %d:\n", i);
    print_toc_entry(&toc_entry);
    printf("---\n");
  }

  close(packfile.fd);

  return 0;
}

void handle_packfile_open_error(
  const char* file,
  const TS_PACKFILE_OPEN_RESULT err
) {
  printf("Failed to open %s: ", file);

  switch (err) {
    case TS_PACKFILE_OPEN_ERR_FD:
      printf("file not found or cannot be opened\n");
      break;
    case TS_PACKFILE_OPEN_ERR_NOT_A_PACKFILE:
      printf("file is not a packfile\n");
      break;
    case TS_PACKFILE_OPEN_ERR_BAD_TOC_SZ:
      printf("bad ToC size\n");
      break;
  }

  exit(-1);
}

void print_toc_entry(PTS_PACKFILE_TOC_ENTRY e) {
  char name[49];

  //
  // Make sure the filename is NULL-terminated.
  //

  memcpy(name, e->filename, 48);
  name[48] = '\0';

  //
  // Print the filename. This does seem rather odd to check if the first 4 bytes
  // of the filename are 0. But note that this *is* something done in the
  // TimeSplitters code itself. So we will be faithful and do it here too.
  //

  if (*((uint32_t*)e->filename) == 0) {
    printf("name      :   <empty>\n");
  } else {
    printf("name      :   %s\n", e->filename);
  }

  printf("offset    :   %x\n", e->offset);
  printf("size      :   %x\n", e->size);
  printf("field_0x38:   %x\n", e->field_0x38);
}
