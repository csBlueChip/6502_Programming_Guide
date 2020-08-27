// gcc  6502-auto.c  -o table  -g

//----------------------------------------------------------------------------- ----------------------------------------
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------- ----------------------------------------
#define PEEK8(p)     ( *((uint8_t*)(p)) )
#define PEEK16LE(p)  ( PEEK8(p) | (PEEK8( ((uint8_t*)(p)) +1) <<8) )

//----------------------------------------------------------------------------- ----------------------------------------
// Opcodes
//
typedef
	enum opcode {
		// https://www.masswerk.at/6502/6502_instruction_set.html
		// In case of errors:  The *COMMENTS* should be trusted above the enums
		//   *  : (Tcnt += 1) if page boundary is crossed
		//   ** : (Tcnt += 1) if branch occurs on same page
		//      : (Tcnt += 2) if branch occurs to different page
		OP_ADC_ABS  = 0x6D,    // absolute      ADC oper      6D    3     4   
		OP_ADC_ABSX = 0x7D,    // absolute,X    ADC oper,X    7D    3     4*   
		OP_ADC_ABSY = 0x79,    // absolute,Y    ADC oper,Y    79    3     4*   
		OP_ADC_IMM  = 0x69,    // immediate     ADC #oper     69    2     2
		OP_ADC_INDX = 0x61,    // (indirect,X)  ADC (oper,X)  61    2     6   
		OP_ADC_INDY = 0x71,    // (indirect),Y  ADC (oper),Y  71    2     5*   
		OP_ADC_ZP   = 0x65,    // zeropage      ADC oper      65    2     3   
		OP_ADC_ZPX  = 0x75,    // zeropage,X    ADC oper,X    75    2     4   
		OP_AND_ABS  = 0x2D,    // absolute      AND oper      2D    3     4   
		OP_AND_ABSX = 0x3D,    // absolute,X    AND oper,X    3D    3     4*   
		OP_AND_ABSY = 0x39,    // absolute,Y    AND oper,Y    39    3     4*   
		OP_AND_IMM  = 0x29,    // immediate     AND #oper     29    2     2   
		OP_AND_INDX = 0x21,    // (indirect,X)  AND (oper,X)  21    2     6   
		OP_AND_INDY = 0x31,    // (indirect),Y  AND (oper),Y  31    2     5*   
		OP_AND_ZP   = 0x25,    // zeropage      AND oper      25    2     3   
		OP_AND_ZPX  = 0x35,    // zeropage,X    AND oper,X    35    2     4   
		OP_ASL_ABS  = 0x0E,    // absolute      ASL oper      0E    3     6   
		OP_ASL_ABSX = 0x1E,    // absolute,X    ASL oper,X    1E    3     7   
		OP_ASL_ACC  = 0x0A,    // accumulator   ASL A         0A    1     2   
		OP_ASL_ZP   = 0x06,    // zeropage      ASL oper      06    2     5   
		OP_ASL_ZPX  = 0x16,    // zeropage,X    ASL oper,X    16    2     6   
		OP_BCC_REL  = 0x90,    // relative      BCC oper      90    2     2**   
		OP_BCS_REL  = 0xB0,    // relative      BCS oper      B0    2     2**   
		OP_BEQ_REL  = 0xF0,    // relative      BEQ oper      F0    2     2**   
		OP_BIT_ABS  = 0x2C,    // absolute      BIT oper      2C    3     4   
		OP_BIT_ZP   = 0x24,    // zeropage      BIT oper      24    2     3   
		OP_BMI_REL  = 0x30,    // relative      BMI oper      30    2     2**   
		OP_BNE_REL  = 0xD0,    // relative      BNE oper      D0    2     2**   
		OP_BPL_REL  = 0x10,    // relative      BPL oper      10    2     2**   
		OP_BRK_IMP  = 0x00,    // implied       BRK           00    1     7   
		OP_BVC_REL  = 0x50,    // relative      BVC oper      50    2     2**   
		OP_BVS_REL  = 0x70,    // relative      BVC oper      70    2     2**   
		OP_CLC_IMP  = 0x18,    // implied       CLC           18    1     2   
		OP_CLD_IMP  = 0xD8,    // implied       CLD           D8    1     2   
		OP_CLI_IMP  = 0x58,    // implied       CLI           58    1     2   
		OP_CLV_IMP  = 0xB8,    // implied       CLV           B8    1     2   
		OP_CMP_ABS  = 0xCD,    // absolute      CMP oper      CD    3     4   
		OP_CMP_ABSX = 0xDD,    // absolute,X    CMP oper,X    DD    3     4*   
		OP_CMP_ABSY = 0xD9,    // absolute,Y    CMP oper,Y    D9    3     4*   
		OP_CMP_IMM  = 0xC9,    // immediate     CMP #oper     C9    2     2   
		OP_CMP_INDX = 0xC1,    // (indirect,X)  CMP (oper,X)  C1    2     6   
		OP_CMP_INDY = 0xD1,    // (indirect),Y  CMP (oper),Y  D1    2     5*   
		OP_CMP_ZP   = 0xC5,    // zeropage      CMP oper      C5    2     3   
		OP_CMP_ZPX  = 0xD5,    // zeropage,X    CMP oper,X    D5    2     4   
		OP_CPX_ABS  = 0xEC,    // absolute      CPX oper      EC    3     4   
		OP_CPX_IMM  = 0xE0,    // immediate     CPX #oper     E0    2     2   
		OP_CPX_ZP   = 0xE4,    // zeropage      CPX oper      E4    2     3   
		OP_CPY_ABS  = 0xCC,    // absolute      CPY oper      CC    3     4   
		OP_CPY_IMM  = 0xC0,    // immediate     CPY #oper     C0    2     2   
		OP_CPY_ZP   = 0xC4,    // zeropage      CPY oper      C4    2     3   
		OP_DEC_ABS  = 0xCE,    // absolute      DEC oper      CE    3     6   
		OP_DEC_ABSX = 0xDE,    // absolute,X    DEC oper,X    DE    3     7   
		OP_DEC_ZP   = 0xC6,    // zeropage      DEC oper      C6    2     5   
		OP_DEC_ZPX  = 0xD6,    // zeropage,X    DEC oper,X    D6    2     6   
		OP_DEX_IMP  = 0xCA,    // implied       DEC           CA    1     2   
		OP_DEY_IMP  = 0x88,    // implied       DEC           88    1     2   
		OP_EOR_ABS  = 0x4D,    // absolute      EOR oper      4D    3     4   
		OP_EOR_ABSX = 0x5D,    // absolute,X    EOR oper,X    5D    3     4*   
		OP_EOR_ABSY = 0x59,    // absolute,Y    EOR oper,Y    59    3     4*   
		OP_EOR_IMM  = 0x49,    // immediate     EOR #oper     49    2     2   
		OP_EOR_INDX = 0x41,    // (indirect,X)  EOR (oper,X)  41    2     6   
		OP_EOR_INDY = 0x51,    // (indirect),Y  EOR (oper),Y  51    2     5*   
		OP_EOR_ZP   = 0x45,    // zeropage      EOR oper      45    2     3   
		OP_EOR_ZPX  = 0x55,    // zeropage,X    EOR oper,X    55    2     4   
		OP_INC_ABS  = 0xEE,    // absolute      INC oper      EE    3     6   
		OP_INC_ABSX = 0xFE,    // absolute,X    INC oper,X    FE    3     7   
		OP_INC_ZP   = 0xE6,    // zeropage      INC oper      E6    2     5   
		OP_INC_ZPX  = 0xF6,    // zeropage,X    INC oper,X    F6    2     6   
		OP_INX_IMP  = 0xE8,    // implied       INX           E8    1     2   
		OP_INY_IMP  = 0xC8,    // implied       INY           C8    1     2   
		OP_JMP_ABS  = 0x4C,    // absolute      JMP oper      4C    3     3   
		OP_JMP_IND  = 0x6C,    // indirect      JMP (oper)    6C    3     5   
		OP_JSR_ABS  = 0x20,    // absolute      JSR oper      20    3     6   
		OP_LDA_ABS  = 0xAD,    // absolute      LDA oper      AD    3     4   
		OP_LDA_ABSX = 0xBD,    // absolute,X    LDA oper,X    BD    3     4*   
		OP_LDA_ABSY = 0xB9,    // absolute,Y    LDA oper,Y    B9    3     4*   
		OP_LDA_IMM  = 0xA9,    // immediate     LDA #oper     A9    2     2   
		OP_LDA_INDX = 0xA1,    // (indirect,X)  LDA (oper,X)  A1    2     6   
		OP_LDA_INDY = 0xB1,    // (indirect),Y  LDA (oper),Y  B1    2     5*   
		OP_LDA_ZP   = 0xA5,    // zeropage      LDA oper      A5    2     3   
		OP_LDA_ZPX  = 0xB5,    // zeropage,X    LDA oper,X    B5    2     4   
		OP_LDX_ABS  = 0xAE,    // absolute      LDX oper      AE    3     4   
		OP_LDX_ABSY = 0xBE,    // absolute,Y    LDX oper,Y    BE    3     4*   
		OP_LDX_IMM  = 0xA2,    // immediate     LDX #oper     A2    2     2   
		OP_LDX_ZP   = 0xA6,    // zeropage      LDX oper      A6    2     3   
		OP_LDX_ZPY  = 0xB6,    // zeropage,Y    LDX oper,Y    B6    2     4   
		OP_LDY_ABS  = 0xAC,    // absolute      LDY oper      AC    3     4   
		OP_LDY_ABSX = 0xBC,    // absolute,X    LDY oper,X    BC    3     4*   
		OP_LDY_IMM  = 0xA0,    // immediate     LDY #oper     A0    2     2   
		OP_LDY_ZP   = 0xA4,    // zeropage      LDY oper      A4    2     3   
		OP_LDY_ZPX  = 0xB4,    // zeropage,X    LDY oper,X    B4    2     4   
		OP_LSR_ABS  = 0x4E,    // absolute      LSR oper      4E    3     6   
		OP_LSR_ABSX = 0x5E,    // absolute,X    LSR oper,X    5E    3     7   
		OP_LSR_IMM  = 0x4A,    // accumulator   LSR A         4A    1     2   
		OP_LSR_ZP   = 0x46,    // zeropage      LSR oper      46    2     5   
		OP_LSR_ZPX  = 0x56,    // zeropage,X    LSR oper,X    56    2     6   
		OP_NOP_IMP  = 0xEA,    // implied       NOP           EA    1     2   
		OP_ORA_ABS  = 0x0D,    // absolute      ORA oper      0D    3     4   
		OP_ORA_ABSX = 0x1D,    // absolute,X    ORA oper,X    1D    3     4*   
		OP_ORA_ABSY = 0x19,    // absolute,Y    ORA oper,Y    19    3     4*   
		OP_ORA_IMM  = 0x09,    // immediate     ORA #oper     09    2     2   
		OP_ORA_INDX = 0x01,    // (indirect,X)  ORA (oper,X)  01    2     6   
		OP_ORA_INDY = 0x11,    // (indirect),Y  ORA (oper),Y  11    2     5*   
		OP_ORA_ZP   = 0x05,    // zeropage      ORA oper      05    2     3   
		OP_ORA_ZPX  = 0x15,    // zeropage,X    ORA oper,X    15    2     4   
		OP_PHA_IMP  = 0x48,    // implied       PHA           48    1     3   
		OP_PHP_IMP  = 0x08,    // implied       PHP           08    1     3   
		OP_PLA_IMP  = 0x68,    // implied       PLA           68    1     4   
		OP_PLP_IMP  = 0x28,    // implied       PLP           28    1     4   
		OP_ROL_ABS  = 0x2E,    // absolute      ROL oper      2E    3     6   
		OP_ROL_ABSX = 0x3E,    // absolute,X    ROL oper,X    3E    3     7   
		OP_ROL_ACC  = 0x2A,    // accumulator   ROL A         2A    1     2   
		OP_ROL_ZP   = 0x26,    // zeropage      ROL oper      26    2     5   
		OP_ROL_ZPX  = 0x36,    // zeropage,X    ROL oper,X    36    2     6   
		OP_ROR_ABS  = 0x6E,    // absolute      ROR oper      6E    3     6   
		OP_ROR_ABSX = 0x7E,    // absolute,X    ROR oper,X    7E    3     7   
		OP_ROR_ACC  = 0x6A,    // accumulator   ROR A         6A    1     2   
		OP_ROR_ZP   = 0x66,    // zeropage      ROR oper      66    2     5   
		OP_ROR_ZPX  = 0x76,    // zeropage,X    ROR oper,X    76    2     6   
		OP_RTI_IMP  = 0x40,    // implied       RTI           40    1     6   
		OP_RTS_IMP  = 0x60,    // implied       RTS           60    1     6   
		OP_SBC_ABS  = 0xED,    // absolute      SBC oper      ED    3     4   
		OP_SBC_ABSX = 0xFD,    // absolute,X    SBC oper,X    FD    3     4*   
		OP_SBC_ABSY = 0xF9,    // absolute,Y    SBC oper,Y    F9    3     4*   
		OP_SBC_IMM  = 0xE9,    // immediate     SBC #oper     E9    2     2   
		OP_SBC_INDX = 0xE1,    // (indirect,X)  SBC (oper,X)  E1    2     6   
		OP_SBC_INDY = 0xF1,    // (indirect),Y  SBC (oper),Y  F1    2     5*   
		OP_SBC_ZP   = 0xE5,    // zeropage      SBC oper      E5    2     3   
		OP_SBC_ZPX  = 0xF5,    // zeropage,X    SBC oper,X    F5    2     4   
		OP_SEC_IMP  = 0x38,    // implied       SEC           38    1     2   
		OP_SED_IMP  = 0xF8,    // implied       SED           F8    1     2   
		OP_SEI_IMP  = 0x78,    // implied       SEI           78    1     2   
		OP_STA_ABS  = 0x8D,    // absolute      STA oper      8D    3     4   
		OP_STA_ABSX = 0x9D,    // absolute,X    STA oper,X    9D    3     5   
		OP_STA_ABSY = 0x99,    // absolute,Y    STA oper,Y    99    3     5   
		OP_STA_INDX = 0x81,    // (indirect,X)  STA (oper,X)  81    2     6   
		OP_STA_INDY = 0x91,    // (indirect),Y  STA (oper),Y  91    2     6   
		OP_STA_ZP   = 0x85,    // zeropage      STA oper      85    2     3   
		OP_STA_ZPX  = 0x95,    // zeropage,X    STA oper,X    95    2     4   
		OP_STX_ABS  = 0x8E,    // absolute      STX oper      8E    3     4   
		OP_STX_ZP   = 0x86,    // zeropage      STX oper      86    2     3   
		OP_STX_ZPY  = 0x96,    // zeropage,Y    STX oper,Y    96    2     4   
		OP_STY_ABS  = 0x8C,    // absolute      STY oper      8C    3     4   
		OP_STY_ZP   = 0x84,    // zeropage      STY oper      84    2     3   
		OP_STY_ZPX  = 0x94,    // zeropage,X    STY oper,X    94    2     4   
		OP_TAX_IMP  = 0xAA,    // implied       TAX           AA    1     2   
		OP_TAY_IMP  = 0xA8,    // implied       TAY           A8    1     2   
		OP_TSX_IMP  = 0xBA,    // implied       TSX           BA    1     2   
		OP_TXA_IMP  = 0x8A,    // implied       TXA           8A    1     2   
		OP_TXS_IMP  = 0x9A,    // implied       TXS           9A    1     2   
		OP_TYA_IMP  = 0x98,    // implied       TYA           98    1     2   		
	}
opcode_t;

//----------------------------------------------------------------------------- ----------------------------------------
// Addressing Modes
//
typedef	
	enum amode {
		// In order of "increasing opcode value"
		AM_IMP = 0,
		AM_REL,
		AM_ACC,
		AM_IMM,
		AM_ZP,
		AM_ZPX,
		AM_ZPY,
		AM_ABS,
		AM_ABSX,
		AM_ABSY,
		AM_IND,
		AM_INDX,
		AM_INDY,
		
		AM_MAX  // Must be last
	}
amode_t;	

char  amode[AM_MAX][16] = {
	[AM_IMP ] = "Implied", 
	[AM_REL ] = "Relative", 
	[AM_ACC ] = "Accum", 
	[AM_IMM ] = "Immediate", 
	[AM_ZP  ] = "ZeroPage", 
	[AM_ZPX ] = "ZeroPage,X", 
	[AM_ZPY ] = "ZeroPage,Y", 
	[AM_ABS ] = "Abs", 
	[AM_ABSX] = "Abs,X", 
	[AM_ABSY] = "Abs.Y", 
	[AM_IND ] = "(Ind)", 
	[AM_INDX] = "(Ind,X)", 
	[AM_INDY] = "(Ind),Y"
};

//----------------------------------------------------------------------------- ----------------------------------------
// (Processor) Status Word (aka Flags) register
//
typedef
	enum flags {
		// Leftmost (2^7) flag first
		FLG_N    = 0x80,  // Negative    (r & 0x80)
		FLG_V    = 0x40,  // oVerflow    http://6502.org/tutorials/vflag.html
		FLG_X    = 0x20,  // -unused-
		FLG_B    = 0x10,  // BReaK
		FLG_D    = 0x08,  // Decimal     (BCD)
		FLG_I    = 0x04,  // /Interrupt  (Interrupt DISable)
		FLG_Z    = 0x02,  // Zero        (r = 0x00)
		FLG_C    = 0x01,  // Carry       (r > 0xff)
		
		FLG_ALL  = 0xFF,
		FLG_NONE = 0,
	}
flags_t;

//----------------------------------------------------------------------------- ----------------------------------------
// A disassembled instruction
//
typedef
	struct disasm {         // vv---Useful for: D=disassembly; E=emulation
		char     mnem[8];   // D.: mnemonic
		char     oper[16];  // D.: operands
		amode_t  am;        // .E: Addressing Mode
		int      len;       // DE: Instr/macro length
		int      t;         // .E: Tcnt (for instr only (for now))  [+10, or +20 -> footnote #1, #2]
		uint8_t  flg[3];    // .E: [0] clear, [1] set, [2] modify
	}
disasm_t;

//----------------------------------------------------------------------------- ----------------------------------------
// Instruction decoder table
//
typedef
	struct decode {
		char     mnem[8];   // mnemonic
		amode_t  am;        // Addressing Mode
		int      len;       // Instr/macro length
		int      t;         // Tcnt (for instr only (for now))  [+10, or +20 -> footnote #1, #2]   <-- 0_o
		uint8_t  flg[3];    // [0] clear, [1] set, [2] modify
	}
decode_t;

decode_t  op2i[256];

//+============================================================================ ========================================
// Called when the program exits
//
void  setup (void) 
{
	memset(op2i, 0, sizeof(op2i));
	
	//
	// Footnotes:
	//   [1] : (Tcnt += 1) if page boundary is crossed         // +10
	//   [2] : (Tcnt += 1) if branch occurs on same page       // +20
	//       : (Tcnt += 2) if branch occurs to different page  // ...
	//
	//                                    addr    len TCnt |<----------------flags------------------->|
	//   opcode                    mnem     mode   V  VV   | reset  |   set   |         modify        |
	op2i[OP_ADC_ABS ] = (decode_t){"ADC", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_ABSX] = (decode_t){"ADC", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_ABSY] = (decode_t){"ADC", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_IMM ] = (decode_t){"ADC", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_INDX] = (decode_t){"ADC", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_INDY] = (decode_t){"ADC", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_ZP  ] = (decode_t){"ADC", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_ADC_ZPX ] = (decode_t){"ADC", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_AND_ABS ] = (decode_t){"AND", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_ABSX] = (decode_t){"AND", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_ABSY] = (decode_t){"AND", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_IMM ] = (decode_t){"AND", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_INDX] = (decode_t){"AND", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_INDY] = (decode_t){"AND", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_ZP  ] = (decode_t){"AND", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_AND_ZPX ] = (decode_t){"AND", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ASL_ABS ] = (decode_t){"ASL", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ASL_ABSX] = (decode_t){"ASL", AM_ABSX, 3,  7, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ASL_ACC ] = (decode_t){"ASL", AM_ACC , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ASL_ZP  ] = (decode_t){"ASL", AM_ZP  , 2,  5, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ASL_ZPX ] = (decode_t){"ASL", AM_ZPX , 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_BCC_REL ] = (decode_t){"BCC", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BCS_REL ] = (decode_t){"BCS", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BEQ_REL ] = (decode_t){"BEQ", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BIT_ABS ] = (decode_t){"BIT", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z      }};
	op2i[OP_BIT_ZP  ] = (decode_t){"BIT", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z      }};
	op2i[OP_BMI_REL ] = (decode_t){"BMI", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BNE_REL ] = (decode_t){"BNE", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BPL_REL ] = (decode_t){"BPL", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BRK_IMP ] = (decode_t){"BRK", AM_IMP , 1,  7, {FLG_NONE, FLG_I,    FLG_NONE               }};
	op2i[OP_BVC_REL ] = (decode_t){"BVC", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_BVS_REL ] = (decode_t){"BVS", AM_REL , 2, 22, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_CLC_IMP ] = (decode_t){"CLC", AM_IMP , 1,  2, {FLG_C,    FLG_NONE, FLG_NONE               }};
	op2i[OP_CLD_IMP ] = (decode_t){"CLD", AM_IMP , 1,  2, {FLG_D,    FLG_NONE, FLG_NONE               }};
	op2i[OP_CLI_IMP ] = (decode_t){"CLI", AM_IMP , 1,  2, {FLG_I,    FLG_NONE, FLG_NONE               }};
	op2i[OP_CLV_IMP ] = (decode_t){"CLV", AM_IMP , 1,  2, {FLG_V,    FLG_NONE, FLG_NONE               }};
	op2i[OP_CMP_ABS ] = (decode_t){"CMP", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_ABSX] = (decode_t){"CMP", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_ABSY] = (decode_t){"CMP", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_IMM ] = (decode_t){"CMP", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_INDX] = (decode_t){"CMP", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_INDY] = (decode_t){"CMP", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_ZP  ] = (decode_t){"CMP", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CMP_ZPX ] = (decode_t){"CMP", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPX_ABS ] = (decode_t){"CPX", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPX_IMM ] = (decode_t){"CPX", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPX_ZP  ] = (decode_t){"CPX", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPY_ABS ] = (decode_t){"CPY", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPY_IMM ] = (decode_t){"CPY", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_CPY_ZP  ] = (decode_t){"CPY", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_DEC_ABS ] = (decode_t){"DEC", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_DEC_ABSX] = (decode_t){"DEC", AM_ABSX, 3,  7, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_DEC_ZP  ] = (decode_t){"DEC", AM_ZP  , 2,  5, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_DEC_ZPX ] = (decode_t){"DEC", AM_ZPX , 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_DEX_IMP ] = (decode_t){"DEX", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_DEY_IMP ] = (decode_t){"DEY", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_ABS ] = (decode_t){"EOR", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_ABSX] = (decode_t){"EOR", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_ABSY] = (decode_t){"EOR", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_IMM ] = (decode_t){"EOR", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_INDX] = (decode_t){"EOR", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_INDY] = (decode_t){"EOR", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_ZP  ] = (decode_t){"EOR", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_EOR_ZPX ] = (decode_t){"EOR", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INC_ABS ] = (decode_t){"INC", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INC_ABSX] = (decode_t){"INC", AM_ABSX, 3,  7, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INC_ZP  ] = (decode_t){"INC", AM_ZP  , 2,  5, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INC_ZPX ] = (decode_t){"INC", AM_ZPX , 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INX_IMP ] = (decode_t){"INX", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_INY_IMP ] = (decode_t){"INY", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_JMP_ABS ] = (decode_t){"JMP", AM_ABS , 3,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};            
	op2i[OP_JMP_IND ] = (decode_t){"JMP", AM_IND , 3,  5, {FLG_NONE, FLG_NONE, FLG_NONE               }};            
	op2i[OP_JSR_ABS ] = (decode_t){"JSR", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_NONE               }};            
	op2i[OP_LDA_ABS ] = (decode_t){"LDA", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_ABSX] = (decode_t){"LDA", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_ABSY] = (decode_t){"LDA", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_IMM ] = (decode_t){"LDA", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_INDX] = (decode_t){"LDA", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_INDY] = (decode_t){"LDA", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_ZP  ] = (decode_t){"LDA", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDA_ZPX ] = (decode_t){"LDA", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDX_ABS ] = (decode_t){"LDX", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDX_ABSY] = (decode_t){"LDX", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDX_IMM ] = (decode_t){"LDX", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDX_ZP  ] = (decode_t){"LDX", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDX_ZPY ] = (decode_t){"LDX", AM_ZPY , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDY_ABS ] = (decode_t){"LDY", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDY_ABSX] = (decode_t){"LDY", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDY_IMM ] = (decode_t){"LDY", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDY_ZP  ] = (decode_t){"LDY", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LDY_ZPX ] = (decode_t){"LDY", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_LSR_ABS ] = (decode_t){"LSR", AM_ABS , 3,  6, {FLG_N,    FLG_NONE, FLG_Z            |FLG_C}};
	op2i[OP_LSR_ABSX] = (decode_t){"LSR", AM_ABSX, 3,  7, {FLG_N,    FLG_NONE, FLG_Z            |FLG_C}};
	op2i[OP_LSR_IMM ] = (decode_t){"LSR", AM_ACC , 1,  2, {FLG_N,    FLG_NONE, FLG_Z            |FLG_C}};
	op2i[OP_LSR_ZP  ] = (decode_t){"LSR", AM_ZP  , 2,  5, {FLG_N,    FLG_NONE, FLG_Z            |FLG_C}};
	op2i[OP_LSR_ZPX ] = (decode_t){"LSR", AM_ZPX , 2,  6, {FLG_N,    FLG_NONE, FLG_Z            |FLG_C}};
	op2i[OP_NOP_IMP ] = (decode_t){"NOP", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_ORA_ABS ] = (decode_t){"ORA", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_ABSX] = (decode_t){"ORA", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_ABSY] = (decode_t){"ORA", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_IMM ] = (decode_t){"ORA", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_INDX] = (decode_t){"ORA", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_INDY] = (decode_t){"ORA", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_ZP  ] = (decode_t){"ORA", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_ORA_ZPX ] = (decode_t){"ORA", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_PHA_IMP ] = (decode_t){"PHA", AM_IMP , 1,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_PHP_IMP ] = (decode_t){"PHP", AM_IMP , 1,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_PLA_IMP ] = (decode_t){"PLA", AM_IMP , 1,  4, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_PLP_IMP ] = (decode_t){"PLP", AM_IMP , 1,  4, {FLG_NONE, FLG_NONE, FLG_ALL                }};
	op2i[OP_ROL_ABS ] = (decode_t){"ROL", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROL_ABSX] = (decode_t){"ROL", AM_ABSX, 3,  7, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROL_ACC ] = (decode_t){"ROL", AM_ACC , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROL_ZP  ] = (decode_t){"ROL", AM_ZP  , 2,  5, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROL_ZPX ] = (decode_t){"ROL", AM_ZPX , 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROR_ABS ] = (decode_t){"ROR", AM_ABS , 3,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROR_ABSX] = (decode_t){"ROR", AM_ABSX, 3,  7, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROR_ACC ] = (decode_t){"ROR", AM_ACC , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROR_ZP  ] = (decode_t){"ROR", AM_ZP  , 2,  5, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_ROR_ZPX ] = (decode_t){"ROR", AM_ZPX , 2,  6, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z|FLG_C}};
	op2i[OP_RTI_IMP ] = (decode_t){"RTI", AM_IMP , 1,  6, {FLG_NONE, FLG_NONE, FLG_ALL                }};
	op2i[OP_RTS_IMP ] = (decode_t){"RTS", AM_IMP , 1,  6, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_SBC_ABS ] = (decode_t){"SBC", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_ABSX] = (decode_t){"SBC", AM_ABSX, 3, 14, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_ABSY] = (decode_t){"SBC", AM_ABSY, 3, 14, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_IMM ] = (decode_t){"SBC", AM_IMM , 2,  2, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_INDX] = (decode_t){"SBC", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_INDY] = (decode_t){"SBC", AM_INDY, 2, 15, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_ZP  ] = (decode_t){"SBC", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SBC_ZPX ] = (decode_t){"SBC", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_N|FLG_V|FLG_Z|FLG_C}};
	op2i[OP_SEC_IMP ] = (decode_t){"SEC", AM_IMP , 1,  2, {FLG_NONE, FLG_C,    FLG_NONE               }};
	op2i[OP_SED_IMP ] = (decode_t){"SED", AM_IMP , 1,  2, {FLG_NONE, FLG_D,    FLG_NONE               }};
	op2i[OP_SEI_IMP ] = (decode_t){"SEI", AM_IMP , 1,  2, {FLG_NONE, FLG_I,    FLG_NONE               }};
	op2i[OP_STA_ABS ] = (decode_t){"STA", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_ABSX] = (decode_t){"STA", AM_ABSX, 3,  5, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_ABSY] = (decode_t){"STA", AM_ABSY, 3,  5, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_INDX] = (decode_t){"STA", AM_INDX, 2,  6, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_INDY] = (decode_t){"STA", AM_INDY, 2,  6, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_ZP  ] = (decode_t){"STA", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STA_ZPX ] = (decode_t){"STA", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STX_ABS ] = (decode_t){"STX", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STX_ZP  ] = (decode_t){"STX", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STX_ZPY ] = (decode_t){"STX", AM_ZPY , 2,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STY_ABS ] = (decode_t){"STY", AM_ABS , 3,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STY_ZP  ] = (decode_t){"STY", AM_ZP  , 2,  3, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_STY_ZPX ] = (decode_t){"STY", AM_ZPX , 2,  4, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_TAX_IMP ] = (decode_t){"TAX", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_TAY_IMP ] = (decode_t){"TAY", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_TSX_IMP ] = (decode_t){"TSX", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_TXA_IMP ] = (decode_t){"TXA", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
	op2i[OP_TXS_IMP ] = (decode_t){"TXS", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_NONE               }};
	op2i[OP_TYA_IMP ] = (decode_t){"TYA", AM_IMP , 1,  2, {FLG_NONE, FLG_NONE, FLG_N      |FLG_Z      }};
}

//+============================================================================ ========================================
int  decode (uint8_t* pc,  disasm_t* pDis)
{
	decode_t*  pDec = &op2i[*pc];

	strcpy(pDis->mnem, pDec->mnem);
	*pDis->oper = '\0';

	switch (pDis->am = pDec->am) {
		case AM_IMP :  sprintf(pDis->oper, ""                          );  break ;
		case AM_REL :  sprintf(pDis->oper, "lbl"                       );  break ;
		case AM_ACC :  sprintf(pDis->oper, "A"                         );  break ;
		case AM_IMM :  sprintf(pDis->oper, "#$%02X"    , PEEK8   (pc+1));  break ;
		case AM_ZP  :  sprintf(pDis->oper, "$%02X"     , PEEK8   (pc+1));  break ;
		case AM_ZPX :  sprintf(pDis->oper, "$%02X, X"  , PEEK8   (pc+1));  break ;
		case AM_ZPY :  sprintf(pDis->oper, "$%02X, Y"  , PEEK8   (pc+1));  break ;
		case AM_ABS :  sprintf(pDis->oper, "$%04X"     , PEEK16LE(pc+1));  break ;
		case AM_ABSX:  sprintf(pDis->oper, "$%04X, X"  , PEEK16LE(pc+1));  break ;
		case AM_ABSY:  sprintf(pDis->oper, "$%04X, Y"  , PEEK16LE(pc+1));  break ;
		case AM_IND :  sprintf(pDis->oper, "($%04X)"   , PEEK16LE(pc+1));  break ;
		case AM_INDX:  sprintf(pDis->oper, "($%02X, X)", PEEK8   (pc+1));  break ;
		case AM_INDY:  sprintf(pDis->oper, "($%02X), Y", PEEK8   (pc+1));  break ;
		default:       sprintf(pDis->oper, "???"                       );  break ;
	}                                                                  

	pDis->len    = pDec->len;     // iLen
	pDis->t      = pDec->t;       // TCnt
	pDis->flg[0] = pDec->flg[0];  // clear
	pDis->flg[1] = pDec->flg[1];  // set
	pDis->flg[2] = pDec->flg[2];  // modify

	return pDis->len;
}

//+============================================================================ ========================================
int  main (int argc,  char* argv[],  char* envp[])
{
	// Initialise the disassembler
	setup();
	
	// Table Header
	printf("    <table summary=\"\" border=\"1\" cellpadding=\"0\" width=\"100%\">\n");
	printf("      <tr align=\"center\">\n");
	printf("        <th width=\"5%\">&nbsp;</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>0</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>1</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>2</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>3</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>4</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>5</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>6</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>7</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>8</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>9</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>A</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>B</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>C</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>D</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>E</th>\n");
	printf("        <th width=\"5%\"><big><big>&#8209;</big></big>F</th>\n");
	printf("        <th width=\"5%\">&nbsp;</th>\n");
	printf("      </tr>\n");

	// Foreach row
	for (int i = 0;  i <= 0xFF;  NULL) {
		// Row: begin
		printf("\n");
		printf("      <tr>\n");
		printf("        <th align=\"center\">&nbsp;%X<big><big>&#8209;</big></big>&nbsp;</th>\n", i>>4);
		
		// Foreach column
		do {
			uint8_t   ins[3] = {i, 0xA5, 0xB6};
			disasm_t  dis;
			
			if (decode(ins, &dis)) {
				// Cell: begin
				printf("        <td><table summary=\"\" align=\"center\" border=\"0\">\n");

				// I-Len
				printf("          <tr><td align=\"left\"><small><b>           %d       </b></small></td>\n", dis.len);
				
				// T-Cnt
				printf("              <td align=\"right\"><small><b>          %d%.2s    </b></small></td></tr>\n", 
				       (dis.t %10), &"  + ++"[(dis.t/10)*2]);

				// Mnemonic
				printf("          <tr><th colspan=\"2\">       <a href=\"#%s\">%s     </a></th></tr>\n", 
				       dis.mnem, dis.mnem);

				// Addressing mode
				printf("          <tr><td align=\"center\" colspan=\"2\"><small>%s</small></td></tr>\n", amode[dis.am]);
				
				// Flags
				printf("          <tr><td align=\"center\" colspan=\"2\"><small>");
				printf("[");
				for (int j = 7;  j > 0;  j--)
					if (j == 5)  printf("&#8209;") ;
					else         printf("%c", ((dis.flg[0] | dis.flg[1] | dis.flg[2]) & (1 << j)) ? "CZIDB-VN"[j] : "czidb-vn"[j]);
				printf("]");
				
				// Cell: end
				printf("        </table></td>\n");

			} else {  // Invalid instruction
				printf("        <td>&nbsp;</td>\n");
			}
		} while ((++i) & 0xF) ;

		// Row: end
		printf("        <th align=\"center\">&nbsp;%X<big><big>&#8209;</big></big>&nbsp;</th>\n", (i>>4)-1);
		printf("      </tr>\n");
	}// for i

	// Table footer
	printf("      <tr align=\"center\">\n");
	printf("        <th width=\"7%\">&nbsp;</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>0</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>1</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>2</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>3</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>4</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>5</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>6</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>7</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>8</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>9</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>A</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>B</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>C</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>D</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>E</th>\n");
	printf("        <th width=\"7%\"><big><big>&#8209;</big></big>F</th>\n");
	printf("        <th width=\"7%\">&nbsp;</th>\n");
	printf("      </tr>\n");
	printf("    </table>\n");
	
	// Credits
	printf("\n");
	printf("    <table summary=\"\" border=\"0\" cellpadding=\"0\" width=\"100%\">\n");
	printf("      <tr><td align=\"right\">autogenerated from \n");
	printf("        <a href=\"https://www.masswerk.at/6502/6502_instruction_set.html\" target=\"extern6502\">6502 Instruction Set</a>\n");
	printf("        by <a href=\"https://masswerk.at/\" target=\"extern6502\">mass:werk</a>\n");
	printf("      </td></tr>\n");
	printf("    </table>\n");

	return 0;
}
