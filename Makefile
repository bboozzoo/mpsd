LEX_SRCS = scan.l
TARG_SRCS = main.c \
			conf.c \
			core.c \
			tftp.c \
			debug.c \
			$(LEX_SRCS:.l=-lex.c)
TARGET = mpsd
CFLAGS = -std=c99

$(TARGET): $(TARG_SRCS:.c=.o)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS) 

%-lex.c: %.l
	$(LEX) -o $@ $^

clean:
	rm -f $(TARG_SRCS:.c=.o)
	rm -f $(LEX_SRCS:.l=-lex.c)
	rm -f $(TARGET)

