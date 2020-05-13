#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fcgiapp.h"
#include "voltronic_fcgi.h"
#include "version.h"

#if defined(_WIN32) || defined(WIN32)
  #define NEWLINE "\r\n"
#else
  #define NEWLINE "\n"
#endif

#define TAB "  "

static void print_commandline_options(void);
static void parse_command_line_arguments(
  const unsigned int argc, char** argv);

static int fcgi_sock = 0;

static void fcgi_main(FCGX_Request* request) {
  int result = 0;
  do {
    result = FCGX_Accept_r(request);

    if (result == 0) {
      result = handle_fcgi_request(request);
    }

    FCGX_Finish_r(request);
  } while(result == 0);

  FCGX_ShutdownPending();
}

int main(int argc, char** argv) {
  FCGX_Init();

  if (argc >= 0 && argv != 0) {
    parse_command_line_arguments(argc, argv);
  }

  FCGX_Request request;
  if (FCGX_InitRequest(&request, fcgi_sock, 0) != 0) {
    fprintf(stderr, "Could not initialize FCGI" NEWLINE);
    exit(1);
  }

  fcgi_main(&request);

  FCGX_Free(&request, fcgi_sock);
  FCGX_Finish();
}

static void parse_command_line_arguments(const unsigned int argc, char** argv) {
  const char* socket = 0;
  for(unsigned int index = 1; index < argc; ++index) {
    const char* arg = argv[index];
    const size_t arg_length = arg != 0 ? strlen(arg) : 0;
    if (arg_length <= 0) {
      continue;
    }

    if (strcmp("--help", arg) == 0) {
      print_commandline_options();
      exit(0);
    }

    if (strcmp("--version", arg) == 0) {
      printf(VERSION_DESCRIPTION NEWLINE);
      exit(0);
    }

    if (strncmp("--open-fcgi-socket=", arg, 19) == 0) {
      socket = arg + (sizeof(char) * 19);
      continue;
    }

    fprintf(stderr, "Invalid command-line argument '%s'" NEWLINE, arg);
    fprintf(stderr, "--help for a list of permitted command-line options" NEWLINE);
    exit(1);
  }

  if (socket != 0) {
    if ((fcgi_sock = FCGX_OpenSocket(socket, 10)) == -1) {
      fprintf(stderr, "Could not open '%s'; %s" NEWLINE, socket, strerror(errno));
      exit(1);
    }
  }
}

static void print_commandline_options(void) {
  printf("Commandline options:" NEWLINE NEWLINE);
  printf(TAB "--help                           print this screen and terminate" NEWLINE);
  printf(TAB "--version                        print version and terminate" NEWLINE);
  printf(TAB "--open-fcgi-socket=<socket>      start as a standalone process" NEWLINE);
  printf(TAB TAB "<socket> is the Unix domain socket (named pipe in Windows), or a colon followed by a port number." NEWLINE);
  printf(TAB TAB "e.g. \"--open-fcgi-socket=/tmp/fastcgi/mysocket\", \"--open-fcgi-socket=:5000\"" NEWLINE);
}
