## voltronic-inverter/fcgi-interface
FCGI implementation to interface with [Voltronic](http://voltronicpower.com) devices like the Axpert &amp; InfiniSolar

## License
This entire library is licenced under GNU GENERAL PUBLIC LICENSE v3

That means if you want to use this library you can, provided your source code is also also open sourced.

**I will not take lightly any use of this implementation in ANY closed source software**

## Description
Devices from Voltronic are shipped with 4 possible hardware interfaces: RS232, USB, Bluetooth & RS485

All the interfaces share the same underlying communication protocol

## Usage
This library can be used with any fast cgi capable web server

Testing was conducted using nginx & lighttpd, this document describes nginx configuration

Steps to use:

**Alternatively you can search for a pre-existing guide for your platform OR install script at [voltronic-inverter/web](https://github.com/voltronic-inverter/web)**

### Install nginx
- It is recommened you google this step.  There are a LOT of tutorials available online for every conceivable configuration.

### Configure nginx for serial port

**Edit nginx.conf**
```conf
# Feel free to use whatever location suits you
location /voltronic/serial {
  fastcgi_pass   127.0.0.1:9001;

  # Required parameters
  fastcgi_param  REQUEST_METHOD      $request_method;
  fastcgi_param  CONTENT_LENGTH      $content_length;
  fastcgi_param  QUERY_STRING        $query_string;

  # Voltronic device configuration
  # Remove the # in front of #fastcgi_param to uncomment the line
  fastcgi_param  SERIAL_PORT_NAME                     "/dev/tty.usbserial";

  #fastcgi_param  SERIAL_PORT_BAUD_RATE               "2400";  # Optional, default: 2400
  #fastcgi_param  SERIAL_PORT_DATA_BITS               "8";     # Optional, default: 8
  #fastcgi_param  SERIAL_PORT_STOP_BITS               "1";   # Optional, default: 1
  #fastcgi_param  SERIAL_PORT_PARITY                  "none";   # Optional, default: none
  #fastcgi_param  VOLTRONIC_DEVICE_EXPECTS_CRC        "true"; # Optional, default: true
  #fastcgi_param  VOLTRONIC_DEVICE_RESPONDS_WITH_CRC  "true"; # Optional, default: true
  #fastcgi_param  VERIFY_VOLTRONIC_RESPONSE_CRC       "true"; # Optional, default: true

  add_header Cache-Control no-cache always;
  add_header Content-Type "text/plain; charset=UTF-8" always;
}
```

### Configure nginx for usb

**Edit nginx.conf**
```conf
# Feel free to use whatever location suits you
location /voltronic/usb {
  fastcgi_pass   127.0.0.1:9002;

  # Required parameters
  fastcgi_param  REQUEST_METHOD      $request_method;
  fastcgi_param  CONTENT_LENGTH      $content_length;
  fastcgi_param  QUERY_STRING        $query_string;

  # Voltronic device configuration
  # Remove the # in front of #fastcgi_param to uncomment the line
  #fastcgi_param  USB_SERIAL_NUMBER                   "<always seems to be empty>";  # optional
  #fastcgi_param  VOLTRONIC_DEVICE_EXPECTS_CRC        "true"; # Optional, default: true
  #fastcgi_param  VOLTRONIC_DEVICE_RESPONDS_WITH_CRC  "true"; # Optional, default: true
  #fastcgi_param  VERIFY_VOLTRONIC_RESPONSE_CRC       "true"; # Optional, default: true

  add_header Cache-Control no-cache always;
  add_header Content-Type "text/plain; charset=UTF-8" always;
}
```

### Start fcgi2 process
You either need to [build the binary yourself](https://github.com/voltronic-inverter/binaries/tree/master/build) **OR** [choose a precompiled binary](https://github.com/voltronic-inverter/binaries)

On all operating system **other than Windows**, you need to start a FCGI deamon to run the process:
```sh
spawn-fcgi -p 9001 -n voltronic_fcgi_serial
# OR
spawn-fcgi -p 9002 -n voltronic_fcgi_USB
```

On Windows the ports given above are fixed: 9001 for Serial, 9002 for USB

### Test
Send a query to your nginx:

**Basic command:**
`curl -X POST -d 'QPI' 'http://127.0.0.1:8080/voltronic/usb`

This will either produce an error message or for example like `(PI30`
Errors are distinguished by the HTTP code (400, 500, ...)

**Variable timeout:**
`curl -X POST -d 'QPIGS' 'http://127.0.0.1:8080/voltronic/usb?timeout_milliseconds=2000`

## Input methods
Devices from Voltronic are shipped with 4 possible hardware interfaces: RS232, USB, Bluetooth & RS485

All input methods share the same bandwidth & latency.
Although it would appear at surface that USB should be faster, no measureable difference exists to device response time and symbol rate.

USB is also an asynchronous protocol and as such could be influenced by other factors slowing it down further

### Simultaneous communication across multiple interfaces
During testing it was found that simultaneous communication across USB & RS232 for example would result in device lockup.
The device keeps operating, but the device will no longer respond to input or produce output.

As such it is adviced to pick an interface and use it exclusively

### RS232
Nothing special to mention here, synchronous protocol with the following configuration:
- **Baud** *2400*
- **Data bits** *8*
- **Stop bits** *1*
- **Parity** *None* (provided by CRC16)

### USB
The device makes use of an [HID interface](https://en.wikipedia.org/wiki/USB_human_interface_device_class).

**Avoiding the need for root in Linux**

Make use of [**udev**](https://wiki.debian.org/udev) to specify more broad access:

```bash
# may require root
touch /etc/udev﻿/rules.d/15-voltronic.rules
echo 'ATTRS{idVendor}=="0665", ATTRS{idProduct}=="5161", SUBSYSTEMS=="usb", ACTION=="add", MODE="0666", SYMLINK+="hid.voltronic"' > /etc/udev﻿/rules.d/15-voltronic.rules
```

When the device is connected it will present in `/dev/hid.voltronic`.

Note that if multiple devices are to be connected to the same machine, an additional **udev** parameter should be specified such as the device serial number to with different symlink names
