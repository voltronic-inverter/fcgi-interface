#ifndef __U_T_I_L_S__H__
#define __U_T_I_L_S__H__

  unsigned int parse_uint(const char* cstring);

  int is_cstring_empty(const char* cstring);

  int cstring_equals(const char* a, const char* b);

  #if defined(_WIN32) || defined(WIN32)

    #include "windows.h"

    typedef DWORD last_error_t;
    const char* get_error_string(last_error_t value);

    #define SET_LAST_ERROR(_last_error_value_)   SetLastError((_last_error_value_))
    #define GET_LAST_ERROR()                     GetLastError()
    #define GET_ERROR_STRING(_last_error_value_) get_error_string((_last_error_value_))

  #else

    #include <errno.h>
    #include <string.h>

    typedef int last_error_t;

    #define SET_LAST_ERROR(_errno__value_)   errno = (_errno__value_)
    #define GET_LAST_ERROR()                 (errno)
    #define GET_ERROR_STRING(_errno__value_) strerror((_errno__value_))

  #endif

#endif

#ifndef NEWLINE
  #if defined(_WIN32) || defined(WIN32)
    #define NEWLINE "\r\n"
  #else
    #define NEWLINE "\n"
  #endif
#endif
