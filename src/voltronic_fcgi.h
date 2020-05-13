#ifndef __VOLTRONIC__FCGI__H__
#define __VOLTRONIC__FCGI__H__

  /**
   * Used internally by main
   */

  #include "fcgiapp.h"
  #include "voltronic_dev.h"

  int handle_fcgi_request(FCGX_Request* request);

  voltronic_dev_t new_voltronic_dev(FCGX_Request* request);
  const char* parse_env(FCGX_Request* request, const char* env_name, const char* default_value);
  int env_equals(FCGX_Request* request, const char* expected_value, const char* env_name, const char* default_value);

#endif
