#ifndef __VOLTRONIC__FCGI__H__
#define __VOLTRONIC__FCGI__H__

  /**
   * Used internally by main
   */

  #include "voltronic_dev.h"

  voltronic_dev_t new_voltronic_dev(void);
  const char* parse_env(const char* env_name, const char* default_value);
  int env_equals(const char* expected_value, const char* env_name, const char* default_value);
  int voltronic_fcgi(const char* content_length, const char* query_string);

  #if defined(_WIN32) || defined(WIN32)
    const char* fcgi_port(void);
  #endif

#endif
