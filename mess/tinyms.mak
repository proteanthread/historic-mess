MESS=1

# a tiny compile is without Neogeo games
COREDEFS += -DTINY_COMPILE=1
COREDEFS += -DTINY_NAME="driver_coleco"
COREDEFS += -DTINY_POINTER="&driver_coleco"
COREDEFS += -DNEOFREE -DMESS


# uses these CPUs
CPUS+=Z80@

# uses these SOUNDs
SOUNDS+=SN76496@

OBJS =\
	$(OBJ)/mess/machine/coleco.o	 \
	$(OBJ)/mess/systems/coleco.o

	  
# MESS specific core $(OBJ)s
COREOBJS += \
	$(OBJ)/cheat.o  	       \
	$(OBJ)/mess/mess.o			   \
	$(OBJ)/mess/image.o		       \
	$(OBJ)/mess/system.o	       \
	$(OBJ)/mess/device.o	       \
	$(OBJ)/mess/config.o	       \
	$(OBJ)/mess/inputx.o		   \
	$(OBJ)/mess/mesintrf.o	       \
	$(OBJ)/mess/filemngr.o	       \
	$(OBJ)/mess/compcfg.o	       \
	$(OBJ)/mess/tapectrl.o	       \
	$(OBJ)/mess/utils.o            \
	$(OBJ)/mess/eventlst.o         \
	$(OBJ)/mess/formats.o          \
	$(OBJ)/mess/mscommon.o         \
	$(OBJ)/mess/devices/cartslot.o \
	$(OBJ)/mess/devices/messfmts.o \
	$(OBJ)/mess/devices/printer.o  \
	$(OBJ)/mess/devices/cassette.o \
	$(OBJ)/mess/devices/bitbngr.o  \
	$(OBJ)/mess/devices/snapquik.o \
	$(OBJ)/mess/devices/basicdsk.o \
	$(OBJ)/mess/devices/flopdrv.o  \
	$(OBJ)/mess/machine/6551.o     \
	$(OBJ)/mess/vidhrdw/m6845.o    \
	$(OBJ)/mess/machine/msm8251.o  \
	$(OBJ)/mess/machine/tc8521.o   \
	$(OBJ)/mess/vidhrdw/v9938.o    \
	$(OBJ)/mess/vidhrdw/crtc6845.o \
	$(OBJ)/mess/vidhrdw/tms9928a.o \
	$(OBJ)/mess/machine/28f008sa.o \
	$(OBJ)/mess/machine/am29f080.o \
	$(OBJ)/mess/machine/rriot.o    \
	$(OBJ)/mess/machine/riot6532.o \
	$(OBJ)/mess/machine/pit8253.o  \
	$(OBJ)/mess/machine/mc146818.o \
	$(OBJ)/mess/machine/uart8250.o \
	$(OBJ)/mess/machine/pc_mouse.o \
	$(OBJ)/mess/machine/pclpt.o    \
	$(OBJ)/mess/machine/centroni.o \
	$(OBJ)/mess/machine/pckeybrd.o \
	$(OBJ)/mess/machine/pc_fdc_h.o \
	$(OBJ)/mess/devices/pc_flopp.o \
	$(OBJ)/mess/devices/dsk.o      \
	$(OBJ)/mess/machine/d88.o      \
	$(OBJ)/mess/machine/nec765.o   \
	$(OBJ)/mess/machine/wd179x.o   \
	$(OBJ)/mess/machine/serial.o

