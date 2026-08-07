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

static int big_endian;

//
// General init routine to set up some things.
//

void init();

//
// Simple routine that tells us if the packfile given by fd is indeed a proper
// packfile. Returns 1 on true, and 0 on false.
//

int is_pack_file(int fd);

//
// Computes the overall size of the packfile table of contents. N.B. this is not
// the number of packfile entries.
//

size_t get_pack_toc_size(int fd);

//
// Attempts to read a ToC entry from a packfile. On success, returns 1. On fail,
// returns 0.
//

int read_toc_entry(
  int fd,
  const int entry_index,
  // TODO: Maybe put these two in a struct together.
  const int num_toc_entries,
  const int toc_size,
  PTS_PACKFILE_TOC_ENTRY result
);

//
// General purpose routine that reads a 32-bit int as unsigned. N.B. this will
// update the file offset.
//

uint32_t read_uint32(int fd);

void print_toc_entry(PTS_PACKFILE_TOC_ENTRY e);

int main(int argc, char** argv) {
  int entry_index;
  int fd;
  int i;
  int num_toc_entries;
  int result;
  size_t sz;
  TS_PACKFILE_TOC_ENTRY toc_entry;

  //
  // Intro and usage.
  //

  init();
  printf("TSPACK\n");

  if (argc != 2) {
    fprintf(stderr, "usage: tspack PACKFILE_PATH\n");
    exit(-1);
  }

  //
  // Open the file.
  //

  fd = open(argv[1], O_RDONLY);

  if (fd == -1) {
    fprintf(stderr, "failed to open %s\n", argv[1]);
    exit(-1);
  }

  //
  // Verify the file is a packfile.
  //

  if (is_pack_file(fd) == 0) {
    fprintf(stderr, "error: %s is not a packfile!\n", argv[1]);
    close(fd);
    exit(-1);
  }

  //
  // Read the packfile size and number of ToC entries. TODO: Put these in an
  // actual packfile
  //

  sz = get_pack_toc_size(fd);
  num_toc_entries = (int)sz / (int)sizeof(TS_PACKFILE_TOC_ENTRY);

  printf("Packfile size: %lu bytes\n", sz);
  printf("Num ToC entries: %u\n", num_toc_entries);

  //
  // Now attempt to read each ToC entry.
  //

  for (i = 0; i < num_toc_entries; i++) {
    result = read_toc_entry(
      fd,
      i,
      num_toc_entries,
      (int)sz,
      &toc_entry
    );

    if (result == 0) {
      fprintf(stderr, "failed to read %d entry of packfile\n", entry_index);
      close(fd);
      exit(-1);
    }

    printf("Entry %d:\n", i);
    print_toc_entry(&toc_entry);
    printf("---\n");
  }

  close(fd);

  return 0;
}

void init() {
  uint32_t x;

  //
  // Decide if this platform is little or big endian. Decides how we read in
  // ints and stuff.
  //

  x = 1;
  if (*((uint8_t*)(&x)) == 1) {
    big_endian = 0;
  } else {
    big_endian = 1;
  }
}

int is_pack_file(int fd) {
  char magic[4];
  int n;

  n = read(fd, magic, 4);

  if (n != 4) {
    fprintf(stderr, "error while reading header magic\n");
    exit(-1);
  }

  return strncmp("P4CK", magic, 4) == 0;

  //
  // A note on code below: I decided to keep this as personal edification. N.B.
  // it only works on little-endian machines. This here is how it is actually
  // done in the PS2 source code: they compare with the 0x4b433450 value, taking
  // advantage of the PS2's little endian CPU architecture. That said, I decided
  // to go with the above because it is more robust.
  //

  /*uint32_t magic;
  int n;

  n = read(fd, &magic, 4);

  if (n != 4) {
    fprintf(stderr, "error while reading header magic\n");
    exit(-1);
  }

  return magic == 0x4b433450;*/
}

size_t get_pack_toc_size(int fd) {
  off_t currpos;

  //
  // Reset the file offset to byte 8. After the 4-byte magic, we have 4-bytes
  // that are not actually used in packfile loading, so we can skip them. That
  // leaves the last 4 bytes of the start of the file, which is the overall
  // packfile size.
  //

  currpos = lseek(fd, 8, SEEK_SET);

  if (currpos == -1) {
    fprintf(stderr, "failed to reset file offset to byte 4\n");
    exit(-1);
  }

  //
  // Read the next 4 bytes which are the size.
  //

  return read_uint32(fd);
}

int read_toc_entry(
  int fd,
  const int entry_index,
  // TODO: Maybe put these two in a struct together.
  const int num_toc_entries,
  const int toc_size,
  PTS_PACKFILE_TOC_ENTRY result
) {
  off_t currpos;
  int n;

  //
  // Really basic verification of args that we can fail on. N.B. I am not gonna
  // check that num_toc_entries == toc_size / 60 because I feel like such
  // verification isn't appropriate here.
  //

  if (entry_index < 0) {
    return 0;
  }

  if (entry_index >= num_toc_entries) {
    return 0;
  }

  if (!result) {
    return 0;
  }

  //
  // The ToC is the last toc_size number of bytes of the file. So seek to
  // there.
  //

  currpos = lseek(fd, -toc_size, SEEK_END);
  if (currpos == -1) {
    return 0;
  }

  //
  // Now navigate to the proper position and read.
  //

  currpos = lseek(fd, entry_index * sizeof(TS_PACKFILE_TOC_ENTRY), SEEK_CUR);
  if (currpos == -1) {
    return 0;
  }

  n = read(fd, result->filename, 48);
  if (n != 48) {
    return 0;
  }

  result->offset = read_uint32(fd);
  result->size = read_uint32(fd);
  result->field_0x38 = read_uint32(fd);

  return 1;
}

uint32_t read_uint32(int fd) {
  uint8_t buf[4];
  int n;
  uint32_t result;

  //
  // N.B. I had to use uint8_t instead of char. If I used char, then when I do
  // the uint32_t casts below do gross sign extending.
  //

  n = read(fd, buf, 4);

  if (n != 4) {
    fprintf(stderr, "error while reading 4-byte int\n");
    exit(-1);
  }

  result = 0;

  if (big_endian) {
    result =
      (uint32_t)(buf[3])
      | ((uint32_t)(buf[2]) << 8)
      | ((uint32_t)(buf[1]) << 16)
      | ((uint32_t)(buf[0]) << 24);
  } else {
    result =
      (uint32_t)(buf[0])
      | ((uint32_t)(buf[1]) << 8)
      | ((uint32_t)(buf[2]) << 16)
      | ((uint32_t)(buf[3]) << 24);
  }

  return result;
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
