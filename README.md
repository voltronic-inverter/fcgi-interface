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

Testing was conducted using nginx.

Steps to use:

### Install nginx
- It is recommened you google this step.  There are a LOT of tutorials available online for every conceivable configuration.

### Configure nginx for serial port

**Edit nginx.conf**
```conf
location /axpert/command {
  fastcgi_pass   127.0.0.1:9000;
  fastcgi_param  SERIAL_PORT_NAME    "/dev/tty.usbserial";
  fastcgi_param  REQUEST_METHOD      $request_method;
  fastcgi_param  CONTENT_LENGTH      $content_length;

  add_header Cache-Control no-cache always;
  add_header Content-Type "text/plain; charset=UTF-8" always;
}
```

### Configure nginx for usb

**Edit nginx.conf**
```conf
location /axpert/command {
  fastcgi_pass   127.0.0.1:9000;
  #fastcgi_param  USB_SERIAL_NUMBER   "<some serial number here>";  # optional
  fastcgi_param  REQUEST_METHOD      $request_method;
  fastcgi_param  CONTENT_LENGTH      $content_length;

  add_header Cache-Control no-cache always;
  add_header Content-Type "text/plain; charset=UTF-8" always;
}
```

### Start fcgi2 process
You first need to build the binary of your choice as directed below

ie. for serial port, you would run `make serial`
This produces an executable `voltronic_fcgi_serial`

```sh
spawn-fcgi -p 9000 -n voltronic_fcgi_serial
```

### Test
Send a query to your nginx:

`curl -X POST -d 'QPI' 'http://127.0.0.1:8080//axpert/command'`

This will either produce an error message our the output

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
In Linux the device is presented as a [*HIDRaw* device](https://www.kernel.org/doc/Documentation/hid/hidraw.txt)

It is **not** a USB->Serial

So in Linux for example:

**Ruby:**
```ruby
fd = File.open('/dev/hidraw0', IO::RDWR|IO::NONBLOCK) # May need root, or make the file 666 using udev rules
fd.binmode
fd.sync = true
fd.write("QPI\xBE\xAC\r") # Will write QPI => Returns 6
fd.gets("\r") #=> "(PI30\x9A\v\r"
```

**Python:**
```python
import os, sys
fd = open("/dev/hidraw0", os.O_RDWR|os.O_NONBLOCK)
os.write(fd, "QPI\xBE\xAC\r")
os.read(fd, 512)
```

**Avoiding the need for root**

Make use of [**udev**](https://wiki.debian.org/udev) to specify more broad access:

```bash
# may require root
touch /etc/udev﻿/rules.d/15-voltronic.rules
echo 'ATTRS{idVendor}=="0665", ATTRS{idProduct}=="5161", SUBSYSTEMS=="usb", ACTION=="add", MODE="0666", SYMLINK+="hidVoltronic"' > /etc/udev﻿/rules.d/15-voltronic.rules
```

When the device is connected it will present in `/dev/hidVoltronic`.

Note that if multiple devices are to be connected to the same machine, an additional **udev** parameter should be specified such as the device serial number to with different symlink names

### Bluetooth
Newer generation [Axpert devices](http://voltronicpower.com/en-US/Product/Detail/Axpert-King-3KVA-5KVA) feature Bluetooth

No testing has been completed on these devices but Bluetooth simply operates exactly like RS232 and therefore there is no reason to believe it would be otherwise

### RS485
Newer generation [Axpert devices](http://voltronicpower.com/en-US/Product/Detail/Axpert-King-3KVA-5KVA) feature RS485 support

No testing has been completed on these devices but there is no reason to believe the underlying protocol has changed at all

## Dependencies
To remove a lot of the heavy lifting, the library relies:
- [libserialport](https://sigrok.org/wiki/Libserialport)
- [HIDAPI](https://github.com/signal11/hidapi)
- [fgci2](https://github.com/FastCGI-Archives/fcgi2)

## Building

### Dependencies

**Install depedencies:**
Each operating system will have a different list of prerequisites before the dependencies can be built.

See a more detailed list below 


**Build libfcgi:**
```sh
git clone https://github.com/FastCGI-Archives/fcgi2.git lib/libfcgi2/
cd lib/libfcgi2/
./autogen.sh
./configure
make
make install # Requires sudo or su
```

**If you intend on using serial port; Build libserialport:**
```sh
git clone git://sigrok.org/libserialport lib/libserialport/
cd lib/libserialport/
./autogen.sh
./configure
make
make install # Requires sudo or run as su
```

**If you intend on using USB; Build libhidapi:**
```sh
git clone https://github.com/signal11/hidapi.git lib/libhidapi/
cd lib/libhidapi/
./bootstrap
./configure
make
make install # Requires sudo or su
```

### FreeBSD

Tested on FreeBSD 10, 11, 12

Required dependencies:
```sh
su
pkg install gcc git autoconf automake libtool libiconv gmake
```

**gmake** instead of make to build

### Linux

Required dependencies:

**Ubuntu:**
```sh
sudo apt-get clean
sudo apt-get update
sudo apt-get install gcc git autoconf automake libtool pkg-config libudev-dev libusb-1.0-0-dev
```

**Amazon Linux:**
```sh
sudo yum clean all
sudo yum install gcc git autoconf automake libtool pkg-config libudev-devel libusb1-devel
```

**make** to build

### Windows

Built using Ubuntu and miniGW.  Building natively in Windows is a challenge for a better software developer than this author.

**Using Ubuntu:**
```sh
sudo apt-get clean
sudo apt-get update
sudo apt-get install gcc git autoconf automake libtool pkg-config libudev-dev libusb-1.0-0-dev
```

### x86
```sh
sudo apt-get install gcc-mingw-w64-i686
```

### x64
```sh
sudo apt-get install gcc-mingw-w64-x86-64
```

### OSX

The library was developed & tested on OSX High Sierra but the system is setup as a dev machine.
As such the complete list of dependencies have long since been forgotten.

At the very least:
- Homebrew
- gcc
- git

**make** to build
