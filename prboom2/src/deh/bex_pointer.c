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
//	DEH [CODEPTR] Parser
//

#include "p_enemy.h"
#include "e6y.h"//e6y
#include "doomdef.h"

#include "deh/bex_pointer.h"
#include "dsda/state.h"

// BEX extension [CODEPTR]
// Usage: Start block, then each line is:
// FRAME nnn = PointerMnemonic

const deh_bexptr deh_bexptrs[] = // CPhipps - static const
{
  {A_Light0,          "A_Light0"},
  {A_WeaponReady,     "A_WeaponReady"},
  {A_Lower,           "A_Lower"},
  {A_Raise,           "A_Raise"},
  {A_Punch,           "A_Punch"},
  {A_ReFire,          "A_ReFire"},
  {A_FirePistol,      "A_FirePistol"},
  {A_Light1,          "A_Light1"},
  {A_FireShotgun,     "A_FireShotgun"},
  {A_Light2,          "A_Light2"},
  {A_FireShotgun2,    "A_FireShotgun2"},
  {A_CheckReload,     "A_CheckReload"},
  {A_OpenShotgun2,    "A_OpenShotgun2"},
  {A_LoadShotgun2,    "A_LoadShotgun2"},
  {A_CloseShotgun2,   "A_CloseShotgun2"},
  {A_FireCGun,        "A_FireCGun"},
  {A_GunFlash,        "A_GunFlash"},
  {A_FireMissile,     "A_FireMissile"},
  {A_Saw,             "A_Saw"},
  {A_FirePlasma,      "A_FirePlasma"},
  {A_BFGsound,        "A_BFGsound"},
  {A_FireBFG,         "A_FireBFG"},
  {A_BFGSpray,        "A_BFGSpray"},
  {A_Explode,         "A_Explode"},
  {A_Pain,            "A_Pain"},
  {A_PlayerScream,    "A_PlayerScream"},
  {A_Fall,            "A_Fall"},
  {A_XScream,         "A_XScream"},
  {A_Look,            "A_Look"},
  {A_Chase,           "A_Chase"},
  {A_FaceTarget,      "A_FaceTarget"},
  {A_PosAttack,       "A_PosAttack"},
  {A_Scream,          "A_Scream"},
  {A_SPosAttack,      "A_SPosAttack"},
  {A_VileChase,       "A_VileChase"},
  {A_VileStart,       "A_VileStart"},
  {A_VileTarget,      "A_VileTarget"},
  {A_VileAttack,      "A_VileAttack"},
  {A_StartFire,       "A_StartFire"},
  {A_Fire,            "A_Fire"},
  {A_FireCrackle,     "A_FireCrackle"},
  {A_Tracer,          "A_Tracer"},
  {A_SkelWhoosh,      "A_SkelWhoosh"},
  {A_SkelFist,        "A_SkelFist"},
  {A_SkelMissile,     "A_SkelMissile"},
  {A_FatRaise,        "A_FatRaise"},
  {A_FatAttack1,      "A_FatAttack1"},
  {A_FatAttack2,      "A_FatAttack2"},
  {A_FatAttack3,      "A_FatAttack3"},
  {A_BossDeath,       "A_BossDeath"},
  {A_CPosAttack,      "A_CPosAttack"},
  {A_CPosRefire,      "A_CPosRefire"},
  {A_TroopAttack,     "A_TroopAttack"},
  {A_SargAttack,      "A_SargAttack"},
  {A_HeadAttack,      "A_HeadAttack"},
  {A_BruisAttack,     "A_BruisAttack"},
  {A_SkullAttack,     "A_SkullAttack"},
  {A_Metal,           "A_Metal"},
  {A_SpidRefire,      "A_SpidRefire"},
  {A_BabyMetal,       "A_BabyMetal"},
  {A_BspiAttack,      "A_BspiAttack"},
  {A_Hoof,            "A_Hoof"},
  {A_CyberAttack,     "A_CyberAttack"},
  {A_PainAttack,      "A_PainAttack"},
  {A_PainDie,         "A_PainDie"},
  {A_KeenDie,         "A_KeenDie"},
  {A_BrainPain,       "A_BrainPain"},
  {A_BrainScream,     "A_BrainScream"},
  {A_BrainDie,        "A_BrainDie"},
  {A_BrainAwake,      "A_BrainAwake"},
  {A_BrainSpit,       "A_BrainSpit"},
  {A_SpawnSound,      "A_SpawnSound"},
  {A_SpawnFly,        "A_SpawnFly"},
  {A_BrainExplode,    "A_BrainExplode"},
  {A_Detonate,        "A_Detonate"},       // killough 8/9/98
  {A_Mushroom,        "A_Mushroom"},       // killough 10/98
  {A_Die,             "A_Die"},            // killough 11/98
  {A_Spawn,           "A_Spawn", 0, {0}, TI_MISC1},          // killough 11/98
  {A_Turn,            "A_Turn"},           // killough 11/98
  {A_Face,            "A_Face"},           // killough 11/98
  {A_Scratch,         "A_Scratch"},        // killough 11/98
  {A_PlaySound,       "A_PlaySound"},      // killough 11/98
  {A_RandomJump,      "A_RandomJump"},     // killough 11/98
  {A_LineEffect,      "A_LineEffect"},     // killough 11/98

  {A_FireOldBFG,      "A_FireOldBFG"},      // killough 7/19/98: classic BFG firing function
  {A_BetaSkullAttack, "A_BetaSkullAttack"}, // killough 10/98: beta lost souls attacked different
  {A_Stop,            "A_Stop"},

  // [XA] New mbf21 codepointers
  {A_SpawnObject,         "A_SpawnObject", 8, {0}, TI_ARGS1},
  {A_MonsterProjectile,   "A_MonsterProjectile", 5, {0}, TI_ARGS1},
  {A_MonsterBulletAttack, "A_MonsterBulletAttack", 5, {0, 0, 1, 3, 5}},
  {A_MonsterMeleeAttack,  "A_MonsterMeleeAttack", 4, {3, 8, 0, 0}},
  {A_RadiusDamage,        "A_RadiusDamage", 2},
  {A_NoiseAlert,          "A_NoiseAlert", 0},
  {A_HealChase,           "A_HealChase", 2},
  {A_SeekTracer,          "A_SeekTracer", 2},
  {A_FindTracer,          "A_FindTracer", 2, {0, 10}},
  {A_ClearTracer,         "A_ClearTracer", 0},
  {A_JumpIfHealthBelow,   "A_JumpIfHealthBelow", 2},
  {A_JumpIfTargetInSight, "A_JumpIfTargetInSight", 2},
  {A_JumpIfTargetCloser,  "A_JumpIfTargetCloser", 2},
  {A_JumpIfTracerInSight, "A_JumpIfTracerInSight", 2},
  {A_JumpIfTracerCloser,  "A_JumpIfTracerCloser", 2},
  {A_JumpIfFlagsSet,      "A_JumpIfFlagsSet", 3},
  {A_AddFlags,            "A_AddFlags", 2},
  {A_RemoveFlags,         "A_RemoveFlags", 2},
  {A_WeaponProjectile,    "A_WeaponProjectile", 5, {0}, TI_ARGS1},
  {A_WeaponBulletAttack,  "A_WeaponBulletAttack", 5, {0, 0, 1, 5, 3}},
  {A_WeaponMeleeAttack,   "A_WeaponMeleeAttack", 5, {2, 10, 1 * FRACUNIT, 0, 0}},
  {A_WeaponSound,         "A_WeaponSound", 2},
  {A_WeaponAlert,         "A_WeaponAlert", 0},
  {A_WeaponJump,          "A_WeaponJump", 2},
  {A_ConsumeAmmo,         "A_ConsumeAmmo", 1},
  {A_CheckAmmo,           "A_CheckAmmo", 2},
  {A_RefireTo,            "A_RefireTo", 2},
  {A_GunFlashTo,          "A_GunFlashTo", 2},

  // This NULL entry must be the last in the list
  {NULL,              "A_NULL"},  // Ty 05/16/98
};

int deh_numbexptr = sizeof(deh_bexptrs) / sizeof(deh_bexptrs[0]);

// ====================================================================
// deh_procBexCodePointers
// Purpose: Handle [CODEPTR] block, BOOM Extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procBexCodePointers(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;
  char mnemonic[DEH_MAXKEYLEN];  // to hold the codepointer mnemonic
  int i; // looper
  dboolean found; // know if we found this one during lookup or not
  dsda_deh_state_t deh_state;

  // Ty 05/16/98 - initialize it to something, dummy!
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // for this one, we just read 'em until we hit a blank line
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;   // killough 11/98: really exit on blank line

    // killough 8/98: allow hex numbers in input:
    if ((3 != sscanf(inbuffer, "%s %i = %s", key, &indexnum, mnemonic)) || (stricmp(key, "FRAME")))
    { // NOTE: different format from normal
      deh_log("Invalid BEX codepointer line - must start with 'FRAME': '%s'\n", inbuffer);
      return;  // early return
    }

    deh_log("Processing pointer at index %d: %s\n", indexnum, mnemonic);
    if (indexnum < 0)
    {
      deh_log("Pointer number must be positive (%d)\n", indexnum);
      return; // killough 10/98: fix SegViol
    }
    strcpy(key, "A_");  // reusing the key area to prefix the mnemonic
    strcat(key, ptr_lstrip(mnemonic));

    deh_state = dsda_GetDehState(indexnum);

    found = false;
    i= -1; // incremented to start at zero at the top of the loop
    do  // Ty 05/16/98 - fix loop logic to look for null ending entry
    {
      ++i;
      if (!stricmp(key, deh_bexptrs[i].lookup))
      {  // Ty 06/01/98  - add  to states[].action for new djgcc version
        deh_state.state->action = deh_bexptrs[i].cptr; // assign
        deh_log(" - applied %s from codeptr[%d] to states[%d]\n",
                deh_bexptrs[i].lookup, i, indexnum);
        found = true;
      }
    } while (!found && (deh_bexptrs[i].cptr != NULL));

    if (!found)
      deh_log("Invalid frame pointer mnemonic '%s' at %d\n", mnemonic, indexnum);
  }
}

const deh_block deh_block_bex_pointers = { "[CODEPTR]", deh_procBexCodePointers };
