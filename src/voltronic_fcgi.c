#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "voltronic_dev.h"

#define DEFAULT_TIMEOUT_MILLISECONDS     2000
#define TIMEOUT_PARAM_NAME               "timeout_milliseconds"
#define WRITE_CRC_PARAM_NAME             "WRITE_VOLTRONIC_CRC"
#define READ_CRC_PARAM_NAME              "READ_VOLTRONIC_CRC"
#define WRITE_BUFFER_SIZE                1024
#define TIMEOUT_FLUSH_COMMAND            "\r\r"

static char write_buffer[WRITE_BUFFER_SIZE];
static int dev_options = 0;
static voltronic_dev_t dev = 0;

static int initialize_dev(void);
static unsigned int parse_timeout(const char* query_string);

#define TIMEOUT_PARAM_LENGTH (sizeof(TIMEOUT_PARAM_NAME) - 1)
#define TIMEOUT_FLUSH_COMMAND_LENGTH (sizeof(TIMEOUT_FLUSH_COMMAND) - 1)

static int voltronic_fcgi_execute(
  const unsigned int content_length,
  const char* request_content,
  const unsigned int timeout_milliseconds) {

  const int bytes_read = voltronic_dev_execute(
    dev,
    dev_options,
    request_content,
    content_length,
    write_buffer,
    WRITE_BUFFER_SIZE,
    timeout_milliseconds);

  if (bytes_read > 0) {
    fcgi_printf(
      "Status: 200 OK\r\n"
      "\r\n"
      "%.*s",
      bytes_read,
      write_buffer);

    return 1;
  } else {
    const last_error_t last_error = GET_LAST_ERROR();
    const char* errno_str = "";
    if (last_error != 0) {
      errno_str = GET_ERROR_STRING(last_error);
    }

    fcgi_printf(
      "Status: 503 Service Unavailable\r\n"
      "\r\n"
      "%s",
      errno_str);

    voltronic_dev_write(
      dev,
      TIMEOUT_FLUSH_COMMAND,
      TIMEOUT_FLUSH_COMMAND_LENGTH,
      500);

    return 0;
  }
}

int voltronic_fcgi_main(
  const unsigned int content_length,
  const char* request_content) {

  const char* query_string = fcgi_getenv("QUERY_STRING");
  const unsigned int timeout_milliseconds = parse_timeout(query_string);
  int result = 1;
  if (timeout_milliseconds != 0) {
    result = initialize_dev();
    if (result != 0) {
      result = voltronic_fcgi_execute(
        content_length, request_content, timeout_milliseconds);
    }
  }

  return result;
}

static inline void close_dev(void) {
  voltronic_dev_close(dev);
}

static inline int is_true(const char* fcgi_env_name) {
  return cstring_equals("true", fcgi_getenv(fcgi_env_name));
}

static inline int is_false(const char* fcgi_env_name) {
  return cstring_equals("false", fcgi_getenv(fcgi_env_name));
}

static int parse_voltronic_options(void) {
  const char* bad_param_name = 0;
  if (is_true(WRITE_CRC_PARAM_NAME)) {
    dev_options &= ~DISABLE_WRITE_VOLTRONIC_CRC;
  } else if (is_false(WRITE_CRC_PARAM_NAME)) {
    dev_options |= DISABLE_WRITE_VOLTRONIC_CRC;
  } else {
    bad_param_name = WRITE_CRC_PARAM_NAME;
  }

  if (is_true(READ_CRC_PARAM_NAME)) {
    dev_options &= ~DISABLE_PARSE_VOLTRONIC_CRC;
  } else if (is_false(READ_CRC_PARAM_NAME)) {
    dev_options |= DISABLE_PARSE_VOLTRONIC_CRC;
  } else {
    bad_param_name = READ_CRC_PARAM_NAME;
  }

  if (bad_param_name == 0) {
    return 1;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter %s is not set; Expecting \"true\" or \"false\"",
      bad_param_name);

    return 0;
  }
}

static int initialize_dev(void) {
  if (dev != 0) {
    return 1;
  } else {
    dev = new_voltronic_dev();
    if (dev != 0) {
      atexit(close_dev);
      return parse_voltronic_options();
    } else {
      return 0;
    }
  }
}

static unsigned int parse_timeout(const char* query_string) {
  if (query_string != 0) {
    while(1) {
      if (strncmp(query_string, TIMEOUT_PARAM_NAME, TIMEOUT_PARAM_LENGTH) == 0) {
        query_string += TIMEOUT_PARAM_LENGTH;
        if (*query_string == '=') {
          const unsigned int timeout = parse_uint(query_string + sizeof(char));
          if (timeout == 0) {
            fcgi_printf(
              "Status: 400 Bad Request\r\n"
              "\r\n"
              "Invalid %s value specified",
              TIMEOUT_PARAM_NAME);
          }

          return timeout;
        }
      }

      query_string -= sizeof(char);
      while(1) {
        const char ch = *(query_string += sizeof(char));
        if (ch == '&') {
          query_string += sizeof(char);
          break;
        } else if (ch == 0) {
          return DEFAULT_TIMEOUT_MILLISECONDS;
        }
      }
    }
  } else {
    return DEFAULT_TIMEOUT_MILLISECONDS;
  }
}
