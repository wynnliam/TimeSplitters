// Illtyd Wynn, 8-7-2026, TimeSplitters

/*
  A simple API for handling general return types from routines.

  The packfile API began as specific routines inside the tspack tool. Very
  quickly, I came to realize that an actual engine API would need to exist for
  handling packfiles. As I started writing it, I came into a new wrinkle: these
  routines would print error messages and then terminate the program. For this
  API, that's wholly unacceptable. So instead, they needed to return an error
  code which callers would deal with.
*/

#pragma once

typedef enum _TS_RESULT {
  TS_OK = 0,
  // Syscall failed: sys_errno is valid.
  TS_ERR_IO,
  // Short read - truncated file
  TS_ERR_EOF,
  // Packfile magic mismatch
  TS_ERR_NOT_PACK,
  // File structurally invalid (bad TOC size, etc)
  TS_ERR_MALFORMED,
  // No memory error.
  TS_ERR_NOMEM,
  // Entry index out of bounds.
  TS_ERR_RANGE,
  // Caller passed garbage - programmer error.
  TS_ERR_INVALID
} TS_RESULT;
