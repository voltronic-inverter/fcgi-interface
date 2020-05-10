#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fcgi_stdio.h"
#include "voltronic_fcgi.h"
#include "version.h"

static int fcgi_main(
  const char* request_method,
  const char* content_length,
  const char* query_string) {

  if (strcmp("POST", request_method) == 0) {
    if (!voltronic_fcgi(content_length, query_string)) {
      return 1;
    }
  } else if (strcmp("DELETE", request_method) == 0) {
    printf("Status: 200 OK\r\n"
      VERSION_HEADER
      "\r\n"
      "Terminating FCGI process");

    return 1;
  } else {
    printf("Status: 405 Method Not Allowed\r\n"
      "Allow: POST, DELETE\r\n"
      VERSION_HEADER
      "\r\n");
  }

  return 0;
}

static void setup_fcgi_server(void) {
  FCGX_Init();
  FCGX_OpenSocket(fcgi_port(), 500);
}

int main(int argc, char** argv) {

  #if defined(_WIN32) && defined(WIN32)
    setup_fcgi_server();
  #else
    for(unsigned int index = 0; index < argc; ++index) {
      const char* arg = argv[index];
      if ((arg != 0) && (strcmp("--fcgi-server", arg) == 0)) {
        setup_fcgi_server();
        break;
      }
    }
  #endif

  int result = 0;
  while (result == 0 && FCGI_Accept() >= 0) {
    const char* request_method = getenv("REQUEST_METHOD");
    const char* content_length = getenv("CONTENT_LENGTH");
    const char* query_string = getenv("QUERY_STRING");

    if (request_method != 0 && content_length != 0) {
      result = fcgi_main(
        request_method,
        content_length,
        query_string != 0 ? query_string : "");
    }
  }

  return 0;
}
