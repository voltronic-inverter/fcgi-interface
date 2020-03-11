#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fcgi_stdio.h"
#include "voltronic_fcgi.h"

static int fcgi_main(
  const char* request_method,
  const char* content_length,
  const char* query_string) {

  if (strcmp("POST", request_method) == 0) {
    if (!voltronic_fcgi(content_length, query_string)) {
      return 0;
    }
  } else if (strcmp("DELETE", request_method) == 0) {
    printf("Status: 200 OK\r\n"
      "\r\n"
      "Terminating FCGI process");

    return 0;
  } else {
    printf("Status: 405 Method Not Allowed\r\n"
      "Allow: POST, DELETE\r\n"
      "\r\n");
  }

  return 1;
}

int main() {
  while (FCGI_Accept() >= 0) {
    const char* request_method = getenv("REQUEST_METHOD");
    const char* content_length = getenv("CONTENT_LENGTH");
    const char* query_string = getenv("QUERY_STRING");

    if (request_method != 0 && content_length != 0) {
      const int result = fcgi_main(
        request_method,
        content_length,
        query_string != 0 ? query_string : "");

      if (!result) {
        break;
      }
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");
    }
  }
}
