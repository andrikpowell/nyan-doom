//
// Copyright(C) 2026 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	HHE Sprites
//

#include "doomdef.h"
#include "deh/func.h"
#include "heretic/HHE/sprites.h"

//
// While we skip the sprite block, we do use info from this in text block
//

const hhe_sprite_str_t hhe_sprites[] = {
    { {  5720,  7588,  7588 }, "IMPX" },
    { {  5728,  7596,  7596 }, "ACLO" },
    { {  5736,  7604,  7604 }, "PTN1" },
    { {  5744,  7612,  7612 }, "SHLD" },
    { {  5752,  7620,  7620 }, "SHD2" },
    { {  5760,  7628,  7628 }, "BAGH" },
    { {  5768,  7636,  7636 }, "SPMP" },
    { {  5776,  7644,  7644 }, "INVS" },
    { {  5784,  7652,  7652 }, "PTN2" },
    { {  5792,  7660,  7660 }, "SOAR" },
    { {  5800,  7668,  7668 }, "INVU" },
    { {  5808,  7676,  7676 }, "PWBK" },
    { {  5816,  7684,  7684 }, "EGGC" },
    { {  5824,  7692,  7692 }, "EGGM" },
    { {  5832,  7700,  7700 }, "FX01" },
    { {  5840,  7708,  7708 }, "SPHL" },
    { {  5848,  7716,  7716 }, "TRCH" },
    { {  5856,  7724,  7724 }, "FBMB" },
    { {  5864,  7732,  7732 }, "XPL1" },
    { {  5872,  7740,  7740 }, "ATLP" },
    { {  5880,  7748,  7748 }, "PPOD" },
    { {  5888,  7756,  7756 }, "AMG1" },
    { {  5896,  7764,  7764 }, "SPSH" },
    { {  5904,  7772,  7772 }, "LVAS" },
    { {  5912,  7780,  7780 }, "SLDG" },
    { {  5920,  7788,  7788 }, "SKH1" },
    { {  5928,  7796,  7796 }, "SKH2" },
    { {  5936,  7804,  7804 }, "SKH3" },
    { {  5944,  7812,  7812 }, "SKH4" },
    { {  5952,  7820,  7820 }, "CHDL" },
    { {  5960,  7828,  7828 }, "SRTC" },
    { {  5968,  7836,  7836 }, "SMPL" },
    { {  5976,  7844,  7844 }, "STGS" },
    { {  5984,  7852,  7852 }, "STGL" },
    { {  5992,  7860,  7860 }, "STCS" },
    { {  6000,  7868,  7868 }, "STCL" },
    { {  6008,  7876,  7876 }, "KFR1" },
    { {  6016,  7884,  7884 }, "BARL" },
    { {  6024,  7892,  7892 }, "BRPL" },
    { {  6032,  7900,  7900 }, "MOS1" },
    { {  6040,  7908,  7908 }, "MOS2" },
    { {  6048,  7916,  7916 }, "WTRH" },
    { {  6056,  7924,  7924 }, "HCOR" },
    { {  6064,  7932,  7932 }, "KGZ1" },
    { {  6072,  7940,  7940 }, "KGZB" },
    { {  6080,  7948,  7948 }, "KGZG" },
    { {  6088,  7956,  7956 }, "KGZY" },
    { {  6096,  7964,  7964 }, "VLCO" },
    { {  6104,  7972,  7972 }, "VFBL" },
    { {  6112,  7980,  7980 }, "VTFB" },
    { {  6120,  7988,  7988 }, "SFFI" },
    { {  6128,  7996,  7996 }, "TGLT" },
    { {  6136,  8004,  8004 }, "TELE" },
    { {  6144,  8012,  8012 }, "STFF" },
    { {  6152,  8020,  8020 }, "PUF3" },
    { {  6160,  8028,  8028 }, "PUF4" },
    { {  6168,  8036,  8036 }, "BEAK" },
    { {  6176,  8044,  8044 }, "WGNT" },
    { {  6184,  8052,  8052 }, "GAUN" },
    { {  6192,  8060,  8060 }, "PUF1" },
    { {  6200,  8068,  8068 }, "WBLS" },
    { {  6208,  8076,  8076 }, "BLSR" },
    { {  6216,  8084,  8084 }, "FX18" },
    { {  6224,  8092,  8092 }, "FX17" },
    { {  6232,  8100,  8100 }, "WMCE" },
    { {  6240,  8108,  8108 }, "MACE" },
    { {  6248,  8116,  8116 }, "FX02" },
    { {  6256,  8124,  8124 }, "WSKL" },
    { {  6264,  8132,  8132 }, "HROD" },
    { {  6272,  8140,  8140 }, "FX00" },
    { {  6280,  8148,  8148 }, "FX20" },
    { {  6288,  8156,  8156 }, "FX21" },
    { {  6296,  8164,  8164 }, "FX22" },
    { {  6304,  8172,  8172 }, "FX23" },
    { {  6312,  8180,  8180 }, "GWND" },
    { {  6320,  8188,  8188 }, "PUF2" },
    { {  6328,  8196,  8196 }, "WPHX" },
    { {  6336,  8204,  8204 }, "PHNX" },
    { {  6344,  8212,  8212 }, "FX04" },
    { {  6352,  8220,  8220 }, "FX08" },
    { {  6360,  8228,  8228 }, "FX09" },
    { {  6368,  8236,  8236 }, "WBOW" },
    { {  6376,  8244,  8244 }, "CRBW" },
    { {  6384,  8252,  8252 }, "FX03" },
    { {  6392,  8260,  8260 }, "BLOD" },
    { {  6400,  8268,  8268 }, "PLAY" },
    { {  6408,  8276,  8276 }, "FDTH" },
    { {  6416,  8284,  8284 }, "BSKL" },
    { {  6424,  8292,  8292 }, "CHKN" },
    { {  6432,  8300,  8300 }, "MUMM" },
    { {  6440,  8308,  8308 }, "FX15" },
    { {  6448,  8316,  8316 }, "BEAS" },
    { {  6456,  8324,  8324 }, "FRB1" },
    { {  6464,  8332,  8332 }, "SNKE" },
    { {  6472,  8340,  8340 }, "SNFX" },
    { {  6480,  8348,  8348 }, "HEAD" },
    { {  6488,  8356,  8356 }, "FX05" },
    { {  6496,  8364,  8364 }, "FX06" },
    { {  6504,  8372,  8372 }, "FX07" },
    { {  6512,  8380,  8380 }, "CLNK" },
    { {  6520,  8388,  8388 }, "WZRD" },
    { {  6528,  8396,  8396 }, "FX11" },
    { {  6536,  8404,  8404 }, "FX10" },
    { {  6544,  8412,  8412 }, "KNIG" },
    { {  6552,  8420,  8420 }, "SPAX" },
    { {  6560,  8428,  8428 }, "RAXE" },
    { {  6568,  8436,  8436 }, "SRCR" },
    { {  6576,  8444,  8444 }, "FX14" },
    { {  6584,  8452,  8452 }, "SOR2" },
    { {  6592,  8460,  8460 }, "SDTH" },
    { {  6600,  8468,  8468 }, "FX16" },
    { {  6608,  8476,  8476 }, "MNTR" },
    { {  6616,  8484,  8484 }, "FX12" },
    { {  6624,  8492,  8492 }, "FX13" },
    { {  6632,  8500,  8500 }, "AKYY" },
    { {  6640,  8508,  8508 }, "BKYY" },
    { {  6648,  8516,  8516 }, "CKYY" },
    { {  6656,  8524,  8524 }, "AMG2" },
    { {  6664,  8532,  8532 }, "AMM1" },
    { {  6672,  8540,  8540 }, "AMM2" },
    { {  6680,  8548,  8548 }, "AMC1" },
    { {  6688,  8556,  8556 }, "AMC2" },
    { {  6696,  8564,  8564 }, "AMS1" },
    { {  6704,  8572,  8572 }, "AMS2" },
    { {  6712,  8580,  8580 }, "AMP1" },
    { {  6720,  8588,  8588 }, "AMP2" },
    { {  6728,  8596,  8596 }, "AMB1" },
    { {  6736,  8604,  8604 }, "AMB2" },
};

int hhe_numsprites = sizeof(hhe_sprites) / sizeof(hhe_sprites[0]);

//
//
// NOTE - This Sprite Block is unsupported, so we skip it
//
//

// ====================================================================
// hhe_procSprite
// Purpose: Dummy - we do not support the HHE Sprite block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procSprite(DEHFILE *fpin, char *line) // Not supported
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;

  // Too little is known about what this is supposed to do, and
  // there are better ways of handling sprite renaming.  Not supported.

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Ignoring Sprite offset change at index %d: %s\n", indexnum, key);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;      // killough 11/98
    // ignore line
    deh_log("- %s\n", inbuffer);
  }
}

const deh_block hhe_block_sprite = { "Sprite", hhe_procSprite };
