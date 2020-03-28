# define the C compiler to use
CC = gcc
CP = cp -f

#directories
IDIR = include
LDIR = lib
SDIR = src
ODIR = obj

# define any compile-time flags
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wshadow -O3 -flto -fomit-frame-pointer

# add includes
CFLAGS += -I$(IDIR) -I$(LDIR)/fcgi2/include -I$(LDIR)/libserialport -I$(LDIR)/hidapi/hidapi

# shared libraries
SHARED_LIBS = -lfcgi

# define the C source files
SRCS = $(wildcard $(SDIR)/*.c)

# Object files shared by all directives
SHARED_OBJS = $(ODIR)/main.o $(ODIR)/time_util.o $(ODIR)/voltronic_crc.o $(ODIR)/voltronic_dev.o  $(ODIR)/voltronic_fcgi.o

# Directives
default:
	@echo "Different compile options exist; ie. make serial"
	@echo "  serial - Serial port only"
	@echo "  usb - USB support in Mac, Windows, FreeBSD"
	@echo "  hidraw - USB support in Linux using hidraw"
	@echo "  libusb - USB support using libUSB"

serial: $(SHARED_OBJS) $(ODIR)/voltronic_dev_serial.o $(ODIR)/voltronic_fcgi_serial.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lserialport
	$(CP) $@ voltronic_fcgi_serial
	$(RM) $@

usb: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb.o $(ODIR)/voltronic_fcgi_usb.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi
	$(CP) $@ voltronic_fcgi_usb
	$(RM) $@

hidraw: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb.o $(ODIR)/voltronic_fcgi_usb.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-hidraw
	$(CP) $@ voltronic_fcgi_hidraw
	$(RM) $@

libusb: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb.o $(ODIR)/voltronic_fcgi_usb.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-libusb
	$(CP) $@ voltronic_fcgi_libusb
	$(RM) $@

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)/*.o *~ core voltronic_fcgi_serial voltronic_fcgi_usb voltronic_fcgi_hidraw voltronic_fcgi_libusb $(INCDIR)/*~ 
