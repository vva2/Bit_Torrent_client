PROG=	a.out

OBJS=   bencode.o BTsocket.o buffer.o file.o log.o LUrlParser.o message.o peer.o sha1.o status.o torrent.o util.o main.o

CC=	g++
CFLAGS= -g -std=c++11 -pthread
# CFLAGS+= -Wall

CFILES= ./src/bencode.cpp ./src/BTsocket.cpp ./src/buffer.cpp ./src/file.cpp ./src/log.cpp ./src/LUrlParser.cpp ./src/message.cpp ./src/peer.cpp ./src/sha1.cpp ./src/status.cpp ./src/torrent.cpp ./src/util.cpp ./main.cpp

a.out  : $(OBJS)
	make clean && $(CC) $(CFILES) -o $@ $(CFLAGS) 

bencode.o: ./src/bencode.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
BTsocket.o: ./src/BTsocket.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
buffer.o: ./src/buffer.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
file.o: ./src/file.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
log.o: ./src/log.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
LUrlParser.o: ./src/LUrlParser.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
message.o: ./src/message.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
peer.o: ./src/peer.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
sha1.o: ./src/sha1.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
status.o: ./src/status.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
torrent.o: ./src/torrent.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
util.o: ./src/util.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
main.o: ./main.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	rm -f *.o


# g++ main.cpp ./src/torrent.cpp ./src/util.cpp ./src/file.cpp ./src/bencode.cpp ./src/buffer.cpp ./src/LUrlParser.cpp ./src/message.cpp ./src/peer.cpp ./src/sha1.cpp -g -std=c++11

# #Extracted from old make file by Shiv Shankar
# #READ BELOW
# #Redundant files: name2id.cpp

# PROG=	damn

# OBJS=   bencode.o file.o sha1.o torrent.o util.o

# CC=	g++
# # CFLAGS+= -Wall -I. -I../common
# #CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes				  //not in cpp
# # CFLAGS+= -Wmissing-declarations
# # CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
# # CFLAGS+= -Wsign-compare
# #CFLAGS+=-std=c++0x
# # LDADD+=	-levent -lutil -llibbsd
# # -l/usr/lib/x86_64-linux-gnu/libbsd.so -l../common/cmnlib.a

# # LDADD+=	-levent -lutil -L/usr/lib/x86_64-linux-gnu/libbsd
# # LDADD+=	-levent -lutil -lbsd -lbsd-ctor

# # DPADD+= ${LIBEVENT} ${LIBUTIL} ${LIBBSD}

# damn  : $(OBJS) cc -D LIBBSD_OVERLAY -o $(PROG) $(OBJS) $(LDADD)


# area.o: area.cpp
# #	$(CC) -E -c $< | grep -v "^#" > $<.tmp                                    Note://Commented out.As they are redundant.\n
# #	python extr.py --input $<.tmp --match area.c.match --output $<.tmp.c           //Also removed from the below code \n \n       
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)
# #	mv $<.tmp.o $@                                                                 //for obj files                                              

# auth.o: auth.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# ospfd.o: ospfd.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# database.o: database.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# interface.o: interface.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# lsack.o: lsack.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# lsupdate.o: lsupdate.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# neighbor.o: neighbor.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# ospfe.o: ospfe.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# packet.o: packet.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# lsreq.o: lsreq.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)
	
# rde.o: rde.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# rde_lsdb.o: rde_lsdb.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)
	
# in_cksum.o: in_cksum.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# logging.o: logging.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# printconf.o: printconf.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)
	
# conf.o: conf.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# hello.o: hello.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# iso_cksum.o: iso_cksum.cpp
# 	$(CC) -c -o $@ $(CFLAGS) $< $(LDADD)

# #
# #.PHONY: all clean
# # 
# #all: ${OBJS}
# #	@echo "Building.." 
# #	${CC} ${CFLAGS} ${OBJS} -o ${PROG} 

# #%.o: %.c  # % pattern wildcard matching
# #	${CC} ${CFLAGS} -c $*.c ${INCLUDES}

# clean:
# 	rm -f *.o core *.core
