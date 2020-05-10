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

static void setup_fcgi_server(const char* port) {
  FCGX_Init();
  FCGX_OpenSocket(port, 500);
}

static void parse_command_line_arguments(const int argc, const char** argv) {
  const char* port = fcgi_default_port();

  #if defined(_WIN32) && defined(WIN32)
    unsigned int fcgi_server_param = 1;
  #else
    unsigned int fcgi_server_param = 0;
  #endif

  for(unsigned int index = 0; index < argc; ++index) {
    if (argv == 0) {
      continue;
    }

    const char* arg = argv[index];
    const size_t arg_length = arg != 0 ? strlen(arg) : 0;
    if (arg_length <= 0) {
      continue;
    }

    if ((arg_length == 13) && (strcmp("--fcgi-server", arg) == 0)) {
      fcgi_server_param = 1;
    } else if ((arg_length > 12) && (strncmp("--fcgi-port=", arg, 12) == 0)) {
      const size_t port_number_length = arg_length - 12;
      char* copy = malloc(sizeof(char) * (port_number_length + 2));
      copy[0] = ':';
      copy[port_number_length + 1] = 0;
      memcpy(copy + sizeof(char), arg + (sizeof(char) * 12), port_number_length);
      port = copy;
    }
  }

  if (fcgi_server_param) {
    setup_fcgi_server(port);
  }
}

int main(int argc, char** argv) {
  parse_command_line_arguments(argc, argv);

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
