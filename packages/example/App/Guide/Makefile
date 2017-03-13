#CROSS   := mips-linux-gnu-
CROSS	:=
CFLAGS  := -Wall -O2
LIBS 	:= -lpthread

OBJS := guide.o		\
	key.o

guide: $(OBJS)
	$(CROSS)gcc $^ $(LIBS) -o $@

.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf $(OBJS) guide
