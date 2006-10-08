SRC	= test.c \
	  conf.c \
	  log.c \
	  xpath.c \
	  hal.c
OBJ	= $(SRC:.c=.o)
NAME	= test
CC	= gcc
CFLAGS	= -Wall `pkg-config --cflags hal` `pkg-config --cflags libxml-2.0` 
LDFLAGS	= `pkg-config --libs hal` `pkg-config --libs libxml-2.0`

all	: $(NAME)
$(NAME)	: $(OBJ)
	$(CC) -o $(NAME) $(LDFLAGS) $(OBJ)

clean	:
	rm -f $(OBJ)