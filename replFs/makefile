CFLAGS	= -g -Wall -DSUN 
# CFLAGS	= -g -Wall -DDEC
CC	= g++
CCF	= $(CC) $(CFLAGS)

H	= .
C_DIR	= .

INCDIR	= -I$(H)
LIBDIRS = -L$(C_DIR)
LIBS    = -lclientReplFs
includes = $(wildcard *.h)

CLIENT_OBJECTS = networkInstance.o clientInstance.o client.o packet.o
SERVER_OBJECTS = networkInstance.o serverInstance.o server.o packet.o

all:	appl replFsServer

appl:	appl.o $(C_DIR)/libclientReplFs.a
	$(CCF) -o appl appl.o $(LIBDIRS) $(LIBS)

replFsServer: $(SERVER_OBJECTS)
	$(CCF) $(SERVER_OBJECTS) -o $@


$(C_DIR)/libclientReplFs.a:	$(CLIENT_OBJECTS)
	ar cr libclientReplFs.a $(CLIENT_OBJECTS)
	ranlib libclientReplFs.a

%.o: %.cpp %.h $(includes)
	$(CCF) -c $(INCDIR) $< -o $@

clean:
	rm -f appl replFsServer *.o *.a
	rm -r dir

c:
	./appl	
s:
	./replFsServer -port 44024 -mount dir -drop 20
	
