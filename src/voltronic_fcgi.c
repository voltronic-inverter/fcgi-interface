#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "voltronic_crc.h"
#include "voltronic_fcgi.h"
#include "fcgi_stdio.h"

#define DEFAULT_TIMEOUT_MILLISECONDS 2000
#define TIMEOUT_PARAM_NAME "timeout_milliseconds"
#define READ_BUFFER_SIZE   1024
#define WRITE_BUFFER_SIZE  2048
#define TIMEOUT_FLUSH_COMMAND "\r\r"

static char read_buffer[READ_BUFFER_SIZE + 8];
static char write_buffer[WRITE_BUFFER_SIZE + 8];

static voltronic_dev_t dev = 0;

static size_t request_length = 0;
static unsigned int timeout_milliseconds = 0;
static unsigned int successful_io_operations = 0;

static unsigned int fast_parse_int(const char* cstring);
static unsigned int parse_timeout(const char* query_string);
static void clear_buffers(void);
static int initialize_dev(void);
static int fill_read_buffer(void);
static int execute_request(void);

#define TIMEOUT_PARAM_LENGTH (sizeof(TIMEOUT_PARAM_NAME) - 1)
#define TIMEOUT_FLUSH_COMMAND_LENGTH (sizeof(TIMEOUT_FLUSH_COMMAND) - 1)

int voltronic_fcgi(const char* content_length, const char* query_string) {
  request_length = fast_parse_int(content_length);
  timeout_milliseconds = parse_timeout(query_string);

  if (request_length > 0 && timeout_milliseconds > 0) {
    if (initialize_dev()) {
      if (fill_read_buffer()) {
        return execute_request();
      }
    } else {
      return 0;
    }
  } else {
    printf("Status: 400 Bad Request\r\n"
      "\r\n");
  }

  return 1;
}

static int execute_request(void) {
  const int bytes_read = voltronic_dev_execute(
    dev,
    read_buffer,
    request_length,
    write_buffer,
    WRITE_BUFFER_SIZE,
    timeout_milliseconds);

  if (bytes_read > 0) {
    if (successful_io_operations < 0xFF) {
      ++successful_io_operations;
    }

    write_buffer[bytes_read] = 0;

    printf("Status: 200 OK\r\n"
      "Successful-IO-operations: %d\r\n"
      "\r\n"
      "%s",
      successful_io_operations,
      write_buffer);

    return 1;
  } else {
    const char* errno_str = "";
    if (errno > 0) {
      errno_str = strerror(errno);
    }

    printf("Status: 503 Service Unavailable\r\n"
      "\r\n"
      "%s", errno_str);

    voltronic_dev_write(dev, TIMEOUT_FLUSH_COMMAND, TIMEOUT_FLUSH_COMMAND_LENGTH);

    return 0;
  }
}

static int fill_read_buffer(void) {
  if (request_length < READ_BUFFER_SIZE) {
    const size_t bytes_read = fread(
      read_buffer,
      sizeof(char),
      request_length,
      stdin);

    if (bytes_read >= request_length) {
      return 1;
    }
  } else {
    printf("Status: 413 Payload Too Large\r\n"
      "\r\n");
  }

  return 0;
}

static inline void close_dev(void) {
  voltronic_dev_close(dev);
}

static int initialize_dev(void) {
  if (dev != 0) {
    return 1;
  } else if (is_platform_supported()) {
    clear_buffers();
    dev = new_voltronic_dev();
    if (dev != 0) {
      atexit(close_dev);
      return 1;
    }
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "The CRC calculation is not supported on the current platform, "
      "please contact the developer of this CGI library to fix it!");
  }

  return 0;
}

static unsigned int parse_timeout(const char* query_string) {
  while(1) {
    if (strncmp(query_string, TIMEOUT_PARAM_NAME, TIMEOUT_PARAM_LENGTH) == 0) {
      query_string += TIMEOUT_PARAM_LENGTH;
      if (*query_string == '=') {
        return fast_parse_int(query_string + sizeof(char));
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
}

static unsigned int fast_parse_int(const char* cstring) {
  unsigned int value = 0;
  cstring -= sizeof(char);
  for(unsigned int count = 0; count < 8; ++count) {
    cstring += sizeof(char);
    const char ch = *cstring;
    if (ch >= '0' && ch <= '9') {
      value = (value * 10) + (ch - '0');
    } else {
      return value;
    }
  }

  return 0;
}

static inline void clear_buffers(void) {
  memset(read_buffer, 0, READ_BUFFER_SIZE + 1);
  memset(write_buffer, 0, WRITE_BUFFER_SIZE + 1);
}
