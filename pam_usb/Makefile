# Set to 'yes' to include debugging informations, e.g. DEBUG=yes make -e
DEBUG		:= no

# compiler/linker options
CC		:= gcc
CFLAGS		:= $(CFLAGS) -Wall -fPIC `pkg-config --cflags libxml-2.0` \
	`pkg-config --cflags hal-storage`
LIBS		:= `pkg-config --libs libxml-2.0` \
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
PAM_USB			:= pam_usb.so
PAM_USB_LDFLAGS	:= -shared
PAM_USB_DEST	:= $(DESTDIR)/lib/security

# pamusb-check
PAMUSB_CHECK_SRCS	:= src/pamusb-check.c
PAMUSB_CHECK_OBJS	:= $(PAMUSB_CHECK_SRCS:.c=.o)
PAMUSB_CHECK		:= pamusb-check

# Tools
PAMUSB_CONF		:= pamusb-conf
PAMUSB_AGENT	:= pamusb-agent
TOOLS_DEST		:= $(DESTDIR)/usr/bin
TOOLS_SRC		:= tools

# Conf
CONFS			:= doc/pamusb.conf
CONFS_DEST		:= $(DESTDIR)/etc

# Doc
DOCS		:= doc/INSTALLATION doc/CONFIGURATION doc/UPGRADING doc/FAQ
DOCS_DEST	:= $(DESTDIR)/usr/share/doc/pamusb

# Man
MANS		:= doc/pusb_conf.1.gz doc/pusb_hotplug.1.gz doc/pusb_check.1.gz
MANS_DEST	:= $(DESTDIR)/usr/share/man/man1

# Binaries
RM		:= rm
INSTALL		:= install
MKDIR		:= mkdir

ifeq (yes, ${DEBUG})
	CFLAGS := ${CFLAGS} -ggdb
endif

all		: $(PAM_USB) $(PAMUSB_CHECK)

$(PAM_USB)	: $(OBJS) $(PAM_USB_OBJS)
		$(CC) -o $(PAM_USB) $(PAM_USB_LDFLAGS) $(LDFLAGS) $(OBJS) $(PAM_USB_OBJS) $(LIBS)

$(PAMUSB_CHECK)	: $(OBJS) $(PAMUSB_CHECK_OBJS)
		$(CC) -o $(PAMUSB_CHECK) $(LDFLAGS) $(OBJS) $(PAMUSB_CHECK_OBJS) $(LIBS)

%.o		: %.c
		${CC} -c ${CFLAGS} $< -o $@

clean		:
		$(RM) -f $(PAM_USB) $(PAMUSB_CHECK) $(OBJS) $(PAMUSB_CHECK_OBJS) $(PAM_USB_OBJS)

install		: all
		$(MKDIR) -p $(CONFS_DEST) $(DOCS_DEST) $(MANS_DEST) $(TOOLS_DEST) $(PAM_USB_DEST)
		$(INSTALL) -m755 $(PAM_USB) $(PAM_USB_DEST)
		$(INSTALL) -m755 $(PAMUSB_CHECK) $(TOOLS_SRC)/$(PAMUSB_CONF) $(TOOLS_SRC)/$(PAMUSB_AGENT) $(TOOLS_DEST)
		$(INSTALL) -b -m644 $(CONFS) $(CONFS_DEST)
		$(INSTALL) -m644 $(DOCS) $(DOCS_DEST)
		$(INSTALL) -m644 $(MANS) $(MANS_DEST)

deinstall	:
		$(RM) -f $(PAM_USB_DEST)/$(PAM_USB)
		$(RM) -f $(TOOLS_DEST)/$(PAMUSB_CHECK) $(TOOLS_DEST)/$(PAMUSB_CONF) $(TOOLS_DEST)/$(PAMUSB_AGENT)
		$(RM) -rf $(DOCS_DEST)
		$(RM) -f $(MANS_DEST)/pusb_*
