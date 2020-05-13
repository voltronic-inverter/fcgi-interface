#include <string.h>
#include "utils.h"

#if defined(_WIN32) || defined(WIN32)
  #include <stddef.h>
  #include <stdlib.h>
#endif

unsigned int parse_uint(const char* cstring) {
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

  return 0;
}

inline int cstring_equals(const char* a, const char* b) {
  return a != 0 && b != 0 && strcmp(a, b) == 0 ? 1 : 0;
}

inline int is_cstring_empty(const char* cstring) {
  return (cstring != 0) && (*cstring != 0) ? 0 : 1;
}

#if defined(_WIN32) || defined(WIN32)

  #define MAXIMUM_LAST_ERROR_BUFFER 8
  static char** last_error_strings = 0;
  static unsigned int last_error_index = 0;

  static void free_last_error_strings(void) {
    if (last_error_strings != 0) {
      for(unsigned int index = 0; index < MAXIMUM_LAST_ERROR_BUFFER; ++index) {
        char* cstring = last_error_strings[index];
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

  const char* get_error_string(last_error_t last_error) {
    LPWSTR message_buffer = 0;

    const DWORD result = FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      0,
      (DWORD) last_error,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPWSTR)&message_buffer,
      0,
      0);

    if (result > 0) {
      const int length = WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, 0, 0, 0, 0);
      if (length > 0) {
        char* error_cstring = malloc((length + 1) * sizeof(char));
        WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, error_cstring, length, 0, 0);
        error_cstring[length] = 0;

        LocalFree(message_buffer);
        store_last_error_buffer(error_cstring);

        return error_cstring;
      }
    }

    return "";
  }

#endif
