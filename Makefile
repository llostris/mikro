CC = gcc
CFLAGS = -I. #-Wall 
OBJS = eth0.o ipv6.o ndp.o tcp.o pagehtml.o testing.o #test.o #test1.o test2.o 
LIBNAME = libip
LIB = -L. -lip
LIBPATH = -Wl,-rpath=.
TESTS =  test_ndp.o test_eth0.o test_geteth0.o test_getndp.o test_getndp2.o test_tcp.o test_get.o test_server.o #test_client.o test_listen.o
OBJS_D = $(OBJS:.o=_fPIC.o)
TESTS_D = $(TESTS:test_%=dtest_%)

#all: dynamic tests
all: $(OBJS)

# libraries

static: $(OBJS)
	ar -crs $(LIBNAME).a $(OBJS)

dynamic: $(OBJS_D)
	$(CC) $(CFLAGS) -shared -fPIC -o $(LIBNAME).so $(OBJS_D)


# test programs

#tests: $(TESTS) 
#	$(CC) $(LIBPATH) $(TESTS) -o $@.o $(LIB)

tests_static: static $(TESTS)

tests_dynamic: dynamic $(TESTS_D)
	
test_%.o : test_%.c
	$(CC) -static $< -o $@ $(LIB)

dtest_%.o : test_%.c
	$(CC) $(LIBPATH) $< -o $@ $(LIB)
	
	
#$(CC) -o $* $@ $(LFLAGS)	# rzuca bledami
	

# files

%_fPIC.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $<

#
#%.o : %.c
#	$(CC) $(CFLAGS) -c $< -o $@
	#$(CC) -o $* $@ $(LFLAGS)

.PHONY: clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
	rm -f $(OBJS:.o=)


