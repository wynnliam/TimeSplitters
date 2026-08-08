// Illtyd Wynn, 7-29-2026, TimeSplitters

#include <ts/packfile/packfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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

static TS_RESULT packfile_get_pack_toc_size(int fd, int* toc_sz);

//
// General purpose routine that reads a 32-bit int as unsigned. N.B. this will
// update the file offset.
//

static TS_RESULT packfile_read_u32(int fd, uint32_t* val);

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

TS_RESULT packfile_open(const char* path, PTS_PACKFILE pf) {
  int fd;
  TS_RESULT result;
  int toc_sz;

  if (!pf) {
    return TS_ERR_INVALID;
  }

  pf->fd = 0;
  pf->toc_sz = 0;
  pf->num_toc_entries = 0;

  //
  // Open the file, and verify it is a packfile.
  //

  fd = open(path, O_RDONLY);

  if (fd == -1) {
    return TS_ERR_IO;
  }

  if (packfile_is_pack_file(fd) == 0) {
    close(fd);
    return TS_ERR_NOT_PACK;
  }

  //
  // Now read the ToC size and set the member values accordingly.
  //

  result = packfile_get_pack_toc_size(fd, &toc_sz);

  if (toc_sz == 0) {
    close(fd);
    return TS_ERR_MALFORMED;
  }

  pf->fd = fd;
  pf->toc_sz = toc_sz;
  pf->num_toc_entries = pf->toc_sz / (int)sizeof(TS_PACKFILE_TOC_ENTRY);

  return TS_OK;
}

TS_RESULT packfile_read_toc_entry(
  PTS_PACKFILE packfile,
  const int entry_index,
  PTS_PACKFILE_TOC_ENTRY pte
) {
  int fd;
  off_t currpos;
  int n;
  TS_RESULT result;

  //
  // Really basic verification of args that we can fail on. N.B. I am not gonna
  // check that num_toc_entries == toc_size / 60 because I feel like such
  // verification isn't appropriate here.
  //

  if (!packfile || !pte) {
    return TS_ERR_INVALID;
  }

  if (entry_index < 0) {
    return TS_ERR_RANGE;
  }

  if (entry_index >= packfile->num_toc_entries) {
    return TS_ERR_RANGE;
  }

  fd = packfile->fd;

  //
  // The ToC is the last toc_size number of bytes of the file. So seek to
  // there.
  //

  currpos = lseek(fd, -packfile->toc_sz, SEEK_END);
  if (currpos == -1) {
    return TS_ERR_IO;
  }

  //
  // Now navigate to the proper position and read.
  //

  currpos = lseek(fd, entry_index * sizeof(TS_PACKFILE_TOC_ENTRY), SEEK_CUR);
  if (currpos == -1) {
    return TS_ERR_IO;
  }

  n = read(fd, pte->filename, 48);
  if (n != 48) {
    return TS_ERR_EOF;
  }

  result = packfile_read_u32(fd, &(pte->offset));
  if (result != TS_OK) {
    return TS_ERR_EOF;
  }

  result = packfile_read_u32(fd, &(pte->size));
  if (result != TS_OK) {
    return TS_ERR_EOF;
  }

  result = packfile_read_u32(fd, &(pte->field_0x38));
  if (result != TS_OK) {
    return TS_ERR_EOF;
  }

  return TS_OK;
}

TS_RESULT packfile_close(PTS_PACKFILE pf) {
  close(pf->fd);
  return TS_OK;
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

TS_RESULT packfile_get_pack_toc_size(int fd, int* toc_sz) {
  off_t currpos;
  TS_RESULT result;
  uint32_t val;

  if (!toc_sz) {
    return TS_ERR_INVALID;
  }

  //
  // Reset the file offset to byte 8. After the 4-byte magic, we have 4-bytes
  // that are not actually used in packfile loading, so we can skip them. That
  // leaves the last 4 bytes of the start of the file, which is the overall
  // packfile size.
  //

  currpos = lseek(fd, 8, SEEK_SET);

  if (currpos == -1) {
    return TS_ERR_IO;
  }

  //
  // Read the next 4 bytes which are the size.
  //

  result = packfile_read_u32(fd, &val);
  if (result != TS_OK) {
    return TS_ERR_EOF;
  }

  *toc_sz = (int)val;

  return TS_OK;
}

TS_RESULT packfile_read_u32(int fd, uint32_t* val) {
  uint8_t buf[4];
  int n;
  uint32_t result;

  if (!val) {
    return TS_ERR_INVALID;
  }

  *val = 0;

  //
  // N.B. I had to use uint8_t instead of char. If I used char, then when I do
  // the uint32_t casts below do gross sign extending.
  //

  n = read(fd, buf, 4);

  if (n != 4) {
    return TS_ERR_EOF;
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

  *val = result;

  return TS_OK;
}
