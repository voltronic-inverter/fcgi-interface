#ifndef __FCGI__ADAPTER__H__
#define __FCGI__ADAPTER__H__

  typedef int (*fcgi_handler)(
    const char* request_method,
    unsigned int content_length,
    const char* request_content);

  int fcgi_init(const char *socket_path);

  int fcgi_accept(fcgi_handler handler);

  int fcgi_close(void);

  const char* fcgi_getenv(const char* env_name);

  int fcgi_printf(const char *format, ...);

#endif
