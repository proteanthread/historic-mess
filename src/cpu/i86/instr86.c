/****************************************************************************
 *			   real mode i286 emulator v1.4 by Fabrice Frances
 *				 (initial work based on David Hedley's pcemu)
 ****************************************************************************/

/****************************************************************************
 * file will be included in all cpu variants
 * put non i86 instructions in own files (i286, i386, nec)
 * function renaming will be added when neccessary
 * timing value should move to separate array
 ****************************************************************************/

#if !defined(V20) && !defined(I186)
static void PREFIX86(_interrupt)(unsigned int_num)
{
	unsigned dest_seg, dest_off;

	PREFIX(_pushf());
	I.TF = I.IF = 0;

	if (int_num == -1)
		int_num = (*I.irq_callback)(0);

	dest_off = ReadWord(int_num*4);
	dest_seg = ReadWord(int_num*4+2);

	PUSH(I.sregs[CS]);
	PUSH(I.ip);
	I.ip = (WORD)dest_off;
	I.sregs[CS] = (WORD)dest_seg;
	I.base[CS] = SegBase(CS);
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);

}

static void PREFIX86(_trap)(void)
{
	PREFIX(_instruction)[FETCHOP]();
	PREFIX(_interrupt)(1);
}

static void PREFIX86(_external_int)(void)
{
	if( I.pending_irq & NMI_IRQ )
	{
		PREFIX(_interrupt)(I86_NMI_INT);
		I.pending_irq &= ~NMI_IRQ;
	}
	else
	if( I.pending_irq )
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		PREFIX(_interrupt)(-1);
	}
}
#endif

#ifndef I186
static void PREFIX86(_rotate_shift_Byte)(unsigned ModRM, unsigned count)
{
	unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		i86_ICount-=8; /* or 7 if dest is in memory */
	}
	else
	if (count==1)
	{
#ifdef V20
		nec_ICount-=(ModRM >=0xc0 )?2:16;
#else
		i86_ICount-=2;
#endif
		switch (ModRM & 0x38)
	 {
		case 0x00:  /* ROL eb,1 */
		I.CarryVal = src & 0x80;
		  dst=(src<<1)+CF;
		  PutbackRMByte(ModRM,dst);
		I.OverVal = (src^dst)&0x80;
		break;
		case 0x08:  /* ROR eb,1 */
		I.CarryVal = src & 0x01;
		  dst = ((CF<<8)+src) >> 1;
		  PutbackRMByte(ModRM,dst);
		I.OverVal = (src^dst)&0x80;
		break;
		case 0x10:  /* RCL eb,1 */
		  dst=(src<<1)+CF;
		  PutbackRMByte(ModRM,dst);
		  SetCFB(dst);
		I.OverVal = (src^dst)&0x80;
		break;
		case 0x18:  /* RCR eb,1 */
		  dst = ((CF<<8)+src) >> 1;
		  PutbackRMByte(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = (src^dst)&0x80;
		break;
		case 0x20:  /* SHL eb,1 */
		case 0x30:
		  dst = src << 1;
		  PutbackRMByte(ModRM,dst);
		  SetCFB(dst);
		I.OverVal = (src^dst)&0x80;
		I.AuxVal = 1;
		  SetSZPF_Byte(dst);
		break;
		case 0x28:  /* SHR eb,1 */
		  dst = src >> 1;
		  PutbackRMByte(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = src & 0x80;
		I.AuxVal = 1;
		  SetSZPF_Byte(dst);
		break;
		case 0x38:  /* SAR eb,1 */
		  dst = ((INT8)src) >> 1;
		  PutbackRMByte(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = 0;
		I.AuxVal = 1;
		  SetSZPF_Byte(dst);
		break;
	 }
  }
  else
  {
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?7+4*count:19+4*count;
#else
	i86_ICount-=8+4*count; /* or 7+4*count if dest is in memory */
#endif
	 switch (ModRM & 0x38)
	 {
		case 0x00:  /* ROL eb,count */
		for (; count > 0; count--)
		{
			I.CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
		}
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
	  case 0x08:  /* ROR eb,count */
		for (; count > 0; count--)
		{
			I.CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
		}
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
		case 0x10:  /* RCL eb,count */
		for (; count > 0; count--)
		{
				dst = (dst << 1) + CF;
				SetCFB(dst);
		}
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
		case 0x18:  /* RCR eb,count */
		for (; count > 0; count--)
		{
				dst = (CF<<8)+dst;
			I.CarryVal = dst & 0x01;
				dst >>= 1;
		}
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
		case 0x20:
		case 0x30:  /* SHL eb,count */
		  dst <<= count;
		  SetCFB(dst);
		I.AuxVal = 1;
		  SetSZPF_Byte(dst);
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
		case 0x28:  /* SHR eb,count */
		  dst >>= count-1;
		I.CarryVal = dst & 0x1;
		  dst >>= 1;
		  SetSZPF_Byte(dst);
		I.AuxVal = 1;
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
		case 0x38:  /* SAR eb,count */
		  dst = ((INT8)dst) >> (count-1);
		I.CarryVal = dst & 0x1;
		  dst = ((INT8)((BYTE)dst)) >> 1;
		  SetSZPF_Byte(dst);
		I.AuxVal = 1;
		  PutbackRMByte(ModRM,(BYTE)dst);
		break;
	 }
  }
}

static void PREFIX86(_rotate_shift_Word)(unsigned ModRM, unsigned count)
{
  unsigned src = GetRMWord(ModRM);
  unsigned dst=src;

  if (count==0)
  {
	i86_ICount-=8; /* or 7 if dest is in memory */
  }
  else if (count==1)
  {
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
	i86_ICount-=2;
#endif
    switch (ModRM & 0x38)
    {
#if 0
      case 0x00:  /* ROL ew,1 */
        tmp2 = (tmp << 1) + CF;
		SetCFW(tmp2);
		I.OverVal = !(!(tmp & 0x4000)) != CF;
		PutbackRMWord(ModRM,tmp2);
		break;
      case 0x08:  /* ROR ew,1 */
		I.CarryVal = tmp & 0x01;
		tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
		I.OverVal = !(!(tmp & 0x8000)) != CF;
		PutbackRMWord(ModRM,tmp2);
		break;
      case 0x10:  /* RCL ew,1 */
		tmp2 = (tmp << 1) + CF;
		SetCFW(tmp2);
		I.OverVal = (tmp ^ (tmp << 1)) & 0x8000;
		PutbackRMWord(ModRM,tmp2);
		break;
	  case 0x18:  /* RCR ew,1 */
		tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
		I.OverVal = !(!(tmp & 0x8000)) != CF;
		I.CarryVal = tmp & 0x01;
		PutbackRMWord(ModRM,tmp2);
		break;
      case 0x20:  /* SHL ew,1 */
      case 0x30:
		tmp <<= 1;

		SetCFW(tmp);
		SetOFW_Add(tmp,tmp2,tmp2);
		I.AuxVal = 1;
		SetSZPF_Word(tmp);

		PutbackRMWord(ModRM,tmp);
		break;
      case 0x28:  /* SHR ew,1 */
		I.CarryVal = tmp & 0x01;
		I.OverVal = tmp & 0x8000;

		tmp2 = tmp >> 1;

		SetSZPF_Word(tmp2);
		I.AuxVal = 1;
		PutbackRMWord(ModRM,tmp2);
		break;
		case 0x38:  /* SAR ew,1 */
		I.CarryVal = tmp & 0x01;
		I.OverVal = 0;

		tmp2 = (tmp >> 1) | (tmp & 0x8000);

		SetSZPF_Word(tmp2);
		I.AuxVal = 1;
		PutbackRMWord(ModRM,tmp2);
		break;
#else
		case 0x00:  /* ROL ew,1 */
		I.CarryVal = src & 0x8000;
        dst=(src<<1)+CF;
        PutbackRMWord(ModRM,dst);
		I.OverVal = (src^dst)&0x8000;
		break;
		case 0x08:  /* ROR ew,1 */
		I.CarryVal = src & 0x01;
        dst = ((CF<<16)+src) >> 1;
        PutbackRMWord(ModRM,dst);
		I.OverVal = (src^dst)&0x8000;
		break;
      case 0x10:  /* RCL ew,1 */
        dst=(src<<1)+CF;
        PutbackRMWord(ModRM,dst);
        SetCFW(dst);
		I.OverVal = (src^dst)&0x8000;
		break;
      case 0x18:  /* RCR ew,1 */
        dst = ((CF<<16)+src) >> 1;
        PutbackRMWord(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = (src^dst)&0x8000;
		break;
      case 0x20:  /* SHL ew,1 */
      case 0x30:
        dst = src << 1;
        PutbackRMWord(ModRM,dst);
		  SetCFW(dst);
		I.OverVal = (src^dst)&0x8000;
		I.AuxVal = 1;
        SetSZPF_Word(dst);
		break;
      case 0x28:  /* SHR ew,1 */
        dst = src >> 1;
		  PutbackRMWord(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = src & 0x8000;
		I.AuxVal = 1;
        SetSZPF_Word(dst);
		break;
		case 0x38:  /* SAR ew,1 */
        dst = ((INT16)src) >> 1;
        PutbackRMWord(ModRM,dst);
		I.CarryVal = src & 0x01;
		I.OverVal = 0;
		I.AuxVal = 1;
        SetSZPF_Word(dst);
	break;
#endif
    }
  }
  else
  {
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?7+count*4:27+count*4;
#else
	i86_ICount-=8+4*count; /* or 7+4*count if dest is in memory */
#endif

    switch (ModRM & 0x38)
	 {
      case 0x00:  /* ROL ew,count */
		for (; count > 0; count--)
		{
			I.CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
		}
        PutbackRMWord(ModRM,dst);
		break;
      case 0x08:  /* ROR ew,count */
		for (; count > 0; count--)
		{
			I.CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
		}
        PutbackRMWord(ModRM,dst);
		break;
		case 0x10:  /* RCL ew,count */
		for (; count > 0; count--)
		{
            dst = (dst << 1) + CF;
            SetCFW(dst);
		}
        PutbackRMWord(ModRM,dst);
	break;
      case 0x18:  /* RCR ew,count */
		for (; count > 0; count--)
		{
            dst = dst + (CF << 16);
			I.CarryVal = dst & 0x01;
            dst >>= 1;
		}
        PutbackRMWord(ModRM,dst);
		break;
		case 0x20:
      case 0x30:  /* SHL ew,count */
        dst <<= count;
        SetCFW(dst);
		I.AuxVal = 1;
		  SetSZPF_Word(dst);
        PutbackRMWord(ModRM,dst);
		break;
      case 0x28:  /* SHR ew,count */
        dst >>= count-1;
		I.CarryVal = dst & 0x1;
        dst >>= 1;
		  SetSZPF_Word(dst);
		I.AuxVal = 1;
		  PutbackRMWord(ModRM,dst);
		break;
      case 0x38:  /* SAR ew,count */
		  dst = ((INT16)dst) >> (count-1);
		I.CarryVal = dst & 0x01;
		  dst = ((INT16)((WORD)dst)) >> 1;
		  SetSZPF_Word(dst);
		I.AuxVal = 1;
        PutbackRMWord(ModRM,dst);
		break;
    }
  }
}
#endif

static void PREFIX(rep)(int flagval)
{
    /* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
		 loop  to continue for CMPS and SCAS instructions. */

	unsigned next = FETCHOP;
	unsigned count = I.regs.w[CX];

    switch(next)
    {
	 case 0x26:  /* ES: */
		  seg_prefix=TRUE;
		prefix_base=I.base[ES];
		i86_ICount-=2;
		PREFIX(rep)(flagval);
		break;
	 case 0x2e:  /* CS: */
		  seg_prefix=TRUE;
		prefix_base=I.base[CS];
		i86_ICount-=2;
		PREFIX(rep)(flagval);
		break;
	 case 0x36:  /* SS: */
		  seg_prefix=TRUE;
		prefix_base=I.base[SS];
		i86_ICount-=2;
		PREFIX(rep)(flagval);
		break;
	 case 0x3e:  /* DS: */
		  seg_prefix=TRUE;
		prefix_base=I.base[DS];
		i86_ICount-=2;
		PREFIX(rep)(flagval);
		break;
#ifndef I86
	 case 0x6c:  /* REP INSB */
		i86_ICount-=9-count;
		for (; count > 0; count--)
				PREFIX(_insb)();
		I.regs.w[CX]=count;
		break;
	 case 0x6d:  /* REP INSW */
		i86_ICount-=9-count;
		for (; count > 0; count--)
				PREFIX(_insw)();
		I.regs.w[CX]=count;
		break;
	 case 0x6e:  /* REP OUTSB */
		i86_ICount-=9-count;
		for (; count > 0; count--)
				PREFIX(_outsb)();
		I.regs.w[CX]=count;
		break;
	 case 0x6f:  /* REP OUTSW */
		i86_ICount-=9-count;
		for (; count > 0; count--)
				PREFIX(_outsw)();
		I.regs.w[CX]=count;
		break;
#endif
	 case 0xa4:  /* REP MOVSB */
		i86_ICount-=9-count;
		for (; count > 0; count--)
			PREFIX86(_movsb)();
		I.regs.w[CX]=count;
		break;
	 case 0xa5:  /* REP MOVSW */
		i86_ICount-=9-count;
		for (; count > 0; count--)
			PREFIX86(_movsw)();
		I.regs.w[CX]=count;
		break;
	 case 0xa6:  /* REP(N)E CMPSB */
		i86_ICount-=9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsb)();
		I.regs.w[CX]=count;
		break;
	 case 0xa7:  /* REP(N)E CMPSW */
		i86_ICount-=9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsw)();
		I.regs.w[CX]=count;
		break;
	 case 0xaa:  /* REP STOSB */
		i86_ICount-=9-count;
		for (; count > 0; count--)
			PREFIX86(_stosb)();
		I.regs.w[CX]=count;
		break;
	 case 0xab:  /* REP STOSW */
		i86_ICount-=9-count;
		for (; count > 0; count--)
			PREFIX86(_stosw)();
		I.regs.w[CX]=count;
		break;
	 case 0xac:  /* REP LODSB */
		i86_ICount-=9;
		for (; count > 0; count--)
			PREFIX86(_lodsb)();
		I.regs.w[CX]=count;
		break;
	 case 0xad:  /* REP LODSW */
		i86_ICount-=9;
		for (; count > 0; count--)
			PREFIX86(_lodsw)();
		I.regs.w[CX]=count;
		break;
	 case 0xae:  /* REP(N)E SCASB */
		i86_ICount-=9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
			PREFIX86(_scasb)();
		I.regs.w[CX]=count;
		break;
	 case 0xaf:  /* REP(N)E SCASW */
		i86_ICount-=9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
			PREFIX86(_scasw)();
		I.regs.w[CX]=count;
		break;
	 default:
		PREFIX(_instruction)[next]();
	 }
}

#ifndef I186
static void PREFIX86(_add_br8)(void)    /* Opcode 0x00 */
{
	 DEF_br8(dst,src);
	i86_ICount-=3;
	 ADDB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_add_wr16)(void)    /* Opcode 0x01 */
{
    DEF_wr16(dst,src);
	i86_ICount-=3;
	 ADDW(dst,src);
	 PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_add_r8b)(void)    /* Opcode 0x02 */
{
	 DEF_r8b(dst,src);
	i86_ICount-=3;
    ADDB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_add_r16w)(void)    /* Opcode 0x03 */
{
    DEF_r16w(dst,src);
	i86_ICount-=3;
	 ADDW(dst,src);
	 RegWord(ModRM)=dst;
}


static void PREFIX86(_add_ald8)(void)    /* Opcode 0x04 */
{
    DEF_ald8(dst,src);
	i86_ICount-=4;
    ADDB(dst,src);
	I.regs.b[AL]=dst;
}


static void PREFIX86(_add_axd16)(void)    /* Opcode 0x05 */
{
    DEF_axd16(dst,src);
	i86_ICount-=4;
	 ADDW(dst,src);
	I.regs.w[AX]=dst;
}


static void PREFIX86(_push_es)(void)    /* Opcode 0x06 */
{
	i86_ICount-=3;
	PUSH(I.sregs[ES]);
}


static void PREFIX86(_pop_es)(void)    /* Opcode 0x07 */
{
	POP(I.sregs[ES]);
	I.base[ES] = SegBase(ES);
	i86_ICount-=2;
}

static void PREFIX86(_or_br8)(void)    /* Opcode 0x08 */
{
    DEF_br8(dst,src);
	i86_ICount-=3;
	 ORB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_or_wr16)(void)    /* Opcode 0x09 */
{
	 DEF_wr16(dst,src);
	i86_ICount-=3;
    ORW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_or_r8b)(void)    /* Opcode 0x0a */
{
	 DEF_r8b(dst,src);
	i86_ICount-=3;
    ORB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_or_r16w)(void)    /* Opcode 0x0b */
{
    DEF_r16w(dst,src);
	i86_ICount-=3;
    ORW(dst,src);
    RegWord(ModRM)=dst;
}

static void PREFIX86(_or_ald8)(void)    /* Opcode 0x0c */
{
    DEF_ald8(dst,src);
	i86_ICount-=4;
    ORB(dst,src);
	I.regs.b[AL]=dst;
}

static void PREFIX86(_or_axd16)(void)    /* Opcode 0x0d */
{
    DEF_axd16(dst,src);
	i86_ICount-=4;
	 ORW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_push_cs)(void)    /* Opcode 0x0e */
{
	i86_ICount-=3;
	PUSH(I.sregs[CS]);
}

/* Opcode 0x0f invalid */

static void PREFIX86(_adc_br8)(void)    /* Opcode 0x10 */
{
    DEF_br8(dst,src);
    src+=CF;
    ADDB(dst,src);
    PutbackRMByte(ModRM,dst);
#ifdef V20
    nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
    i86_ICount-=3;
#endif
}

static void PREFIX86(_adc_wr16)(void)    /* Opcode 0x11 */
{
    DEF_wr16(dst,src);
    src+=CF;
    ADDW(dst,src);
	 PutbackRMWord(ModRM,dst);
#ifdef V20
    nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
    i86_ICount-=3;
#endif
}

static void PREFIX86(_adc_r8b)(void)    /* Opcode 0x12 */
{
    DEF_r8b(dst,src);
	i86_ICount-=3;
	 src+=CF;
    ADDB(dst,src);
    RegByte(ModRM)=dst;
#ifdef V20
    nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
    i86_ICount-=3;
#endif
}

static void PREFIX86(_adc_r16w)(void)    /* Opcode 0x13 */
{
    DEF_r16w(dst,src);
	 src+=CF;
    ADDW(dst,src);
    RegWord(ModRM)=dst;
#ifdef V20
    nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
    i86_ICount-=3;
#endif
}

static void PREFIX86(_adc_ald8)(void)    /* Opcode 0x14 */
{
    DEF_ald8(dst,src);
	i86_ICount-=4;
    src+=CF;
    ADDB(dst,src);
	I.regs.b[AL] = dst;
    i86_ICount-=4;
}

static void PREFIX86(_adc_axd16)(void)    /* Opcode 0x15 */
{
	 DEF_axd16(dst,src);
	 src+=CF;
    ADDW(dst,src);
	I.regs.w[AX]=dst;
    i86_ICount-=4;
}

static void PREFIX86(_push_ss)(void)    /* Opcode 0x16 */
{
	PUSH(I.sregs[SS]);
#ifdef V20
	nec_ICount-=10;                         /* OPCODE.LST says 8-12...so 10 */
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_pop_ss)(void)    /* Opcode 0x17 */
{
	POP(I.sregs[SS]);
	I.base[SS] = SegBase(SS);
	PREFIX(_instruction)[FETCHOP](); /* no interrupt before next instruction */
#ifdef V20
	nec_ICount-=10;                         /* OPCODE.LST says 8-12...so 10 */
#else
	i86_ICount-=2;
#endif
}

static void PREFIX86(_sbb_br8)(void)    /* Opcode 0x18 */
{
    DEF_br8(dst,src);
    src+=CF;
    SUBB(dst,src);
    PutbackRMByte(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sbb_wr16)(void)    /* Opcode 0x19 */
{
    DEF_wr16(dst,src);
    src+=CF;
	 SUBW(dst,src);
	 PutbackRMWord(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sbb_r8b)(void)    /* Opcode 0x1a */
{
	 DEF_r8b(dst,src);
	 src+=CF;
    SUBB(dst,src);
    RegByte(ModRM)=dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sbb_r16w)(void)    /* Opcode 0x1b */
{
    DEF_r16w(dst,src);
	src+=CF;
    SUBW(dst,src);
    RegWord(ModRM)= dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sbb_ald8)(void)    /* Opcode 0x1c */
{
    DEF_ald8(dst,src);
    src+=CF;
    SUBB(dst,src);
	I.regs.b[AL] = dst;
	i86_ICount-=4;
}

static void PREFIX86(_sbb_axd16)(void)    /* Opcode 0x1d */
{
    DEF_axd16(dst,src);
	 src+=CF;
    SUBW(dst,src);
	I.regs.w[AX]=dst;
	i86_ICount-=4;
}

static void PREFIX86(_push_ds)(void)    /* Opcode 0x1e */
{
	PUSH(I.sregs[DS]);
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_pop_ds)(void)    /* Opcode 0x1f */
{
	POP(I.sregs[DS]);
	I.base[DS] = SegBase(DS);
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
}

static void PREFIX86(_and_br8)(void)    /* Opcode 0x20 */
{
    DEF_br8(dst,src);
    ANDB(dst,src);
    PutbackRMByte(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_and_wr16)(void)    /* Opcode 0x21 */
{
    DEF_wr16(dst,src);
    ANDW(dst,src);
    PutbackRMWord(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_and_r8b)(void)    /* Opcode 0x22 */
{
    DEF_r8b(dst,src);
    ANDB(dst,src);
    RegByte(ModRM)=dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_and_r16w)(void)    /* Opcode 0x23 */
{
    DEF_r16w(dst,src);
	 ANDW(dst,src);
    RegWord(ModRM)=dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_and_ald8)(void)    /* Opcode 0x24 */
{
    DEF_ald8(dst,src);
    ANDB(dst,src);
	I.regs.b[AL] = dst;
	i86_ICount-=4;
}

static void PREFIX86(_and_axd16)(void)    /* Opcode 0x25 */
{
    DEF_axd16(dst,src);
    ANDW(dst,src);
	I.regs.w[AX]=dst;
	i86_ICount-=4;
}

static void PREFIX86(_es)(void)    /* Opcode 0x26 */
{
	 seg_prefix=TRUE;
	prefix_base=I.base[ES];
	i86_ICount-=2;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_daa)(void)    /* Opcode 0x27 */
{
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		I.regs.b[AL] = tmp = I.regs.b[AL] + 6;
		I.AuxVal = 1;
		I.CarryVal |= tmp & 0x100;
	}

	if (CF || (I.regs.b[AL] > 0x9f))
	{
		I.regs.b[AL] += 0x60;
		I.CarryVal = 1;
	}

	SetSZPF_Byte(I.regs.b[AL]);
#ifdef V20
	nec_ICount-=3;
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_sub_br8)(void)    /* Opcode 0x28 */
{
    DEF_br8(dst,src);
	i86_ICount-=3;
    SUBB(dst,src);
    PutbackRMByte(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sub_wr16)(void)    /* Opcode 0x29 */
{
    DEF_wr16(dst,src);
    SUBW(dst,src);
    PutbackRMWord(ModRM,dst);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sub_r8b)(void)    /* Opcode 0x2a */
{
    DEF_r8b(dst,src);
	 SUBB(dst,src);
    RegByte(ModRM)=dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sub_r16w)(void)    /* Opcode 0x2b */
{
    DEF_r16w(dst,src);
    SUBW(dst,src);
    RegWord(ModRM)=dst;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_sub_ald8)(void)    /* Opcode 0x2c */
{
    DEF_ald8(dst,src);
    SUBB(dst,src);
	I.regs.b[AL] = dst;
	i86_ICount-=4;
}

static void PREFIX86(_sub_axd16)(void)    /* Opcode 0x2d */
{
	 DEF_axd16(dst,src);
	i86_ICount-=4;
    SUBW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_cs)(void)    /* Opcode 0x2e */
{
    seg_prefix=TRUE;
	prefix_base=I.base[CS];
	i86_ICount-=2;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_das)(void)    /* Opcode 0x2f */
{
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		I.regs.b[AL] = tmp = I.regs.b[AL] - 6;
		I.AuxVal = 1;
		I.CarryVal |= tmp & 0x100;
	}

	if (CF || (I.regs.b[AL] > 0x9f))
	{
		I.regs.b[AL] -= 0x60;
		I.CarryVal = 1;
	}

	SetSZPF_Byte(I.regs.b[AL]);
#ifdef V20
	nec_ICount-=7;
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_xor_br8)(void)    /* Opcode 0x30 */
{
    DEF_br8(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
    XORB(dst,src);
	 PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_xor_wr16)(void)    /* Opcode 0x31 */
{
	 DEF_wr16(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
	i86_ICount-=3;
#endif
	 XORW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_xor_r8b)(void)    /* Opcode 0x32 */
{
    DEF_r8b(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
    XORB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_xor_r16w)(void)    /* Opcode 0x33 */
{
    DEF_r16w(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
    XORW(dst,src);
	 RegWord(ModRM)=dst;
}

static void PREFIX86(_xor_ald8)(void)    /* Opcode 0x34 */
{
	 DEF_ald8(dst,src);
	i86_ICount-=4;
    XORB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_xor_axd16)(void)    /* Opcode 0x35 */
{
    DEF_axd16(dst,src);
	i86_ICount-=4;
    XORW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_ss)(void)    /* Opcode 0x36 */
{
	 seg_prefix=TRUE;
	prefix_base=I.base[SS];
	i86_ICount-=2;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_aaa)(void)    /* Opcode 0x37 */
{
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
    {
		I.regs.b[AL] += 6;
		I.regs.b[AH] += 1;
		I.AuxVal = 1;
		I.CarryVal = 1;
    }
	else
	{
		I.AuxVal = 0;
		I.CarryVal = 0;
    }
	I.regs.b[AL] &= 0x0F;
#ifdef V20
	nec_ICount-=3;
#else
	i86_ICount-=8;
#endif
}

static void PREFIX86(_cmp_br8)(void)    /* Opcode 0x38 */
{
    DEF_br8(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
    SUBB(dst,src);
}

static void PREFIX86(_cmp_wr16)(void)    /* Opcode 0x39 */
{
	 DEF_wr16(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
    SUBW(dst,src);
}

static void PREFIX86(_cmp_r8b)(void)    /* Opcode 0x3a */
{
    DEF_r8b(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=3;
#endif
    SUBB(dst,src);
}

static void PREFIX86(_cmp_r16w)(void)    /* Opcode 0x3b */
{
	 DEF_r16w(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=3;
#endif
    SUBW(dst,src);
}

static void PREFIX86(_cmp_ald8)(void)    /* Opcode 0x3c */
{
    DEF_ald8(dst,src);
	i86_ICount-=4;
    SUBB(dst,src);
}

static void PREFIX86(_cmp_axd16)(void)    /* Opcode 0x3d */
{
    DEF_axd16(dst,src);
	i86_ICount-=4;
    SUBW(dst,src);
}

static void PREFIX86(_ds)(void)    /* Opcode 0x3e */
{
	 seg_prefix=TRUE;
	prefix_base=I.base[DS];
	i86_ICount-=2;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_aas)(void)    /* Opcode 0x3f */
{
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
    {
		I.regs.b[AL] -= 6;
		I.regs.b[AH] -= 1;
		I.AuxVal = 1;
		I.CarryVal = 1;
    }
	else
	{
		I.AuxVal = 0;
		I.CarryVal = 0;
    }
	I.regs.b[AL] &= 0x0F;
#ifdef V20
	nec_ICount-=3;
#else
	i86_ICount-=8;
#endif
}

#ifdef V20
#define IncWordReg(Reg) 					\
{											\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	SetOFW_Add(tmp1,tmp,1); 				\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1; 					\
	nec_ICount-=2; \
}
#else
#define IncWordReg(Reg) 					\
{											\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	SetOFW_Add(tmp1,tmp,1); 				\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1; 					\
	i86_ICount-=3;							\
}
#endif

static void PREFIX86(_inc_ax)(void)    /* Opcode 0x40 */
{
    IncWordReg(AX);
}

static void PREFIX86(_inc_cx)(void)    /* Opcode 0x41 */
{
    IncWordReg(CX);
}

static void PREFIX86(_inc_dx)(void)    /* Opcode 0x42 */
{
	 IncWordReg(DX);
}

static void PREFIX(_inc_bx)(void)    /* Opcode 0x43 */
{
	 IncWordReg(BX);
}

static void PREFIX86(_inc_sp)(void)    /* Opcode 0x44 */
{
    IncWordReg(SP);
}

static void PREFIX86(_inc_bp)(void)    /* Opcode 0x45 */
{
	 IncWordReg(BP);
}

static void PREFIX86(_inc_si)(void)    /* Opcode 0x46 */
{
	 IncWordReg(SI);
}

static void PREFIX86(_inc_di)(void)    /* Opcode 0x47 */
{
    IncWordReg(DI);
}

#ifdef V20
#define DecWordReg(Reg) \
{ \
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; \
    SetOFW_Sub(tmp1,1,tmp); \
    SetAF(tmp1,tmp,1); \
	 SetSZPF_Word(tmp1); \
	I.regs.w[Reg]=tmp1; \
	i86_ICount-=2; \
}
#else
#define DecWordReg(Reg) \
{ \
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; \
    SetOFW_Sub(tmp1,1,tmp); \
    SetAF(tmp1,tmp,1); \
	 SetSZPF_Word(tmp1); \
	I.regs.w[Reg]=tmp1; \
	i86_ICount-=3; \
}
#endif

static void PREFIX86(_dec_ax)(void)    /* Opcode 0x48 */
{
    DecWordReg(AX);
}

static void PREFIX86(_dec_cx)(void)    /* Opcode 0x49 */
{
	 DecWordReg(CX);
}

static void PREFIX86(_dec_dx)(void)    /* Opcode 0x4a */
{
	 DecWordReg(DX);
}

static void PREFIX86(_dec_bx)(void)    /* Opcode 0x4b */
{
	 DecWordReg(BX);
}

static void PREFIX86(_dec_sp)(void)    /* Opcode 0x4c */
{
    DecWordReg(SP);
}

static void PREFIX86(_dec_bp)(void)    /* Opcode 0x4d */
{
    DecWordReg(BP);
}

static void PREFIX86(_dec_si)(void)    /* Opcode 0x4e */
{
    DecWordReg(SI);
}

static void PREFIX86(_dec_di)(void)    /* Opcode 0x4f */
{
    DecWordReg(DI);
}

static void PREFIX86(_push_ax)(void)    /* Opcode 0x50 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[AX]);
}

static void PREFIX86(_push_cx)(void)    /* Opcode 0x51 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[CX]);
}

static void PREFIX86(_push_dx)(void)    /* Opcode 0x52 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[DX]);
}

static void PREFIX86(_push_bx)(void)    /* Opcode 0x53 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[BX]);
}

static void PREFIX86(_push_sp)(void)    /* Opcode 0x54 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[SP]);
}

static void PREFIX86(_push_bp)(void)    /* Opcode 0x55 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[BP]);
}


static void PREFIX86(_push_si)(void)    /* Opcode 0x56 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[SI]);
}

static void PREFIX86(_push_di)(void)    /* Opcode 0x57 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	PUSH(I.regs.w[DI]);
}

static void PREFIX86(_pop_ax)(void)    /* Opcode 0x58 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[AX]);
}

static void PREFIX86(_pop_cx)(void)    /* Opcode 0x59 */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[CX]);
}

static void PREFIX86(_pop_dx)(void)    /* Opcode 0x5a */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[DX]);
}

static void PREFIX86(_pop_bx)(void)    /* Opcode 0x5b */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[BX]);
}

static void PREFIX86(_pop_sp)(void)    /* Opcode 0x5c */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[SP]);
}

static void PREFIX86(_pop_bp)(void)    /* Opcode 0x5d */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[BP]);
}

static void PREFIX86(_pop_si)(void)    /* Opcode 0x5e */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[SI]);
}

static void PREFIX86(_pop_di)(void)    /* Opcode 0x5f */
{
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
	POP(I.regs.w[DI]);
}

static void PREFIX86(_jo)(void)    /* Opcode 0x70 */
{
	int tmp = (int)((INT8)FETCH);
	if (OF)
	{
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jno)(void)    /* Opcode 0x71 */
{
	int tmp = (int)((INT8)FETCH);
	if (!OF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jb)(void)    /* Opcode 0x72 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnb)(void)    /* Opcode 0x73 */
{
	int tmp = (int)((INT8)FETCH);
	if (!CF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jz)(void)    /* Opcode 0x74 */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnz)(void)    /* Opcode 0x75 */
{
	int tmp = (int)((INT8)FETCH);
	if (!ZF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jbe)(void)    /* Opcode 0x76 */
{
	int tmp = (int)((INT8)FETCH);
    if (CF || ZF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnbe)(void)    /* Opcode 0x77 */
{
	int tmp = (int)((INT8)FETCH);
	 if (!(CF || ZF)) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_js)(void)    /* Opcode 0x78 */
{
	int tmp = (int)((INT8)FETCH);
    if (SF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jns)(void)    /* Opcode 0x79 */
{
	int tmp = (int)((INT8)FETCH);
    if (!SF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jp)(void)    /* Opcode 0x7a */
{
	int tmp = (int)((INT8)FETCH);
    if (PF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnp)(void)    /* Opcode 0x7b */
{
	int tmp = (int)((INT8)FETCH);
    if (!PF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jl)(void)    /* Opcode 0x7c */
{
	int tmp = (int)((INT8)FETCH);
    if ((SF!=OF)&&!ZF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnl)(void)    /* Opcode 0x7d */
{
	int tmp = (int)((INT8)FETCH);
    if (ZF||(SF==OF)) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jle)(void)    /* Opcode 0x7e */
{
	int tmp = (int)((INT8)FETCH);
    if (ZF||(SF!=OF)) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_jnle)(void)    /* Opcode 0x7f */
{
	int tmp = (int)((INT8)FETCH);
    if ((SF==OF)&&!ZF) {
		I.ip = (WORD)(I.ip+tmp);
#ifdef V20
		nec_ICount-=14;
#else
		i86_ICount-=16;
#endif
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=4;
}

static void PREFIX86(_80pre)(void)    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	 unsigned dst = GetRMByte(ModRM);
    unsigned src = FETCH;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:18;
#else
	i86_ICount-=4;
#endif
    switch (ModRM & 0x38)
	 {
    case 0x00:  /* ADD eb,d8 */
        ADDB(dst,src);
        PutbackRMByte(ModRM,dst);
	break;
    case 0x08:  /* OR eb,d8 */
        ORB(dst,src);
		  PutbackRMByte(ModRM,dst);
	break;
    case 0x10:  /* ADC eb,d8 */
        src+=CF;
        ADDB(dst,src);
		  PutbackRMByte(ModRM,dst);
	break;
    case 0x18:  /* SBB eb,b8 */
		  src+=CF;
		  SUBB(dst,src);
        PutbackRMByte(ModRM,dst);
	break;
    case 0x20:  /* AND eb,d8 */
        ANDB(dst,src);
		  PutbackRMByte(ModRM,dst);
	break;
    case 0x28:  /* SUB eb,d8 */
        SUBB(dst,src);
        PutbackRMByte(ModRM,dst);
	break;
    case 0x30:  /* XOR eb,d8 */
        XORB(dst,src);
		  PutbackRMByte(ModRM,dst);
	break;
    case 0x38:  /* CMP eb,d8 */
        SUBB(dst,src);
	break;
	 }
}


static void PREFIX86(_81pre)(void)    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	 unsigned dst = GetRMWord(ModRM);
    unsigned src = FETCH;
    src+= (FETCH << 8);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:26;
#else
	i86_ICount-=2;
#endif

	 switch (ModRM & 0x38)
    {
    case 0x00:  /* ADD ew,d16 */
		  ADDW(dst,src);
		  PutbackRMWord(ModRM,dst);
	break;
    case 0x08:  /* OR ew,d16 */
        ORW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
	 case 0x10:  /* ADC ew,d16 */
		  src+=CF;
		ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x18:  /* SBB ew,d16 */
        src+=CF;
		  SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x20:  /* AND ew,d16 */
        ANDW(dst,src);
		  PutbackRMWord(ModRM,dst);
	break;
    case 0x28:  /* SUB ew,d16 */
        SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x30:  /* XOR ew,d16 */
		  XORW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x38:  /* CMP ew,d16 */
        SUBW(dst,src);
	break;
    }
}

static void PREFIX86(_82pre)(void)	 /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:18;
#else
	i86_ICount-=2;
#endif
	 switch (ModRM & 0x38)
	 {
	case 0x00:	/* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x08:	/* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x10:	/* ADC eb,d8 */
		  src+=CF;
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x18:	/* SBB eb,d8 */
        src+=CF;
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x20:	/* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x28:	/* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x30:	/* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
	break;
	case 0x38:	/* CMP eb,d8 */
		SUBB(dst,src);
	break;
	 }
}

static void PREFIX86(_83pre)(void)    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
    unsigned dst = GetRMWord(ModRM);
    unsigned src = (WORD)((INT16)((INT8)FETCH));
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:26;
#else
	i86_ICount-=2;
#endif

	 switch (ModRM & 0x38)
    {
    case 0x00:  /* ADD ew,d16 */
        ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x08:  /* OR ew,d16 */
		  ORW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x10:  /* ADC ew,d16 */
        src+=CF;
		  ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
	 case 0x18:  /* SBB ew,d16 */
		  src+=CF;
        SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
	 case 0x20:  /* AND ew,d16 */
        ANDW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x28:  /* SUB ew,d16 */
		  SUBW(dst,src);
		  PutbackRMWord(ModRM,dst);
	break;
    case 0x30:  /* XOR ew,d16 */
		  XORW(dst,src);
        PutbackRMWord(ModRM,dst);
	break;
    case 0x38:  /* CMP ew,d16 */
        SUBW(dst,src);
	break;
    }
}

static void PREFIX86(_test_br8)(void)    /* Opcode 0x84 */
{
    DEF_br8(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:10;
#else
	i86_ICount-=3;
#endif
    ANDB(dst,src);
}

static void PREFIX86(_test_wr16)(void)    /* Opcode 0x85 */
{
    DEF_wr16(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:14;
#else
	i86_ICount-=3;
#endif
	 ANDW(dst,src);
}

static void PREFIX86(_xchg_br8)(void)    /* Opcode 0x86 */
{
    DEF_br8(dst,src);
#ifdef V20
	// V30
        if (ModRM >= 0xc0) nec_ICount-=3;
        else nec_ICount-=(EO & 1 )?24:16;
#else
	i86_ICount-=4;
#endif
    RegByte(ModRM)=dst;
    PutbackRMByte(ModRM,src);
}

static void PREFIX86(_xchg_wr16)(void)    /* Opcode 0x87 */
{
    DEF_wr16(dst,src);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?3:24;
#else
	i86_ICount-=4;
#endif
    RegWord(ModRM)=dst;
    PutbackRMWord(ModRM,src);
}

static void PREFIX86(_mov_br8)(void)    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
    BYTE src = RegByte(ModRM);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:9;
#else
	i86_ICount-=2;
#endif
    PutRMByte(ModRM,src);
}

static void PREFIX86(_mov_wr16)(void)    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
    WORD src = RegWord(ModRM);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:13;
#else
	i86_ICount-=2;
#endif
    PutRMWord(ModRM,src);
}

static void PREFIX86(_mov_r8b)(void)    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
    BYTE src = GetRMByte(ModRM);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:11;
#else
	i86_ICount-=2;
#endif
    RegByte(ModRM)=src;
}

static void PREFIX86(_mov_r16w)(void)    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	 WORD src = GetRMWord(ModRM);
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:15;
#else
	i86_ICount-=2;
#endif
	 RegWord(ModRM)=src;
}

static void PREFIX86(_mov_wsreg)(void)    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:12;
#else
	i86_ICount-=2;
#endif
	if (ModRM & 0x20) return;	/* HJB 12/13/98 1xx is invalid */
	PutRMWord(ModRM,I.sregs[(ModRM & 0x38) >> 3]);
}

static void PREFIX86(_lea)(void)    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
#ifdef V20
	nec_ICount-=4;
#else
	i86_ICount-=2;
#endif
	(void)(*GetEA[ModRM])();
	RegWord(ModRM)=EO;	/* HJB 12/13/98 effective offset (no segment part) */
}

static void PREFIX86(_mov_sregw)(void)    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
    WORD src = GetRMWord(ModRM);

#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:13;
#else
	i86_ICount-=2;
#endif
    switch (ModRM & 0x38)
    {
    case 0x00:  /* mov es,ew */
	I.sregs[ES] = src;
	I.base[ES] = SegBase(ES);
	break;
    case 0x18:  /* mov ds,ew */
	I.sregs[DS] = src;
	I.base[DS] = SegBase(DS);
	break;
    case 0x10:  /* mov ss,ew */
	I.sregs[SS] = src;
	I.base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
	PREFIX(_instruction)[FETCHOP]();
	break;
    case 0x08:  /* mov cs,ew */
	break;  /* doesn't do a jump far */
    }
}

static void PREFIX86(_popw)(void)    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
    WORD tmp;
	 POP(tmp);
#ifdef V20
	nec_ICount-=21;
#else
	i86_ICount-=4;
#endif
	 PutRMWord(ModRM,tmp);
}


#define XchgAXReg(Reg) \
{ \
    WORD tmp; \
	tmp = I.regs.w[Reg]; \
	I.regs.w[Reg] = I.regs.w[AX]; \
	I.regs.w[AX] = tmp; \
	i86_ICount-=3; \
}


static void PREFIX86(_nop)(void)    /* Opcode 0x90 */
{
    /* this is XchgAXReg(AX); */
#ifdef V20
	nec_ICount-=2;
#else
	i86_ICount-=3;
#endif
}

static void PREFIX86(_xchg_axcx)(void)    /* Opcode 0x91 */
{
	 XchgAXReg(CX);
}

static void PREFIX86(_xchg_axdx)(void)    /* Opcode 0x92 */
{
    XchgAXReg(DX);
}

static void PREFIX86(_xchg_axbx)(void)    /* Opcode 0x93 */
{
	 XchgAXReg(BX);
}

static void PREFIX86(_xchg_axsp)(void)    /* Opcode 0x94 */
{
	 XchgAXReg(SP);
}

static void PREFIX86(_xchg_axbp)(void)    /* Opcode 0x95 */
{
    XchgAXReg(BP);
}

static void PREFIX86(_xchg_axsi)(void)    /* Opcode 0x96 */
{
	 XchgAXReg(SI);
}

static void PREFIX86(_xchg_axdi)(void)    /* Opcode 0x97 */
{
	 XchgAXReg(DI);
}

static void PREFIX86(_cbw)(void)    /* Opcode 0x98 */
{
	i86_ICount-=2;
	I.regs.b[AH] = (I.regs.b[AL] & 0x80) ? 0xff : 0;
}

static void PREFIX86(_cwd)(void)    /* Opcode 0x99 */
{
	i86_ICount-=5;
	I.regs.w[DX] = (I.regs.b[AH] & 0x80) ? 0xffff : 0;
}

static void PREFIX86(_call_far)(void)
{
    unsigned tmp, tmp2;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp2 = FETCH;
	tmp2 += FETCH << 8;

	PUSH(I.sregs[CS]);
	PUSH(I.ip);

	I.ip = (WORD)tmp;
	I.sregs[CS] = (WORD)tmp2;
	I.base[CS] = SegBase(CS);
#ifdef V20
	nec_ICount-=39;
#else
	i86_ICount-=14;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_wait)(void)    /* Opcode 0x9b */
{
#ifdef V20
	nec_ICount-=7;   /* 2+5n (n = number of times POLL pin sampled) */
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_pushf)(void)    /* Opcode 0x9c */
{
#ifdef V20
	nec_ICount-=10;
    PUSH( CompressFlags() | 0xe000 );
#else
	i86_ICount-=3;
    PUSH( CompressFlags() | 0xf000 );
#endif
}

static void PREFIX86(_popf)(void)    /* Opcode 0x9d */
{
	 unsigned tmp;
    POP(tmp);
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=2;
#endif
    ExpandFlags(tmp);

	if (I.TF) PREFIX(_trap)();
}

static void PREFIX86(_sahf)(void)    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (I.regs.b[AH] & 0xd5);
#ifdef V20
	nec_ICount-=3;
#endif
    ExpandFlags(tmp);
}

static void PREFIX86(_lahf)(void)    /* Opcode 0x9f */
{
	I.regs.b[AH] = CompressFlags() & 0xff;
#ifdef V20
	nec_ICount-=2;
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_mov_aldisp)(void)    /* Opcode 0xa0 */
{
	 unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=4;
#endif
	I.regs.b[AL] = GetMemB(DS, addr);
}

static void PREFIX86(_mov_axdisp)(void)    /* Opcode 0xa1 */
{
	 unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

#ifdef V20
	nec_ICount-=14;
#else
	i86_ICount-=4;
#endif
	I.regs.b[AL] = GetMemB(DS, addr);
	I.regs.b[AH] = GetMemB(DS, addr+1);
}

static void PREFIX86(_mov_dispal)(void)    /* Opcode 0xa2 */
{
    unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

#ifdef V20
	nec_ICount-=9;
#else
	i86_ICount-=3;
#endif
	PutMemB(DS, addr, I.regs.b[AL]);
}

static void PREFIX86(_mov_dispax)(void)    /* Opcode 0xa3 */
{
	 unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;
#ifdef V20
	nec_ICount-=13;
#else
	i86_ICount-=3;
#endif
	PutMemB(DS, addr, I.regs.b[AL]);
	PutMemB(DS, addr+1, I.regs.b[AH]);
}

static void PREFIX86(_movsb)(void)    /* Opcode 0xa4 */
{
	BYTE tmp = GetMemB(DS,I.regs.w[SI]);
	PutMemB(ES,I.regs.w[DI], tmp);
	I.regs.w[DI] += -2 * I.DF + 1;
	I.regs.w[SI] += -2 * I.DF + 1;
#ifdef V20
	nec_ICount-=19; // 11+8n
#else
	i86_ICount-=5;
#endif
}

static void PREFIX86(_movsw)(void)    /* Opcode 0xa5 */
{
	WORD tmp = GetMemW(DS,I.regs.w[SI]);
	PutMemW(ES,I.regs.w[DI], tmp);
	I.regs.w[DI] += -4 * I.DF + 2;
	I.regs.w[SI] += -4 * I.DF + 2;
#ifdef V20
	nec_ICount-=19; // 11+8n
#else
	i86_ICount-=5;
#endif
}

static void PREFIX86(_cmpsb)(void)    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, I.regs.w[DI]);
	unsigned src = GetMemB(DS, I.regs.w[SI]);
    SUBB(src,dst); /* opposite of the usual convention */
	I.regs.w[DI] += -2 * I.DF + 1;
	I.regs.w[SI] += -2 * I.DF + 1;
#ifdef V20
	nec_ICount-=14;
#else
	i86_ICount-=10;
#endif
}

static void PREFIX86(_cmpsw)(void)    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, I.regs.w[DI]);
	unsigned src = GetMemW(DS, I.regs.w[SI]);
	 SUBW(src,dst); /* opposite of the usual convention */
	I.regs.w[DI] += -4 * I.DF + 2;
	I.regs.w[SI] += -4 * I.DF + 2;
#ifdef V20
	nec_ICount-=14;
#else
	i86_ICount-=10;
#endif
}

static void PREFIX86(_test_ald8)(void)    /* Opcode 0xa8 */
{
    DEF_ald8(dst,src);
	i86_ICount-=4;
    ANDB(dst,src);
}

static void PREFIX86(_test_axd16)(void)    /* Opcode 0xa9 */
{
    DEF_axd16(dst,src);
	i86_ICount-=4;
    ANDW(dst,src);
}

static void PREFIX86(_stosb)(void)    /* Opcode 0xaa */
{
	PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
	I.regs.w[DI] += -2 * I.DF + 1;
#ifdef V20
	nec_ICount-=5;
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_stosw)(void)    /* Opcode 0xab */
{
	PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
	PutMemB(ES,I.regs.w[DI]+1,I.regs.b[AH]);
	I.regs.w[DI] += -4 * I.DF + 2;
#ifdef V20
	nec_ICount-=5;
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_lodsb)(void)    /* Opcode 0xac */
{
	I.regs.b[AL] = GetMemB(DS,I.regs.w[SI]);
	I.regs.w[SI] += -2 * I.DF + 1;
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=6;
#endif
}

static void PREFIX86(_lodsw)(void)    /* Opcode 0xad */
{
	I.regs.w[AX] = GetMemW(DS,I.regs.w[SI]);
	I.regs.w[SI] +=  -4 * I.DF + 2;
#ifdef V20
	nec_ICount-=10;
#else
	i86_ICount-=6;
#endif
}

static void PREFIX86(_scasb)(void)    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, I.regs.w[DI]);
	unsigned dst = I.regs.b[AL];
    SUBB(dst,src);
	I.regs.w[DI] += -2 * I.DF + 1;
#ifdef V20
	nec_ICount-=12;
#else
	i86_ICount-=9;
#endif
}

static void PREFIX86(_scasw)(void)    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, I.regs.w[DI]);
	unsigned dst = I.regs.w[AX];
    SUBW(dst,src);
	I.regs.w[DI] += -4 * I.DF + 2;
#ifdef V20
	nec_ICount-=12;
#else
	i86_ICount-=9;
#endif
}

static void PREFIX86(_mov_ald8)(void)    /* Opcode 0xb0 */
{
	I.regs.b[AL] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_cld8)(void)    /* Opcode 0xb1 */
{
	I.regs.b[CL] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_dld8)(void)    /* Opcode 0xb2 */
{
	I.regs.b[DL] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_bld8)(void)    /* Opcode 0xb3 */
{
	I.regs.b[BL] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_ahd8)(void)    /* Opcode 0xb4 */
{
	I.regs.b[AH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_chd8)(void)    /* Opcode 0xb5 */
{
	I.regs.b[CH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_dhd8)(void)    /* Opcode 0xb6 */
{
	I.regs.b[DH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_bhd8)(void)    /* Opcode 0xb7 */
{
	I.regs.b[BH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_axd16)(void)    /* Opcode 0xb8 */
{
	I.regs.b[AL] = FETCH;
	I.regs.b[AH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_cxd16)(void)    /* Opcode 0xb9 */
{
	I.regs.b[CL] = FETCH;
	I.regs.b[CH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_dxd16)(void)    /* Opcode 0xba */
{
	I.regs.b[DL] = FETCH;
	I.regs.b[DH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_bxd16)(void)    /* Opcode 0xbb */
{
	I.regs.b[BL] = FETCH;
	I.regs.b[BH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_spd16)(void)    /* Opcode 0xbc */
{
	I.regs.b[SPL] = FETCH;
	I.regs.b[SPH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_bpd16)(void)    /* Opcode 0xbd */
{
	I.regs.b[BPL] = FETCH;
	I.regs.b[BPH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_sid16)(void)    /* Opcode 0xbe */
{
	I.regs.b[SIL] = FETCH;
	I.regs.b[SIH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_mov_did16)(void)    /* Opcode 0xbf */
{
	I.regs.b[DIL] = FETCH;
	I.regs.b[DIH] = FETCH;
	i86_ICount-=4;
}

static void PREFIX86(_ret_d16)(void)    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(I.ip);
	I.regs.w[SP]+=count;
#ifdef V20
	nec_ICount-=22; // near 20-24
#else
	i86_ICount-=14;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_ret)(void)    /* Opcode 0xc3 */
{
	POP(I.ip);
#ifdef V20
	nec_ICount-=17; // near 15-19
#else
	i86_ICount-=10;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_les_dw)(void)    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
    WORD tmp = GetRMWord(ModRM);

    RegWord(ModRM)= tmp;
	I.sregs[ES] = GetnextRMWord;
	I.base[ES] = SegBase(ES);
#ifdef V20
	nec_ICount-=22;   /* 18-26 */
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_lds_dw)(void)    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
    WORD tmp = GetRMWord(ModRM);

    RegWord(ModRM)=tmp;
	I.sregs[DS] = GetnextRMWord;
	I.base[DS] = SegBase(DS);
#ifdef V20
	nec_ICount-=22;   /* 18-26 */
#else
	i86_ICount-=4;
#endif
}

static void PREFIX86(_mov_bd8)(void)    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:11;
#else
	i86_ICount-=4;
#endif
	 PutImmRMByte(ModRM);
}

static void PREFIX86(_mov_wd16)(void)    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:15;
#else
	i86_ICount-=4;
#endif
	 PutImmRMWord(ModRM);
}

static void PREFIX86(_retf_d16)(void)    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(I.ip);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
	I.regs.w[SP]+=count;
#ifdef V20
	nec_ICount-=25; // 21-29
#else
	i86_ICount-=13;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_retf)(void)    /* Opcode 0xcb */
{
	POP(I.ip);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
#ifdef V20
	nec_ICount-=28; // 24-32
#else
	i86_ICount-=14;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_int3)(void)    /* Opcode 0xcc */
{
#ifdef V20
	nec_ICount-=38; // 38-50
        PREFIX(_interrupt)(3,0);
#else
	i86_ICount-=16;
	PREFIX(_interrupt)(3);
#endif
}

static void PREFIX86(_int)(void)    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
#ifdef V20
	nec_ICount-=38; // 38-50
        PREFIX(_interrupt)(int_num,0);
#else
	i86_ICount-=15;
	PREFIX(_interrupt)(int_num);
#endif
}

static void PREFIX86(_into)(void)    /* Opcode 0xce */
{
	 if (OF) {
#ifdef V20
	nec_ICount-=52;
        PREFIX(_interrupt)(4,0);
      } else nec_ICount-=3;   /* 3 or 52! */
#else
		i86_ICount-=17;
		PREFIX(_interrupt)(4);
	} else i86_ICount-=4;
#endif
}

static void PREFIX86(_iret)(void)    /* Opcode 0xcf */
{
#ifdef V20
	nec_ICount-=32; // 27-39
#else
	i86_ICount-=12;
#endif
	POP(I.ip);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
	 PREFIX(_popf)();
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_rotshft_b)(void)    /* Opcode 0xd0 */
{
	PREFIX(_rotate_shift_Byte)(FETCHOP,1);
}


static void PREFIX86(_rotshft_w)(void)    /* Opcode 0xd1 */
{
	PREFIX(_rotate_shift_Word)(FETCHOP,1);
}


static void PREFIX86(_rotshft_bcl)(void)    /* Opcode 0xd2 */
{
	PREFIX(_rotate_shift_Byte)(FETCHOP,I.regs.b[CL]);
}

static void PREFIX86(_rotshft_wcl)(void)    /* Opcode 0xd3 */
{
	PREFIX(_rotate_shift_Word)(FETCHOP,I.regs.b[CL]);
}

/* OB: Opcode works on NEC V-Series but not the Variants              */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.              */
static void PREFIX86(_aam)(void)    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;

#ifndef V20
	i86_ICount-=83;
	 if (mult == 0)
		PREFIX(_interrupt)(0);
	 else
	 {
		I.regs.b[AH] = I.regs.b[AL] / mult;
		I.regs.b[AL] %= mult;

		SetSZPF_Word(I.regs.w[AX]);
	 }
#else

	if (mult == 0)
		PREFIX(_interrupt)(0,0);
    else
    {
		I.regs.b[AH] = I.regs.b[AL] / 10;
		I.regs.b[AL] %= 10;
		SetSZPF_Word(I.regs.w[AX]);
		nec_ICount-=15;
    }
#endif
}

static void PREFIX86(_aad)(void)    /* Opcode 0xd5 */
{
#ifndef V20
	unsigned mult = FETCH;

	i86_ICount-=60;
	I.regs.b[AL] = I.regs.b[AH] * mult + I.regs.b[AL];
	I.regs.b[AH] = 0;

	SetZF(I.regs.b[AL]);
	SetPF(I.regs.b[AL]);
	I.SignVal = 0;
#else
/* OB: Opcode works on NEC V-Series but not the Variants 	*/
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.					*/
	unsigned mult=FETCH;				/* eat operand = ignore ! */

	I.regs.b[AL] = I.regs.b[AH] * 10 + I.regs.b[AL];
	I.regs.b[AH] = 0;

	SetZF(I.regs.b[AL]);
	SetPF(I.regs.b[AL]);
	I.SignVal = 0;
	nec_ICount-=7;
	mult=0;
#endif
}


static void PREFIX86(_xlat)(void)    /* Opcode 0xd7 */
{
	unsigned dest = I.regs.w[BX]+I.regs.b[AL];

#ifdef V20
	nec_ICount-=9;  // V30
#else
	i86_ICount-=5;
#endif
	I.regs.b[AL] = GetMemB(DS, dest);
}

static void PREFIX86(_escape)(void)    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	i86_ICount-=2;
    GetRMByte(ModRM);
}

static void PREFIX86(_loopne)(void)    /* Opcode 0xe0 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

    if (!ZF && tmp) {
#ifdef V20
	nec_ICount-=14;
#else
	i86_ICount-=19;
#endif
	I.ip = (WORD)(I.ip+disp);
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=5;
}

static void PREFIX86(_loope)(void)    /* Opcode 0xe1 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

	 if (ZF && tmp) {
#ifdef V20
	nec_ICount-=14;
#else
	i86_ICount-=18;
#endif
	I.ip = (WORD)(I.ip+disp);
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else
#ifdef V20
		nec_ICount-=5;
#else
		i86_ICount-=6;
#endif
}

static void PREFIX86(_loop)(void)    /* Opcode 0xe2 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

    if (tmp) {
#ifdef V20
	nec_ICount-=13;
#else
	i86_ICount-=17;
#endif
	I.ip = (WORD)(I.ip+disp);
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else i86_ICount-=5;
}

static void PREFIX86(_jcxz)(void)    /* Opcode 0xe3 */
{
	int disp = (int)((INT8)FETCH);

	if (I.regs.w[CX] == 0) {
#ifdef V20
	nec_ICount-=13;
#else
	i86_ICount-=18;
#endif
	I.ip = (WORD)(I.ip+disp);
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
	} else
#ifdef V20
		nec_ICount-=5;
#else
		i86_ICount-=6;
#endif
}

static void PREFIX86(_inal)(void)    /* Opcode 0xe4 */
{
	unsigned port = FETCH;

#ifdef V20
	nec_ICount-=9;
#else
	i86_ICount-=10;
#endif
	I.regs.b[AL] = read_port(port);
}

static void PREFIX86(_inax)(void)    /* Opcode 0xe5 */
{
	unsigned port = FETCH;

#ifdef V20
	nec_ICount-=13;
#else
	i86_ICount-=14;
#endif
	I.regs.b[AL] = read_port(port);
	I.regs.b[AH] = read_port(port+1);
}

static void PREFIX86(_outal)(void)    /* Opcode 0xe6 */
{
	unsigned port = FETCH;

#ifdef V20
	nec_ICount-=8;
#else
	i86_ICount-=10;
#endif
	write_port(port, I.regs.b[AL]);
}

static void PREFIX86(_outax)(void)    /* Opcode 0xe7 */
{
	unsigned port = FETCH;

#ifdef V20
	nec_ICount-=12;
#else
	i86_ICount-=14;
#endif
	write_port(port, I.regs.b[AL]);
	write_port(port+1, I.regs.b[AH]);
}

static void PREFIX86(_call_d16)(void)    /* Opcode 0xe8 */
{
	unsigned tmp = FETCH;
	tmp += FETCH << 8;

	PUSH(I.ip);
	I.ip = (WORD)(I.ip+(INT16)tmp);
#ifdef V20
	nec_ICount-=24; // 21-29
#else
	i86_ICount-=12;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_jmp_d16)(void)    /* Opcode 0xe9 */
{
	int tmp = FETCH;
	tmp += FETCH << 8;

	I.ip = (WORD)(I.ip+(INT16)tmp);
	i86_ICount-=15;
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_jmp_far)(void)    /* Opcode 0xea */
{
	 unsigned tmp,tmp1;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp1 = FETCH;
	tmp1 += FETCH << 8;

	I.sregs[CS] = (WORD)tmp1;
	I.base[CS] = SegBase(CS);
	I.ip = (WORD)tmp;
#ifdef V20
	nec_ICount-=27; // 27-35
#else
	i86_ICount-=15;
#endif
	CHANGE_PC((I.base[CS]+I.ip) & I.amask);
}

static void PREFIX86(_jmp_d8)(void)    /* Opcode 0xeb */
{
	int tmp = (int)((INT8)FETCH);
	I.ip = (WORD)(I.ip+tmp);
#ifdef V20
	nec_ICount-=12;
#else
	i86_ICount-=15;
#endif
}

static void PREFIX86(_inaldx)(void)    /* Opcode 0xec */
{
	i86_ICount-=8;
	I.regs.b[AL] = read_port(I.regs.w[DX]);
}

static void PREFIX86(_inaxdx)(void)    /* Opcode 0xed */
{
	unsigned port = I.regs.w[DX];

	i86_ICount-=12;
	I.regs.b[AL] = read_port(port);
	I.regs.b[AH] = read_port(port+1);
}

static void PREFIX86(_outdxal)(void)    /* Opcode 0xee */
{
	i86_ICount-=8;
	write_port(I.regs.w[DX], I.regs.b[AL]);
}

static void PREFIX86(_outdxax)(void)    /* Opcode 0xef */
{
	unsigned port = I.regs.w[DX];

	i86_ICount-=12;
	write_port(port, I.regs.b[AL]);
	write_port(port+1, I.regs.b[AH]);
}

/* I think thats not a V20 instruction...*/
static void PREFIX86(_lock)(void)    /* Opcode 0xf0 */
{
	i86_ICount-=2;
	PREFIX(_instruction)[FETCHOP]();  /* un-interruptible */
}
#endif

static void PREFIX(_repne)(void)    /* Opcode 0xf2 */
{
	 PREFIX(rep)(0);
}

static void PREFIX(_repe)(void)    /* Opcode 0xf3 */
{
	 PREFIX(rep)(1);
}

#ifndef I186
static void PREFIX86(_hlt)(void)    /* Opcode 0xf4 */
{
	I.ip--;
	i86_ICount=0;
}

static void PREFIX86(_cmc)(void)    /* Opcode 0xf5 */
{
	i86_ICount-=2;
	I.CarryVal = !CF;
}

static void PREFIX86(_f6pre)(void)
{
	/* Opcode 0xf6 */
	unsigned ModRM = FETCH;
    unsigned tmp = (unsigned)GetRMByte(ModRM);
    unsigned tmp2;


    switch (ModRM & 0x38)
    {
    case 0x00:  /* TEST Eb, data8 */
    case 0x08:  /* ??? */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:11;
#else
		i86_ICount-=5;
#endif
		tmp &= FETCH;

		I.CarryVal = I.OverVal = I.AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;

    case 0x10:  /* NOT Eb */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:16;
#else
		i86_ICount-=3;
#endif
		PutbackRMByte(ModRM,~tmp);
		break;

	 case 0x18:  /* NEG Eb */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:16;
#else
		i86_ICount-=3;
#endif
        tmp2=0;
        SUBB(tmp2,tmp);
        PutbackRMByte(ModRM,tmp2);
		break;
    case 0x20:  /* MUL AL, Eb */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?30:36;
#else
		i86_ICount-=77;
#endif
		{
			UINT16 result;
			tmp2 = I.regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (UINT16)tmp2*tmp;
			I.regs.w[AX]=(WORD)result;

			SetZF(I.regs.w[AX]);
			I.CarryVal = I.OverVal = (I.regs.b[AH] != 0);
		}
		break;
	 case 0x28:  /* IMUL AL, Eb */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?30:39;
#else
		i86_ICount-=80;
#endif
		{
			INT16 result;

			tmp2 = (unsigned)I.regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (INT16)((INT8)tmp2)*(INT16)((INT8)tmp);
			I.regs.w[AX]=(WORD)result;

			SetZF(I.regs.w[AX]);

			I.CarryVal = I.OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
    case 0x30:  /* DIV AL, Ew */
#ifndef V20
		i86_ICount-=90;
#endif
		{
			UINT16 result;

			result = I.regs.w[AX];

			if (tmp)
			{
				if ((result / tmp) > 0xff)
				{
#ifdef V20
	PREFIX(_interrupt)(0,0);
#else
					PREFIX(_interrupt)(0);
#endif
					break;
				}
				else
				{
					I.regs.b[AH] = result % tmp;
					I.regs.b[AL] = result / tmp;
				}
			}
			else
			{
#ifdef V20
	PREFIX(_interrupt)(0,0);
#else
				PREFIX(_interrupt)(0);
#endif
				break;
			}
		}
#ifdef V20
		nec_ICount-=(ModRM >=0xc0 )?25:35;
#endif
		break;
    case 0x38:  /* IDIV AL, Ew */
#ifndef V20
		i86_ICount-=106;
#endif
		{

			INT16 result;

			result = I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT16)((INT8)tmp);

				if ((result /= (INT16)((INT8)tmp)) > 0xff)
				{
#ifdef V20
	PREFIX(_interrupt)(0,0);
#else
					PREFIX(_interrupt)(0);
#endif
					break;
				}
				else
				{
					I.regs.b[AL] = result;
					I.regs.b[AH] = tmp2;
				}
			}
			else
			{
#ifdef V20
	PREFIX(_interrupt)(0,0);
#else
				PREFIX(_interrupt)(0);
#endif
				break;
			}
		}
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?43:53;
#endif
		break;
    }
}


static void PREFIX86(_f7pre)(void)
{
	/* Opcode 0xf7 */
	unsigned ModRM = FETCH;
	 unsigned tmp = GetRMWord(ModRM);
    unsigned tmp2;


    switch (ModRM & 0x38)
    {
    case 0x00:  /* TEST Ew, data16 */
    case 0x08:  /* ??? */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?4:15;
#else
		i86_ICount-=3;
#endif
		tmp2 = FETCH;
		tmp2 += FETCH << 8;

		tmp &= tmp2;

		I.CarryVal = I.OverVal = I.AuxVal = 0;
		SetSZPF_Word(tmp);
		break;

    case 0x10:  /* NOT Ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
		i86_ICount-=3;
#endif
		tmp = ~tmp;
		PutbackRMWord(ModRM,tmp);
		break;

    case 0x18:  /* NEG Ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
		i86_ICount-=3;
#endif
        tmp2 = 0;
        SUBW(tmp2,tmp);
        PutbackRMWord(ModRM,tmp2);
		break;
    case 0x20:  /* MUL AX, Ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?30:36;
#else
		i86_ICount-=129;
#endif
		{
			UINT32 result;
			tmp2 = I.regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (UINT32)tmp2*tmp;
			I.regs.w[AX]=(WORD)result;
            result >>= 16;
			I.regs.w[DX]=result;

			SetZF(I.regs.w[AX] | I.regs.w[DX]);
			I.CarryVal = I.OverVal = (I.regs.w[DX] != 0);
		}
		break;

    case 0x28:  /* IMUL AX, Ew */
		i86_ICount-=150;
		{
			INT32 result;

			tmp2 = I.regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (INT32)((INT16)tmp2)*(INT32)((INT16)tmp);
			I.CarryVal = I.OverVal = (result >> 15 != 0) && (result >> 15 != -1);

			I.regs.w[AX]=(WORD)result;
			result = (WORD)(result >> 16);
			I.regs.w[DX]=result;

			SetZF(I.regs.w[AX] | I.regs.w[DX]);
		}
		break;
	 case 0x30:  /* DIV AX, Ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?25:35;
#else
		i86_ICount-=158;
#endif
		{
			UINT32 result;

			result = (I.regs.w[DX] << 16) + I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % tmp;
				if ((result / tmp) > 0xffff)
				{
#ifdef V20
					PREFIX(_interrupt)(0,0);
#else
					PREFIX(_interrupt)(0);
#endif
					break;
				}
				else
				{
					I.regs.w[DX]=tmp2;
					result /= tmp;
					I.regs.w[AX]=result;
				}
			}
			else
			{
#ifdef V20
				PREFIX(_interrupt)(0,0);
#else
				PREFIX(_interrupt)(0);
#endif
				break;
			}
		}
		break;
    case 0x38:  /* IDIV AX, Ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?43:53;
#else
		i86_ICount-=180;
#endif
		{
			INT32 result;

			result = (I.regs.w[DX] << 16) + I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT32)((INT16)tmp);
				if ((result /= (INT32)((INT16)tmp)) > 0xffff)
				{
#ifdef V20
					PREFIX(_interrupt)(0,0);
#else
					PREFIX(_interrupt)(0);
#endif
					break;
				}
				else
				{
					I.regs.w[AX]=result;
					I.regs.w[DX]=tmp2;
				}
			}
			else
			{
#ifdef V20
				PREFIX(_interrupt)(0,0);
#else
				PREFIX(_interrupt)(0);
#endif
				break;
			}
		}
		break;
    }
}


static void PREFIX86(_clc)(void)    /* Opcode 0xf8 */
{
	i86_ICount-=2;
	I.CarryVal = 0;
}

static void PREFIX86(_stc)(void)    /* Opcode 0xf9 */
{
	i86_ICount-=2;
	I.CarryVal = 1;
}

static void PREFIX86(_cli)(void)    /* Opcode 0xfa */
{
	i86_ICount-=2;
	SetIF(0);
}

static void PREFIX86(_sti)(void)    /* Opcode 0xfb */
{
	i86_ICount-=2;
	SetIF(1);
}

static void PREFIX86(_cld)(void)    /* Opcode 0xfc */
{
	i86_ICount-=2;
	SetDF(0);
}

static void PREFIX86(_std)(void)    /* Opcode 0xfd */
{
	i86_ICount-=2;
	SetDF(1);
}

static void PREFIX86(_fepre)(void)    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	 unsigned tmp = GetRMByte(ModRM);
    unsigned tmp1;
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:16;
#else
	i86_ICount-=3; /* 2 if dest is in memory */
#endif
    if ((ModRM & 0x38) == 0)  /* INC eb */
	 {
		tmp1 = tmp+1;
		SetOFB_Add(tmp1,tmp,1);
    }
	 else  /* DEC eb */
    {
		tmp1 = tmp-1;
		SetOFB_Sub(tmp1,1,tmp);
    }

    SetAF(tmp1,tmp,1);
    SetSZPF_Byte(tmp1);

    PutbackRMByte(ModRM,(BYTE)tmp1);
}


static void PREFIX86(_ffpre)(void)    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
    unsigned tmp;
    unsigned tmp1;

    switch(ModRM & 0x38)
    {
    case 0x00:  /* INC ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?2:24;
#else
		i86_ICount-=3; /* 2 if dest is in memory */
#endif
		tmp = GetRMWord(ModRM);
		tmp1 = tmp+1;

		SetOFW_Add(tmp1,tmp,1);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

    case 0x08:  /* DEC ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?16:24;
#else
		i86_ICount-=3; /* 2 if dest is in memory */
#endif
		tmp = GetRMWord(ModRM);
		tmp1 = tmp-1;

		SetOFW_Sub(tmp1,1,tmp);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

    case 0x10:  /* CALL ew */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?16:20;
#else
		i86_ICount-=9; /* 8 if dest is in memory */
#endif
		tmp = GetRMWord(ModRM);
		PUSH(I.ip);
		I.ip = (WORD)tmp;
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
		break;

	case 0x18:  /* CALL FAR ea */
#ifdef V20
	nec_ICount-=(ModRM >=0xc0 )?16:26;
#else
		i86_ICount-=11;
#endif
		tmp = I.sregs[CS];	/* HJB 12/13/98 need to skip displacements of EA */
		tmp1 = GetRMWord(ModRM);
		I.sregs[CS] = GetnextRMWord;
		I.base[CS] = SegBase(CS);
		PUSH(tmp);
		PUSH(I.ip);
		I.ip = tmp1;
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
		break;

    case 0x20:  /* JMP ea */
#ifdef V20
		nec_ICount-=13;
#else
		i86_ICount-=11; /* 8 if address in memory */
#endif
		I.ip = GetRMWord(ModRM);
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
		break;

    case 0x28:  /* JMP FAR ea */
#ifdef V20
		nec_ICount-=15;
#else
		i86_ICount-=4;
#endif
		I.ip = GetRMWord(ModRM);
		I.sregs[CS] = GetnextRMWord;
		I.base[CS] = SegBase(CS);
		CHANGE_PC((I.base[CS]+I.ip) & I.amask);
		break;

    case 0x30:  /* PUSH ea */
#ifdef V20
		nec_ICount-=4;
#else
		i86_ICount-=3;
#endif
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	 }
}


static void PREFIX86(_invalid)(void)
{
	int addr=((I.sregs[CS]<<4)+I.ip)&I.amask;
	 /* makes the cpu loops forever until user resets it */
	/*{ extern int debug_key_pressed; debug_key_pressed = 1; } */
	logerror("illegal instruction %.2x at %.5x\n",PEEKBYTE(addr), addr);
	I.ip--;
	i86_ICount-=10;
}
#endif