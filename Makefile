CC = gcc
CFLAGS = -I. #-Wall 
OBJS = eth0.o ipv6.o ndp.o tcp.o testing.o test.o #test1.o test2.o 
LIBNAME = libip
LIB = -L. -lip
LIBPATH = -Wl,-rpath=.
TESTS =  test_ndp.o test_eth0.o test_geteth0.o test_getndp.o test_getndp2.o test_tcp.o test_get.o test_client.o test_listen.o
OBJ_D = $(OBJS:.o=_fPIC.o)

#all: dynamic tests
all: $(OBJS)

# libraries

static: $(OBJS)
	ar -crs $(LIBNAME).a $(OBJS)

#dynamic: $(OBJ_D)
#	$(CC) $(CFLAGS) -shared -fPIC -o $(LIBNAME).so $(OBJ_D)


# test programs

#tests: $(TESTS) 
#	$(CC) $(LIBPATH) $(TESTS) -o $@.o $(LIB)
tests_static: static $(TESTS)
	
test_%.o : test_%.c
	$(CC) -static $< -o $@ $(LIB)
	
#$(CC) -o $* $@ $(LFLAGS)	# rzuca bledami
	

$(OBJ_D): $(SRC)
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


