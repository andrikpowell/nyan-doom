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
//	HHE Thing
//

#include "doomdef.h"
#include "e6y.h"//e6y
#include "lprintf.h"

#include "heretic/hhe/frame.h"
#include "heretic/hhe/version.h"
#include "deh/func.h"
#include "dsda/mobjinfo.h"

// MOBJINFO - Hehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types.  Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but hehacked considers them to start at #1. ***
// CPhipps - static const

enum
{
    HHE_MOBJINFO_DOOMEDNUM,
    HHE_MOBJINFO_SPAWNSTATE,
    HHE_MOBJINFO_SPAWNHEALTH,
    HHE_MOBJINFO_SEESTATE,
    HHE_MOBJINFO_SEESOUND,
    HHE_MOBJINFO_REACTIONTIME,
    HHE_MOBJINFO_ATTACKSOUND,
    HHE_MOBJINFO_PAINSTATE,
    HHE_MOBJINFO_PAINCHANCE,
    HHE_MOBJINFO_PAINSOUND,
    HHE_MOBJINFO_MELEESTATE,
    HHE_MOBJINFO_MISSILESTATE,
    HHE_MOBJINFO_CRASHSTATE, // NEW - Heretic Burning frame
    HHE_MOBJINFO_DEATHSTATE,
    HHE_MOBJINFO_XDEATHSTATE,
    HHE_MOBJINFO_DEATHSOUND,
    HHE_MOBJINFO_SPEED,
    HHE_MOBJINFO_RADIUS,
    HHE_MOBJINFO_HEIGHT,
    HHE_MOBJINFO_MASS,
    HHE_MOBJINFO_DAMAGE,
    HHE_MOBJINFO_ACTIVESOUND,
    HHE_MOBJINFO_FLAGS,
    HHE_MOBJINFO_FLAGS2,

    // MISC
    HHE_MOBJINFO_BLOODCOLOR,

    // MBF2y
    HHE_MOBJINFO_OBITUARY,
    HHE_MOBJINFO_MELEE_OBITUARY,
    HHE_MOBJINFO_SELF_OBITUARY,

    HHE_MOBJINFOMAX
};

static const char *hhe_mobjinfo_fields[] =
{
  "ID #",                // .doomednum
  "Initial frame",       // .spawnstate
  "Hit points",          // .spawnhealth
  "First moving frame",  // .seestate
  "Alert sound",         // .seesound
  "Reaction time",       // .reactiontime
  "Attack sound",        // .attacksound
  "Injury frame",        // .painstate
  "Pain chance",         // .painchance
  "Pain sound",          // .painsound
  "Close attack frame",  // .meleestate
  "Far attack frame",    // .missilestate
  "Burning frame",       // .crashstate
  "Death frame",         // .deathstate
  "Exploding frame",     // .xdeathstate
  "Death sound",         // .deathsound
  "Speed",               // .speed
  "Width",               // .radius
  "Height",              // .height
  "Mass",                // .mass
  "Missile damage",      // .damage
  "Action sound",        // .activesound
  "Bits 1",              // .flags
  "Bits 2",              // .flags2

  // misc
  "Blood color",         // .bloodcolor

  // MBF2y
  "Obituary",            // .obituary
  "Melee obituary",      // .obituary_melee
  "Self obituary",       // .obituary_self

  NULL
};

// Strings that are used to indicate flags ("Bits" in mobjinfo)
// This is an array of bit masks that are related to p_mobj.h
// values, using the smae names without the MF_ in front.
// Ty 08/27/98 new code
//
// killough 10/98:
//
// Convert array to struct to allow multiple values, make array size variable

// CPhipps - static const
static const struct deh_flag_s hhe_mobjflags[] = {
  {"SPECIAL",      MF_SPECIAL}, // call  P_Specialthing when touched
  {"SOLID",        MF_SOLID}, // block movement
  {"SHOOTABLE",    MF_SHOOTABLE}, // can be hit
  {"NOSECTOR",     MF_NOSECTOR}, // invisible but touchable
  {"NOBLOCKMAP",   MF_NOBLOCKMAP}, // inert but displayable
  {"AMBUSH",       MF_AMBUSH}, // deaf monster
  {"JUSTHIT",      MF_JUSTHIT}, // will try to attack right back
  {"JUSTATTACKED", MF_JUSTATTACKED}, // take at least 1 step before attacking
  {"SPAWNCEILING", MF_SPAWNCEILING}, // initially hang from ceiling
  {"NOGRAVITY",    MF_NOGRAVITY}, // don't apply gravity during play
  {"DROPOFF",      MF_DROPOFF}, // can jump from high places
  {"PICKUP",       MF_PICKUP}, // will pick up items
  {"NOCLIP",       MF_NOCLIP}, // goes through walls
  {"SLIDE",        MF_SLIDE}, // keep info about sliding along walls
  {"FLOAT",        MF_FLOAT}, // allow movement to any height
  {"TELEPORT",     MF_TELEPORT}, // don't cross lines or look at heights
  {"MISSILE",      MF_MISSILE}, // don't hit same species, explode on block
  {"DROPPED",      MF_DROPPED}, // dropped, not spawned (like ammo clip)
  {"SHADOW",       MF_SHADOW}, // (NOT FUZZY draw) draw translucency - ghosts
  {"NOBLOOD",      MF_NOBLOOD}, // puffs instead of blood when shot
  {"CORPSE",       MF_CORPSE}, // so it will slide down steps when dead
  {"INFLOAT",      MF_INFLOAT}, // float but not to target height
  {"COUNTKILL",    MF_COUNTKILL}, // count toward the kills total
  {"COUNTITEM",    MF_COUNTITEM}, // count toward the items total
  {"SKULLFLY",     MF_SKULLFLY}, // special handling for flying skulls
  {"NOTDMATCH",    MF_NOTDMATCH}, // do not spawn in deathmatch
  {"TRANSLATION1", MF_TRANSLATION1}, // use translation table for color (players)
  {"TRANSLATION2", MF_TRANSLATION2}, // use translation table for color (players)
  { NULL }
};

static const struct deh_flag_s hhe_mobjflags2[] = {
  {"LOGRAV",          MF2_LOGRAV}, // logical gravity
  {"WINDTHRUST",      MF2_WINDTHRUST}, // gets pushed around by the wind
  {"FLOORBOUNCE",     MF2_FLOORBOUNCE}, // bounces off the floor
  {"THRUGHOST",       MF2_THRUGHOST}, // missile will pass through ghosts
  {"FLY",             MF2_FLY}, // fly mode is active
  {"FOOTCLIP",        MF2_FOOTCLIP}, // if feet are allowed to be clipped
  {"SPAWNFLOAT",      MF2_SPAWNFLOAT}, // spawn random float z
  {"NOTELEPORT",      MF2_NOTELEPORT}, // does not teleport
  {"RIP",             MF2_RIP}, // missile rips through solid
  {"PUSHABLE",        MF2_PUSHABLE}, // can be pushed by other moving
  {"SLIDE2",          MF2_SLIDE}, // slides against walls
  {"ONMOBJ",          MF2_ONMOBJ}, // mobj is resting on top of another
  {"PASSMOBJ",        MF2_PASSMOBJ}, // Enable z block checking (pass over / under)
  {"CANNOTPUSH",      MF2_CANNOTPUSH}, // cannot push other pushable mobjs
  {"FEETARECLIPPED",  MF2_FEETARECLIPPED}, // a mobj's feet are now being cut
  {"BOSS",            MF2_BOSS}, // mobj is a major boss
  {"FIREDAMAGE",      MF2_FIREDAMAGE}, // does fire damage
  {"NODMGTHRUST",     MF2_NODMGTHRUST}, // does not thrust target when
  {"TELESTOMP",       MF2_TELESTOMP}, // mobj can stomp another
  {"FLOATBOB",        MF2_FLOATBOB}, // use float bobbing z movement
  {"DONTDRAW",        MF2_DONTDRAW}, // don't generate a vissprite
  { NULL }
};

static uint64_t hhe_stringToFlags(char *strval, const struct deh_flag_s *flags)
{
  uint64_t value;

  for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
    const struct deh_flag_s *flag;

    for (flag = flags; flag->name; flag++) {
      if (deh_strcasecmp(strval, flag->name)) continue;

      value |= flag->value;
      break;
    }

    if (!flag->name)
      deh_log("Could not find HHE bit mnemonic %s\n", strval);
  }

  return value;
}

uint64_t hhe_stringToMobjFlags(char *strval)
{
  return hhe_stringToFlags(strval, hhe_mobjflags);
}

uint64_t hhe_stringToMobjFlags2(char *strval)
{
  return hhe_stringToFlags(strval, hhe_mobjflags2);
}

//---------------------------------------------------------------------------
// To be on the safe, compatible side, we manually convert HHE bitflags
// to prboom types - POPE
//---------------------------------------------------------------------------
static uint64_t getConvertedHHEBits(uint64_t bits) {
  static const uint64_t bitMap[28] = {
    /* cf linuxdoom-1.10 p_mobj.h */
    MF_SPECIAL, // 0 Can be picked up - When touched the thing can be picked up.
    MF_SOLID, // 1 Obstacle - The thing is solid and will not let you (or others) pass through it
    MF_SHOOTABLE, // 2 Shootable - Can be shot.
    MF_NOSECTOR, // 3 Total Invisibility - Invisible, but can be touched
    MF_NOBLOCKMAP, // 4 Don't use the blocklinks (inert but displayable)
    MF_AMBUSH, // 5 Semi deaf - The thing is a deaf monster
    MF_JUSTHIT, // 6 In pain - Will try to attack right back after being hit
    MF_JUSTATTACKED, // 7 Steps before attack - Will take at least one step before attacking
    MF_SPAWNCEILING, // 8 Hangs from ceiling - When the level starts, this thing will be at ceiling height.
    MF_NOGRAVITY, // 9 No gravity - Gravity does not affect this thing
    MF_DROPOFF, // 10 Travels over cliffs - Monsters normally do not walk off ledges/steps they could not walk up. With this set they can walk off any height of cliff. Usually only used for flying monsters.
    MF_PICKUP, // 11 Pick up items - The thing can pick up gettable items.
    MF_NOCLIP, // 12 No clipping - Thing can walk through walls.
    MF_SLIDE, // 13 Slides along walls - Keep info about sliding along walls (don't really know much about this one).
    MF_FLOAT, // 14 Floating - Thing can move to any height
    MF_TELEPORT, // 15 Semi no clipping - Don't cross lines or look at teleport heights. (don't really know much about this one either).
    MF_MISSILE, // 16 Projectiles - Behaves like a projectile, explodes when hitting something that blocks movement
    MF_DROPPED, // 17 Disappearing weapon - Dropped, not spawned (can be crushed) I have not had much success in using this one.
    MF_SHADOW, // 18 Drawn translucent - Ghosts / Shadowsphere.
    MF_NOBLOOD, // 19 Puffs (vs. bleeds) - Don't spawn blood splatters when hit.
    MF_CORPSE, // 20 Sliding helpless - Will slide down steps when dead.
    MF_INFLOAT, // 21 No auto levelling - float but not to target height (?)
    MF_COUNTKILL, // 22 Affects kill % - counted as a killable enemy and affects percentage kills on level summary.
    MF_COUNTITEM, // 23 Affects item % - affects percentage items gathered on level summary.
    MF_SKULLFLY, // 24 Running - used internally for swooping/charging monsters.
    MF_NOTDMATCH, // 25 Not in deathmatch - do not spawn in deathmatch (like keys)
    MF_TRANSLATION1, // 26 Color 1 (yellow / blue)
    MF_TRANSLATION2, // 27 Color 2 (red / blue)
    // Unused 28
    // Unused 29
    // Unused 30
    // Unused 31
  };
  int i;
  uint64_t shiftBits = bits;
  uint64_t convertedBits = 0;
  for (i=0; i<28; i++) {
    if (shiftBits & 0x1) convertedBits |= bitMap[i];
    shiftBits >>= 1;
  }
  return convertedBits;
}

static uint64_t getConvertedHHEBits2(uint64_t bits) {
  static const uint64_t bitMap[21] = {
  MF2_LOGRAV, // 0 Subject to Inertia - logical gravity
  MF2_WINDTHRUST, // 1 Blown by Wind - gets pushed around by the wind
  MF2_FLOORBOUNCE, // 2 Bounces - bounces off the floor
  MF2_THRUGHOST, // 3 Goes Through Invis - missile will pass through ghosts
  MF2_FLY, // 4 Pushable Up Cliffs - fly mode is active
  MF2_FOOTCLIP, // 5 Walks IN Liquids - if feet are allowed to be clipped
  MF2_SPAWNFLOAT, // 6 Starts on Floor - spawn random float z
  MF2_NOTELEPORT, // 7 Does Not Teleport
  MF2_RIP, // 8 Dragon Claw Spikes - missile rips through solid
  MF2_PUSHABLE, // 9 Pushable - can be pushed by other moving
  MF2_SLIDE, // 10 Only Hits Monsters - slides against walls
  MF2_ONMOBJ, // 11 internal - mobj is resting on top of another
  MF2_PASSMOBJ, // 12 internal - Enable z block checking (pass over / under)
  MF2_CANNOTPUSH, // 13 Particles - cannot push other pushable mobjs
  MF2_FEETARECLIPPED, // 14 internal -  a mobj's feet are now being cut
  MF2_BOSS, // 15 Varying Damage - mobj is a major boss
  MF2_FIREDAMAGE, // 16 Causes Burning Death - does fire damage
  MF2_NODMGTHRUST, // 17 Does Not Push Things - does not thrust target when
  MF2_TELESTOMP, // 18 Telefrags - mobj can stomp another
  MF2_FLOATBOB, // 19 Bobbing - use float bobbing z movement
  MF2_DONTDRAW, // 20 internal - don't generate a vissprite
  };
  int i;
  uint64_t shiftBits = bits;
  uint64_t convertedBits = 0;
  for (i=0; i<21; i++) {
    if (shiftBits & 0x1) convertedBits |= bitMap[i];
    shiftBits >>= 1;
  }
  return convertedBits;
}

static int HHE_TranslateState(int s)
{
  // HHE state refs use the same numbering as Frame blocks.
  // If negatives have meaning (often -1), preserve them.
  if (s < 0) return s;
  return HHE_MapHereticFrameNumber(s);
}

// Gotta translate certain states for Heretic :)
static void setHereticMobjInfoValue(int mobjInfoIndex, int keyIndex, uint64_t value) {
  mobjinfo_t *mi;
  int shifted_value;
  if (mobjInfoIndex >= num_mobj_types || mobjInfoIndex < 0) return;
  mi = &mobjinfo[mobjInfoIndex];
  shifted_value = HHE_TranslateState((int)value);
  switch (keyIndex) {
    // we must translate state number based on HHE version :(
    case HHE_MOBJINFO_SPAWNSTATE:   mi->spawnstate = shifted_value; return;
    case HHE_MOBJINFO_SEESTATE:     mi->seestate = shifted_value; return;
    case HHE_MOBJINFO_PAINSTATE:    mi->painstate = shifted_value; return;
    case HHE_MOBJINFO_MELEESTATE:   mi->meleestate = shifted_value; return;
    case HHE_MOBJINFO_MISSILESTATE: mi->missilestate = shifted_value; return;
    case HHE_MOBJINFO_CRASHSTATE:   mi->crashstate = shifted_value; return;
    case HHE_MOBJINFO_DEATHSTATE:   mi->deathstate = shifted_value; return;
    case HHE_MOBJINFO_XDEATHSTATE:  mi->xdeathstate = shifted_value; return;

    // Rest of the normal stuff
    case HHE_MOBJINFO_DOOMEDNUM:    mi->doomednum = (int)value; return;
    case HHE_MOBJINFO_SPAWNHEALTH:  mi->spawnhealth = (int)value; return;
    case HHE_MOBJINFO_SEESOUND:     mi->seesound = (int)value; return;
    case HHE_MOBJINFO_REACTIONTIME: mi->reactiontime = (int)value; return;
    case HHE_MOBJINFO_ATTACKSOUND:  mi->attacksound = (int)value; return;
    case HHE_MOBJINFO_PAINCHANCE:   mi->painchance = (int)value; return;
    case HHE_MOBJINFO_PAINSOUND:    mi->painsound = (int)value; return;
    case HHE_MOBJINFO_DEATHSOUND:   mi->deathsound = (int)value; return;
    case HHE_MOBJINFO_SPEED:        mi->speed = (int)value; return;
    case HHE_MOBJINFO_RADIUS:       mi->radius = (int)value; return;
    case HHE_MOBJINFO_HEIGHT:       mi->height = (int)value; return;
    case HHE_MOBJINFO_MASS:         mi->mass = (int)value; return;
    case HHE_MOBJINFO_DAMAGE:       mi->damage = (int)value; return;
    case HHE_MOBJINFO_ACTIVESOUND:  mi->activesound = (int)value; return;
    case HHE_MOBJINFO_FLAGS:        mi->flags = value; return;
    case HHE_MOBJINFO_FLAGS2:       mi->flags2 = value; return;

    // misc
    case HHE_MOBJINFO_BLOODCOLOR: mi->bloodcolor = V_BloodColor((int)value); return;

    default: return;
  }
}

int HHE_MapHereticThingType(int type)
{
    // Heretic 1.0 had an extra entry in the mobjinfo table that was removed
    // in later versions. This has been added back into the table for
    // compatibility. However, it also means that if we're loading a patch
    // for a later version, we need to translate to the index used internally.

    if (deh_hhe_version > deh_hhe_1_0)
      if (type >= (HERETIC_MT_PHOENIXFX_REMOVED - HERETIC_MT_ZERO)) // translate from Heretic "0" -meltface-
        ++type;

    return type;
}

int dsda_TranslateHereticMobjIndex(int index)
{
    // Heretic's "0" entry must be translated
    return dsda_GetDehMobjIndex(HERETIC_MT_ZERO + HHE_MapHereticThingType(index - 1)) + 1;
}

// ====================================================================
// hhe_procThing
// Purpose: Handle HHE Thing block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
// Ty 8/27/98 - revised to also allow mnemonics for
// bit masks for monster attributes
//

static void hhe_procThing(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All hhe values are ints or longs
  int indexnum;
  int internal_index;
  int ix;
  char *strval;
  dsda_deh_mobjinfo_t hhe_mobjinfo;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Thing line: '%s'\n", inbuffer);

  // killough 8/98: allow rex numbers in input:
  ix = sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("count=%d, Thing %d\n", ix, indexnum);

  // Note that the mobjinfo[] array is base zero, but object numbers
  // in the hehacked file start with one.  Grumble.
  //
  // Note that Heretic HHE versions have different indexes
  // HHE Thing numbers are 1-based like DEH.

  internal_index = dsda_TranslateHereticMobjIndex(indexnum) - 1;

  hhe_mobjinfo = dsda_GetDehMobjInfo(internal_index);

  // now process the stuff
  // Note that for Things we can look up the key and use its offset
  // in the array of key strings as an int offset in the structure

  // get a line until a blank or end of file--it's not
  // blank now because it has our incoming key in it
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    int bGetData;

    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);  // toss the end of line

    // killough 11/98: really bail out on blank lines (break != continue)
    if (!*inbuffer) break;  // bail out with blank line between sections

    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData)
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    // I'm reordering this here, cuz in deh/thing.c this is confusing as hell
    for (ix = 0; hhe_mobjinfo_fields[ix]; ix++) {
      if (deh_strcasecmp(key, hhe_mobjinfo_fields[ix])) continue; // Unknown key? skip parsing

      // -----------------------
      // Bits 1
      // -----------------------
      if (!deh_strcasecmp(key, "Bits 1"))
      {
        if (bGetData == 1)
        {
          // Numeric form: Bits 1 = <mask>
          value = getConvertedHHEBits(value);
        }
        else
        {
          // Mnemonic form: Bits 1 = FLAG + FLAG ...
          value = hhe_stringToMobjFlags(strval);

          // Don't worry about conversion -- simply print values
          deh_log("Bits 1 = 0x%08lX%08lX\n",
                  (unsigned long)(value>>32) & 0xffffffff,
                  (unsigned long)value & 0xffffffff);
        }

        hhe_mobjinfo.info->flags = value;
        *hhe_mobjinfo.edited_bits = true;
      }

      // -----------------------
      // Bits 2
      // -----------------------
      else if (!deh_strcasecmp(key, "Bits 2"))
      {
        if (bGetData == 1)
        {
          // Numeric form: Bits 2 = <mask>
          value = getConvertedHHEBits2(value);
        }
        else
        {
          // Mnemonic form: Bits 2 = FLAG + FLAG ...
          value = hhe_stringToMobjFlags2(strval);

          // Don't worry about conversion -- simply print values
          deh_log("Bits 2 = 0x%08lX%08lX\n",
                  (unsigned long)(value>>32) & 0xffffffff,
                  (unsigned long)value & 0xffffffff);
        }

        hhe_mobjinfo.info->flags2 = value;
        *hhe_mobjinfo.edited_bits = true;
      }

      // -----------------------
      // Standard Value Set
      // -----------------------
      else 
      {
        // Example: "Initial frame = "
        setHereticMobjInfoValue(internal_index, ix, value);
      }

      // -----------------------
      // Obituary Stuffs
      // -----------------------
      switch (ix)
      {
        case HHE_MOBJINFO_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Obituary is empty\n");
            break;
          }

          deh_log("Setting obituary for thing %d\n", indexnum);
          hhe_mobjinfo.info->obituary = Z_Strdup(stripped_string);
          break;
        }

        case HHE_MOBJINFO_MELEE_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Melee obituary is empty\n");
            break;
          }

          deh_log("Setting melee obituary for thing %d\n", indexnum);
          hhe_mobjinfo.info->obituary_melee = Z_Strdup(stripped_string);
          break;
        }

        case HHE_MOBJINFO_SELF_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Self obituary is empty\n");
            break;
          }

          deh_log("Setting self obituary for thing %d\n", indexnum);
          hhe_mobjinfo.info->obituary_self = Z_Strdup(stripped_string);
          break;
        }

        default:
          break;
      }

      deh_log("Assigned 0x%08lx%08lx to %s(%d) at index %d\n",
              (unsigned long)(value>>32) & 0xffffffff,
              (unsigned long)value & 0xffffffff, key, indexnum, ix);
    }
  }
}

const deh_block hhe_block_thing = { "Thing", hhe_procThing };
