/***************************************************************************

	Killer Instinct hardware

	driver by Aaron Giles and Bryan McPhail

	Games supported:
		* Killer Instinct
		* Killer Instinct 2

	Known bugs:
		* several tests hang because of our hacking screen flipping code

***************************************************************************/

#include "driver.h"
#include "cpu/mips/mips3.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "sndhrdw/dcs.h"
#include "kinst.h"


/* constants */
#define MASTER_CLOCK	50000000


/* local variables */
static data32_t *rombase, *rambase1, *rambase2;
static data32_t *kinst_control;
static data32_t *kinst_speedup;

static const UINT8 *control_map;



/*************************************
 *
 *	Machine init
 *
 *************************************/

static MACHINE_INIT( kinst )
{
	cpu_setbank(1, rambase1 + 0x1000/4);
	cpu_setbank(2, rombase);
	cpu_setbank(3, rambase1);
	cpu_setbank(4, rambase2 + 0x90000/4);

	ide_controller_reset(0);
}



/*************************************
 *
 *	Interrupt handling
 *
 *************************************/

static void irq0_stop(int param)
{
	cpu_set_irq_line(0, 0, CLEAR_LINE);
}


static INTERRUPT_GEN( irq0_start )
{
	cpu_set_irq_line(0, 0, ASSERT_LINE);
	timer_set(TIME_IN_USEC(50), 0, irq0_stop);
}


static void ide_interrupt(int state)
{
	cpu_set_irq_line(0, 1, state);
}


static struct ide_interface ide_intf =
{
	ide_interrupt
};



/*************************************
 *
 *	IDE controller access
 *
 *************************************/

static READ32_HANDLER( ide_controller_r )
{
	return midway_ide_asic_r(offset / 2, mem_mask);
}


static WRITE32_HANDLER( ide_controller_w )
{
	midway_ide_asic_w(offset / 2, data, mem_mask);
}


static READ32_HANDLER( ide_controller_extra_r )
{
	return ide_controller32_0_r(0x3f6/4, 0xff00ffff) >> 16;
}


static WRITE32_HANDLER( ide_controller_extra_w )
{
	ide_controller32_0_w(0x3f6/4, data << 16, 0xff00ffff);
}



/*************************************
 *
 *	Control handling
 *
 *************************************/

static READ32_HANDLER( kinst_control_r )
{
	data32_t result;
	UINT32 temp;

	offset = control_map[offset / 2];
	result = kinst_control[offset];

	switch (offset)
	{
		case 2:		/* $90 -- sound return */
			result = 0xffff0000 | readinputport(offset);
			temp = dcs_control_r();
			result &= ~0x0002;
			if (temp & 0x800)
				result |= 0x0002;
			break;

		case 0:		/* $80 */
		case 1:		/* $88 */
		case 3:		/* $98 */
			result = 0xffff0000 | readinputport(offset);
			break;

		case 4:		/* $a0 */
			result = 0xffff0000 | readinputport(offset);
			if (activecpu_get_pc() == 0x802d428)
				cpu_spinuntil_int();
			break;
	}

	return result;
}


static WRITE32_HANDLER( kinst_control_w )
{
	data32_t olddata;

	offset = control_map[offset / 2];
	olddata = kinst_control[offset];
	COMBINE_DATA(&kinst_control[offset]);

	switch (offset)
	{
		case 0:		/* $80 - VRAM buffer control?? */
			kinst_buffer_vram(&rambase1[0x30000/4]);
			break;

		case 2:		/* $90 - sound control */
			if (!(olddata & 0x02) && (kinst_control[offset] & 0x02))
				dcs_data_w(kinst_control[3]);
			break;

		case 3:		/* $98 - sound data */
			break;
	}
}



/*************************************
 *
 *	Speedups
 *
 *************************************/

static void cause_polling(offs_t pc, data32_t value)
{
	/* watch for code that loops on the Cause register, waiting for an interrupt */
	switch (pc & 0x1fffffff)
	{
		case 0x8001408:	/* KI */
		case 0x8001f68:	/* KI */
		case 0x8012368:	/* KI */
		case 0x8002360:	/* KI2 */
		case 0x80024d4:	/* KI2 */
		case 0x8003358:	/* KI2 */
		case 0x801a308:	/* KI2 */
			/* waiting for VBLANK interrupt */
			if (!(value & 0x400))
				cpu_spinuntil_int();
			break;

		case 0x8010340:	/* KI */
		case 0x802d428:	/* KI */
		case 0x801bdbc:	/* KI2 */
			/* waiting for IDE interrupt */
			if (!(value & 0x800))
				cpu_spinuntil_int();
			break;

		case 0x8012354:	/* KI */
		case 0x80263bc:	/* KI */
		case 0x8000c8c:	/* KI2 */
		case 0x801a2f4:	/* KI2 */
		case 0x8023aac:	/* KI2 */
			/* other cases */
			break;

		default:
			printf("Hit @ %08X\n", pc);
			break;
	}
}


static void end_spin(int param)
{
	cpu_triggerint(0);
}


static READ32_HANDLER( kinst_speedup_r )
{
	if (activecpu_get_pc() == 0x88029890 ||	/* KI */
		activecpu_get_pc() == 0x8802c2d0	/* KI2 */)
	{
		UINT32 r3 = activecpu_get_reg(MIPS3_R3);
		UINT32 r26 = activecpu_get_reg(MIPS3_R26) - *kinst_speedup;
		if (r26 < r3)
		{
			timer_set(TIME_IN_CYCLES((r3 - r26) * 2, 0), 0, end_spin);
			cpu_spinuntil_int();
		}
	}
	return *kinst_speedup;
}



/*************************************
 *
 *	Main CPU memory handlers
 *
 *************************************/

static MEMORY_READ32_START( main_readmem )
	{ 0x00000000, 0x00000fff, MRA32_BANK4 },
	{ 0x00001000, 0x0007ffff, MRA32_BANK1 },	/* -> rambase1 + 0x1000 */
	{ 0x80000000, 0x8007ffff, MRA32_RAM },
	{ 0x88000000, 0x887fffff, MRA32_RAM },
	{ 0x9fc00000, 0x9fc7ffff, MRA32_BANK2 },	/* -> rombase */
	{ 0xa0000000, 0xa007ffff, MRA32_BANK3 },	/* -> rambase1 */
	{ 0xb0000080, 0xb00000ff, kinst_control_r },
	{ 0xb0000100, 0xb000013f, ide_controller_r },
	{ 0xb0000170, 0xb0000173, ide_controller_extra_r },
	{ 0xbfc00000, 0xbfc7ffff, MRA32_ROM },
MEMORY_END


static MEMORY_WRITE32_START( main_writemem )
	{ 0x00000000, 0x00000fff, MWA32_BANK4 },
	{ 0x00001000, 0x0007ffff, MWA32_BANK1 },
	{ 0x80000000, 0x8007ffff, MWA32_RAM, &rambase1 },
	{ 0x88000000, 0x887fffff, MWA32_RAM, &rambase2 },
	{ 0x9fc00000, 0x9fc7ffff, MWA32_ROM },
	{ 0xa0000000, 0xa007ffff, MWA32_BANK3 },
	{ 0xb0000080, 0xb00000ff, kinst_control_w, &kinst_control },
	{ 0xb0000100, 0xb000013f, ide_controller_w },
	{ 0xb0000170, 0xb0000173, ide_controller_extra_w },
	{ 0xbfc00000, 0xbfc7ffff, MWA32_ROM, &rombase },
MEMORY_END




/*************************************
 *
 *	Port definitions
 *
 *************************************/

INPUT_PORTS_START( kinst )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BITX(0x1000, IP_ACTIVE_LOW, IPT_SERVICE, DEF_STR( Service_Mode ), KEYCODE_F2, IP_JOY_NONE )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* door */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 | IPF_PLAYER2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* bill */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* coin door */

	PORT_START
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BITX(0x0004, IP_ACTIVE_LOW, 0, "Volume Up", KEYCODE_EQUALS, IP_JOY_NONE )
	PORT_BITX(0x0008, IP_ACTIVE_LOW, 0, "Volume Down", KEYCODE_MINUS, IP_JOY_NONE )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )	/* verify */

	PORT_START
	PORT_DIPNAME( 0x0003, 0x0003, "Blood Level" )
	PORT_DIPSETTING(      0x0003, "High")
	PORT_DIPSETTING(      0x0002, "Medium")
	PORT_DIPSETTING(      0x0001, "Low")
	PORT_DIPSETTING(      0x0000, "None")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0004, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Finishing Moves" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0008, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, "Display Warning" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0010, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x3000, 0x3000, "Country" )
	PORT_DIPSETTING(      0x3000, "USA" )
	PORT_DIPSETTING(      0x2000, "Germany" )
	PORT_DIPSETTING(      0x1000, "France" )
/*	PORT_DIPSETTING(      0x0000, "USA" )*/
	PORT_DIPNAME( 0x4000, 0x4000, "Coin Counters" )
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
INPUT_PORTS_END



/*************************************
 *
 *	Machine driver
 *
 *************************************/

static struct mips3_config config =
{
	16384,				/* code cache size */
	16384,				/* data cache size */
	cause_polling		/* Cause register polling callback */
};


MACHINE_DRIVER_START( kinst )

	/* basic machine hardware */
	MDRV_CPU_ADD(R4600LE, MASTER_CLOCK*2)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_MEMORY(main_readmem,main_writemem)
	MDRV_CPU_VBLANK_INT(irq0_start,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_INIT(kinst)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_VISIBLE_AREA(0, 319, 0, 239)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_PALETTE_INIT(kinst)
	MDRV_VIDEO_START(kinst)
	MDRV_VIDEO_UPDATE(kinst)

	/* sound hardware */
	MDRV_IMPORT_FROM(dcs_audio)
MACHINE_DRIVER_END



/*************************************
 *
 *	ROM definition(s)
 *
 *************************************/

ROM_START( kinst )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )		/* dummy region for R3000 */

	ROM_REGION( ADSP2100_SIZE + 0x800000, REGION_CPU2, 0 )	/* ADSP-2105 data */
	ROM_LOAD( "u10-l1", ADSP2100_SIZE + 0x000000, 0x80000, 0xb6cc155f )
	ROM_LOAD( "u11-l1", ADSP2100_SIZE + 0x100000, 0x80000, 0x0b5e05df )
	ROM_LOAD( "u12-l1", ADSP2100_SIZE + 0x200000, 0x80000, 0xd05ce6ad )
	ROM_LOAD( "u13-l1", ADSP2100_SIZE + 0x300000, 0x80000, 0x7d0954ea )
	ROM_LOAD( "u33-l1", ADSP2100_SIZE + 0x400000, 0x80000, 0x8bbe4f0c )
	ROM_LOAD( "u34-l1", ADSP2100_SIZE + 0x500000, 0x80000, 0xb2e73603 )
	ROM_LOAD( "u35-l1", ADSP2100_SIZE + 0x600000, 0x80000, 0x0aaef4fc )
	ROM_LOAD( "u36-l1", ADSP2100_SIZE + 0x700000, 0x80000, 0x0577bb60 )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )	/* 512k for R4600 code */
	ROM_LOAD( "u98-l14", 0x00000, 0x80000, 0xafedb75f )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "kinst.chd", 0, 0x6d4c2f152c9a18ab3a9b05b8804306a8 )
ROM_END


ROM_START( kinst2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )		/* dummy region for R3000 */

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )	/* 512k for R4600 code */
	ROM_LOAD( "ki2-l14.u98", 0x00000, 0x80000, 0x27d0285e )

	ROM_REGION( ADSP2100_SIZE + 0x800000, REGION_CPU2, 0 )	/* ADSP-2105 data */
	ROM_LOAD( "ki2_l1.u10", ADSP2100_SIZE + 0x000000, 0x80000, 0xfdf6ed51 )
	ROM_LOAD( "ki2_l1.u11", ADSP2100_SIZE + 0x100000, 0x80000, 0xf9e70024 )
	ROM_LOAD( "ki2_l1.u12", ADSP2100_SIZE + 0x200000, 0x80000, 0x2994c199 )
	ROM_LOAD( "ki2_l1.u13", ADSP2100_SIZE + 0x300000, 0x80000, 0x3fe6327b )
	ROM_LOAD( "ki2_l1.u33", ADSP2100_SIZE + 0x400000, 0x80000, 0x6f4dcdcf )
	ROM_LOAD( "ki2_l1.u34", ADSP2100_SIZE + 0x500000, 0x80000, 0x5db48206 )
	ROM_LOAD( "ki2_l1.u35", ADSP2100_SIZE + 0x600000, 0x80000, 0x7245ce69 )
	ROM_LOAD( "ki2_l1.u36", ADSP2100_SIZE + 0x700000, 0x80000, 0x8920acbb )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "kinst2.chd", 0, 0x2563b089b316f2c8636d78af661ac656 )
ROM_END



/*************************************
 *
 *	Driver initialization
 *
 *************************************/

static DRIVER_INIT( kinst )
{
	static const UINT8 kinst_control_map[8] = { 0,1,2,3,4,5,6,7 };
	UINT8 *features;

	dcs_init();
	memcpy(rombase, memory_region(REGION_USER1), memory_region_length(REGION_USER1));

	/* set up the control register mapping */
	control_map = kinst_control_map;

	/* spin up the hard disk */
	ide_controller_init(0, &ide_intf);

	/* tweak the model number so we pass the check */
	features = ide_get_features(0);
	features[27*2+0] = 0x54;
	features[27*2+1] = 0x53;
	features[28*2+0] = 0x31;
	features[28*2+1] = 0x39;
	features[29*2+0] = 0x30;
	features[29*2+1] = 0x35;
	features[30*2+0] = 0x47;
	features[30*2+1] = 0x41;
	features[31*2+0] = 0x20;
	features[31*2+1] = 0x20;

	/* optimize one of the non-standard loops */
	kinst_speedup = install_mem_read32_handler(0, 0x8808f5bc, 0x8808f5bf, kinst_speedup_r);
}


static DRIVER_INIT( kinst2 )
{
	static const UINT8 kinst2_control_map[8] = { 2,4,1,0,3,5,6,7 };
	UINT8 *features;

	// read: $80 on ki2 = $90 on ki
	// read: $88 on ki2 = $a0 on ki
	// write: $80 on ki2 = $90 on ki
	// write: $90 on ki2 = $88 on ki
	// write: $98 on ki2 = $80 on ki
	// write: $a0 on ki2 = $98 on ki

	dcs_init();
	memcpy(rombase, memory_region(REGION_USER1), memory_region_length(REGION_USER1));

	/* set up the control register mapping */
	control_map = kinst2_control_map;

	/* spin up the hard disk */
	ide_controller_init(0, &ide_intf);

	/* tweak the model number so we pass the check */
	features = ide_get_features(0);
	features[10*2+0] = 0x30;
	features[10*2+1] = 0x30;
	features[11*2+0] = 0x54;
	features[11*2+1] = 0x53;
	features[12*2+0] = 0x31;
	features[12*2+1] = 0x39;
	features[13*2+0] = 0x30;
	features[13*2+1] = 0x35;
	features[14*2+0] = 0x47;
	features[14*2+1] = 0x41;

	/* optimize one of the non-standard loops */
	kinst_speedup = install_mem_read32_handler(0, 0x887ff544, 0x887ff547, kinst_speedup_r);
}



/*************************************
 *
 *	Game driver(s)
 *
 *************************************/

GAME( 1994, kinst,	0,		kinst, kinst,  kinst,	ROT0, "Rare", "Killer Instinct (v1.0)" )
GAME( 1994, kinst2, 0,		kinst, kinst,  kinst2,	ROT0, "Rare", "Killer Instinct 2 (v2.1)" )