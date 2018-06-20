

LUA ?= lua

default : $(LUNUM_A) $(LUNUM_SO)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.def: %.o
	$(DLLTOOL) -z $@ $^

lunum.o : lunum.c array_class.lc
	$(CC) $(CFLAGS) -c $< -I'$(LUA_INC)'

lunum_capi.o : lunum_capi.c
	$(CC) $(CFLAGS) -c $< -I'$(LUA_INC)'

array_class.o : array_class.c
	$(CC) $(CFLAGS) -c $< -I'$(LUA_INC)'

$(LUNUM_SO) : numarray.o lunum.o lunum_capi.o array_class.o lunum_epoc32.o lunum.def
	$(SO) -o $(LUNUM_SO) $^ $(CLIBS)

array_class.lc : array_class.lua
	$(LUA) bin2c.lua +$< > $@

clean :
	rm -f *.o $(LUNUM_A) $(LUNUM_SO) *.lc
