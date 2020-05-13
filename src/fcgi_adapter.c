#include <string.h>
#include "fcgi_adapter.h"
#include "fcgiapp.h"
#include "utils.h"

#define REQUEST_MAX_LENGTH  1024

static int fcgi_socket = -1;
static FCGX_Request fcgx_request;
static char request_content[REQUEST_MAX_LENGTH];

int fcgi_init(const char *socket_path) {
  static int init_attempted = 0;
  if (init_attempted == 0) {
    init_attempted = 1;

    if (fcgi_socket == -1) {
      if (socket_path != 0) {
        fcgi_socket = FCGX_OpenSocket(socket_path, 16);
      } else {
        fcgi_socket = 0;
      }

      if (fcgi_socket != -1) {
        FCGX_Init();
        if (FCGX_InitRequest(&fcgx_request, fcgi_socket, 0) == 0) {
          memset(request_content, 0, REQUEST_MAX_LENGTH);
          return 1;
        }
      }
    }

    return 0;
  }

  return fcgi_socket >= 0 ? 1 : 0;
}

int fcgi_close(void) {
  if (fcgi_socket != -1) {
    FCGX_ShutdownPending();
    FCGX_Free(&fcgx_request, fcgi_socket);
    fcgi_socket = -1;
    return 1;
  } else {
    return 0;
  }
}

int fcgi_accept(fcgi_handler handler) {
  int result = 0;
  if (FCGX_Accept_r(&fcgx_request) == 0) {
    const char* request_method = FCGX_GetParam("REQUEST_METHOD", fcgx_request.envp);
    const char* content_length = FCGX_GetParam("CONTENT_LENGTH", fcgx_request.envp);

    if (request_method != 0) {
      if (content_length != 0) {
        const unsigned int request_length_uint = parse_uint(content_length);
        if (request_length_uint < REQUEST_MAX_LENGTH) {
          const int length = FCGX_GetStr(request_content, request_length_uint, fcgx_request.in);
          if (length == (int) request_length_uint) {
            request_content[length] = 0;
            result = handler(request_method, request_length_uint, request_content) != 0 ? 1 : 0;
            memset(request_content, 0, REQUEST_MAX_LENGTH);
          } else {
            fcgi_printf(
              "Status: 408 Request Timeout\r\n"
              "\r\n");

            result = 1;
          }
        } else {
          fcgi_printf(
            "Status: 413 Payload Too Large\r\n"
            "\r\n",
            "Content-Length >= %d",
            REQUEST_MAX_LENGTH);

          result = 1;
        }
      } else {
        fcgi_printf(
          "Status: 500 Internal Server Error\r\n"
          "\r\n"
          "FastCGI parameter CONTENT_LENGTH is not set");
      }
    } else {
      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "FastCGI parameter REQUEST_METHOD is not set");
    }
  }

  FCGX_Finish_r(&fcgx_request);
  return result;
}

const char* fcgi_getenv(const char* env_name) {
  const char* value = FCGX_GetParam(env_name, fcgx_request.envp);
  return value != 0 ? value : "";
}

int fcgi_printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  const int result = FCGX_VFPrintF(fcgx_request.out, format, ap);
  va_end(ap);
  return result;
}
