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

typedef struct _TS_PACKFILE {
  // File descriptor of this packfile.
  int fd;
  // The size in bytes of the table of contents.
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
// Attempts to open a packfile located at path. The results are filled into pf.
// Returns 1 on success and 0 on fail. TODO: Proper fail code plz.
//

int packfile_open(const char* path, PTS_PACKFILE pf);

//
// Closes the file descriptor owned by pf. Returns result of that close.
//

int packfile_close(PTS_PACKFILE pf);
