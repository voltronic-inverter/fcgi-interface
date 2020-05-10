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
      VERSION_DESCRIPTION
      "\r\n\r\n"
      "Terminating FCGI process");

    return 1;
  } else {
    printf("Status: 405 Method Not Allowed\r\n"
      "Allow: POST, DELETE\r\n"
      VERSION_DESCRIPTION
      "\r\n\r\n");
  }

  return 0;
}

static void parse_command_line_arguments(unsigned int argc, char** argv) {
  const char* socket = 0;
  for(unsigned int index = 0; index < argc; ++index) {
    const char* arg = argv[index];
    const size_t arg_length = arg != 0 ? strlen(arg) : 0;
    if (arg_length <= 0) {
      continue;
    }

    if (strcmp("--version", arg) == 0) {
      fprintf(stdout, VERSION_DESCRIPTION "\n");
      exit(0);
    } else if ((arg_length > 19) && (strncmp("--fcgi-open-socket=", arg, 19) == 0)) {
      socket = arg + (sizeof(char) * 19);
    }
  }

  if (socket != 0) {
    const size_t length = strlen(socket);
    if (length > 0) {
      FCGX_Init();
      FCGX_OpenSocket(socket, 10);
    }
  }
}

int main(int argc, char** argv) {
  if (argc >= 0 && argv != 0) {
    parse_command_line_arguments(argc, argv);
  }

  int result = FCGI_Accept();
  if (result < 0) {
    fprintf(stderr, "FCGI_Accept() failed\n");
    fprintf(stderr, "Did you start the process with spawn_fcgi (recommended apprach)\n");
    fprintf(stderr, "You can start the process as standalone instead (see below).\n\n");
    fprintf(stderr, "Commandline options:\n");
    fprintf(stderr, "  --version                        print version and terminate\n\n");
    fprintf(stderr, "  --fcgi-open-socket=<socket>      start as a standalone process\n");
    fprintf(stderr, "    <socket> is the Unix domain socket (named pipe in Windows), or a colon followed by a port number.\n");
    fprintf(stderr, "    e.g. \"--fcgi-open-socket=/tmp/fastcgi/mysocket\", \"--fcgi-open-socket=:5000\"\n");
    exit(1);
  }

  result = 0;
  do {
    const char* request_method = getenv("REQUEST_METHOD");
    const char* content_length = getenv("CONTENT_LENGTH");
    const char* query_string = getenv("QUERY_STRING");

    if (request_method != 0 && content_length != 0) {
      result = fcgi_main(
        request_method,
        content_length,
        query_string != 0 ? query_string : "");
    }
  } while(result == 0 && FCGI_Accept() >= 0);

  FCGI_Finish();

  return 0;
}
