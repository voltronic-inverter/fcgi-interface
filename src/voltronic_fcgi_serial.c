#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "voltronic_dev_serial.h"

#define IS_PARSE_ERROR_SET(_parse__status_) \
  ((*(_parse__status_)) == 0)

#define SET_PARSING_ERROR(_parse__status_) \
  (*(_parse__status_)) = 0

#define DEFAULT_BAUD_RATE  2400
#define DEFAULT_DATA_BITS  DATA_BITS_EIGHT
#define DEFAULT_STOP_BITS  STOP_BITS_ONE
#define DEFAULT_PARITY     SERIAL_PARITY_NONE;

static const char* parse_serial_port_name(int* parse_status) {
  const char* value = fcgi_getenv("SERIAL_PORT_NAME");
  if (is_cstring_empty(value) && !IS_PARSE_ERROR_SET(parse_status)) {
    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "FastCGI parameter SERIAL_PORT_NAME is not set; Expecting COM port (Windows) or /dev/ or similar path (Posix)");

    SET_PARSING_ERROR(parse_status);
  }

  return value;
}

static baud_rate_t parse_baud_rate(int* parse_status) {
  const char* baud_rate_cstring = fcgi_getenv("SERIAL_PORT_BAUD_RATE");
  if (!is_cstring_empty(baud_rate_cstring)) {
    const unsigned int baud_rate = parse_uint(baud_rate_cstring);
    if (baud_rate != 0) {
      return (baud_rate_t) baud_rate;
    } else if (!IS_PARSE_ERROR_SET(parse_status)) {
      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "FastCGI parameter SERIAL_PORT_BAUD_RATE=\"%s\" is invalid; Expecting a positive Integer",
        baud_rate);

      SET_PARSING_ERROR(parse_status);
    }
  }

  return DEFAULT_BAUD_RATE;
}

static data_bits_t parse_data_bits(int* parse_status) {
  const char* data_bits = fcgi_getenv("SERIAL_PORT_DATA_BITS");
  if (!is_cstring_empty(data_bits)) {
    if (cstring_equals("5", data_bits)) {
      return DATA_BITS_FIVE;
    } else if (cstring_equals("6", data_bits)) {
      return DATA_BITS_SIX;
    } else if (cstring_equals("7", data_bits)) {
      return DATA_BITS_SEVEN;
    } else if (cstring_equals("8", data_bits)) {
      return DATA_BITS_EIGHT;
    } else if (!IS_PARSE_ERROR_SET(parse_status)) {
      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "FastCGI parameter SERIAL_PORT_DATA_BITS=\"%s\" is invalid; Expecting a value from the set [5, 6, 7, 8]",
        data_bits);

      SET_PARSING_ERROR(parse_status);
    }
  }

  return DEFAULT_DATA_BITS;
}

static stop_bits_t parse_stop_bits(int* parse_status) {
  const char* stop_bits = fcgi_getenv("SERIAL_PORT_STOP_BITS");
  if (!is_cstring_empty(stop_bits)) {
    if (cstring_equals("1", stop_bits)) {
      return STOP_BITS_ONE;
    } else if (cstring_equals("1.5", stop_bits)) {
      return STOP_BITS_ONE_AND_ONE_HALF;
    } else if (cstring_equals("2", stop_bits)) {
      return STOP_BITS_TWO;
    } else if (!IS_PARSE_ERROR_SET(parse_status)) {
      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "FastCGI parameter SERIAL_PORT_STOP_BITS=\"%s\" is invalid; Expecting a value from the set [1, 1.5, 2]",
        stop_bits);

      SET_PARSING_ERROR(parse_status);
    }
  }

  return DEFAULT_STOP_BITS;
}

static serial_parity_t parse_parity(int* parse_status) {
  const char* parity = fcgi_getenv("SERIAL_PORT_PARITY");
  if (!is_cstring_empty(parity)) {
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
    } else if (!IS_PARSE_ERROR_SET(parse_status)) {
      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "FastCGI parameter SERIAL_PORT_STOP_BITS=\"%s\" is invalid; Expecting a value from the set [none, even, odd, mark, space]",
        parity);

      SET_PARSING_ERROR(parse_status);
    }
  }

  return DEFAULT_PARITY;
}

voltronic_dev_t new_voltronic_dev(void) {
  int parse_status = 1;

  const char* port_name = parse_serial_port_name(&parse_status);
  const baud_rate_t baud_rate = parse_baud_rate(&parse_status);
  const data_bits_t data_bits = parse_data_bits(&parse_status);
  const stop_bits_t stop_bits = parse_stop_bits(&parse_status);
  const serial_parity_t parity = parse_parity(&parse_status);

  if (parse_status) {
    const voltronic_dev_t dev = voltronic_serial_create(
      port_name,
      baud_rate,
      data_bits,
      stop_bits,
      parity);

    if (dev != 0) {
      return dev;
    } else {
      const last_error_t last_error = GET_LAST_ERROR();
      const char* errno_prefix = "";
      const char* errno_str = "";
      if (last_error != 0) {
        errno_prefix = "; ";
        errno_str = GET_ERROR_STRING(last_error);
      }

      fcgi_printf(
        "Status: 500 Internal Server Error\r\n"
        "\r\n"
        "Could not open serial connection to '%s'%s%s",
        port_name, errno_prefix, errno_str);
    }
  }

  return 0;
}
