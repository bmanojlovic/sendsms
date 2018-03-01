PATH=/usr/bin:/bin:/opt/cross/bin/:/usr/local/bin:/usr/vac/bin:/usr/vacpp/bin
CC=gcc -Wall -g
STRIP=strip -s
AIX_CC=gxlc -g
AIX_STRIP=strip
WIN_CC=i686-w64-mingw32-gcc -Wall -g
WIN_STRIP=/usr/bin/i686-w64-mingw32-strip -s
TARGET=sendsms
OBJECT=sendsms.o
SOURCE=sendsms.c
WINLIBS=-lwsock32 -lws2_32
WINICLUDES=
LIBS= 
all: clean lin lin-static win

aix:
	${AIX_CC} -c ${SOURCE} -o ${OBJECT}
	${AIX_CC} -o ${TARGET} ${OBJECT}
	${AIX_STRIP} ${TARGET}
lin: 
	${CC} -c ${SOURCE} -o ${OBJECT}
	${CC} ${LIBS} -o ${TARGET} ${OBJECT}
	${STRIP} ${TARGET}

lin-static: 
	${CC} -static -c ${SOURCE} -o ${OBJECT}
	${CC} ${LIBS} -static -o ${TARGET}-static ${OBJECT}
	${STRIP} ${TARGET}-static

win:
	${WIN_CC} -DMINGW32 ${WINICLUDES} -c ${SOURCE} -o ${OBJECT}
	${WIN_CC} -DMINGW32 -o ${TARGET}.exe ${OBJECT} ${WINLIBS}
	${WIN_STRIP} ${TARGET}.exe

clean:
	rm -f ${TARGET}
	rm -f ${TARGET}-static
	rm -f ${TARGET}.exe
	rm -f ${OBJECT}
	rm -f *~
