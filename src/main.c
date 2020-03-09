#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fcgi_stdio.h"
#include "voltronic_fcgi.h"

static void fastcgi_loop();

#define READ_BUFFER_SIZE   1024
#define WRITE_BUFFER_SIZE  2048

static char read_buffer[READ_BUFFER_SIZE];
static char write_buffer[WRITE_BUFFER_SIZE + 8];
static voltronic_dev_t dev = 0;
static int devErrorCount = 0;

int main() {
  while (FCGI_Accept() >= 0) {
    fastcgi_loop();

    memset(read_buffer, 0, READ_BUFFER_SIZE);
    memset(write_buffer, 0, WRITE_BUFFER_SIZE);

    /*
    * Process has reached its error threshold,
    * allow it to die so another can be create
    */
    if (devErrorCount > 5) {
      break;
    }
  }

  if (dev != 0) {
    voltronic_dev_close(dev);
  }

  return 0;
}

static int ensure_dev_inititialized();
static size_t read_request_data();

static void fastcgi_loop() {
  if (ensure_dev_inititialized()) {
    const size_t request_length = read_request_data();
    if (request_length > 0) {
        const int bytes_read = voltronic_dev_execute(dev,
          read_buffer,
          request_length,
          write_buffer,
          WRITE_BUFFER_SIZE,
          2000);

        if (bytes_read > 0) {
          write_buffer[bytes_read] = 0;
          printf("Status: 200 OK\r\n"
            "\r\n"
            "%s", write_buffer);

          return;
        } else {
          ++devErrorCount;

          printf("Status: 503 Service Unavailable\r\n"
            "\r\n");

          return;
        }
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");

      return;
    }

    return;
  }
}

static int validate_post_request_method() {
  const char* request_method = getenv("REQUEST_METHOD");
  if (request_method != 0 && strcmp(request_method, "POST") == 0) {
    return 1;
  } else {
    printf("Status: 405 Method Not Allowed\r\n"
      "Allow: POST\r\n"
      "\r\n");

    return 0;
  }
}

static unsigned int get_content_length() {
  if (validate_post_request_method()) {
    const char* content_length = getenv("CONTENT_LENGTH");
    if (content_length != 0) {
      long len = strtol(content_length, 0, 10);
      if (len > 0) {
        if (len < READ_BUFFER_SIZE) {
          return (unsigned int) len;
        } else {
          printf("Status: 413 Payload Too Large\r\n"
            "\r\n");

          return 0;
        }
      } else {
        printf("Status: 400 Bad Request\r\n"
          "\r\n");

        return 0;
      }
    } else {
      printf("Status: 411 Length Required\r\n"
        "\r\n");

      return 0;
    }
  } else {
    return 0;
  }
}

static size_t read_request_data() {
  unsigned int content_length = get_content_length();
  if (content_length != 0) {
    size_t bytes_read = fread(read_buffer, sizeof(char), content_length, stdin);
    if (bytes_read >= content_length) {
      return content_length;
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");

      return 0;
    }
  } else {
    return 0;
  }
}

static int ensure_dev_inititialized() {
  if (dev != 0) {
    return 1;
  } else {
    const voltronic_dev_t new_dev = new_voltronic_dev();
    if (new_dev != 0) {
      dev = new_dev;
      devErrorCount = 0;
      return 1;
    } else {
      ++devErrorCount;
      return 0;
    }
  }
}
