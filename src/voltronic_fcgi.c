#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "voltronic_fcgi.h"
#include "fcgi_stdio.h"

#define DEFAULT_TIMEOUT_MILLISECONDS 2000
#define TIMEOUT_PARAM_NAME         "timeout_milliseconds"
#define WRITE_CRC_ON_EXECUTE_NAME  "write_voltronic_crc"
#define READ_CRC_ON_EXECUTE_NAME   "voltronic_crc_on_read"
#define VERIFY_CRC_ON_EXECUTE_NAME "verify_voltronic_crc_on_read"
#define READ_BUFFER_SIZE   128
#define WRITE_BUFFER_SIZE  1024
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
    write_buffer[bytes_read] = 0;

    printf("Status: 200 OK\r\n"
      "Successful-IO-operations: %d\r\n"
      "\r\n"
      "%s",
      ++successful_io_operations,
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

    voltronic_dev_write(dev, TIMEOUT_FLUSH_COMMAND, TIMEOUT_FLUSH_COMMAND_LENGTH, 500);

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
  } else {
    clear_buffers();
    dev = new_voltronic_dev();
    if (dev != 0) {
      atexit(close_dev);

      if (env_equals("false", WRITE_CRC_ON_EXECUTE_NAME, "true")) {
        unset_voltronic_dev_opt(dev, VOLTRONIC_WRITE_CRC_ON_EXECUTE);
      }

      if (env_equals("false", READ_CRC_ON_EXECUTE_NAME, "true")) {
        unset_voltronic_dev_opt(dev, VOLTRONIC_READ_CRC_ON_EXECUTE);
      }

      if (env_equals("false", VERIFY_CRC_ON_EXECUTE_NAME, "true")) {
        unset_voltronic_dev_opt(dev, VOLTRONIC_VERIFY_CRC_ON_EXECUTE);
      }

      return 1;
    }
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
  for(unsigned int count = 0; count < 8; ++count) {
    const char ch = *cstring;

    if (ch >= '0' && ch <= '9') {
      value = (value * 10) + (ch - '0');
    } else {
      return value;
    }

    cstring += sizeof(char);
  }

  return 0;
}

const char* parse_env(const char* env_name, const char* default_value) {
  const char* value = getenv(env_name);
  return value != 0 ? value : default_value;
}

int env_equals(const char* expected_value, const char* env_name, const char* default_value) {
  const char* value = parse_env(env_name, default_value);
  if (strcmp(expected_value, value) == 0) {
    return 1;
  } else {
    return 0;
  }
}

static inline void clear_buffers(void) {
  memset(read_buffer, 0, READ_BUFFER_SIZE + 1);
  memset(write_buffer, 0, WRITE_BUFFER_SIZE + 1);
}
