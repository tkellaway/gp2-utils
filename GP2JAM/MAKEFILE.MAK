.autodepend

CC		= bcc32
##CCFLAGS	= -c -v -ml -w3 -Ie:\bc4\include
CCFLAGS	= -c -v -WX -w3 -a1 -Ie:\bc4\include
LINK		= bcc32
LINKFLAGS	= -WX -v -a1

TARGET	= gp2jam
SRC		= gp2jam.cpp jam.cpp bmp.cpp
OBJ		= $(SRC:.cpp=.obj)
COV		= $(SRC:.cpp=.cov)

!if !$d(COVERAGE)
COVERAGE	= 0
!endif

.cpp.obj:
!if $(COVERAGE) == 1
		chk32 -Rcodecov -K6 -Ie:\bc4\include -X$* $< > $*.ctp
		$(CC) $(CCFLAGS) -D_COVER_ -P $*.ctp
!else
		$(CC) $(CCFLAGS) $<
!endif

.ctp.cov:
		cover $*.cpp $*.ctp dump.tp > $*.cov

all:		$(TARGET).exe

dump.obj:	dump.cpp dump.h
!if $(COVERAGE)
		ctptable $(SRC)
		$(CC) $(CCFLAGS) -D_COVER_ dump.cpp
!else
		$(CC) $(CCFLAGS) dump.cpp
!endif

$(TARGET).exe:	$(OBJ) dump.obj makefile.mak
		$(LINK) @&&!
			$(LINKFLAGS)
			-e$(TARGET).exe
			$(OBJ) dump.obj
!

$(COV):	dump.tp

cover:	$(COV)


strip:
		tdstrp32 -s $(TARGET).EXE

zip:
		tdstrp32 -s $(TARGET).EXE
		if exist gp2jam.zip del gp2jam.zip
		pkzip gp2jam.zip gp2jam.txt gp2jam.htm gp2jam.exe 32rtm.exe dpmi32vm.ovl

tidy:
		if exist *.obj del *.obj
		if exist *.tds del *.tds
		if exist *.ctp del *.ctp

tidyall:
		if exist *.obj del *.obj
		if exist *.tds del *.tds
		if exist *.ctp del *.ctp
		if exist dump1.i del dump1.i
		if exist dump2.i del dump2.i
		if exist dump.tp del dump.tp
		if exist tdconfig.td del tdconfig.td
		if exist gp2jam.tr del gp2jam.tr
		if exist gp2jam.tds del gp2jam.tds
		if exist *.cov del *.cov
