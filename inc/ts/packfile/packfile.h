// Illtyd Wynn, 7-28-2026, TimeSplitters

/*
  Here, we define the packfile type. The packfile is used in TimeSplitters to
  store all game assets.

  About the Packfile Format:
  As far as I can tell, this format is unique to TimeSplitters. I did some
  research, and it appears variations of it are also used in TimeSplitters 2 and
  TimeSplitters: Future Perfect. But we're only concerned with the OG TS1
  version.

  So the packfile has the following format:
  * "P4CK" - 4 bytes
  * Size of the Table of Contents in bytes. Let's say this is Q
  * Raw file format itself
  * Last Q bytes: The Table of Contents (TOC)

  Using this format + lseek, we really only need to define a packfile's TOC
  entry.
*/

#pragma once

#include <stdint.h>
#include <ts/err/err.h>

typedef struct _TS_PACKFILE {
  // File descriptor of this packfile.
  int fd;
  // The size in bytes of the table of contents. TODO: Shouldn't this be a
  // uint32_t?
  int toc_sz;
  // The number of ToC entries at the bottom of the file.
  int num_toc_entries;
} TS_PACKFILE, *PTS_PACKFILE;

//
// Defines an entry in the packfile's table of contents. Each of these is 60
// bytes in size. N.B. I am still reverse engineering this, so I am not sure
// the exact contents of each arg.
//

typedef struct _TS_PACKFILE_TOC_ENTRY {
  // The asset file this entry points to. Yes this does have a limit of 48
  // chars.
  char filename[48];
  // Offset in bytes into the packfile.
  uint32_t offset;
  // Size in bytes of the file BUT ONLY IF field_0x38 is 0. Otherwise we need to
  // use a to-be-discerned decompression scheme.
  uint32_t size;
  // This is used as some kind of flag in the packfile.
  uint32_t field_0x38;
} TS_PACKFILE_TOC_ENTRY, *PTS_PACKFILE_TOC_ENTRY;

//
// This routine must be called before anymore of the API can be used. It
// initializes internal flags needed for reading packfiles.
//

void packfile_lib_init();

//
// Attempts to open a packfile located at path.
// Returns one of the following:
//  * TS_OK - Success. Values in pf are actually meaningful.
//  * TS_ERR_INVALID - pf is NULL.
//  * TS_ERR_IO - Failed to open file.
//  * TS_ERR_NOT_PACK - Given file is not a packfile.
//  * TS_ERR_MALFORMED - ToC size is 0 or failed to be read.
//

TS_RESULT packfile_open(const char* path, PTS_PACKFILE pf);

//
// Attempts to read a ToC entry from a packfile. On success, returns 1. On fail,
// returns 0. Returns one of the following results:
//  * TS_OK - Success. pte filled with entry_index of packfile ToC.
//  * TS_ERR_INVALID - either packfile or pte or both are NULL.
//  * TS_ERR_RANGE - entry_index is either < 0 or >= number of toc entries in
//    the packfile.
//  * TS_ERR_IO - failed to seek position of specified toc entry.
//  * TS_ERR_EOF - hit end of packfile trying to read toc entry.
//

TS_RESULT packfile_read_toc_entry(
  PTS_PACKFILE packfile,
  const int entry_index,
  PTS_PACKFILE_TOC_ENTRY pte
);

//
// Closes the file descriptor owned by pf. Returns result of that close.
//

TS_RESULT packfile_close(PTS_PACKFILE pf);
