#ifndef __VOLTRONIC__FCGI__H__
#define __VOLTRONIC__FCGI__H__

  #include "voltronic_dev.h"

  int voltronic_fcgi_main(
    const unsigned int content_length,
    const char* request_content);

  voltronic_dev_t new_voltronic_dev(void);

#endif
