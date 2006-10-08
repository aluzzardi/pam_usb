SRC	= test.c \
	  conf.c \
	  log.c \
	  xpath.c \
	  hal.c \
	  otp.c \
	  volume.c \
	  device.c
OBJ	= $(SRC:.c=.o)
NAME	= test
CC	= gcc
CFLAGS	= -Wall `pkg-config --cflags hal` `pkg-config --cflags libxml-2.0` -ggdb
LDFLAGS	= `pkg-config --libs hal-storage` `pkg-config --libs libxml-2.0`

all	: $(NAME)
$(NAME)	: $(OBJ)
	$(CC) -o $(NAME) $(LDFLAGS) $(OBJ)

clean	:
	rm -f $(OBJ)