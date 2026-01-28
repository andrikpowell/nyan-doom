//
// Copyright(C) 1999-2004 by Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright(C) 2005-2006 by Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
//	DEH Strings
//

#include "doomstat.h"
#include "doomdef.h"
#include "dstrings.h"  // to get the initial values

#include "deh/func.h"
#include "deh/strings.h"

// #include "d_deh.h" -- we don't do that here but we declare the
// variables.  This externalizes everything that there is a string
// set for in the language files.  See d_deh.h for detailed comments,
// original English values etc.  These are set to the macro values,
// which are set by D_ENGLSH.H or D_FRENCH.H(etc).  BEX files are a
// better way of changing these strings globally by language.

// ====================================================================
// Any of these can be changed using the bex extensions

const char *s_PRESSKEY    = PRESSKEY;
const char *s_PRESSYN     = PRESSYN;
const char *s_QUITMSG     = QUITMSG;
const char *s_QUITMSG1    = QUITMSG1;
const char *s_QUITMSG2    = QUITMSG2;
const char *s_QUITMSG3    = QUITMSG3;
const char *s_QUITMSG4    = QUITMSG4;
const char *s_QUITMSG5    = QUITMSG5;
const char *s_QUITMSG6    = QUITMSG6;
const char *s_QUITMSG7    = QUITMSG7;
const char *s_QUITMSG8    = QUITMSG8;
const char *s_QUITMSG9    = QUITMSG9;
const char *s_QUITMSG10   = QUITMSG10;
const char *s_QUITMSG11   = QUITMSG11;
const char *s_QUITMSG12   = QUITMSG12;
const char *s_QUITMSG13   = QUITMSG13;
const char *s_QUITMSG14   = QUITMSG14;
const char *s_QSAVESPOT   = QSAVESPOT; // PRESSKEY;
const char *s_SAVEDEAD    = SAVEDEAD;  // PRESSKEY; // remove duplicate y/n
const char *s_NEWGAME     = NEWGAME;   // PRESSKEY;
const char *s_RESTARTLEVEL= RESTARTLEVEL; // PRESSYN;
const char *s_NIGHTMARE   = NIGHTMARE; // PRESSYN;
const char *s_SWSTRING    = SWSTRING;  // PRESSKEY;
const char *s_MSGOFF      = MSGOFF;
const char *s_MSGON       = MSGON;
const char *s_NETEND      = NETEND;    // PRESSKEY;
const char *s_ENDGAME     = ENDGAME;   // PRESSYN; // killough 4/4/98: end
const char *s_DOSY        = DOSY;
const char *s_DETAILHI    = DETAILHI;
const char *s_DETAILLO    = DETAILLO;
const char *s_GAMMALVL0   = GAMMALVL0;
const char *s_GAMMALVL1   = GAMMALVL1;
const char *s_GAMMALVL2   = GAMMALVL2;
const char *s_GAMMALVL3   = GAMMALVL3;
const char *s_GAMMALVL4   = GAMMALVL4;
const char *s_EMPTYSTRING = EMPTYSTRING;
const char *s_GOTARMOR    = GOTARMOR;
const char *s_GOTMEGA     = GOTMEGA;
const char *s_GOTHTHBONUS = GOTHTHBONUS;
const char *s_GOTARMBONUS = GOTARMBONUS;
const char *s_GOTSTIM     = GOTSTIM;
const char *s_GOTMEDINEED = GOTMEDINEED;
const char *s_GOTMEDIKIT  = GOTMEDIKIT;
const char *s_GOTSUPER    = GOTSUPER;
const char *s_GOTBLUECARD = GOTBLUECARD;
const char *s_GOTYELWCARD = GOTYELWCARD;
const char *s_GOTREDCARD  = GOTREDCARD;
const char *s_GOTBLUESKUL = GOTBLUESKUL;
const char *s_GOTYELWSKUL = GOTYELWSKUL;
const char *s_GOTREDSKULL = GOTREDSKULL;
const char *s_GOTINVUL    = GOTINVUL;
const char *s_GOTBERSERK  = GOTBERSERK;
const char *s_GOTINVIS    = GOTINVIS;
const char *s_GOTSUIT     = GOTSUIT;
const char *s_GOTMAP      = GOTMAP;
const char *s_GOTVISOR    = GOTVISOR;
const char *s_GOTMSPHERE  = GOTMSPHERE;
const char *s_GOTCLIP     = GOTCLIP;
const char *s_GOTCLIPBOX  = GOTCLIPBOX;
const char *s_GOTROCKET   = GOTROCKET;
const char *s_GOTROCKBOX  = GOTROCKBOX;
const char *s_GOTCELL     = GOTCELL;
const char *s_GOTCELLBOX  = GOTCELLBOX;
const char *s_GOTSHELLS   = GOTSHELLS;
const char *s_GOTSHELLBOX = GOTSHELLBOX;
const char *s_GOTBACKPACK = GOTBACKPACK;
const char *s_GOTBFG9000  = GOTBFG9000;
const char *s_GOTCHAINGUN = GOTCHAINGUN;
const char *s_GOTCHAINSAW = GOTCHAINSAW;
const char *s_GOTLAUNCHER = GOTLAUNCHER;
const char *s_GOTPLASMA   = GOTPLASMA;
const char *s_GOTSHOTGUN  = GOTSHOTGUN;
const char *s_GOTSHOTGUN2 = GOTSHOTGUN2;
const char *s_PD_BLUEO    = PD_BLUEO;
const char *s_PD_REDO     = PD_REDO;
const char *s_PD_YELLOWO  = PD_YELLOWO;
const char *s_PD_BLUEK    = PD_BLUEK;
const char *s_PD_REDK     = PD_REDK;
const char *s_PD_YELLOWK  = PD_YELLOWK;
const char *s_PD_BLUEC    = PD_BLUEC;
const char *s_PD_REDC     = PD_REDC;
const char *s_PD_YELLOWC  = PD_YELLOWC;
const char *s_PD_BLUES    = PD_BLUES;
const char *s_PD_REDS     = PD_REDS;
const char *s_PD_YELLOWS  = PD_YELLOWS;
const char *s_PD_ANY      = PD_ANY;
const char *s_PD_ALL3     = PD_ALL3;
const char *s_PD_ALL6     = PD_ALL6;
const char *s_GGSAVED     = GGSAVED;
const char *s_HUSTR_MSGU  = HUSTR_MSGU;
const char *s_HUSTR_E1M1  = HUSTR_E1M1;
const char *s_HUSTR_E1M2  = HUSTR_E1M2;
const char *s_HUSTR_E1M3  = HUSTR_E1M3;
const char *s_HUSTR_E1M4  = HUSTR_E1M4;
const char *s_HUSTR_E1M5  = HUSTR_E1M5;
const char *s_HUSTR_E1M6  = HUSTR_E1M6;
const char *s_HUSTR_E1M7  = HUSTR_E1M7;
const char *s_HUSTR_E1M8  = HUSTR_E1M8;
const char *s_HUSTR_E1M9  = HUSTR_E1M9;
const char *s_HUSTR_E2M1  = HUSTR_E2M1;
const char *s_HUSTR_E2M2  = HUSTR_E2M2;
const char *s_HUSTR_E2M3  = HUSTR_E2M3;
const char *s_HUSTR_E2M4  = HUSTR_E2M4;
const char *s_HUSTR_E2M5  = HUSTR_E2M5;
const char *s_HUSTR_E2M6  = HUSTR_E2M6;
const char *s_HUSTR_E2M7  = HUSTR_E2M7;
const char *s_HUSTR_E2M8  = HUSTR_E2M8;
const char *s_HUSTR_E2M9  = HUSTR_E2M9;
const char *s_HUSTR_E3M1  = HUSTR_E3M1;
const char *s_HUSTR_E3M2  = HUSTR_E3M2;
const char *s_HUSTR_E3M3  = HUSTR_E3M3;
const char *s_HUSTR_E3M4  = HUSTR_E3M4;
const char *s_HUSTR_E3M5  = HUSTR_E3M5;
const char *s_HUSTR_E3M6  = HUSTR_E3M6;
const char *s_HUSTR_E3M7  = HUSTR_E3M7;
const char *s_HUSTR_E3M8  = HUSTR_E3M8;
const char *s_HUSTR_E3M9  = HUSTR_E3M9;
const char *s_HUSTR_E4M1  = HUSTR_E4M1;
const char *s_HUSTR_E4M2  = HUSTR_E4M2;
const char *s_HUSTR_E4M3  = HUSTR_E4M3;
const char *s_HUSTR_E4M4  = HUSTR_E4M4;
const char *s_HUSTR_E4M5  = HUSTR_E4M5;
const char *s_HUSTR_E4M6  = HUSTR_E4M6;
const char *s_HUSTR_E4M7  = HUSTR_E4M7;
const char *s_HUSTR_E4M8  = HUSTR_E4M8;
const char *s_HUSTR_E4M9  = HUSTR_E4M9;
const char *s_HUSTR_1     = HUSTR_1;
const char *s_HUSTR_2     = HUSTR_2;
const char *s_HUSTR_3     = HUSTR_3;
const char *s_HUSTR_4     = HUSTR_4;
const char *s_HUSTR_5     = HUSTR_5;
const char *s_HUSTR_6     = HUSTR_6;
const char *s_HUSTR_7     = HUSTR_7;
const char *s_HUSTR_8     = HUSTR_8;
const char *s_HUSTR_9     = HUSTR_9;
const char *s_HUSTR_10    = HUSTR_10;
const char *s_HUSTR_11    = HUSTR_11;
const char *s_HUSTR_12    = HUSTR_12;
const char *s_HUSTR_13    = HUSTR_13;
const char *s_HUSTR_14    = HUSTR_14;
const char *s_HUSTR_15    = HUSTR_15;
const char *s_HUSTR_16    = HUSTR_16;
const char *s_HUSTR_17    = HUSTR_17;
const char *s_HUSTR_18    = HUSTR_18;
const char *s_HUSTR_19    = HUSTR_19;
const char *s_HUSTR_20    = HUSTR_20;
const char *s_HUSTR_21    = HUSTR_21;
const char *s_HUSTR_22    = HUSTR_22;
const char *s_HUSTR_23    = HUSTR_23;
const char *s_HUSTR_24    = HUSTR_24;
const char *s_HUSTR_25    = HUSTR_25;
const char *s_HUSTR_26    = HUSTR_26;
const char *s_HUSTR_27    = HUSTR_27;
const char *s_HUSTR_28    = HUSTR_28;
const char *s_HUSTR_29    = HUSTR_29;
const char *s_HUSTR_30    = HUSTR_30;
const char *s_HUSTR_31    = HUSTR_31;
const char *s_HUSTR_32    = HUSTR_32;
const char *s_HUSTR_33    = HUSTR_33;
const char *s_PHUSTR_1    = PHUSTR_1;
const char *s_PHUSTR_2    = PHUSTR_2;
const char *s_PHUSTR_3    = PHUSTR_3;
const char *s_PHUSTR_4    = PHUSTR_4;
const char *s_PHUSTR_5    = PHUSTR_5;
const char *s_PHUSTR_6    = PHUSTR_6;
const char *s_PHUSTR_7    = PHUSTR_7;
const char *s_PHUSTR_8    = PHUSTR_8;
const char *s_PHUSTR_9    = PHUSTR_9;
const char *s_PHUSTR_10   = PHUSTR_10;
const char *s_PHUSTR_11   = PHUSTR_11;
const char *s_PHUSTR_12   = PHUSTR_12;
const char *s_PHUSTR_13   = PHUSTR_13;
const char *s_PHUSTR_14   = PHUSTR_14;
const char *s_PHUSTR_15   = PHUSTR_15;
const char *s_PHUSTR_16   = PHUSTR_16;
const char *s_PHUSTR_17   = PHUSTR_17;
const char *s_PHUSTR_18   = PHUSTR_18;
const char *s_PHUSTR_19   = PHUSTR_19;
const char *s_PHUSTR_20   = PHUSTR_20;
const char *s_PHUSTR_21   = PHUSTR_21;
const char *s_PHUSTR_22   = PHUSTR_22;
const char *s_PHUSTR_23   = PHUSTR_23;
const char *s_PHUSTR_24   = PHUSTR_24;
const char *s_PHUSTR_25   = PHUSTR_25;
const char *s_PHUSTR_26   = PHUSTR_26;
const char *s_PHUSTR_27   = PHUSTR_27;
const char *s_PHUSTR_28   = PHUSTR_28;
const char *s_PHUSTR_29   = PHUSTR_29;
const char *s_PHUSTR_30   = PHUSTR_30;
const char *s_PHUSTR_31   = PHUSTR_31;
const char *s_PHUSTR_32   = PHUSTR_32;
const char *s_THUSTR_1    = THUSTR_1;
const char *s_THUSTR_2    = THUSTR_2;
const char *s_THUSTR_3    = THUSTR_3;
const char *s_THUSTR_4    = THUSTR_4;
const char *s_THUSTR_5    = THUSTR_5;
const char *s_THUSTR_6    = THUSTR_6;
const char *s_THUSTR_7    = THUSTR_7;
const char *s_THUSTR_8    = THUSTR_8;
const char *s_THUSTR_9    = THUSTR_9;
const char *s_THUSTR_10   = THUSTR_10;
const char *s_THUSTR_11   = THUSTR_11;
const char *s_THUSTR_12   = THUSTR_12;
const char *s_THUSTR_13   = THUSTR_13;
const char *s_THUSTR_14   = THUSTR_14;
const char *s_THUSTR_15   = THUSTR_15;
const char *s_THUSTR_16   = THUSTR_16;
const char *s_THUSTR_17   = THUSTR_17;
const char *s_THUSTR_18   = THUSTR_18;
const char *s_THUSTR_19   = THUSTR_19;
const char *s_THUSTR_20   = THUSTR_20;
const char *s_THUSTR_21   = THUSTR_21;
const char *s_THUSTR_22   = THUSTR_22;
const char *s_THUSTR_23   = THUSTR_23;
const char *s_THUSTR_24   = THUSTR_24;
const char *s_THUSTR_25   = THUSTR_25;
const char *s_THUSTR_26   = THUSTR_26;
const char *s_THUSTR_27   = THUSTR_27;
const char *s_THUSTR_28   = THUSTR_28;
const char *s_THUSTR_29   = THUSTR_29;
const char *s_THUSTR_30   = THUSTR_30;
const char *s_THUSTR_31   = THUSTR_31;
const char *s_THUSTR_32   = THUSTR_32;
const char *s_AMSTR_FOLLOWON     = AMSTR_FOLLOWON;
const char *s_AMSTR_FOLLOWOFF    = AMSTR_FOLLOWOFF;
const char *s_AMSTR_GRIDON       = AMSTR_GRIDON;
const char *s_AMSTR_GRIDOFF      = AMSTR_GRIDOFF;
const char *s_AMSTR_MARKEDSPOT   = AMSTR_MARKEDSPOT;
const char *s_AMSTR_MARKSCLEARED = AMSTR_MARKSCLEARED;
// CPhipps - automap rotate & overlay
const char* s_AMSTR_ROTATEON     = AMSTR_ROTATEON;
const char* s_AMSTR_ROTATEOFF    = AMSTR_ROTATEOFF;
const char* s_AMSTR_OVERLAYON    = AMSTR_OVERLAYON;
const char* s_AMSTR_OVERLAYOFF   = AMSTR_OVERLAYOFF;
// e6y: textured automap
const char* s_AMSTR_TEXTUREDON   = AMSTR_TEXTUREDON;
const char* s_AMSTR_TEXTUREDOFF  = AMSTR_TEXTUREDOFF;

const char *s_STSTR_MUS          = STSTR_MUS;
const char *s_STSTR_NOMUS        = STSTR_NOMUS;
const char *s_STSTR_DQDON        = STSTR_DQDON;
const char *s_STSTR_DQDOFF       = STSTR_DQDOFF;
const char *s_STSTR_KFAADDED     = STSTR_KFAADDED;
const char *s_STSTR_FAADDED      = STSTR_FAADDED;
const char *s_STSTR_NCON         = STSTR_NCON;
const char *s_STSTR_NCOFF        = STSTR_NCOFF;
const char *s_STSTR_BEHOLD       = STSTR_BEHOLD;
const char *s_STSTR_BEHOLDX      = STSTR_BEHOLDX;
const char *s_STSTR_CHOPPERS     = STSTR_CHOPPERS;
const char *s_STSTR_CLEV         = STSTR_CLEV;
const char *s_STSTR_COMPON       = STSTR_COMPON;
const char *s_STSTR_COMPOFF      = STSTR_COMPOFF;
const char *s_E1TEXT     = E1TEXT;
const char *s_E2TEXT     = E2TEXT;
const char *s_E3TEXT     = E3TEXT;
const char *s_E4TEXT     = E4TEXT;
const char *s_C1TEXT     = C1TEXT;
const char *s_C2TEXT     = C2TEXT;
const char *s_C3TEXT     = C3TEXT;
const char *s_C4TEXT     = C4TEXT;
const char *s_C5TEXT     = C5TEXT;
const char *s_C6TEXT     = C6TEXT;
const char *s_P1TEXT     = P1TEXT;
const char *s_P2TEXT     = P2TEXT;
const char *s_P3TEXT     = P3TEXT;
const char *s_P4TEXT     = P4TEXT;
const char *s_P5TEXT     = P5TEXT;
const char *s_P6TEXT     = P6TEXT;
const char *s_T1TEXT     = T1TEXT;
const char *s_T2TEXT     = T2TEXT;
const char *s_T3TEXT     = T3TEXT;
const char *s_T4TEXT     = T4TEXT;
const char *s_T5TEXT     = T5TEXT;
const char *s_T6TEXT     = T6TEXT;
const char *s_CC_ZOMBIE  = CC_ZOMBIE;
const char *s_CC_SHOTGUN = CC_SHOTGUN;
const char *s_CC_HEAVY   = CC_HEAVY;
const char *s_CC_IMP     = CC_IMP;
const char *s_CC_DEMON   = CC_DEMON;
const char *s_CC_LOST    = CC_LOST;
const char *s_CC_CACO    = CC_CACO;
const char *s_CC_HELL    = CC_HELL;
const char *s_CC_BARON   = CC_BARON;
const char *s_CC_ARACH   = CC_ARACH;
const char *s_CC_PAIN    = CC_PAIN;
const char *s_CC_REVEN   = CC_REVEN;
const char *s_CC_MANCU   = CC_MANCU;
const char *s_CC_ARCH    = CC_ARCH;
const char *s_CC_SPIDER  = CC_SPIDER;
const char *s_CC_CYBER   = CC_CYBER;
const char *s_CC_HERO    = CC_HERO;
// Ty 03/30/98 - new substitutions for background textures
//               during int screens
const char *bgflatE1     = "FLOOR4_8"; // end of DOOM Episode 1
const char *bgflatE2     = "SFLR6_1";  // end of DOOM Episode 2
const char *bgflatE3     = "MFLR8_4";  // end of DOOM Episode 3
const char *bgflatE4     = "MFLR8_3";  // end of DOOM Episode 4
const char *bgflat06     = "SLIME16";  // DOOM2 after MAP06
const char *bgflat11     = "RROCK14";  // DOOM2 after MAP11
const char *bgflat20     = "RROCK07";  // DOOM2 after MAP20
const char *bgflat30     = "RROCK17";  // DOOM2 after MAP30
const char *bgflat15     = "RROCK13";  // DOOM2 going MAP15 to MAP31
const char *bgflat31     = "RROCK19";  // DOOM2 going MAP31 to MAP32
const char *bgcastcall   = "BOSSBACK"; // Panel behind cast call
// data access Disk
const char *s_stcdrom      = "STCDROM";
const char *s_stdisk       = "STDISK";

/* Ty 05/03/98 - externalized
 * cph - updated for prboom */
const char *savegamename = PROJECT_TARNAME"-savegame";

// obituaries
const char *s_OB_CRUSH = OB_CRUSH;
const char *s_OB_FALLING = OB_FALLING;
const char *s_OB_SLIME = OB_SLIME;
const char *s_OB_LAVA = OB_LAVA;
const char *s_OB_KILLEDSELF = OB_KILLEDSELF;
const char *s_OB_VOODOO = OB_VOODOO;
const char *s_OB_MONTELEFRAG = OB_MONTELEFRAG;
const char *s_OB_DEFAULT = OB_DEFAULT;
const char *s_OB_MPDEFAULT = OB_MPDEFAULT;

const char *s_OB_UNDEADHIT = OB_UNDEADHIT;
const char *s_OB_IMPHIT = OB_IMPHIT;
const char *s_OB_CACOHIT = OB_CACOHIT;
const char *s_OB_DEMONHIT = OB_DEMONHIT;
const char *s_OB_SPECTREHIT = OB_SPECTREHIT;
const char *s_OB_BARONHIT = OB_BARONHIT;
const char *s_OB_KNIGHTHIT = OB_KNIGHTHIT;

const char *s_OB_ZOMBIE = OB_ZOMBIE;
const char *s_OB_SHOTGUY = OB_SHOTGUY;
const char *s_OB_VILE = OB_VILE;
const char *s_OB_UNDEAD = OB_UNDEAD;
const char *s_OB_FATSO = OB_FATSO;
const char *s_OB_CHAINGUY = OB_CHAINGUY;
const char *s_OB_SKULL = OB_SKULL;
const char *s_OB_IMP = OB_IMP;
const char *s_OB_CACO = OB_CACO;
const char *s_OB_BARON = OB_BARON;
const char *s_OB_KNIGHT = OB_KNIGHT;
const char *s_OB_SPIDER = OB_SPIDER;
const char *s_OB_BABY = OB_BABY;
const char *s_OB_CYBORG = OB_CYBORG;
const char *s_OB_WOLFSS = OB_WOLFSS;

const char *s_OB_MPFIST = OB_MPFIST;
const char *s_OB_MPCHAINSAW = OB_MPCHAINSAW;
const char *s_OB_MPPISTOL = OB_MPPISTOL;
const char *s_OB_MPSHOTGUN = OB_MPSHOTGUN;
const char *s_OB_MPSSHOTGUN = OB_MPSSHOTGUN;
const char *s_OB_MPCHAINGUN = OB_MPCHAINGUN;
const char *s_OB_MPROCKET = OB_MPROCKET;
const char *s_OB_MPPLASMARIFLE = OB_MPPLASMARIFLE;
const char *s_OB_MPBFG_BOOM = OB_MPBFG_BOOM;
const char *s_OB_MPTELEFRAG = OB_MPTELEFRAG;

const char *deh_newlevel = "NEWLEVEL"; // CPhipps - const

// DOOM shareware/registered/retail (Ultimate) names.
// CPhipps - const**const
const char **const mapnames[] =
{
  &s_HUSTR_E1M1,
  &s_HUSTR_E1M2,
  &s_HUSTR_E1M3,
  &s_HUSTR_E1M4,
  &s_HUSTR_E1M5,
  &s_HUSTR_E1M6,
  &s_HUSTR_E1M7,
  &s_HUSTR_E1M8,
  &s_HUSTR_E1M9,

  &s_HUSTR_E2M1,
  &s_HUSTR_E2M2,
  &s_HUSTR_E2M3,
  &s_HUSTR_E2M4,
  &s_HUSTR_E2M5,
  &s_HUSTR_E2M6,
  &s_HUSTR_E2M7,
  &s_HUSTR_E2M8,
  &s_HUSTR_E2M9,

  &s_HUSTR_E3M1,
  &s_HUSTR_E3M2,
  &s_HUSTR_E3M3,
  &s_HUSTR_E3M4,
  &s_HUSTR_E3M5,
  &s_HUSTR_E3M6,
  &s_HUSTR_E3M7,
  &s_HUSTR_E3M8,
  &s_HUSTR_E3M9,

  &s_HUSTR_E4M1,
  &s_HUSTR_E4M2,
  &s_HUSTR_E4M3,
  &s_HUSTR_E4M4,
  &s_HUSTR_E4M5,
  &s_HUSTR_E4M6,
  &s_HUSTR_E4M7,
  &s_HUSTR_E4M8,
  &s_HUSTR_E4M9,

  &deh_newlevel,  // spares?  Unused.
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,

  &deh_newlevel,  // spares?  Unused.
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel
};

// CPhipps - const**const
const char **const mapnames2[] = // DOOM 2 map names.
{
  &s_HUSTR_1,
  &s_HUSTR_2,
  &s_HUSTR_3,
  &s_HUSTR_4,
  &s_HUSTR_5,
  &s_HUSTR_6,
  &s_HUSTR_7,
  &s_HUSTR_8,
  &s_HUSTR_9,
  &s_HUSTR_10,
  &s_HUSTR_11,

  &s_HUSTR_12,
  &s_HUSTR_13,
  &s_HUSTR_14,
  &s_HUSTR_15,
  &s_HUSTR_16,
  &s_HUSTR_17,
  &s_HUSTR_18,
  &s_HUSTR_19,
  &s_HUSTR_20,

  &s_HUSTR_21,
  &s_HUSTR_22,
  &s_HUSTR_23,
  &s_HUSTR_24,
  &s_HUSTR_25,
  &s_HUSTR_26,
  &s_HUSTR_27,
  &s_HUSTR_28,
  &s_HUSTR_29,
  &s_HUSTR_30,
  &s_HUSTR_31,
  &s_HUSTR_32,
  &s_HUSTR_33,
};

// CPhipps - const**const
const char **const mapnamesp[] = // Plutonia WAD map names.
{
  &s_PHUSTR_1,
  &s_PHUSTR_2,
  &s_PHUSTR_3,
  &s_PHUSTR_4,
  &s_PHUSTR_5,
  &s_PHUSTR_6,
  &s_PHUSTR_7,
  &s_PHUSTR_8,
  &s_PHUSTR_9,
  &s_PHUSTR_10,
  &s_PHUSTR_11,

  &s_PHUSTR_12,
  &s_PHUSTR_13,
  &s_PHUSTR_14,
  &s_PHUSTR_15,
  &s_PHUSTR_16,
  &s_PHUSTR_17,
  &s_PHUSTR_18,
  &s_PHUSTR_19,
  &s_PHUSTR_20,

  &s_PHUSTR_21,
  &s_PHUSTR_22,
  &s_PHUSTR_23,
  &s_PHUSTR_24,
  &s_PHUSTR_25,
  &s_PHUSTR_26,
  &s_PHUSTR_27,
  &s_PHUSTR_28,
  &s_PHUSTR_29,
  &s_PHUSTR_30,
  &s_PHUSTR_31,
  &s_PHUSTR_32,
};

// CPhipps - const**const
const char **const mapnamest[] = // TNT WAD map names.
{
  &s_THUSTR_1,
  &s_THUSTR_2,
  &s_THUSTR_3,
  &s_THUSTR_4,
  &s_THUSTR_5,
  &s_THUSTR_6,
  &s_THUSTR_7,
  &s_THUSTR_8,
  &s_THUSTR_9,
  &s_THUSTR_10,
  &s_THUSTR_11,

  &s_THUSTR_12,
  &s_THUSTR_13,
  &s_THUSTR_14,
  &s_THUSTR_15,
  &s_THUSTR_16,
  &s_THUSTR_17,
  &s_THUSTR_18,
  &s_THUSTR_19,
  &s_THUSTR_20,

  &s_THUSTR_21,
  &s_THUSTR_22,
  &s_THUSTR_23,
  &s_THUSTR_24,
  &s_THUSTR_25,
  &s_THUSTR_26,
  &s_THUSTR_27,
  &s_THUSTR_28,
  &s_THUSTR_29,
  &s_THUSTR_30,
  &s_THUSTR_31,
  &s_THUSTR_32,
};

#define DEH_NEWLEVEL "NEW LEVEL"

const char* og_mapnames[] =
{
  HUSTR_E1M1, HUSTR_E1M2, HUSTR_E1M3, HUSTR_E1M4, HUSTR_E1M5, HUSTR_E1M6, HUSTR_E1M7, HUSTR_E1M8, HUSTR_E1M9,
  HUSTR_E2M1, HUSTR_E2M2, HUSTR_E2M3, HUSTR_E2M4, HUSTR_E2M5, HUSTR_E2M6, HUSTR_E2M7, HUSTR_E2M8, HUSTR_E2M9,
  HUSTR_E3M1, HUSTR_E3M2, HUSTR_E3M3, HUSTR_E3M4,HUSTR_E3M5, HUSTR_E3M6, HUSTR_E3M7, HUSTR_E3M8, HUSTR_E3M9,
  HUSTR_E4M1, HUSTR_E4M2, HUSTR_E4M3, HUSTR_E4M4, HUSTR_E4M5, HUSTR_E4M6, HUSTR_E4M7, HUSTR_E4M8, HUSTR_E4M9,
  DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL,
  DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL, DEH_NEWLEVEL,
};

const char* og_mapnames2[] =
{
  HUSTR_1, HUSTR_2, HUSTR_3, HUSTR_4, HUSTR_5, HUSTR_6, HUSTR_7, HUSTR_8, HUSTR_9, HUSTR_10, HUSTR_11,
  HUSTR_12, HUSTR_13, HUSTR_14, HUSTR_15, HUSTR_16, HUSTR_17, HUSTR_18, HUSTR_19, HUSTR_20, HUSTR_21,
  HUSTR_22, HUSTR_23, HUSTR_24, HUSTR_25, HUSTR_26, HUSTR_27, HUSTR_28, HUSTR_29, HUSTR_30, HUSTR_31, HUSTR_32, HUSTR_33,
};

const char* og_mapnamesp[] =
{
  PHUSTR_1, PHUSTR_2, PHUSTR_3, PHUSTR_4, PHUSTR_5, PHUSTR_6, PHUSTR_7, PHUSTR_8, PHUSTR_9, PHUSTR_10, PHUSTR_11,
  PHUSTR_12, PHUSTR_13, PHUSTR_14, PHUSTR_15, PHUSTR_16, PHUSTR_17, PHUSTR_18, PHUSTR_19, PHUSTR_20, PHUSTR_21,
  PHUSTR_22, PHUSTR_23, PHUSTR_24, PHUSTR_25, PHUSTR_26, PHUSTR_27, PHUSTR_28, PHUSTR_29, PHUSTR_30, PHUSTR_31, PHUSTR_32,
};

const char* og_mapnamest[] =
{
  THUSTR_1, THUSTR_2, THUSTR_3, THUSTR_4, THUSTR_5, THUSTR_6, THUSTR_7, THUSTR_8, THUSTR_9, THUSTR_10, THUSTR_11,
  THUSTR_12, THUSTR_13, THUSTR_14, THUSTR_15, THUSTR_16, THUSTR_17, THUSTR_18, THUSTR_19, THUSTR_20, THUSTR_21,
  THUSTR_22, THUSTR_23, THUSTR_24, THUSTR_25, THUSTR_26, THUSTR_27, THUSTR_28, THUSTR_29, THUSTR_30, THUSTR_31, THUSTR_32,
};
