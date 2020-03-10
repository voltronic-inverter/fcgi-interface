#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fcgi_stdio.h"
#include "voltronic_fcgi.h"

static void voltronic_fcgi_loop(void);

#define READ_BUFFER_SIZE   1024
#define WRITE_BUFFER_SIZE  2048

static char read_buffer[READ_BUFFER_SIZE];
static char write_buffer[WRITE_BUFFER_SIZE + 8];

static voltronic_dev_t dev = 0;

static unsigned int devErrorCount = 0;
static unsigned int lastWasIOError = 0;

static int fcgi_main(const char* request_method) {
  if (strcmp("PUT", request_method) == 0) {

    voltronic_fcgi_loop();

    memset(read_buffer, 0, READ_BUFFER_SIZE);
    memset(write_buffer, 0, WRITE_BUFFER_SIZE);

    /*
     * Process has reached its error threshold,
     * allow it to die so another can be create
     */
    if (devErrorCount > 3) {
      return 1;
    }

  } else if (strcmp("DELETE", request_method) == 0) {

    printf("Status: 200 OK\r\n"
      "\r\n"
      "Terminating FCGI process");

    return 1;

  } else {

    printf("Status: 405 Method Not Allowed\r\n"
      "Allow: PUT, DELETE\r\n"
      "\r\n");

  }

  return 0;
}

static int ensure_dev_inititialized(void);
static size_t read_request_data(void);
static void voltronic_dev_clear(void);

static void voltronic_fcgi_loop() {
  if (ensure_dev_inititialized()) {
    const size_t request_length = read_request_data();
    if (request_length > 0) {
      if (lastWasIOError) {
        voltronic_dev_clear();
      }

      const int bytes_read = voltronic_dev_execute(
        dev,
        read_buffer,
        request_length,
        write_buffer,
        WRITE_BUFFER_SIZE,
        2000);

      if (bytes_read > 0) {
        if (!lastWasIOError) {
            devErrorCount = 0;
        }

        lastWasIOError = 0;
        write_buffer[bytes_read] = 0;

        printf("Status: 200 OK\r\n"
          "\r\n"
          "%s", write_buffer);
      } else {
        lastWasIOError = 1;
        ++devErrorCount;

        const char* errno_str = "";
        if (errno > 0) {
          errno_str = strerror(errno);
        }

        printf("Status: 503 Service Unavailable\r\n"
          "\r\n"
          "%s", errno_str);
      }
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");
    }
  }
}

static void voltronic_dev_clear() {
  for (unsigned int count = 0; count < 255; ++count) {
    const int result = voltronic_dev_read(
      dev, read_buffer, READ_BUFFER_SIZE, 100);

    if (result <= 0) {
      break;
    }
  }
}

static unsigned int get_content_length() {
  const char* content_length = getenv("CONTENT_LENGTH");
  if (content_length != 0) {
    long len = strtol(content_length, 0, 10);
    if (len > 0) {
      if (len < READ_BUFFER_SIZE) {
        return (unsigned int) len;
      } else {
        printf("Status: 413 Payload Too Large\r\n"
          "\r\n");
      }
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");
    }
  } else {
    printf("Status: 411 Length Required\r\n"
      "\r\n");
  }

  return 0;
}

static size_t read_request_data() {
  unsigned int content_length = get_content_length();
  if (content_length != 0) {
    const size_t bytes_read = fread(read_buffer, sizeof(char), content_length, stdin);
    if (bytes_read >= content_length) {
      return content_length;
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");
    }
  }

  return 0;
}

static int ensure_dev_inititialized() {
  if (dev != 0) {
    return 1;
  } else {
    dev = new_voltronic_dev();
    if (dev != 0) {
      devErrorCount = 0;
      return 1;
    } else {
      ++devErrorCount;
      return 0;
    }
  }
}

int main() {
  while (FCGI_Accept() >= 0) {
    const char* request_method = getenv("REQUEST_METHOD");
    if (request_method != 0) {
      if (fcgi_main(request_method)) {
        break;
      }
    } else {
      printf("Status: 400 Bad Request\r\n"
        "\r\n");
    }
  }

  if (dev != 0) {
    voltronic_dev_close(dev);
  }

  return 0;
}
