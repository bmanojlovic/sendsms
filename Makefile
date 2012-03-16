PATH=/usr/bin:/bin:/opt/cross/bin/:/usr/local/bin:/usr/vac/bin:/usr/vacpp/bin
CC=gcc -Wall -g
AIX_CC=gxlc -g -DAIX
AIX_STRIP=strip
STRIP=strip -s
WIN_GCC=i386-mingw32msvc-gcc -Wall -g
WIN_STRIP=i386-mingw32msvc-strip -s
TARGET=sendsms
OBJECT=sendsms.o
SOURCE=sendsms.c
WINLIBS=-lwsock32
WINICLUDES=
LIBS= 
all: clean lin

aix:
	${AIX_CC} -c ${SOURCE} -o ${OBJECT}
	${AIX_CC} -o ${TARGET} ${OBJECT}
	${AIX_STRIP} ${TARGET}
lin: 
	${CC} -c ${SOURCE} -o ${OBJECT}
	${CC} ${LIBS} -o ${TARGET} ${OBJECT}
	${STRIP} ${TARGET}
win:
	${WIN_GCC} -DMINGW32 ${WINICLUDES} -c ${SOURCE} -o ${OBJECT}
	${WIN_GCC} -DMINGW32 -o ${TARGET}.exe ${OBJECT} ${WINLIBS}
	${WIN_STRIP} ${TARGET}.exe

clean:
	rm -f ${TARGET}
	rm -f ${TARGET}.exe
	rm -f ${OBJECT}
	rm -f *~
