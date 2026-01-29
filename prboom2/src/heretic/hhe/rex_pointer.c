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
//	HHE [CODEPTR] Parser
//

#include "heretic/p_action.h"
#include "e6y.h"//e6y
#include "doomdef.h"

#include "heretic/hhe/rex_pointer.h"
#include "heretic/hhe/frame.h"
#include "dsda/state.h"

// REX extension [CODEPTR]
// Usage: Start block, then each line is:
// FRAME nnn = PointerMnemonic

const hhe_rexptr hhe_rexptrs[] = // CPhipps - static const
{
  { {  77680,  80144,  80208 },   A_AccTeleGlitter,       "A_AccTeleGlitter" },
  { {  78608,  81104,  81168 },   A_AddPlayerCorpse,      "A_AddPlayerCorpse" },
  { { 115808, 118000, 118240 },   A_AddPlayerRain,        "A_AddPlayerRain" },
  { { 112272, 114480, 114720 },   A_BeakAttackPL1,        "A_BeakAttackPL1" },
  { { 112448, 114656, 114896 },   A_BeakAttackPL2,        "A_BeakAttackPL2" },
  { { 111856, 114176, 114416 },   A_BeakRaise,            "A_BeakRaise" },
  { { 111568, 113888, 114128 },   A_BeakReady,            "A_BeakReady" },
  { {  74640,  77120,  77184 },   A_BeastAttack,          "A_BeastAttack" },
  { {  70480,  72992,  73056 },   A_BeastPuff,            "A_BeastPuff" },
  { {  73120,  75600,  75664 },   A_BlueSpark,            "A_BlueSpark" },
  { { 115456, 117648, 117888 },   A_BoltSpark,            "A_BoltSpark" },
  { {  77344,  79808,  79872 },   A_BossDeath,            "A_BossDeath" },
  { {  69328,  71856,  71920 },   A_Chase,                "A_Chase" },
  { {      0,  80976,  81040 },   A_CheckBurnGone,        "A_CheckBurnGone" },
  { {  78480,  80944,  81008 },   A_CheckSkullDone,       "A_CheckSkullDone" },
  { {  78448,  80912,  80976 },   A_CheckSkullFloor,      "A_CheckSkullFloor" },
  { {  71376,  73888,  73952 },   A_ChicAttack,           "A_ChicAttack" },
  { {  71488,  74000,  74064 },   A_ChicChase,            "A_ChicChase" },
  { {  71456,  73968,  74032 },   A_ChicLook,             "A_ChicLook" },
  { {  71520,  74032,  74096 },   A_ChicPain,             "A_ChicPain" },
  { {  75792,  78208,  78272 },   A_ClinkAttack,          "A_ClinkAttack" },
  { { 108432, 110816, 111056 },   A_ContMobjSound,        "A_ContMobjSound" },
  { { 114752, 116944, 117184 },   A_DeathBallImpact,      "A_DeathBallImpact" },
  { {  70016,  72528,  72592 },   A_DripBlood,            "A_DripBlood" },
  { {  77472,  79936,  80000 },   A_ESound,               "A_ESound" },
  { {  76784,  79248,  79312 },   A_Explode,              "A_Explode" },
  { {  69872,  72400,  72464 },   A_FaceTarget,           "A_FaceTarget" },
  { {  71568,  74080,  74144 },   A_Feathers,             "A_Feathers" },
  { { 112928, 115136, 115376 },   A_FireBlasterPL1,       "A_FireBlasterPL1" },
  { { 113072, 115280, 115520 },   A_FireBlasterPL2,       "A_FireBlasterPL2" },
  { { 115232, 117424, 117664 },   A_FireCrossbowPL1,      "A_FireCrossbowPL1" },
  { { 115312, 117504, 117744 },   A_FireCrossbowPL2,      "A_FireCrossbowPL2" },
  { { 113152, 115360, 115600 },   A_FireGoldWandPL1,      "A_FireGoldWandPL1" },
  { { 113296, 115504, 115744 },   A_FireGoldWandPL2,      "A_FireGoldWandPL2" },
  { { 113760, 115968, 116208 },   A_FireMacePL1,          "A_FireMacePL1" },
  { { 114624, 116816, 117056 },   A_FireMacePL2,          "A_FireMacePL2" },
  { { 116368, 118544, 118784 },   A_FirePhoenixPL1,       "A_FirePhoenixPL1" },
  { { 116736, 118896, 119136 },   A_FirePhoenixPL2,       "A_FirePhoenixPL2" },
  { { 115568, 117760, 118000 },   A_FireSkullRodPL1,      "A_FireSkullRodPL1" },
  { { 115648, 117840, 118080 },   A_FireSkullRodPL2,      "A_FireSkullRodPL2" },
  { { 117120, 119280, 119520 },   A_FlameEnd,             "A_FlameEnd" },
  { {  78704,  81200,  81264 },   A_FlameSnd,             "A_FlameSnd" },
  { { 117152, 119312, 119552 },   A_FloatPuff,            "A_FloatPuff" },
  { {  78512,  81008,  81072 },   A_FreeTargMobj,         "A_FreeTargMobj" },
  { { 117184, 119344, 119584 },   A_GauntletAttack,       "A_GauntletAttack" },
  { {  73232,  75712,  75776 },   A_GenWizard,            "A_GenWizard" },
  { {  75872,  78304,  78368 },   A_GhostOff,             "A_GhostOff" },
  { {  74752,  77232,  77296 },   A_HeadAttack,           "A_HeadAttack" },
  { {  75488,  77984,  78048 },   A_HeadFireGrow,         "A_HeadFireGrow" },
  { {  75328,  77824,  77888 },   A_HeadIceImpact,        "A_HeadIceImpact" },
  { { 116336, 118512, 118752 },   A_HideInCeiling,        "A_HideInCeiling" },
  { {  78736,  81232,  81296 },   A_HideThing,            "A_HideThing" },
  { {  70976,  73488,  73552 },   A_ImpDeath,             "A_ImpDeath" },
  { {  70304,  72816,  72880 },   A_ImpExplode,           "A_ImpExplode" },
  { {  70592,  73104,  73168 },   A_ImpMeAttack,          "A_ImpMeAttack" },
  { {  70672,  73184,  73248 },   A_ImpMsAttack,          "A_ImpMsAttack" },
  { {  70880,  73392,  73456 },   A_ImpMsAttack2,         "A_ImpMsAttack2" },
  { {  71024,  73536,  73600 },   A_ImpXDeath1,           "A_ImpXDeath1" },
  { {  71072,  73584,  73648 },   A_ImpXDeath2,           "A_ImpXDeath2" },
  { {  77728,  80192,  80256 },   A_InitKeyGizmo,         "A_InitKeyGizmo" },
  { { 116720, 118880, 119120 },   A_InitPhoenixPL2,       "A_InitPhoenixPL2" },
  { {  70160,  72672,  72736 },   A_KnightAttack,         "A_KnightAttack" },
  { { 117648, 119824, 120064 },   A_Light0,               "A_Light0" },
  { {  69200,  71728,  71792 },   A_Look,                 "A_Look" },
  { { 111760, 114080, 114320 },   A_Lower,                "A_Lower" },
  { { 114032, 116224, 116464 },   A_MaceBallImpact,       "A_MaceBallImpact" },
  { { 114192, 116384, 116624 },   A_MaceBallImpact2,      "A_MaceBallImpact2" },
  { { 113904, 116112, 116352 },   A_MacePL1Check,         "A_MacePL1Check" },
  { {  77104,  79568,  79632 },   A_MakePod,              "A_MakePod" },
  { {  73648,  76128,  76192 },   A_MinotaurAtk1,         "A_MinotaurAtk1" },
  { {  74112,  76592,  76656 },   A_MinotaurAtk2,         "A_MinotaurAtk2" },
  { {  74352,  76832,  76896 },   A_MinotaurAtk3,         "A_MinotaurAtk3" },
  { {  74032,  76512,  76576 },   A_MinotaurCharge,       "A_MinotaurCharge" },
  { {  73760,  76240,  76304 },   A_MinotaurDecide,       "A_MinotaurDecide" },
  { {  74528,  77008,  77072 },   A_MntrFloorFire,        "A_MntrFloorFire" },
  { {  71808,  74288,  74352 },   A_MummyAttack,          "A_MummyAttack" },
  { {  71920,  74400,  74464 },   A_MummyAttack2,         "A_MummyAttack2" },
  { {  72016,  74496,  74560 },   A_MummyFX1Seek,         "A_MummyFX1Seek" },
  { {  72048,  74528,  74592 },   A_MummySoul,            "A_MummySoul" },
  { {  76400,  78832,  78896 },   A_NoBlocking,           "A_NoBlocking" },
  { {  69984,  72496,  72560 },   A_Pain,                 "A_Pain" },
  { { 116496, 118656, 118896 },   A_PhoenixPuff,          "A_PhoenixPuff" },
  { {  76896,  79360,  79424 },   A_PodPain,              "A_PodPain" },
  { { 116272, 118448, 118688 },   A_RainImpact,           "A_RainImpact" },
  { { 111920, 114240, 114480 },   A_Raise,                "A_Raise" },
  { { 111696, 114016, 114256 },   A_ReFire,               "A_ReFire" },
  { {  77056,  79520,  79584 },   A_RemovePod,            "A_RemovePod" },
  { { 116480,      0,      0 },   A_RemovedPhoenixFunc,   "A_RemovedPhoenixFunc" },
  { {  81952,  84464,  84528 },   A_RestoreArtifact,      "A_RestoreArtifact" },
  { {  82048,  84544,  84608 },   A_RestoreSpecialThing1, "A_RestoreSpecialThing1" },
  { {  82128,  84592,  84656 },   A_RestoreSpecialThing2, "A_RestoreSpecialThing2" },
  { {  76144,  78576,  78640 },   A_Scream,               "A_Scream" },
  { { 117104, 119264, 119504 },   A_ShutdownPhoenixPL2,   "A_ShutdownPhoenixPL2" },
  { {  78288,  80752,  80816 },   A_SkullPop,             "A_SkullPop" },
  { { 115776, 117968, 118208 },   A_SkullRodPL2Seek,      "A_SkullRodPL2Seek" },
  { { 115984, 118176, 118416 },   A_SkullRodStorm,        "A_SkullRodStorm" },
  { {  75632,  78048,  78112 },   A_SnakeAttack,          "A_SnakeAttack" },
  { {  75712,  78128,  78192 },   A_SnakeAttack2,         "A_SnakeAttack2" },
  { {  72144,  74624,  74688 },   A_Sor1Chase,            "A_Sor1Chase" },
  { {  72096,  74576,  74640 },   A_Sor1Pain,             "A_Sor1Pain" },
  { {  73392,  75872,  75936 },   A_Sor2DthInit,          "A_Sor2DthInit" },
  { {  73424,  75904,  75968 },   A_Sor2DthLoop,          "A_Sor2DthLoop" },
  { {  73584,  76064,  76128 },   A_SorDBon,              "A_SorDBon" },
  { {  73552,  76032,  76096 },   A_SorDExp,              "A_SorDExp" },
  { {  73520,  76000,  76064 },   A_SorDSph,              "A_SorDSph" },
  { {  73488,  75968,  76032 },   A_SorRise,              "A_SorRise" },
  { {  73616,  76096,  76160 },   A_SorSightSnd,          "A_SorSightSnd" },
  { {  73456,  75936,  76000 },   A_SorZap,               "A_SorZap" },
  { {  72480,  74960,  75024 },   A_SorcererRise,         "A_SorcererRise" },
  { { 115088, 117280, 117520 },   A_SpawnRippers,         "A_SpawnRippers" },
  { {  77520,  79984,  80048 },   A_SpawnTeleGlitter,     "A_SpawnTeleGlitter" },
  { {  77600,  80064,  80128 },   A_SpawnTeleGlitter2,    "A_SpawnTeleGlitter2" },
  { {  72192,  74672,  74736 },   A_Srcr1Attack,          "A_Srcr1Attack" },
  { {  72896,  75376,  75440 },   A_Srcr2Attack,          "A_Srcr2Attack" },
  { {  72816,  75296,  75360 },   A_Srcr2Decide,          "A_Srcr2Decide" },
  { { 112640, 114848, 115088 },   A_StaffAttackPL1,       "A_StaffAttackPL1" },
  { { 112784, 114992, 115232 },   A_StaffAttackPL2,       "A_StaffAttackPL2" },
  { {  78752,  81248,  81312 },   A_UnHideThing,          "A_UnHideThing" },
  { {  78080,  80544,  80608 },   A_VolcBallImpact,       "A_VolcBallImpact" },
  { {  77856,  80320,  80384 },   A_VolcanoBlast,         "A_VolcanoBlast" },
  { {  77824,  80288,  80352 },   A_VolcanoSet,           "A_VolcanoSet" },
  { { 111168, 113488, 113728 },   A_WeaponReady,          "A_WeaponReady" },
  { {  75168,  77664,  77728 },   A_WhirlwindSeek,        "A_WhirlwindSeek" },
  { {  75888,  78320,  78384 },   A_WizAtk1,              "A_WizAtk1" },
  { {  75920,  78352,  78416 },   A_WizAtk2,              "A_WizAtk2" },
  { {  75952,  78384,  78448 },   A_WizAtk3,              "A_WizAtk3" },

  // This NULL entry must be the last in the list
  { {      0,      0,      0 },   NULL,                   "A_NULL"},  // Ty 05/16/98
};

int hhe_numrexptr = sizeof(hhe_rexptrs) / sizeof(hhe_rexptrs[0]);

// ====================================================================
// hhe_procRexCodePointers
// Purpose: Handle [CODEPTR] block, RAVEN Extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procRexCodePointers(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;
  char mnemonic[DEH_MAXKEYLEN];  // to hold the codepointer mnemonic
  int i; // looper
  dboolean found; // know if we found this one during lookup or not
  dsda_deh_state_t hhe_state;
  int mapped;

  // Ty 05/16/98 - initialize it to something, dummy!
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // for this one, we just read 'em until we hit a blank line
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;   // killough 11/98: really exit on blank line

    // killough 8/98: allow rex numbers in input:
    if ((3 != sscanf(inbuffer, "%s %i = %s", key, &indexnum, mnemonic)) || (stricmp(key, "FRAME")))
    { // NOTE: different format from normal
      deh_log("Invalid REX codepointer line - must start with 'FRAME': '%s'\n", inbuffer);
      continue;  // early return
    }

    deh_log("Processing pointer at index %d: %s\n", indexnum, mnemonic);

    if (indexnum < 0)
    {
      deh_log("Pointer number must be positive (%d)\n", indexnum);
      continue; // killough 10/98: fix SegViol
    }

    // Heretic only - get correct frame per version
    mapped = HHE_MapHereticFrameNumber(indexnum);

    if (mapped < 0 || mapped >= num_states)
    {
      deh_log("Invalid mapped frame number: %d -> %d\n", indexnum, mapped);
      continue;
    }
    indexnum = mapped;

    // Now do REX codepointer stuff (string based)

    strcpy(key, "A_");  // reusing the key area to prefix the mnemonic
    strcat(key, ptr_lstrip(mnemonic));

    hhe_state = dsda_GetDehState(indexnum);

    found = false;
    i= -1; // incremented to start at zero at the top of the loop
    do  // Ty 05/16/98 - fix loop logic to look for null ending entry
    {
      ++i;
      if (!stricmp(key, hhe_rexptrs[i].lookup))
      {  // Ty 06/01/98  - add  to states[].action for new djgcc version
        hhe_state.state->action = hhe_rexptrs[i].cptr; // assign
        deh_log(" - applied %s from codeptr[%d] to states[%d]\n",
                hhe_rexptrs[i].lookup, i, indexnum);
        found = true;
      }
    } while (!found && (hhe_rexptrs[i].cptr != NULL));

    if (!found)
      deh_log("Invalid frame pointer mnemonic '%s' at %d (mapped to %d)\n", mnemonic, mapped, indexnum);
  }
}

const deh_block hhe_block_rex_pointers = { "[CODEPTR]", hhe_procRexCodePointers };
