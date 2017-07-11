.SUFFIXES: .c .o

COMPILE = $(CC)  -Wall -D_GNU_SOURCE -g -O2 -Wno-error
INC_PATH = 
LIB_PATH =  -lpthread -ldl -rdynamic -lfastcommon
TARGET_PATH = /usr/bin

ALL_OBJS = lexer.o  token.o
ALL_PRGS = test

all: $(ALL_PRGS)

$(ALL_PRGS): $(ALL_OBJS)

.o:
	$(COMPILE) -o $@ $<  $(LIB_PATH) $(INC_PATH)
.c:
	$(COMPILE) -o $@ $<  $(ALL_OBJS) $(LIB_PATH) $(INC_PATH)
.c.o:
	$(COMPILE) -c -o $@ $<  $(INC_PATH)

install:
	mkdir -p $(TARGET_PATH)
	cp -f $(ALL_PRGS) $(TARGET_PATH)
clean:
	rm -f *.o $(ALL_OBJS) $(ALL_PRGS)
