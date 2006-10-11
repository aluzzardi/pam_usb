# Set to 'yes' to include debugging informations, e.g. DEBUG=yes make -e
DEBUG		:= no

# compiler/linker options
CC		:= gcc
CFLAGS		:= -Wall `pkg-config --cflags libxml-2.0` \
	`pkg-config --cflags hal-storage`
LDFLAGS		:= `pkg-config --libs libxml-2.0` \
	`pkg-config --libs hal-storage`

# common source files
SRCS		:= src/conf.c \
		   src/log.c \
		   src/xpath.c \
		   src/hal.c \
		   src/pad.c \
		   src/volume.c \
		   src/local.c \
		   src/device.c
OBJS		:= $(SRCS:.c=.o)

# pam_usb
PAM_USB_SRCS	:= src/pam.c
PAM_USB_OBJS	:= $(PAM_USB_SRCS:.c=.o)
PAM_USB		:= pam_usb.so
PAM_USB_LDFLAGS	:= -shared $(LDFLAGS)

# pusb_check
PUSB_CHECK_SRCS	:= src/pusb_check.c
PUSB_CHECK_OBJS	:= $(PUSB_CHECK_SRCS:.c=.o)
PUSB_CHECK	:= pusb_check

ifeq (yes, ${DEBUG})
	CFLAGS := ${CFLAGS} -ggdb
endif

all		: $(PAM_USB) $(PUSB_CHECK)

$(PAM_USB)	: $(OBJS) $(PAM_USB_OBJS)
		$(CC) -o $(PAM_USB) $(PAM_USB_LDFLAGS) $(OBJS) $(PAM_USB_OBJS)

$(PUSB_CHECK)	: $(OBJS) $(PUSB_CHECK_OBJS)
		$(CC) -o $(PUSB_CHECK) $(LDFLAGS) $(OBJS) $(PUSB_CHECK_OBJS)

%.o		: %.c
		${CC} -c ${CFLAGS} $< -o $@

clean		:
		rm -f $(PAM_USB) $(PUSB_CHECK) $(OBJS) $(PUSB_CHECK_OBJS) $(PAM_USB_OBJS)
