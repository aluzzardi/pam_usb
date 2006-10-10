DEBUG	:= no
SRCS	:= src/pusb_check.c \
	   src/conf.c \
	   src/log.c \
	   src/xpath.c \
	   src/hal.c \
	   src/pad.c \
	   src/volume.c \
	   src/local.c \
	   src/device.c
OBJS	:= $(SRCS:.c=.o)
NAME	:= pusb_check
CC	:= gcc
CFLAGS	:= -Wall `pkg-config --cflags hal` `pkg-config --cflags libxml-2.0`
LDFLAGS	:= `pkg-config --libs hal-storage` `pkg-config --libs libxml-2.0`

ifeq (yes, ${DEBUG})
	CFLAGS := ${CFLAGS} -ggdb
endif

all	: $(NAME)
$(NAME)	: $(OBJS)
	$(CC) -o $(NAME) $(LDFLAGS) $(OBJS)

%.o	: %.c
	${CC} -c ${CFLAGS} $< -o $@

clean	:
	rm -f $(OBJS)