#ifndef __U_T_I_L_S__H__
#define __U_T_I_L_S__H__

  unsigned int parse_uint(const char* cstring);

  int cstring_equals(const char* a, const char* b);

  #if defined(_WIN32) || defined(WIN32)

    typedef struct {
        int errno;
        DWORD last_error;
    } errno_t;

  #else

    typedef int errno_t;

  #endif

  void reset_last_error(void);
  int get_last_error(errno_t* error_num);
  void set_last_error(const errno_t* error_num);
  const char* get_error_string(const errno_t* error_num);

#endif

#ifndef NEWLINE
  #if defined(_WIN32) || defined(WIN32)
    #define NEWLINE "\r\n"
  #else
    #define NEWLINE "\n"
  #endif
#endif
