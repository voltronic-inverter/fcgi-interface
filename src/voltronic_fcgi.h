#ifndef __VOLTRONIC__FCGI__H__
#define __VOLTRONIC__FCGI__H__

  /**
   * Used internally by main
   */

  #include "voltronic_dev.h"

  voltronic_dev_t new_voltronic_dev(void);
  int voltronic_fcgi(const char* content_length, const char* query_string);

#endif
