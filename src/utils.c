#include <string.h>
#include "utils.h"
#include <errno.h>

#if defined(_WIN32) || defined(WIN32)
  #include <stddef.h>
  #include <stdlib.h>
  #include "windows.h"
#endif

unsigned int parse_uint(const char* cstring) {
  if (cstring != 0) {
    unsigned int value = 0;
    for(unsigned int count = 0; count < 8; ++count) {
      const char ch = *cstring;

      if (ch >= '0' && ch <= '9') {
        value = (value * 10) + (ch - '0');
      } else {
        return value;
      }

      cstring += sizeof(char);
    }
  }

  return 0;
}

inline int cstring_equals(const char* a, const char* b) {
  return a != 0 && b != 0 && strcmp(a, b) == 0 ? 1 : 0;
}

#if defined(_WIN32) || defined(WIN32)

  static char* last_error_string(DWORD last_error);

  inline void reset_last_error(void) {
    SetLastError(0);
    errno = 0;
  }

  void get_last_error(errno_t* error_num) {
    const int errnum = errno;
    const DWORD last_error = GetLastError();

    error_num->errno = errnum;
    errnum->last_error = last_error;

    return (errnum != 0 || last_error != 0) ? 1 : 0;
  }

  void set_last_error(const errno_t* error_num) {
    if (error_num->last_error != 0) {
      SetLastError(error_num->last_error);
    }

    if (error_num->errno != 0) {
      errno = error_num->errno;
    }
  }

  const char* get_error_string(const errno_t* error_num) {
    const char* error_message = 0;
    if (error_num->last_error != 0) {
      error_message = last_error_string(error_num->last_error);
    } else if (error_num->errno != 0) {
      error_message = strerror(error_num->errno);
    }

    return error_message != 0 ? error_message : "";
  }

#else

  inline void reset_last_error(void) {
    errno = 0;
  }

  inline int get_last_error(errno_t* error_num) {
    return (*error_num = errno) != 0 ? 1 : 0;
  }

  inline void set_last_error(const errno_t* error_num) {
    errno = *error_num;
  }

  const char* get_error_string(const errno_t* error_num) {
    const char* error_message = strerror(*error_num);
    return error_message != 0 ? error_message : "";
  }

#endif

#if defined(_WIN32) || defined(WIN32)

  #define MAXIMUM_LAST_ERROR_BUFFER 8
  static char** last_error_strings = 0;
  static unsigned int last_error_index = 0;

  static void free_last_error_strings(void) {
    if (last_error_strings != 0) {
      for(unsigned int index = 0; index < MAXIMUM_LAST_ERROR_BUFFER; ++index) {
        const char* cstring = last_error_strings[index];
        if (cstring != 0) {
          free(cstring);
        }
      }
      free(last_error_strings);
    }
  }

  static void store_last_error_buffer(char* error_cstring) {
    if (last_error_strings == 0) {
      last_error_strings = malloc(sizeof(void*) * MAXIMUM_LAST_ERROR_BUFFER);
      for(unsigned int index = 0; index < MAXIMUM_LAST_ERROR_BUFFER; ++index) {
        last_error_strings[index] = 0;
      }

      atexit(free_last_error_strings);
    }

    const unsigned int index = last_error_index++ % MAXIMUM_LAST_ERROR_BUFFER;
    char* current = last_error_strings[index];
    if (current != 0) {
      free(current);
    }

    last_error_strings[index] = error_cstring;
  }

  static char* last_error_string(DWORD last_error) {
    LPTSTR message_buffer = nullptr;

    const DWORD result = FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      NULL,
      last_error,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&message_buffer,
      0,
      NULL);

    if (result > 0) {
      char* error_cstring = 0;

      #ifdef UNICODE
        const int length = WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, NULL, 0, NULL, NULL);
        error_cstring = malloc((length + 1) * sizeof(char));
        WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, error_cstring, length, NULL, NULL);
        error_cstring[length] = 0;
      #else
        error_cstring = malloc((result + 1) * sizeof(char));
        memcpy(error_cstring, message_buffer, result);
        error_cstring[result] = 0;
      #endif
    }

    LocalFree(message_buffer);

    store_last_error_buffer(error_cstring);
  }

#endif
