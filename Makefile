#
# CC Command
#
ifneq ($(WIN32),)
  #Unix Environments
  LDFLAGS = -L../lib -llibjpeg  -lopengl32 -lgdi32 -lglu32  ../lib/frontpanel.dll
  ifneq (,$(findstring solaris,$(OSTYPE)))
    OS_CCDEFS = -lm -lsocket -lnsl -lrt -lpthread -D_GNU_SOURCE
  else
    ifneq (,$(findstring darwin,$(OSTYPE)))
      OS_CCDEFS = -D_GNU_SOURCE
    else
      OS_CCDEFS = -lrt -lm -D_GNU_SOURCE
    endif
  endif
  CC = gcc -std=c99 -U__STRICT_ANSI__ -g $(OS_CCDEFS) -I . -I ../include
  ifeq ($(USE_NETWORK),)
  else
    NETWORK_OPT = -DUSE_NETWORK -isystem /usr/local/include /usr/local/lib/libpcap.a
  endif
else
  #Win32 Environments
	  #LDFLAGS = -mwindows -mconsole -lm -lwsock32 -lopengl32 -lglu32 -lwinmm -lfrontpanel
	  LDFLAGS = -mwindows -mconsole -lm -lwsock32 -lwinmm  -Wl,-Bstatic -static -static-libstdc++ -static-libgcc ../lib/libfrontpanel.dll -lopengl32 -lglu32
  CC = gcc -std=c99 -U__STRICT_ANSI__ -O2 -I. -I ../include
  EXE = .exe
  ifeq ($(USE_NETWORK),)
  else
    NETWORK_OPT = -DUSE_NETWORK -lwpcap -lpacket
  endif
endif

#
# Common Libraries
#
BIN = bin/
SIMD = nova_src
SIM = ${SIMD}/scp.c ${SIMD}/sim_console.c ${SIMD}/sim_fio.c ${SIMD}/sim_timer.c ${SIMD}/sim_sock.c \
	${SIMD}/sim_tmxr.c ${SIMD}/sim_ether.c ${SIMD}/sim_tape.c


#
# Emulator source files and compile time options
#

#NOVAD = NOVA
NOVAD = nova_src
NOVA = ${NOVAD}/nova_sys.c ${NOVAD}/nova_cpu.c ${NOVAD}/nova_dkp.c \
	${NOVAD}/nova_dsk.c ${NOVAD}/nova_lp.c ${NOVAD}/nova_mta.c \
	${NOVAD}/nova_plt.c ${NOVAD}/nova_pt.c ${NOVAD}/nova_clk.c \
	${NOVAD}/nova_tt.c ${NOVAD}/nova_tt1.c ${NOVAD}/nova_qty.c ${NOVAD}/nova_panel.c
NOVA_OPT = -I ${NOVAD}


#
# Build everything
#
ALL = nova

all : ${ALL}

clean :
ifeq ($(WIN32),)
	${RM} ${BIN}*
else
	if exist BIN\*.exe del /q BIN\*.exe
endif

#
# Individual builds
#

nova : ${BIN}nova${EXE}

${BIN}nova${EXE} : ${NOVA} ${SIM}
	${CC} ${NOVA} ${SIM} ${NOVA_OPT} -o $@ ${LDFLAGS}

