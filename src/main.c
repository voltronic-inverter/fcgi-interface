#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "version.h"

#define TAB "  "

static int fcgi_main(
  const char* request_method,
  unsigned int content_length,
  const char* request_content) {

  int result;
  if (cstring_equals("POST", request_method)) {
    result = voltronic_fcgi_main(content_length, request_content);
  } else if (cstring_equals("DELETE", request_method)) {
    fcgi_printf(
      "Status: 200 OK\r\n"
      "\r\n"
      "Terminating FCGI process");

    result = 0;
  } else {
    fcgi_printf(
      "Status: 405 Method Not Allowed\r\n"
      "Allow: POST, DELETE\r\n"
      "\r\n");

    result = 1;
  }

  SET_LAST_ERROR(0);
  return result;
}

int main(int argc, char** argv) {
  for(int argv_index = 1; argv_index < argc; ++argv_index) {
    const char* arg = argv[argv_index];
    const size_t arg_length = arg != 0 ? strlen(arg) : 0;
    if (arg_length <= 0) {
      continue;
    }

    if (strncmp("--fcgi-socket=", arg, 14) == 0) {
      const char* socket = &arg[14];
      if (*socket != 0) {
        printf("Opening FCGI socket \"%s\"" NEWLINE, socket);
        fcgi_init(socket);
        continue;
      } else {
        fprintf(stderr, "fcgi-socket is empty" NEWLINE);
        exit(1);
      }
    }

    if (strcmp("--help", arg) == 0) {
      printf("Commandline options:" NEWLINE NEWLINE);
      printf(TAB "--help                    print this screen and terminate" NEWLINE);
      printf(TAB "--version                 print version and terminate" NEWLINE);
      printf(TAB "--fcgi-socket=<socket>    start as a standalone process" NEWLINE);
      printf(TAB TAB "<socket> is the Unix domain socket (named pipe in Windows), or a colon followed by a port number" NEWLINE);
      printf(TAB TAB "e.g. \"--open-fcgi-socket=/tmp/fastcgi/mysocket\", \"--open-fcgi-socket=:5000\"" NEWLINE);
      exit(0);
    }

    if (strcmp("--version", arg) == 0) {
      printf(VOLTRONIC_FCGI_VERSION NEWLINE);
      exit(0);
    }

    fprintf(stderr, "Invalid command-line argument '%s'" NEWLINE, arg);
    fprintf(stderr, "--help for a list of permitted command-line options" NEWLINE);
    exit(1);
  }

  if (!fcgi_init(0)) {
    exit(1);
  }

  while(fcgi_accept(&fcgi_main)) {
  }

  last_error_t last_error =GET_LAST_ERROR();
  if (last_error != 0) {
    fprintf(stderr, "%s" NEWLINE, GET_ERROR_STRING(last_error));
  }

  fcgi_close();
}
