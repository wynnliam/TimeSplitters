// Illtyd Wynn, 7-29-2026, TimeSplitters

#include <ts/packfile/packfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// TODO: Fix for fails!!!

//
// Boolean flag that tells us if this machine is big endian or not.
//

static int big_endian;

/* UTILITY ROUTINES */

//
// Simple routine that tells us if the packfile given by fd is indeed a proper
// packfile. Returns 1 on true, and 0 on false.
//

static int packfile_is_pack_file(int fd);

//
// Computes the overall size of the packfile table of contents. N.B. this is not
// the number of packfile entries.
//

static size_t packfile_get_pack_toc_size(int fd);

//
// General purpose routine that reads a 32-bit int as unsigned. N.B. this will
// update the file offset.
//

static uint32_t packfile_read_uint32(int fd);

/* PACKFILE IMPL */

void packfile_lib_init() {
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

TS_PACKFILE_OPEN_RESULT packfile_open(const char* path, PTS_PACKFILE pf) {
  int fd;

  //
  // Open the file, and verify it is a packfile.
  //

  fd = open(path, O_RDONLY);

  if (fd == -1) {
    return TS_PACKFILE_OPEN_ERR_FD;
  }

  if (packfile_is_pack_file(fd) == 0) {
    return TS_PACKFILE_OPEN_ERR_NOT_A_PACKFILE;
  }

  pf->fd = fd;
  // TODO: Error check this.
  pf->toc_sz = packfile_get_pack_toc_size(fd);
  pf->num_toc_entries = pf->toc_sz / (int)sizeof(TS_PACKFILE_TOC_ENTRY);

  return TS_PACKFILE_OPEN_SUCCESS;
}

int packfile_read_toc_entry(
  PTS_PACKFILE packfile,
  const int entry_index,
  PTS_PACKFILE_TOC_ENTRY result
) {
  int fd;
  off_t currpos;
  int n;

  //
  // Really basic verification of args that we can fail on. N.B. I am not gonna
  // check that num_toc_entries == toc_size / 60 because I feel like such
  // verification isn't appropriate here.
  //

  if (!packfile || !result) {
    return 0;
  }

  if (entry_index < 0) {
    return 0;
  }

  if (entry_index >= packfile->num_toc_entries) {
    return 0;
  }

  fd = packfile->fd;

  //
  // The ToC is the last toc_size number of bytes of the file. So seek to
  // there.
  //

  currpos = lseek(fd, -packfile->toc_sz, SEEK_END);
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

  result->offset = packfile_read_uint32(fd);
  result->size = packfile_read_uint32(fd);
  result->field_0x38 = packfile_read_uint32(fd);

  return 1;
}

int packfile_close(PTS_PACKFILE pf) {
  return 0;
}

/* UTILITY ROUTINE IMPL */

int packfile_is_pack_file(int fd) {
  char magic[4];
  int n;

  n = read(fd, magic, 4);

  if (n != 4) {
    return 0;
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
    return 0;
  }

  return magic == 0x4b433450;*/
}

size_t packfile_get_pack_toc_size(int fd) {
  off_t currpos;

  //
  // Reset the file offset to byte 8. After the 4-byte magic, we have 4-bytes
  // that are not actually used in packfile loading, so we can skip them. That
  // leaves the last 4 bytes of the start of the file, which is the overall
  // packfile size.
  //

  currpos = lseek(fd, 8, SEEK_SET);

  if (currpos == -1) {
    return 0;
  }

  //
  // Read the next 4 bytes which are the size.
  //

  return (size_t)packfile_read_uint32(fd);
}

uint32_t packfile_read_uint32(int fd) {
  uint8_t buf[4];
  int n;
  uint32_t result;

  //
  // N.B. I had to use uint8_t instead of char. If I used char, then when I do
  // the uint32_t casts below do gross sign extending.
  //

  n = read(fd, buf, 4);

  if (n != 4) {
    return 0;
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
