# this is a MESS build
MESS = 1

# core defines
COREDEFS += -DNEOFREE -DMESS

# to split the mess into parts
# (problem with tinymess is to only compile for 1 system)
#MESS_EXCLUDE_AMSTRAD = 1
#MESS_EXCLUDE_CBM = 1
#MESS_EXCLUDE_IBMPC = 1
#MESS_EXCLUDE_SHARP = 1
#MESS_EXCLUDE_SINCLAIR = 1
#MESS_EXCLUDE_ACORN = 1

# CPU cores used in MESS
CPUS+=Z80@
CPUS+=Z80GB@
CPUS+=CDP1802@
#CPUS+=8080@
#CPUS+=8085A@
CPUS+=M6502@
CPUS+=M65C02@
#CPUS+=M65SC02@
CPUS+=M65CE02@
CPUS+=M6509@
CPUS+=M6510@
CPUS+=M6510T@
CPUS+=M7501@
CPUS+=M8502@
CPUS+=M4510@
CPUS+=N2A03@
CPUS+=H6280@
CPUS+=I86@
CPUS+=I88@
#CPUS+=I186@
#CPUS+=I188@
CPUS+=I286@
#CPUS+=I8035@
CPUS+=I8039@
CPUS+=I8048@
CPUS+=M6800@
#CPUS+=M6801@
#CPUS+=M6802@
CPUS+=M6803@
CPUS+=M6808@
CPUS+=HD63701@
CPUS+=NSC8105@
#CPUS+=M6805@
#CPUS+=M68705@
#CPUS+=HD63705@
CPUS+=HD6309@
CPUS+=M6809@
#CPUS+=KONAMI@
CPUS+=M68000@
#CPUS+=M68010@
#CPUS+=M68EC020@
#CPUS+=M68020@
#CPUS+=T11@
#CPUS+=S2650@
CPUS+=F8@
CPUS+=CP1600@
#CPUS+=TMS34010@
CPUS+=TMS9900@
#CPUS+=TMS9940@
#CPUS+=TMS9980@
#CPUS+=TMS9985@
#CPUS+=TMS9989@
CPUS+=TMS9995@
#CPUS+=TMS99105A@
#CPUS+=TMS99110A@
#CPUS+=TMS320C10@
CPUS+=PDP1@
#CPUS+=ADSP2100@
#CPUS+=ADSP2105@
#CPUS+=MIPS@
CPUS+=SH2@
CPUS+=SC61860@
CPUS+=ARM@
CPUS+=G65816@
#CPUS+=SPC700@

# SOUND cores used in MESS
SOUNDS+=CUSTOM@
SOUNDS+=SAMPLES@
SOUNDS+=DAC@
SOUNDS+=AY8910@
#SOUNDS+=YM2203@
# enable only one of the following two
SOUNDS+=YM2151@
#SOUNDS+=YM2151_ALT@
SOUNDS+=YM2608@
SOUNDS+=YM2610@
#SOUNDS+=YM2610B@
SOUNDS+=YM2612@
#SOUNDS+=YM3438@
SOUNDS+=YM2413@
SOUNDS+=YM3812@
#SOUNDS+=YMZ280B@
#SOUNDS+=YM3526@
#SOUNDS+=Y8950@
#SOUNDS+=SN76477@
SOUNDS+=SN76496@
SOUNDS+=POKEY@
SOUNDS+=TIA@
SOUNDS+=NES@
SOUNDS+=ASTROCADE@
#SOUNDS+=NAMCO@
#SOUNDS+=TMS36XX@
SOUNDS+=TMS5110@
SOUNDS+=TMS5220@
#SOUNDS+=VLM5030@
#SOUNDS+=ADPCM@
SOUNDS+=OKIM6295@
#SOUNDS+=MSM5205@
#SOUNDS+=UPD7759@
#SOUNDS+=HC55516@
#SOUNDS+=K005289@
#SOUNDS+=K007232@
#SOUNDS+=K051649@
#SOUNDS+=K053260@
#SOUNDS+=SEGAPCM@
#SOUNDS+=RF5C68@
#SOUNDS+=CEM3394@
#SOUNDS+=C140@
SOUNDS+=QSOUND@
SOUNDS+=SAA1099@
SOUNDS+=SPEAKER@
SOUNDS+=WAVE@
SOUNDS+=BEEP@

# Archive definitions
DRVLIBS = \
		$(OBJ)/advision.a \
		$(OBJ)/sega.a     \
		$(OBJ)/coleco.a   \
		$(OBJ)/atari.a    \
		$(OBJ)/nintendo.a \
		$(OBJ)/dragon.a   \
		$(OBJ)/kaypro.a   \
		$(OBJ)/trs80.a    \
		$(OBJ)/cgenie.a   \
		$(OBJ)/pdp1.a     \
		$(OBJ)/apple1.a   \
		$(OBJ)/apple2.a   \
		$(OBJ)/mac.a	    \
		$(OBJ)/ti99.a     \
		$(OBJ)/bally.a    \
		$(OBJ)/p2000.a    \
		$(OBJ)/nec.a	    \
		$(OBJ)/ep128.a    \
		$(OBJ)/ascii.a    \
		$(OBJ)/tangerin.a \
		$(OBJ)/vtech.a    \
		$(OBJ)/jupiter.a  \
		$(OBJ)/mbee.a     \
		$(OBJ)/nascom1.a  \
		$(OBJ)/bbc.a	    \
		$(OBJ)/cpschngr.a \
		$(OBJ)/mtx.a	    \
		$(OBJ)/acorn.a    \
		$(OBJ)/samcoupe.a \
		$(OBJ)/gce.a	    \
		$(OBJ)/kim1.a     \
		$(OBJ)/lisa.a     \
		$(OBJ)/aquarius.a \
		$(OBJ)/avigo.a    \
		$(OBJ)/rca.a	    \
		$(OBJ)/fairch.a   \
		$(OBJ)/magnavox.a \
		$(OBJ)/teamconc.a \
		#$(OBJ)/motorola.a \

ifndef MESS_EXCLUDE_ACORN
DRVLIBS += $(OBJ)/acorn.a    
else
COREDEFS += -DMESS_EXCLUDE_ACORN
endif

ifndef MESS_EXCLUDE_AMSTRAD
DRVLIBS += $(OBJ)/amstrad.a
else
COREDEFS += -DMESS_EXCLUDE_AMSTRAD
endif

ifndef MESS_EXCLUDE_CBM
DRVLIBS += $(OBJ)/cbm.a
else
COREDEFS += -DMESS_EXCLUDE_CBM
endif

ifndef MESS_EXCLUDE_IBMPC
DRVLIBS += $(OBJ)/pc.a
else
COREDEFS += -DMESS_EXCLUDE_IBMPC
endif

ifndef MESS_EXCLUDE_SHARP
DRVLIBS += $(OBJ)/sharp.a
else
COREDEFS += -DMESS_EXCLUDE_SHARP
endif

ifndef MESS_EXCLUDE_SINCLAIR
DRVLIBS += $(OBJ)/sinclair.a
else
COREDEFS += -DMESS_EXCLUDE_SINCLAIR
endif

$(OBJ)/mess/system.o: src/mess.mak

$(OBJ)/coleco.a:   \
	  $(OBJ)/mess/vidhrdw/tms9928a.o \
	  $(OBJ)/mess/machine/coleco.o	 \
	  $(OBJ)/mess/vidhrdw/coleco.o	 \
	  $(OBJ)/mess/systems/coleco.o

$(OBJ)/sega.a:	   \
	  $(OBJ)/mess/vidhrdw/smsvdp.o	 \
	  $(OBJ)/mess/machine/sms.o	 \
	  $(OBJ)/mess/systems/sms.o	 \
	  $(OBJ)/mess/vidhrdw/genesis.o  \
	  $(OBJ)/mess/machine/genesis.o  \
	  $(OBJ)/mess/sndhrdw/genesis.o  \
	  $(OBJ)/mess/systems/genesis.o  \
	  $(OBJ)/mess/systems/saturn.o

$(OBJ)/atari.a:    \
	  $(OBJ)/mess/machine/atari.o	 \
	  $(OBJ)/mess/vidhrdw/antic.o	 \
	  $(OBJ)/mess/vidhrdw/gtia.o	 \
	  $(OBJ)/mess/vidhrdw/atari.o	 \
	  $(OBJ)/mess/systems/atari.o	 \
	  $(OBJ)/mess/vidhrdw/a7800.o	 \
	  $(OBJ)/mess/machine/a7800.o	 \
	  $(OBJ)/mess/systems/a7800.o	 \
	  $(OBJ)/mess/machine/riot.o	 \
	  $(OBJ)/mess/machine/a2600.o	 \
	  $(OBJ)/mess/systems/a2600.o

$(OBJ)/gce.a:	   \
	  $(OBJ)/mess/vidhrdw/vectrex.o  \
	  $(OBJ)/mess/machine/vectrex.o  \
	  $(OBJ)/mess/systems/vectrex.o

$(OBJ)/nintendo.a: \
	  $(OBJ)/mess/machine/nes_mmc.o  \
	  $(OBJ)/mess/vidhrdw/nes.o	 \
	  $(OBJ)/mess/machine/nes.o	 \
	  $(OBJ)/mess/systems/nes.o	 \
	  $(OBJ)/mess/vidhrdw/gb.o	 \
	  $(OBJ)/mess/machine/gb.o	 \
	  $(OBJ)/mess/systems/gb.o	 \
	  $(OBJ)/mess/sndhrdw/snes.o	 \
	  $(OBJ)/mess/vidhrdw/snes.o	 \
	  $(OBJ)/mess/machine/snes.o	 \
	  $(OBJ)/mess/systems/snes.o


$(OBJ)/cbm.a:	   \
	  $(OBJ)/mess/vidhrdw/amiga.o	 \
	  $(OBJ)/mess/machine/amiga.o	 \
	  $(OBJ)/mess/systems/amiga.o	 \
	  $(OBJ)/mess/vidhrdw/crtc6845.o \
	  $(OBJ)/mess/machine/tpi6525.o  \
	  $(OBJ)/mess/machine/cbmieeeb.o \
	  $(OBJ)/mess/vidhrdw/pet.o	 \
	  $(OBJ)/mess/systems/pet.o	 \
	  $(OBJ)/mess/machine/pet.o	 \
	  $(OBJ)/mess/systems/cbmb.o	 \
	  $(OBJ)/mess/machine/cbmb.o	 \
	  $(OBJ)/mess/vidhrdw/vic6560.o  \
	  $(OBJ)/mess/sndhrdw/vic6560.o  \
	  $(OBJ)/mess/machine/vc20.o	 \
	  $(OBJ)/mess/machine/vc20tape.o \
	  $(OBJ)/mess/machine/vc1541.o	 \
	  $(OBJ)/mess/systems/vc20.o	 \
	  $(OBJ)/mess/vidhrdw/ted7360.o  \
	  $(OBJ)/mess/sndhrdw/ted7360.o  \
	  $(OBJ)/mess/machine/c1551.o	 \
	  $(OBJ)/mess/machine/c16.o	 \
	  $(OBJ)/mess/systems/c16.o	 \
	  $(OBJ)/mess/machine/cbm.o	 \
	  $(OBJ)/mess/machine/cbmdrive.o \
	  $(OBJ)/mess/systems/c64.o	 \
	  $(OBJ)/mess/machine/cia6526.o  \
	  $(OBJ)/mess/machine/c64.o	 \
	  $(OBJ)/mess/vidhrdw/vic6567.o  \
	  $(OBJ)/mess/sndhrdw/sid6581.o  \
	  $(OBJ)/mess/systems/c65.o	 \
	  $(OBJ)/mess/machine/c65.o	 \
	  $(OBJ)/mess/systems/c128.o	 \
	  $(OBJ)/mess/vidhrdw/vdc8563.o  \
	  $(OBJ)/mess/vidhrdw/praster.o  \
	  $(OBJ)/mess/machine/c128.o	 \
	  $(OBJ)/mess/sndhrdw/mixing.o	 \
	  $(OBJ)/mess/sndhrdw/envelope.o \
	  $(OBJ)/mess/sndhrdw/samples.o  \
	  $(OBJ)/mess/sndhrdw/6581_.o

$(OBJ)/dragon.a:   \
	  $(OBJ)/mess/formats/cocopak.o  \
	  $(OBJ)/mess/formats/cococas.o  \
	  $(OBJ)/mess/vidhrdw/m6847.o	 \
	  $(OBJ)/mess/machine/mc10.o	 \
	  $(OBJ)/mess/systems/mc10.o	 \
	  $(OBJ)/mess/vidhrdw/dragon.o	 \
	  $(OBJ)/mess/machine/dragon.o	 \
	  $(OBJ)/mess/systems/dragon.o

$(OBJ)/trs80.a:    \
	  $(OBJ)/mess/machine/trs80.o	 \
	  $(OBJ)/mess/vidhrdw/trs80.o	 \
	  $(OBJ)/mess/systems/trs80.o

$(OBJ)/cgenie.a:   \
	  $(OBJ)/mess/machine/cgenie.o	 \
	  $(OBJ)/mess/vidhrdw/cgenie.o	 \
	  $(OBJ)/mess/sndhrdw/cgenie.o	 \
	  $(OBJ)/mess/systems/cgenie.o

$(OBJ)/pdp1.a:	   \
	  $(OBJ)/mess/vidhrdw/pdp1.o	 \
	  $(OBJ)/mess/machine/pdp1.o	 \
	  $(OBJ)/mess/systems/pdp1.o

$(OBJ)/kaypro.a:   \
	  $(OBJ)/mess/machine/cpm_bios.o \
	  $(OBJ)/mess/vidhrdw/kaypro.o	 \
	  $(OBJ)/mess/sndhrdw/kaypro.o	 \
	  $(OBJ)/mess/machine/kaypro.o	 \
	  $(OBJ)/mess/systems/kaypro.o	 \

$(OBJ)/sinclair.a: \
	  $(OBJ)/mess/eventlst.o \
	  $(OBJ)/mess/vidhrdw/border.o	 \
	  $(OBJ)/mess/vidhrdw/spectrum.o \
	  $(OBJ)/mess/machine/spectrum.o \
	  $(OBJ)/mess/systems/spectrum.o \
	  $(OBJ)/mess/vidhrdw/zx.o	 \
	  $(OBJ)/mess/machine/zx.o	 \
	  $(OBJ)/mess/systems/zx.o

$(OBJ)/apple1.a:   \
	  $(OBJ)/machine/6821pia.o	 \
	  $(OBJ)/mess/vidhrdw/apple1.o	 \
	  $(OBJ)/mess/machine/apple1.o	 \
	  $(OBJ)/mess/systems/apple1.o

$(OBJ)/apple2.a:   \
	  $(OBJ)/mess/machine/ay3600.o	 \
	  $(OBJ)/mess/machine/ap_disk2.o \
	  $(OBJ)/mess/vidhrdw/apple2.o	 \
	  $(OBJ)/mess/machine/apple2.o	 \
	  $(OBJ)/mess/systems/apple2.o

$(OBJ)/mac.a: \
	  $(OBJ)/mess/sndhrdw/mac.o	 \
	  $(OBJ)/mess/machine/iwm.o	 \
	  $(OBJ)/mess/vidhrdw/mac.o	 \
	  $(OBJ)/mess/machine/mac.o	 \
	  $(OBJ)/mess/systems/mac.o

$(OBJ)/avigo.a: \
	  $(OBJ)/mess/systems/avigo.o	 \
	  $(OBJ)/mess/vidhrdw/avigo.o	 \
	  $(OBJ)/mess/machine/28f008sa.o

$(OBJ)/rca.a: \
	  $(OBJ)/mess/systems/studio2.o  \
	  $(OBJ)/mess/vidhrdw/studio2.o  \

$(OBJ)/fairch.a: \
	  $(OBJ)/mess/systems/channelf.o \
	  $(OBJ)/mess/vidhrdw/channelf.o \
	  $(OBJ)/mess/sndhrdw/channelf.o

$(OBJ)/ti99.a:	   \
	  $(OBJ)/mess/machine/tms9901.o  \
	  $(OBJ)/mess/machine/ti99_4x.o  \
	  $(OBJ)/mess/systems/ti99_4x.o  \
	 #$(OBJ)/mess/systems/ti99_2.o	 \
	 #$(OBJ)/mess/systems/ti990_4.o  \


$(OBJ)/bally.a:    \
	  $(OBJ)/sound/astrocde.o	 \
	  $(OBJ)/mess/vidhrdw/astrocde.o \
	  $(OBJ)/mess/machine/astrocde.o \
	  $(OBJ)/mess/systems/astrocde.o

$(OBJ)/pc.a:	   \
	  $(OBJ)/mess/machine/tandy1t.o  \
	  $(OBJ)/mess/machine/amstr_pc.o \
	  $(OBJ)/mess/machine/at.o	 \
	  $(OBJ)/mess/machine/dma8237.o  \
	  $(OBJ)/mess/machine/pic8259.o  \
	  $(OBJ)/mess/vidhrdw/vga.o	 \
	  $(OBJ)/mess/sndhrdw/pc.o	 \
	  $(OBJ)/mess/vidhrdw/pc_cga.o	 \
	  $(OBJ)/mess/vidhrdw/pc_mda.o	 \
	  $(OBJ)/mess/vidhrdw/pc_t1t.o	 \
	  $(OBJ)/mess/machine/pc.o	 \
	  $(OBJ)/mess/machine/pc_fdc.o	 \
	  $(OBJ)/mess/machine/pc_hdc.o	 \
	  $(OBJ)/mess/machine/pc_ide.o	 \
	  $(OBJ)/mess/systems/pc.o

$(OBJ)/p2000.a:    \
	  $(OBJ)/mess/vidhrdw/saa5050.o   \
	  $(OBJ)/mess/vidhrdw/p2000m.o	  \
	  $(OBJ)/mess/systems/p2000t.o	  \
	  $(OBJ)/mess/machine/p2000t.o	  \
	  $(OBJ)/mess/machine/mc6850.o	  \
	  $(OBJ)/mess/vidhrdw/uk101.o	  \
	  $(OBJ)/mess/machine/uk101.o	  \
	  $(OBJ)/mess/systems/uk101.o

$(OBJ)/amstrad.a:  \
	  $(OBJ)/mess/machine/flopdrv.o  \
	  $(OBJ)/mess/vidhrdw/amstrad.o  \
	  $(OBJ)/mess/vidhrdw/kc.o	 \
	  $(OBJ)/mess/machine/amstrad.o  \
	  $(OBJ)/mess/machine/kc.o	 \
	  $(OBJ)/mess/systems/amstrad.o  \
	  $(OBJ)/mess/vidhrdw/pcw.o	 \
	  $(OBJ)/mess/systems/pcw.o	 \
	  $(OBJ)/mess/vidhrdw/nc.o	 \
	  $(OBJ)/mess/systems/nc.o	 \
	  $(OBJ)/mess/machine/nc.o	 \
	  $(OBJ)/mess/machine/tc8521.o		\
	  $(OBJ)/mess/systems/pcw16.o		\
	  $(OBJ)/mess/machine/28f008sa.o	\
      $(OBJ)/mess/machine/msm8251.o     \
	  $(OBJ)/mess/vidhrdw/pcw16.o

$(OBJ)/nec.a:	   \
	  $(OBJ)/mess/vidhrdw/vdc.o	 \
	  $(OBJ)/mess/machine/pce.o	 \
	  $(OBJ)/mess/systems/pce.o

$(OBJ)/ep128.a :   \
	  $(OBJ)/mess/sndhrdw/dave.o	 \
	  $(OBJ)/mess/vidhrdw/epnick.o	 \
	  $(OBJ)/mess/vidhrdw/enterp.o	 \
	  $(OBJ)/mess/machine/enterp.o	 \
	  $(OBJ)/mess/systems/enterp.o

$(OBJ)/ascii.a :   \
	  $(OBJ)/mess/sndhrdw/scc.o	 	 \
	  $(OBJ)/mess/machine/msx.o	 	 \
	  $(OBJ)/mess/formats/fmsx_cas.o \
	  $(OBJ)/mess/systems/msx.o

$(OBJ)/kim1.a :    \
	  $(OBJ)/mess/vidhrdw/kim1.o	 \
	  $(OBJ)/mess/machine/kim1.o	 \
	  $(OBJ)/mess/systems/kim1.o

$(OBJ)/tangerin.a :\
	  $(OBJ)/mess/vidhrdw/oric.o	 \
	  $(OBJ)/mess/machine/oric.o	 \
	  $(OBJ)/mess/systems/oric.o	 \
	  $(OBJ)/mess/vidhrdw/microtan.o \
	  $(OBJ)/mess/machine/microtan.o \
	  $(OBJ)/mess/systems/microtan.o

$(OBJ)/vtech.a :   \
	  $(OBJ)/mess/vidhrdw/vtech1.o	 \
	  $(OBJ)/mess/machine/vtech1.o	 \
	  $(OBJ)/mess/systems/vtech1.o	 \
	  $(OBJ)/mess/vidhrdw/vtech2.o	 \
	  $(OBJ)/mess/machine/vtech2.o	 \
	  $(OBJ)/mess/systems/vtech2.o

$(OBJ)/jupiter.a : \
	  $(OBJ)/mess/vidhrdw/jupiter.o  \
	  $(OBJ)/mess/machine/jupiter.o  \
	  $(OBJ)/mess/systems/jupiter.o

$(OBJ)/mbee.a :    \
	  $(OBJ)/mess/vidhrdw/mbee.o	 \
	  $(OBJ)/mess/machine/mbee.o	 \
	  $(OBJ)/mess/systems/mbee.o


$(OBJ)/advision.a: \
	  $(OBJ)/mess/vidhrdw/advision.o \
	  $(OBJ)/mess/machine/advision.o \
	  $(OBJ)/mess/systems/advision.o

$(OBJ)/nascom1.a:  \
	  $(OBJ)/mess/vidhrdw/nascom1.o  \
	  $(OBJ)/mess/machine/nascom1.o  \
	  $(OBJ)/mess/systems/nascom1.o

$(OBJ)/bbc.a:	   \
	  $(OBJ)/mess/machine/i8271.o	 \
	  $(OBJ)/mess/vidhrdw/m6845.o	 \
	  $(OBJ)/mess/vidhrdw/bbc.o	 \
	  $(OBJ)/mess/machine/bbc.o	 \
	  $(OBJ)/mess/systems/bbc.o

$(OBJ)/cpschngr.a: \
	  $(OBJ)/machine/eeprom.o	 \
	  $(OBJ)/vidhrdw/cps1.o 	 \
	  $(OBJ)/mess/systems/cpschngr.o

$(OBJ)/mtx.a:	   \
	  $(OBJ)/mess/systems/mtx.o

$(OBJ)/lisa.a:	   \
	  $(OBJ)/mess/machine/iwm_lisa.o \
	  $(OBJ)/mess/machine/lisa.o	 \
	  $(OBJ)/mess/systems/lisa.o

$(OBJ)/acorn.a:    \
	  $(OBJ)/mess/machine/atom.o	 \
	  $(OBJ)/mess/vidhrdw/atom.o	 \
	  $(OBJ)/mess/systems/atom.o	 \
	  $(OBJ)/mess/systems/a310.o	 \
	  $(OBJ)/mess/systems/z88.o	 \
	  $(OBJ)/mess/vidhrdw/z88.o


$(OBJ)/samcoupe.a: \
	  $(OBJ)/mess/machine/coupe.o	 \
	  $(OBJ)/mess/vidhrdw/coupe.o	 \
	  $(OBJ)/mess/systems/coupe.o

$(OBJ)/sharp.a:    \
	  $(OBJ)/mess/machine/mz700.o	 \
	  $(OBJ)/mess/vidhrdw/mz700.o	 \
	  $(OBJ)/mess/systems/mz700.o	 \
	  $(OBJ)/mess/machine/pocketc.o  \
	  $(OBJ)/mess/vidhrdw/pocketc.o  \
	  $(OBJ)/mess/systems/pocketc.o

$(OBJ)/aquarius.a: \
	  $(OBJ)/mess/machine/aquarius.o \
	  $(OBJ)/mess/vidhrdw/aquarius.o \
	  $(OBJ)/mess/systems/aquarius.o

$(OBJ)/magnavox.a: \
	  $(OBJ)/mess/machine/odyssey2.o \
	  $(OBJ)/mess/vidhrdw/odyssey2.o \
	  $(OBJ)/mess/systems/odyssey2.o

$(OBJ)/teamconc.a: \
	$(OBJ)/mess/vidhrdw/comquest.o \
	$(OBJ)/mess/systems/comquest.o

$(OBJ)/motorola.a: \
	  $(OBJ)/mess/vidhrdw/mekd2.o	 \
	  $(OBJ)/mess/machine/mekd2.o	 \
	  $(OBJ)/mess/systems/mekd2.o



# MESS specific core $(OBJ)s
COREOBJS += \
		$(OBJ)/mess/mess.o             \
		$(OBJ)/mess/system.o           \
		$(OBJ)/mess/config.o           \
		$(OBJ)/mess/filemngr.o         \
		$(OBJ)/mess/tapectrl.o         \
		$(OBJ)/mess/utils.o \
		$(OBJ)/mess/machine/6522via.o \
		$(OBJ)/mess/machine/pit8253.o  \
		$(OBJ)/mess/machine/mc146818.o \
		$(OBJ)/mess/machine/uart8250.o \
		$(OBJ)/mess/machine/pc_mouse.o \
		$(OBJ)/mess/machine/pclpt.o	 \
		$(OBJ)/mess/machine/pckeybrd.o \
		$(OBJ)/mess/machine/pc_fdc_h.o \
		$(OBJ)/mess/machine/pc_flopp.o \
		$(OBJ)/mess/machine/basicdsk.o \
		$(OBJ)/mess/machine/wd179x.o   \
		$(OBJ)/mess/diskctrl.o \
		$(OBJ)/mess/machine/dsk.o \
		$(OBJ)/mess/machine/nec765.o

#8255 already in mame core

# additional tools
TOOLS +=  dat2html$(EXE) mkhdimg$(EXE) imgtool$(EXE) 

dat2html$(EXE): $(OBJ)/mess/tools/dat2html.o $(OBJ)/mess/utils.o
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

mkhdimg$(EXE):	$(OBJ)/mess/tools/mkhdimg.o
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

imgtool$(EXE):	     \
	  $(IMGTOOL_OBJS) \
	  $(OBJ)/mess/tools/stubs.o   \
	  $(OBJ)/mess/config.o	      \
	  $(OBJ)/unzip.o	      \
	  $(OBJ)/mess/utils.o	\
	  $(OBJ)/mess/tools/main.o    \
	  $(OBJ)/mess/tools/imgtool.o \
	  $(OBJ)/mess/tools/imgwave.o \
	  $(OBJ)/mess/tools/cococas.o   \
	  $(OBJ)/mess/formats/cococas.o  \
	  $(OBJ)/mess/tools/rsdos.o   \
	  $(OBJ)/mess/tools/stream.o  \
	  $(OBJ)/mess/tools/t64.o     \
	  $(OBJ)/mess/tools/lynx.o    \
	  $(OBJ)/mess/tools/crt.o     \
	  $(OBJ)/mess/tools/d64.o     \
	  $(OBJ)/mess/tools/fat.o     \
	  $(OBJ)/mess/tools/pchd.o    \
	  $(OBJ)/mess/tools/zip.o     \
	  $(OBJ)/mess/tools/fs.o
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@


#transdsk$(EXE):       \
#      $(TRANSDISK_OBJS) \
#      $(OBJ)/mess/tools/stubs.o   \
#      $(OBJ)/mess/tools/transdsk/nec765.o   
#    @echo Linking $@...
#    $(LD) $(LDFLAGS) $(LIBS) $^ -o $@


# text files
ifeq ($(OS),msdos)
TEXTS = mess.txt
mess.txt: $(EMULATOR)
	@echo Generating $@...
	@$(EMULATOR) -listtext > mess.txt
	@$(EMULATOR) -listdevices >> mess.txt
endif

mess/makedep/makedep$(EXE): $(wildcard mess/makedep/*.c) $(wildcard mess/makedep/*.h)
	make -Cmess/makedep

src/$(NAME).dep depend: mess/makedep/makedep$(EXE) src/$(TARGET).mak src/rules.mak src/core.mak
	mess/makedep/makedep$(EXE) -f - -p$(NAME).obj/ -q -- $(INCLUDE_PATH) -- src/*.c \
	src/cpu/*/*.c src/sound/*.c mess/systems/*.c mess/machine/*.c mess/vidhrdw/*.c mess/sndhrdw/*.c \
	mess/tools/*.c mess/formats/*.c >src/$(NAME).dep

## uncomment the following line to include dependencies
ifeq (src/$(NAME).dep,$(wildcard src/$(NAME).dep))
include src/$(NAME).dep
endif
