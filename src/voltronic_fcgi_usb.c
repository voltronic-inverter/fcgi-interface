#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "voltronic_fcgi.h"
#include "voltronic_dev_usb.h"
#include "version.h"

voltronic_dev_t new_voltronic_dev(FCGX_Request* request) {
  const char* serial_number = parse_env(request, "USB_SERIAL_NUMBER", 0);

  const voltronic_dev_t dev = voltronic_usb_create(
    0x0665,
    0x5161,
    serial_number);

  if (dev != 0) {
    return dev;
  } else {
    const char* serial_prefix = "";
    const char* serial_number_str = "";
    if (serial_number != 0) {
      serial_prefix = ", serial_number=";
      serial_number_str = serial_number;
    }

    const char* errno_prefix = "";
    const char* errno_str = "";
    if (errno > 0) {
      errno_prefix = "; ";
      errno_str = strerror(errno);
    }

    FCGX_FPrintF(request->out,
      "Status: 500 Internal Server Error\r\n"
      VERSION_DESCRIPTION "\r\n"
      "\r\n"
      "Could not connect to any USB device with vendor_id=0x0665, "
      "product_id=0x5161%s%s%s%s",
      serial_prefix, serial_number_str, errno_prefix, errno_str);

    return 0;
  }
}
