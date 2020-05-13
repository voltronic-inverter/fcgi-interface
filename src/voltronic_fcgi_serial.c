#include <errno.h>
#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "voltronic_dev_serial.h"

static const char* parse_serial_port_name(int* parse_result) {
  const char* value = fcgi_getenv("SERIAL_PORT_NAME");
  if (value != 0) {
    return value;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_NAME is not set; Expecting COM port (Windows) or /dev/ or similar path (Posix)");

    *parse_result = 0;
    return "";
  }
}

static baud_rate_t parse_baud_rate(int* parse_result) {
  const char* baud_rate_cstring = fcgi_getenv("SERIAL_PORT_BAUD_RATE");
  if (baud_rate_cstring == 0) {
    return 2400;
  }

  const unsigned int baud_rate = parse_uint(baud_rate_cstring);
  if (baud_rate != 0) {
    return (baud_rate_t) baud_rate;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_BAUD_RATE=\"%s\" is invalid; Expecting a positive Integer",
      baud_rate);

    *parse_result = 0;
    return 0;
  }
}

static data_bits_t parse_data_bits(int* parse_result) {
  const char* data_bits = fcgi_getenv("SERIAL_PORT_DATA_BITS");
  if (data_bits == 0) {
    return DATA_BITS_EIGHT;
  }

  if (cstring_equals("5", data_bits)) {
    return DATA_BITS_FIVE;
  } else if (cstring_equals("6", data_bits)) {
    return DATA_BITS_SIX;
  } else if (cstring_equals("7", data_bits)) {
    return DATA_BITS_SEVEN;
  } else if (cstring_equals("8", data_bits)) {
    return DATA_BITS_EIGHT;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_DATA_BITS=\"%s\" is invalid; Expecting a value from the set [5, 6, 7, 8]",
      data_bits);

    *parse_result = 0;

    return DATA_BITS_EIGHT;
  }
}

static stop_bits_t parse_stop_bits(int* parse_result) {
  const char* stop_bits = fcgi_getenv("SERIAL_PORT_STOP_BITS");
  if (stop_bits == 0) {
    return STOP_BITS_ONE;
  }

  if (cstring_equals("1", stop_bits)) {
    return STOP_BITS_ONE;
  } else if (cstring_equals("1.5", stop_bits)) {
    return STOP_BITS_ONE_AND_ONE_HALF;
  } else if (cstring_equals("2", stop_bits)) {
    return STOP_BITS_TWO;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_STOP_BITS=\"%s\" is invalid; Expecting a value from the set [1, 1.5, 2]",
      stop_bits);

    *parse_result = 0;

    return STOP_BITS_ONE;
  }
}

static serial_parity_t parse_parity(int* parse_result) {
  const char* parity = fcgi_getenv("SERIAL_PORT_PARITY");
  if (parity == 0) {
    return SERIAL_PARITY_NONE;
  }

  if (cstring_equals("none", parity)) {
    return SERIAL_PARITY_NONE;
  } else if (cstring_equals("odd", parity)) {
    return SERIAL_PARITY_ODD;
  } else if (cstring_equals("even", parity)) {
    return SERIAL_PARITY_EVEN;
  } else if (cstring_equals("mark", parity)) {
    return SERIAL_PARITY_MARK;
  } else if (cstring_equals("space", parity)) {
    return SERIAL_PARITY_SPACE;
  } else {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_STOP_BITS=\"%s\" is invalid; Expecting a value from the set [none, even, odd, mark, space]",
      parity);

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
            reset_last_error();
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
              errno_t errnum;
              if (get_last_error(&errnum)) {
                errno_prefix = "; ";
                errno_str = get_error_string(&errnum);
              }

              fcgi_printf(
                "Status: 500 Internal Server Error\r\n"
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
