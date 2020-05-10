#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "voltronic_fcgi.h"
#include "voltronic_dev_serial.h"
#include "fcgi_stdio.h"

static const char* parse_serial_port_name(int* parse_result) {
  const char* value = getenv("SERIAL_PORT_NAME");
  if (value != 0) {
    return value;
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "SERIAL_PORT_NAME fastcgi parameter not specified");

    *parse_result = 0;
    return "";
  }
}

static baud_rate_t parse_baud_rate(int* parse_result) {
  const char* cstring = parse_env("SERIAL_PORT_BAUD_RATE", "2400");

  unsigned int value = 0;
  for(unsigned int count = 0; count < 8; ++count) {
    const char ch = *cstring;

    if (ch >= '0' && ch <= '9') {
      value = (value * 10) + (ch - '0');
    } else {
      return (baud_rate_t) value;
    }

    cstring += sizeof(char);
  }

  printf("Status: 500 Internal Server Error\r\n"
    "\r\n"
    "SERIAL_PORT_BAUD_RATE fastcgi parameter specified is invalid");

  *parse_result = 0;
  return 0;
}

static data_bits_t parse_data_bits(int* parse_result) {
  const char* value = parse_env("SERIAL_PORT_DATA_BITS", "8");
  if (strcmp("5", value) == 0) {
    return DATA_BITS_FIVE;
  } else if (strcmp("6", value) == 0) {
    return DATA_BITS_SIX;
  } else if (strcmp("7", value) == 0) {
    return DATA_BITS_SEVEN;
  } else if (strcmp("8", value) == 0) {
    return DATA_BITS_EIGHT;
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "SERIAL_PORT_DATA_BITS fastcgi parameter specified is invalid");

    *parse_result = 0;

    return DATA_BITS_EIGHT;
  }
}

static stop_bits_t parse_stop_bits(int* parse_result) {
  const char* value = parse_env("SERIAL_PORT_STOP_BITS", "1");
  if (strcmp("1", value) == 0) {
    return STOP_BITS_ONE;
  } else if (strcmp("1.5", value) == 0) {
    return STOP_BITS_ONE_AND_ONE_HALF;
  } else if (strcmp("2", value) == 0) {
    return STOP_BITS_TWO;
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "SERIAL_PORT_STOP_BITS fastcgi parameter specified is invalid");

    *parse_result = 0;

    return STOP_BITS_ONE;
  }
}

static serial_parity_t parse_parity(int* parse_result) {
  const char* value = parse_env("SERIAL_PORT_PARITY", "none");
  if (strcmp("none", value) == 0) {
    return SERIAL_PARITY_NONE;
  } else if (strcmp("odd", value) == 0) {
    return SERIAL_PARITY_ODD;
  } else if (strcmp("even", value) == 0) {
    return SERIAL_PARITY_EVEN;
  } else if (strcmp("mark", value) == 0) {
    return SERIAL_PARITY_MARK;
  } else if (strcmp("space", value) == 0) {
    return SERIAL_PARITY_SPACE;
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "SERIAL_PORT_PARITY fastcgi parameter specified is invalid");

    *parse_result = 0;

    return SERIAL_PARITY_NONE;
  }
}

voltronic_dev_t new_voltronic_dev(void) {
  int parse_result = 1;

  const char* port_name = parse_serial_port_name(&parse_result);
  if (parse_result) {
    const baud_rate_t baud_rate = parse_baud_rate(&parse_result);
    if (parse_result) {
      const data_bits_t data_bits = parse_data_bits(&parse_result);
      if (parse_result) {
        const stop_bits_t stop_bits = parse_stop_bits(&parse_result);
        if (parse_result) {
          const serial_parity_t parity = parse_parity(&parse_result);
          if (parse_result) {
            const voltronic_dev_t dev = voltronic_serial_create(
              port_name,
              baud_rate,
              data_bits,
              stop_bits,
              parity);

            if (dev != 0) {
              return dev;
            } else {
              const char* errno_prefix = "";
              const char* errno_str = "";
              if (errno > 0) {
                errno_prefix = "; ";
                errno_str = strerror(errno);
              }

              printf("Status: 500 Internal Server Error\r\n"
                "\r\n"
                "Could not open serial connection to '%s'%s%s",
                port_name, errno_prefix, errno_str);
            }
          }
        }
      }
    }
  }

  return 0;
}

const char* fcgi_default_port(void) {
  return ":9001";
}
