// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Zero 2 Alpha as one of the games made from Street Fighter Alpha 2's program, whose sources it
// builds (games/sfa2/src; places.h there): where the places those sources name are in this game.
#pragma once

#include <array>
#include <cstdint>

#include "game.h"

namespace sfa2::family {

struct Place {
    uint32_t sfa2, here;
};
inline constexpr Place kPlaceList[] = {
    // places: begin (tools/places.py; a line ending "by hand" is kept)
    {0x001096, 0x001096}, // an instruction, 2 on alike
    {0x0013CE, 0x0013CE}, // a table between two that moved alike (other bytes)
    {0x0014CC, 0x0014CC}, // an instruction, 24 on alike
    {0x004B6C, 0x004EF4}, // an instruction, 12 on alike
    {0x005594, 0x005970}, // the round's steps' table: the same code before it (005978 on) by hand
    {0x00559C, 0x005978}, // the same bytes, once only
    {0x005820, 0x005C0A}, // the same bytes, once only
    {0x0058DE, 0x005CCC}, // the same code (tst.w 138(a5), the players' records) by hand
    {0x005A52, 0x005EB2}, // an instruction, 6 on alike
    {0x0065C4, 0x006E5A}, // the same bytes, once only
    {0x0066B0, 0x006F7E}, // an instruction, 4 on alike
    {0x00735C, 0x007D50}, // the same bytes, once only
    {0x007504, 0x007F52}, // an instruction, 3 on alike
    {0x007684, 0x00802E}, // an instruction, 8 on alike
    {0x007914, 0x0082C0}, // the same bytes, once only
    {0x00797C, 0x00832A}, // the stage's number doubled and kept (the code before it calls a new routine, 09D0F8) by hand
    {0x00B444, 0x00C4FE}, // an instruction, 17 on alike
    {0x00B50C, 0x00C5C6}, // an instruction, 24 on alike
    {0x00ECCE, 0x00EE14}, // an instruction, 4 on alike
    {0x00F89A, 0x00F8DE}, // an instruction, 9 on alike
    {0x00FDA0, 0x00FCEE}, // an instruction, 24 on alike
    {0x010352, 0x0103D8}, // an instruction, 8 on alike
    {0x010370, 0x0103F6}, // an instruction, 15 on alike
    {0x0103AA, 0x010430}, // an instruction, 19 on alike
    {0x01054C, 0x0105D4}, // a place the code names so (1/1)
    {0x010564, 0x0105F4}, // an instruction, 24 on alike
    {0x010682, 0x010712}, // a place the code names so (2/2)
    {0x0106F6, 0x010786}, // an instruction, 5 on alike
    {0x010708, 0x010798}, // an instruction, 24 on alike
    {0x0107B0, 0x010840}, // an instruction, 7 on alike
    {0x0107F0, 0x010880}, // a place the code names so (2/2)
    {0x010834, 0x0108C4}, // a place the code names so (2/2)
    {0x010870, 0x010900}, // the same entry of a jump table, 13 on alike
    {0x0108A4, 0x010934}, // the same entry of a jump table, 5 on alike
    {0x0108C6, 0x010956}, // the same entry of a jump table, 6 on alike
    {0x0108E2, 0x010972}, // an instruction, 24 on alike
    {0x010C58, 0x010CF0}, // a place the code names so (2/2)
    {0x010C76, 0x010D0E}, // an instruction, 8 on alike
    {0x010CB6, 0x010D4E}, // an instruction, 5 on alike
    {0x010D26, 0x010DBE}, // an instruction, 19 on alike
    {0x010D94, 0x010E2C}, // an instruction, 7 on alike
    {0x010DF0, 0x010E88}, // an instruction, 7 on alike
    {0x010FD8, 0x011090}, // an instruction, 24 on alike
    {0x011092, 0x01114A}, // an instruction, 24 on alike
    {0x0112CE, 0x011386}, // an instruction, 21 on alike
    {0x011324, 0x0113DC}, // an instruction, 13 on alike
    {0x0113AA, 0x011462}, // an instruction, 24 on alike
    {0x011438, 0x0114F0}, // a place the code names so (1/1)
    {0x011456, 0x01150E}, // a place the code names so (1/1)
    {0x011494, 0x01154C}, // the same bytes, once only
    {0x011594, 0x01164C}, // the same bytes, once only
    {0x0127A4, 0x012792}, // an instruction, 24 on alike
    {0x01462C, 0x01463E}, // an instruction, 1 on alike
    {0x01467A, 0x01468C}, // an instruction, 1 on alike
    {0x01472E, 0x014740}, // an instruction, 1 on alike
    {0x014874, 0x014886}, // an instruction, 1 on alike
    {0x0148C8, 0x0148DA}, // an instruction, 1 on alike
    {0x014916, 0x014928}, // an instruction, 1 on alike
    {0x0149F4, 0x014A06}, // an instruction, 1 on alike
    {0x0157A4, 0x0157F8}, // an instruction, 11 on alike
    {0x01906A, 0x0190C4}, // where the code calls or jumps to it (8/8), not its first instruction's place
    {0x019216, 0x019276}, // an instruction, 4 on alike
    {0x01963A, 0x0196A6}, // the same entry of a jump table, 7 on alike
    {0x01975A, 0x0197C6}, // an instruction, 9 on alike
    {0x0197CC, 0x019896}, // an instruction, 1 on alike
    {0x01A7F6, 0x01B1FE}, // an instruction, 11 on alike
    {0x01B662, 0x01C06A}, // an instruction, 15 on alike
    {0x01B6DA, 0x01C0E2}, // an instruction, 8 on alike
    {0x01BFE0, 0x01C9E8}, // an instruction, 3 on alike
    {0x01C1D0, 0x01CAF2}, // an instruction, 3 on alike
    {0x01C9FE, 0x01D280}, // an instruction, 19 on alike
    {0x01CA34, 0x01D2B6}, // an instruction, 17 on alike
    {0x01CA9C, 0x01D31E}, // an instruction, 4 on alike
    {0x01CAB8, 0x01D33A}, // an instruction, 23 on alike
    {0x01CB40, 0x01D3C2}, // an instruction, 24 on alike
    {0x01DFCC, 0x01E98E}, // an instruction, 12 on alike
    {0x01E308, 0x01ECEA}, // an instruction, 12 on alike
    {0x01E582, 0x01EF70}, // an instruction, 12 on alike
    {0x01E640, 0x01F02E}, // an instruction, 12 on alike
    {0x01E6AE, 0x01F09C}, // an instruction, 12 on alike
    {0x01F738, 0x020102}, // an instruction, 7 on alike
    {0x01F89E, 0x020274}, // the same code (cmpi.b #3,AC(a5) on) by hand
    {0x01F93C, 0x020312}, // an instruction, 6 on alike
    {0x01F9A6, 0x02037C}, // an instruction, 15 on alike
    {0x01FE5E, 0x020854}, // an instruction, 3 on alike
    {0x01FF0A, 0x0208F8}, // an instruction, 3 on alike
    {0x01FFF2, 0x0209E0}, // an instruction, 3 on alike
    {0x02109A, 0x020CBA}, // an instruction, 24 on alike
    {0x021112, 0x020D32}, // an instruction, 3 on alike
    {0x0211C8, 0x020DAA}, // an instruction, 3 on alike
    {0x0212A8, 0x020E60}, // a place the code names so (2/2)
    {0x021300, 0x020EB8}, // a place the code names so (2/2)
    {0x02140A, 0x020FC2}, // after the entrance's place is set: this game has no test of 102(a5) there by hand
    {0x021430, 0x020FE2}, // an instruction, 2 on alike
    {0x02145E, 0x021016}, // an instruction, 1 on alike
    {0x02149A, 0x021052}, // an instruction, 6 on alike
    {0x0214C0, 0x021078}, // an instruction, 4 on alike
    {0x0214E0, 0x021098}, // an instruction, 22 on alike
    {0x021614, 0x0211E2}, // an instruction, 4 on alike
    {0x021620, 0x0211EE}, // the same entry of a jump table, 11 on alike
    {0x02171E, 0x0212FA}, // a place the code names so (2/2)
    {0x021732, 0x02130E}, // an instruction, 4 on alike
    {0x021742, 0x02131E}, // a place the code names so (2/2)
    {0x021880, 0x021464}, // an instruction, 4 on alike
    {0x021890, 0x021474}, // a place the code names so (2/2)
    {0x021990, 0x021574}, // an instruction, 4 on alike
    {0x0219A0, 0x021584}, // a place the code names so (2/2)
    {0x021A64, 0x021648}, // an instruction, 3 on alike
    {0x021A70, 0x021654}, // a place the code names so (2/2)
    {0x021D22, 0x021906}, // an instruction, 4 on alike
    {0x021D32, 0x021916}, // a place the code names so (2/2)
    {0x021E04, 0x0219E8}, // an instruction, 23 on alike
    {0x021E54, 0x021A38}, // an instruction, 4 on alike
    {0x021E64, 0x021A48}, // a place the code names so (2/2)
    {0x022000, 0x021BE4}, // an instruction, 10 on alike
    {0x022026, 0x021C0A}, // an instruction, 3 on alike
    {0x022032, 0x021C16}, // a place the code names so (2/2)
    {0x022094, 0x021C78}, // an instruction, 5 on alike
    {0x0220A4, 0x021C88}, // an instruction, 6 on alike
    {0x0220B8, 0x021C9C}, // an instruction, 5 on alike
    {0x0220D4, 0x021CB8}, // the same entry of a jump table, 14 on alike
    {0x022116, 0x021CFA}, // the same entry of a jump table, 4 on alike
    {0x02212A, 0x021D0E}, // a place the code names so (2/2)
    {0x022130, 0x021D14}, // an instruction, 3 on alike
    {0x02213C, 0x021D20}, // a place the code names so (2/2)
    {0x022140, 0x021D24}, // an instruction, 1 on alike
    {0x022258, 0x021E48}, // an instruction, 20 on alike
    {0x022544, 0x02214A}, // an instruction, 16 on alike
    {0x0225E6, 0x0221EC}, // an instruction, 3 on alike
    {0x0225F2, 0x0221F8}, // a place the code names so (2/2)
    {0x0225FE, 0x022204}, // the same entry of a jump table, 3 on alike
    {0x02260A, 0x022210}, // a place the code names so (2/2)
    {0x0226C8, 0x0222CE}, // an instruction, 8 on alike
    {0x0226E6, 0x0222EC}, // the same entry of a jump table, 3 on alike
    {0x0226F2, 0x0222F8}, // a place the code names so (2/2)
    {0x0227B8, 0x0223BE}, // the same entry of a jump table, 3 on alike
    {0x0227C4, 0x0223CA}, // a place the code names so (2/2)
    {0x022AEA, 0x022706}, // an instruction, 2 on alike
    {0x022B52, 0x02276A}, // the same entry of a jump table, 3 on alike
    {0x022B5E, 0x022776}, // a place the code names so (2/2)
    {0x022D20, 0x022938}, // an instruction, 20 on alike
    {0x022DAC, 0x0229C4}, // the same entry of a jump table, 8 on alike
    {0x022DD4, 0x0229EC}, // the same entry of a jump table, 19 on alike
    {0x022F1E, 0x022B36}, // the same entry of a jump table, 5 on alike
    {0x023136, 0x022D6E}, // an instruction, 12 on alike
    {0x02319C, 0x022DD4}, // an instruction, 15 on alike
    {0x0231D4, 0x022E0C}, // an instruction, 2 on alike
    {0x02322A, 0x022E62}, // an instruction, 7 on alike
    {0x023242, 0x022E7A}, // an instruction, 7 on alike
    {0x023294, 0x022ECC}, // an instruction, 7 on alike
    {0x0232AA, 0x022EE2}, // an instruction, 4 on alike
    {0x0232BC, 0x022EF4}, // an instruction, 13 on alike
    {0x0232F6, 0x022F2E}, // an instruction, 10 on alike
    {0x02331A, 0x022F52}, // an instruction, 11 on alike
    {0x0233A8, 0x022FE0}, // an instruction, 3 on alike
    {0x0233B2, 0x022FEA}, // an instruction, 5 on alike
    {0x0233C6, 0x022FFE}, // an instruction, 12 on alike
    {0x0233FA, 0x023032}, // an instruction, 2 on alike
    {0x023402, 0x02303A}, // an instruction, 4 on alike
    {0x023414, 0x02304C}, // an instruction, 4 on alike
    {0x02342A, 0x023062}, // an instruction, 4 on alike
    {0x02343C, 0x023074}, // an instruction, 4 on alike
    {0x0235F6, 0x0231D8}, // an instruction, 18 on alike
    {0x023638, 0x02321A}, // an instruction, 24 on alike
    {0x0236F0, 0x0232D2}, // an instruction, 24 on alike
    {0x0237BE, 0x0233A0}, // an instruction, 19 on alike
    {0x023930, 0x023512}, // an instruction, 2 on alike
    {0x02399A, 0x02357C}, // an instruction, 2 on alike
    {0x0239E6, 0x0235C8}, // an instruction, 12 on alike
    {0x023A1A, 0x0235FC}, // an instruction, 5 on alike
    {0x023A2E, 0x023610}, // an instruction, 11 on alike
    {0x023A6E, 0x023650}, // an instruction, 12 on alike
    {0x023A9A, 0x02367C}, // an instruction, 12 on alike
    {0x023AC6, 0x0236A8}, // an instruction, 10 on alike
    {0x023B00, 0x0236E2}, // an instruction, 12 on alike
    {0x023B2E, 0x023710}, // an instruction, 3 on alike
    {0x023B6A, 0x02374E}, // an instruction, 3 on alike
    {0x023B74, 0x023758}, // an instruction, 14 on alike
    {0x023BEC, 0x0237D0}, // an instruction, 24 on alike
    {0x023C4E, 0x023832}, // an instruction, 24 on alike
    {0x023CD4, 0x0238B8}, // an instruction, 4 on alike
    {0x023D3C, 0x023928}, // an instruction, 9 on alike
    {0x023D5E, 0x02394A}, // an instruction, 3 on alike
    {0x023D6A, 0x023956}, // an instruction, 15 on alike
    {0x023E70, 0x023A6C}, // an instruction, 3 on alike
    {0x023E82, 0x023A86}, // an instruction, 12 on alike
    {0x023EB0, 0x023AB4}, // an instruction, 11 on alike
    {0x024004, 0x023C0A}, // an instruction, 11 on alike
    {0x02402C, 0x023C32}, // a place the code names so (1/1)
    {0x024068, 0x023C6E}, // a place the code names so (1/1)
    {0x02515A, 0x024DA8}, // an instruction, 2 on alike
    {0x025162, 0x024DB0}, // an instruction, 2 on alike
    {0x0260E8, 0x025C8A}, // an instruction, 7 on alike
    {0x026518, 0x0260D4}, // an instruction, 10 on alike
    {0x026C96, 0x0267E8}, // an instruction, 16 on alike
    {0x026DB0, 0x026902}, // an instruction, 24 on alike
    {0x026EBC, 0x026A06}, // an instruction, 24 on alike
    {0x027038, 0x026B82}, // an instruction, 14 on alike
    {0x027204, 0x026D4E}, // an instruction, 3 on alike
    {0x02720E, 0x026D58}, // an instruction, 11 on alike
    {0x0272EC, 0x026E32}, // the score for a stun: called at the same place; this game has no test of A5(a6) at its head by hand
    {0x027320, 0x026E60}, // an instruction, 5 on alike
    {0x027482, 0x026FC2}, // an instruction, 10 on alike
    {0x0277CA, 0x027136}, // an instruction, 12 on alike
    {0x027E32, 0x0277DE}, // an instruction, 5 on alike
    {0x027E94, 0x027840}, // an instruction, 3 on alike
    {0x027FE2, 0x027952}, // an instruction, 14 on alike
    {0x028034, 0x0279A4}, // an instruction, 20 on alike
    {0x028086, 0x0279F6}, // an instruction, 11 on alike
    {0x0280BA, 0x027A2A}, // an instruction, 8 on alike
    {0x0280D4, 0x027A44}, // an instruction, 8 on alike
    {0x028BCE, 0x02853E}, // a place the code names so (4/4)
    {0x0295EC, 0x028F5C}, // an instruction, 3 on alike
    {0x029CEA, 0x029660}, // an instruction, 24 on alike
    {0x029E38, 0x0297AE}, // an instruction, 3 on alike
    {0x029FD2, 0x029948}, // a place the code names so (1/1)
    {0x02A012, 0x029988}, // a place the code names so (1/1)
    {0x02A052, 0x0299C8}, // a place the code names so (1/1)
    {0x0322D6, 0x031CB8}, // an instruction, 13 on alike
    {0x0342E8, 0x033C6A}, // an instruction, 14 on alike
    {0x03CB92, 0x03CA2A}, // an instruction, 24 on alike
    {0x04431E, 0x04479A}, // an instruction, 8 on alike
    {0x04433C, 0x0447B8}, // an instruction, 8 on alike
    {0x04FEE8, 0x053522}, // an instruction, 2 on alike
    {0x05CD62, 0x060C4E}, // an instruction, 2 on alike
    {0x05DA40, 0x0618F4}, // an instruction, 24 on alike
    {0x05DEBE, 0x061D9A}, // an instruction, 21 on alike
    {0x05DF28, 0x061E04}, // an instruction, 21 on alike
    {0x05DF92, 0x061E6E}, // an instruction, 24 on alike
    {0x0628D8, 0x066B6C}, // a chosen character's colours from the button: here a routine of its own, called from two places by hand
    {0x08318E, 0x088670}, // a place the code names so (1/1)
    {0x09764C, 0x09D68E}, // an instruction, 7 on alike
    {0x097AEC, 0x09DC3E}, // a place the code names so (1/1)
    {0x097B4C, 0x09DCBE}, // a place the code names so (1/1)
    {0x097BAC, 0x09DD3E}, // a place the code names so (1/1)
    {0x097C0C, 0x09DDBE}, // a place the code names so (1/1)
    {0x097C6C, 0x09DE3E}, // a place the code names so (1/1)
    {0x097CCC, 0x09DEBE}, // a place the code names so (1/1)
    {0x097D2C, 0x09DF3E}, // a place the code names so (1/1)
    {0x09808C, 0x09E3BE}, // a place the code names so (1/1)
    {0x098090, 0x09E3C2}, // a place the code names so (1/1)
    {0x09814C, 0x022654}, // the stun's lengths: here one row in the code (022636), not one by character by hand
    {0x0984AC, 0x09E53E}, // a place the code names so (1/1)
    {0x09862C, 0x09E73E}, // a place the code names so (1/1)
    {0x09868C, 0x09E7BE}, // a place the code names so (2/2)
    {0x09874C, 0x09E8BE}, // a place the code names so (2/2)
    {0x0987AC, 0x09E93E}, // a place the code names so (1/1)
    {0x09886C, 0x09EA3E}, // a place the code names so (2/2)
    {0x09892C, 0x09EB3E}, // a place the code names so (2/2)
    {0x09928C, 0x09F7BE}, // a place the code names so (1/1)
    {0x0992EC, 0x09F83E}, // a place the code names so (1/1)
    {0x09934C, 0x09F8BE}, // a place the code names so (1/1)
    {0x0993AC, 0x09F93E}, // a place the code names so (1/1)
    {0x09940C, 0x09F9BE}, // a place the code names so (1/1)
    {0x09AE84, 0x0A1C7E}, // the characters' own routines as a stun begins (by character, a long each; 022620) by hand
    {0x09AF44, 0x0A1D7E}, // a place the code names so (1/1)
    {0x09AFC4, 0x0A1DFE}, // a place the code names so (1/1)
    {0x09B044, 0x0A1E7E}, // a place the code names so (2/2)
    {0x09BDC4, 0x0A2FFE}, // a place the code names so (1/1)
    {0x09BE44, 0x0A307E}, // a place the code names so (1/1)
    {0x09BFC4, 0x0A31FE}, // a place the code names so (1/1)
    {0x09C044, 0x0A327E}, // a place the code names so (4/4)
    {0x09C444, 0x0A367E}, // a place the code names so (8/8)
    {0x09C844, 0x0A3A7E}, // a place the code names so (2/2)
    {0x09CB44, 0x0A3E7E}, // a place the code names so (2/2)
    {0x09CE44, 0x0A427E}, // a place the code names so (2/2)
    {0x09D144, 0x0A467E}, // a place the code names so (2/2)
    {0x09E1C4, 0x0A56FE}, // a place the code names so (3/3)
    {0x09E1DC, 0x0A571E}, // a place the code names so (3/3)
    {0x26D0E0, 0x260504}, // a place the code names so (5/5)
    {0x272AE0, 0x267D44}, // a place the code names so (3/3)
    {0xFF0C68, 0xFF0C68}, // a variable the code names so (7/7)
    {0xFF21D6, 0xFF21DC}, // a variable the code names so (6/6)
    {0xFF21DC, 0xFF21E2}, // a variable the code names so (6/6)
    {0xFF21E0, 0xFF21E6}, // a variable the code names so (6/6)
    {0xFF27F8, 0xFFEE52}, // a variable the code names so (14/14)
    {0xFF27FA, 0xFFEE54}, // a variable the code names so (18/18)
    {0xFF27FC, 0xFFEE56}, // a variable the code names so (14/14)
    {0xFF27FE, 0xFFEE58}, // a variable the code names so (18/18)
    {0xFF2800, 0xFFEE5A}, // a variable the code names so (8/8)
    {0xFF2802, 0xFFEE5C}, // a variable the code names so (9/9)
    {0xFF2804, 0xFFEE5E}, // a variable the code names so (8/8)
    {0xFF2806, 0xFFEE60}, // a variable the code names so (4/4)
    {0xFF280E, 0xFFEE68}, // a variable the code names so (4/4)
    {0xFF2816, 0xFFEE70}, // a variable the code names so (4/4)
    {0xFF281E, 0xFFEE7C}, // a variable the code names so (5/5)
    {0xFF2822, 0xFFEE80}, // a variable the code names so (6/6)
    {0xFF2826, 0xFFEE84}, // a variable the code names so (2/2)
    {0xFF2828, 0xFFEE86}, // a variable the code names so (11/11)
    {0xFF2829, 0xFFEE87}, // a variable the code names so (4/4)
    {0xFF2B44, 0xFF2AEA}, // a variable the code names so (6/6)
    {0xFF8004, 0xFF8004}, // a variable the code names so (54/54)
    {0xFF8008, 0xFF8008}, // a variable the code names so (119/119)
    {0xFF800C, 0xFF800C}, // a variable the code names so (85/85)
    {0xFF8010, 0xFF8010}, // a variable the code names so (60/60)
    {0xFF8026, 0xFF8026}, // a variable the code names so (38/38)
    {0xFF8030, 0xFF8030}, // a variable the code names so (2/2)
    {0xFF8032, 0xFF8032}, // a variable the code names so (37/37)
    {0xFF8058, 0xFF8058}, // a variable the code names so (13/13)
    {0xFF805A, 0xFF805A}, // a variable the code names so (4/4)
    {0xFF805C, 0xFF805C}, // a variable the code names so (6/6)
    {0xFF805E, 0xFF805E}, // a variable the code names so (3/3)
    {0xFF8060, 0xFF8060}, // a variable the code names so (12/12)
    {0xFF8064, 0xFF8064}, // a variable the code names so (7/7)
    {0xFF8068, 0xFF8068}, // a variable the code names so (3/3)
    {0xFF806A, 0xFF806A}, // a variable the code names so (3/3)
    {0xFF8071, 0xFF8071}, // a variable the code names so (3/3)
    {0xFF8076, 0xFF8076}, // a variable the code names so (6/6)
    {0xFF8080, 0xFF8080}, // a variable the code names so (10/10)
    {0xFF8085, 0xFF8085}, // a variable the code names so (4/4)
    {0xFF8087, 0xFF8087}, // a variable the code names so (6/6)
    {0xFF8088, 0xFF8088}, // a variable the code names so (3/3)
    {0xFF808A, 0xFF808A}, // a variable the code names so (17/17)
    {0xFF808F, 0xFF808F}, // a variable the code names so (19/19)
    {0xFF8090, 0xFF8090}, // a variable the code names so (102/102)
    {0xFF8096, 0xFF8096}, // a variable the code names so (9/9)
    {0xFF8098, 0xFF8098}, // a variable the code names so (17/17)
    {0xFF8099, 0xFF8099}, // a variable the code names so (9/9)
    {0xFF80A0, 0xFF80A0}, // a variable the code names so (10/10)
    {0xFF80A1, 0xFF80A1}, // a variable the code names so (14/14)
    {0xFF80A2, 0xFF80A2}, // a variable the code names so (10/10)
    {0xFF80A3, 0xFF80A3}, // a variable the code names so (10/10)
    {0xFF80A4, 0xFF80A4}, // a variable the code names so (10/10)
    {0xFF80A5, 0xFF80A5}, // a variable the code names so (13/13)
    {0xFF80A8, 0xFF80A8}, // a variable the code names so (17/17)
    {0xFF80AC, 0xFF80AC}, // a variable the code names so (38/38)
    {0xFF80AE, 0xFF80AE}, // a variable the code names so (5/5)
    {0xFF80B4, 0xFF80B4}, // a variable the code names so (16/16)
    {0xFF80BB, 0xFF80BB}, // a variable the code names so (8/8)
    {0xFF80BD, 0xFF80BD}, // a variable the code names so (18/18)
    {0xFF80BE, 0xFF80BE}, // a variable the code names so (6/6)
    {0xFF80C3, 0xFF80C3}, // a variable between two that moved alike
    {0xFF80D1, 0xFF80D1}, // a variable the code names so (7/7)
    {0xFF8100, 0xFF8100}, // a variable the code names so (47/47)
    {0xFF8107, 0xFF8107}, // a variable the code names so (9/9)
    {0xFF8108, 0xFF8108}, // a variable the code names so (4/4)
    {0xFF8109, 0xFF8109}, // a variable the code names so (8/8)
    {0xFF810A, 0xFF810A}, // a variable the code names so (4/4)
    {0xFF810C, 0xFF810C}, // a variable the code names so (15/15)
    {0xFF810D, 0xFF810D}, // a variable the code names so (25/25)
    {0xFF810E, 0xFF810E}, // a variable the code names so (22/22)
    {0xFF810F, 0xFF810F}, // a variable the code names so (13/13)
    {0xFF811C, 0xFF811C}, // a variable the code names so (7/7)
    {0xFF811D, 0xFF811D}, // a variable the code names so (4/4)
    {0xFF811E, 0xFF811E}, // a variable the code names so (3/3)
    {0xFF8123, 0xFF8123}, // a variable the code names so (9/9)
    {0xFF8124, 0xFF8124}, // a variable the code names so (12/12)
    {0xFF8125, 0xFF8125}, // a variable the code names so (28/28)
    {0xFF812B, 0xFF812B}, // a variable the code names so (8/8)
    {0xFF812D, 0xFF812D}, // a variable the code names so (2/2)
    {0xFF8135, 0xFF8135}, // a variable the code names so (5/5)
    {0xFF8136, 0xFF8136}, // a variable between two that moved alike
    {0xFF8138, 0xFF8138}, // a variable the code names so (37/37)
    {0xFF814B, 0xFF814B}, // a variable the code names so (3/3)
    {0xFF815A, 0xFF815A}, // a variable the code names so (4/4)
    {0xFF815C, 0xFF815C}, // a variable the code names so (3/3)
    {0xFF8200, 0xFF8200}, // a variable the code names so (6/6)
    {0xFF8280, 0xFF8280}, // a variable the code names so (5/5)
    {0xFF8290, 0xFF8290}, // a variable the code names so (23/23)
    {0xFF8294, 0xFF8294}, // a variable the code names so (31/31)
    {0xFF82A0, 0xFF82A0}, // a variable the code names so (27/27)
    {0xFF82A8, 0xFF82A8}, // a variable between two that moved alike
    {0xFF8300, 0xFF8300}, // a variable the code names so (4/4)
    {0xFF8400, 0xFF8400}, // a variable the code names so (74/76)
    {0xFF8450, 0xFF8450}, // a variable the code names so (4/4)
    {0xFF8495, 0xFF8495}, // a variable the code names so (5/5)
    {0xFF853D, 0xFF853D}, // a variable the code names so (14/14)
    {0xFF865E, 0xFF865E}, // a variable the code names so (1/1)
    {0xFF8800, 0xFF8800}, // a variable the code names so (63/64)
    {0xFF8850, 0xFF8850}, // a variable the code names so (5/5)
    {0xFF893D, 0xFF893D}, // a variable the code names so (11/12)
    {0xFF8A5E, 0xFF8A5E}, // a variable the code names so (1/1)
    {0xFF8C00, 0xFF8C00}, // the helpers' records: where Alpha 2 has them (the votes were split) by hand
    {0xFF8C95, 0xFF8C95}, // a variable the code names so (2/2)
    {0xFF8D3D, 0xFF8D3D}, // a variable the code names so (7/11)
    {0xFF8E5E, 0xFF8E5E}, // a variable the code names so (1/1)
    {0xFF9000, 0xFF9000}, // (the second) by hand
    {0xFF913D, 0xFF913D}, // a variable the code names so (6/10)
    {0xFF925E, 0xFF925E}, // a variable the code names so (1/1)
    {0xFF9400, 0xFF9400}, // a variable the code names so (5/5)
    {0xFF9C00, 0xFF9C00}, // a variable the code names so (2/2)
    {0xFFA100, 0xFFA100}, // a variable the code names so (3/3)
    {0xFFB500, 0xFFB500}, // a variable the code names so (2/2)
    {0xFFB700, 0xFFB700}, // a variable the code names so (2/2)
    {0xFFBF00, 0xFFBF00}, // a variable the code names so (4/4)
    {0xFFCF60, 0xFFCF60}, // a variable the code names so (3/3)
    {0xFFDF60, 0xFFDF60}, // a variable the code names so (4/4)
    {0xFFDF64, 0xFFDF64}, // a variable the code names so (1/1)
    {0xFFDF65, 0xFFDF65}, // a variable the code names so (3/3)
    {0xFFDF68, 0xFFDF68}, // a variable the code names so (4/4)
    {0xFFDF69, 0xFFDF69}, // a variable the code names so (2/2)
    {0xFFE0F0, 0xFFE0F0}, // a variable the code names so (3/3)
    {0xFFE0F2, 0xFFE0F2}, // a variable the code names so (3/3)
    {0xFFE0F4, 0xFFE0F4}, // a variable the code names so (6/6)
    {0xFFE0F6, 0xFFE0F6}, // a variable the code names so (5/5)
    {0xFFE0F8, 0xFFE0F8}, // a variable the code names so (5/5)
    {0xFFE0FA, 0xFFE0FA}, // a variable the code names so (3/3)
    {0xFFE114, 0xFFE114}, // a variable the code names so (3/3)
    {0xFFE115, 0xFFE115}, // a variable the code names so (3/3)
    {0xFFE116, 0xFFE116}, // a variable the code names so (30/30)
    {0xFFE117, 0xFFE117}, // a variable the code names so (6/6)
    {0xFFE118, 0xFFE118}, // a variable the code names so (5/5)
    {0xFFE119, 0xFFE119}, // a variable the code names so (2/2)
    {0xFFE11A, 0xFFE11A}, // a variable the code names so (6/6)
    {0xFFE202, 0xFFE202}, // a variable the code names so (5/5)
    // places: end
};
inline constexpr auto kPlaces = std::to_array(kPlaceList);

// Who a player is (a player's byte +82), by number: seen on the name plates and in the fights with each number put
// in (the versus movie, the byte held). Alpha 2's 24, then the EX forms of M. Bison, Sagat, Ken and Ryu (Champion
// Edition's moves) that this game adds; 21 and 22, Alpha 2's "other forms" of Zangief and Dhalsim, are their EX forms
// too. (The tables by character have 32 rows: 28 and 29 give a Chun-Li and a Sakura with other colours and attacks of
// their own that no code on the board was seen to choose; left out.)
inline constexpr int kCharacters = 28;
inline constexpr const char *kCharacterNames[kCharacters] = {
    "Ryu", "Ken", "Akuma", "Charlie", "Chun-Li", "Adon", "Sodom", "Guy", "Birdie", "Rose", "M. Bison", "Sagat", "Dan", "Sakura",
    "Rolento", "Dhalsim", "Zangief", "Gen", "Chun-Li (classic)", "Gen (another form)", "Shin Akuma", "Zangief (EX)",
    "Dhalsim (EX)", "Evil Ryu", "M. Bison (EX)", "Sagat (EX)", "Ken (EX)", "Ryu (EX)"};
// Whose other form each of those after the first eighteen is (src/livetag.cpp: its call as it tags out is its first
// form's)
inline constexpr uint8_t kFormOf[kCharacters - 18] = {0x04, 0x11, 0x02, 0x10, 0x0F, 0x00, 0x0A, 0x0B, 0x01, 0x00};

// The hidden characters the "Hidden characters" feature gives for one who chooses the first form (src/training.cpp),
// and what it says. Ryu gives Evil Ryu (his EX form is a training session's and the modes' choice).
struct HiddenForm {
    uint8_t first, other;
};
inline constexpr HiddenForm kHiddenForms[] = {{0, 23}, {1, 26}, {2, 20}, {4, 18}, {10, 24}, {11, 25}, {15, 22}, {16, 21}};
inline constexpr const char *kHiddenWords =
    "Choosing Ryu, Ken, Akuma, Chun-Li, M. Bison, Sagat, Dhalsim or Zangief gives their hidden form instead: Evil Ryu, "
    "Shin Akuma, the classic Chun-Li, and the EX forms of the others. For player 1, player 2 or both.";

// The ROM file whose colours PalMod changes, and which set it is (src/colours.cpp)
inline constexpr const char *kColoursFile = "szaa.07", *kColoursFileWhose = "the Asian set's, 960826";

} // namespace sfa2::family
