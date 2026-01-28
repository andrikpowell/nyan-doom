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
//	DEH Thing
//

#include "doomdef.h"
#include "e6y.h"//e6y
#include "lprintf.h"

#include "deh/func.h"
#include "dsda/mobjinfo.h"

// MOBJINFO - Dehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types.  Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but dehacked considers them to start at #1. ***
// CPhipps - static const

enum
{
    DEH_MOBJINFO_DOOMEDNUM,
    DEH_MOBJINFO_SPAWNSTATE,
    DEH_MOBJINFO_SPAWNHEALTH,
    DEH_MOBJINFO_SEESTATE,
    DEH_MOBJINFO_SEESOUND,
    DEH_MOBJINFO_REACTIONTIME,
    DEH_MOBJINFO_ATTACKSOUND,
    DEH_MOBJINFO_PAINSTATE,
    DEH_MOBJINFO_PAINCHANCE,
    DEH_MOBJINFO_PAINSOUND,
    DEH_MOBJINFO_MELEESTATE,
    DEH_MOBJINFO_MISSILESTATE,
    DEH_MOBJINFO_DEATHSTATE,
    DEH_MOBJINFO_XDEATHSTATE,
    DEH_MOBJINFO_DEATHSOUND,
    DEH_MOBJINFO_SPEED,
    DEH_MOBJINFO_RADIUS,
    DEH_MOBJINFO_HEIGHT,
    DEH_MOBJINFO_MASS,
    DEH_MOBJINFO_DAMAGE,
    DEH_MOBJINFO_ACTIVESOUND,
    DEH_MOBJINFO_FLAGS,
    DEH_MOBJINFO_FLAGS2,
    DEH_MOBJINFO_RAISESTATE,
    DEH_MOBJINFO_DROPPEDITEM, // DEHEXTRA

    // MBF21
    DEH_MOBJINFO_INFIGHTING_GROUP,
    DEH_MOBJINFO_PROJECTILE_GROUP,
    DEH_MOBJINFO_SPLASH_GROUP,
    DEH_MOBJINFO_MBF21_FLAGS,
    DEH_MOBJINFO_RIPSOUND,
    DEH_MOBJINFO_ALTSPEED,
    DEH_MOBJINFO_MELEERANGE,

    // MISC
    DEH_MOBJINFO_BLOODCOLOR,

    // MBF2y
    DEH_MOBJINFO_OBITUARY,
    DEH_MOBJINFO_MELEE_OBITUARY,
    DEH_MOBJINFO_SELF_OBITUARY,

    DEH_MOBJINFOMAX
};

static const char *deh_mobjinfo_fields[] =
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
  "Death frame",         // .deathstate
  "Exploding frame",     // .xdeathstate
  "Death sound",         // .deathsound
  "Speed",               // .speed
  "Width",               // .radius
  "Height",              // .height
  "Mass",                // .mass
  "Missile damage",      // .damage
  "Action sound",        // .activesound
  "Bits",                // .flags
  "Bits2",               // .flags
  "Respawn frame",       // .raisestate
  "Dropped item",        // .droppeditem

  // mbf21
  "Infighting group",    // .infighting_group
  "Projectile group",    // .projectile_group
  "Splash group",        // .splash_group
  "MBF21 Bits",          // .flags2
  "Rip sound",           // .ripsound
  "Fast speed",          // .altspeed
  "Melee range",         // .meleerange

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
static const struct deh_flag_s deh_mobjflags[] = {
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
  {"SHADOW",       MF_SHADOW}, // use fuzzy draw like spectres
  {"NOBLOOD",      MF_NOBLOOD}, // puffs instead of blood when shot
  {"CORPSE",       MF_CORPSE}, // so it will slide down steps when dead
  {"INFLOAT",      MF_INFLOAT}, // float but not to target height
  {"COUNTKILL",    MF_COUNTKILL}, // count toward the kills total
  {"COUNTITEM",    MF_COUNTITEM}, // count toward the items total
  {"SKULLFLY",     MF_SKULLFLY}, // special handling for flying skulls
  {"NOTDMATCH",    MF_NOTDMATCH}, // do not spawn in deathmatch

  // killough 10/98: TRANSLATION consists of 2 bits, not 1:

  {"TRANSLATION",  MF_TRANSLATION1}, // for Boom bug-compatibility
  {"TRANSLATION1", MF_TRANSLATION1}, // use translation table for color (players)
  {"TRANSLATION2", MF_TRANSLATION2}, // use translation table for color (players)
  {"UNUSED1",      MF_TRANSLATION2}, // unused bit # 1 -- For Boom bug-compatibility
  {"UNUSED2",      MF_UNUSED2},      // unused bit # 2 -- For Boom compatibility
  {"UNUSED3",      MF_UNUSED3},      // unused bit # 3 -- For Boom compatibility
  {"UNUSED4",      MF_TRANSLUCENT},  // unused bit # 4 -- For Boom compatibility
  {"TRANSLUCENT",  MF_TRANSLUCENT},  // apply translucency to sprite (BOOM)
  {"TOUCHY",       MF_TOUCHY},       // dies on contact with solid objects (MBF)
  {"BOUNCES",      MF_BOUNCES},      // bounces off floors, ceilings and maybe walls (MBF)
  {"FRIEND",       MF_FRIEND},       // a friend of the player(s) (MBF)
  { NULL }
};

const struct deh_flag_s deh_mobjflags_standard[] = {
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
  {"SHADOW",       MF_SHADOW}, // use fuzzy draw like spectres
  {"NOBLOOD",      MF_NOBLOOD}, // puffs instead of blood when shot
  {"CORPSE",       MF_CORPSE}, // so it will slide down steps when dead
  {"INFLOAT",      MF_INFLOAT}, // float but not to target height
  {"COUNTKILL",    MF_COUNTKILL}, // count toward the kills total
  {"COUNTITEM",    MF_COUNTITEM}, // count toward the items total
  {"SKULLFLY",     MF_SKULLFLY}, // special handling for flying skulls
  {"NOTDMATCH",    MF_NOTDMATCH}, // do not spawn in deathmatch
  {"TRANSLATION1", MF_TRANSLATION1}, // use translation table for color (players)
  {"TRANSLATION2", MF_TRANSLATION2}, // use translation table for color (players)
  {"TOUCHY",       MF_TOUCHY}, // dies on contact with solid objects (MBF)
  {"BOUNCES",      MF_BOUNCES}, // bounces off floors, ceilings and maybe walls (MBF)
  {"FRIEND",       MF_FRIEND}, // a friend of the player(s) (MBF)
  {"TRANSLUCENT",  MF_TRANSLUCENT}, // apply translucency to sprite (BOOM)
  { NULL }
};

const struct deh_flag_s deh_mobjflags_mbf21[] = {
  {"LOGRAV",         MF2_LOGRAV}, // low gravity
  {"SHORTMRANGE",    MF2_SHORTMRANGE}, // short missile range
  {"DMGIGNORED",     MF2_DMGIGNORED}, // other things ignore its attacks
  {"NORADIUSDMG",    MF2_NORADIUSDMG}, // doesn't take splash damage
  {"FORCERADIUSDMG", MF2_FORCERADIUSDMG}, // causes splash damage even if target immune
  {"HIGHERMPROB",    MF2_HIGHERMPROB}, // higher missile attack probability
  {"RANGEHALF",      MF2_RANGEHALF}, // use half distance for missile attack probability
  {"NOTHRESHOLD",    MF2_NOTHRESHOLD}, // no targeting threshold
  {"LONGMELEE",      MF2_LONGMELEE}, // long melee range
  {"BOSS",           MF2_BOSS}, // full volume see / death sound + splash immunity
  {"MAP07BOSS1",     MF2_MAP07BOSS1}, // Tag 666 "boss" on doom 2 map 7
  {"MAP07BOSS2",     MF2_MAP07BOSS2}, // Tag 667 "boss" on doom 2 map 7
  {"E1M8BOSS",       MF2_E1M8BOSS}, // E1M8 boss
  {"E2M8BOSS",       MF2_E2M8BOSS}, // E2M8 boss
  {"E3M8BOSS",       MF2_E3M8BOSS}, // E3M8 boss
  {"E4M6BOSS",       MF2_E4M6BOSS}, // E4M6 boss
  {"E4M8BOSS",       MF2_E4M8BOSS}, // E4M8 boss
  {"RIP",            MF2_RIP}, // projectile rips through targets
  {"FULLVOLSOUNDS",  MF2_FULLVOLSOUNDS}, // full volume see / death sound
  { NULL }
};

static uint64_t deh_stringToFlags(char *strval, const struct deh_flag_s *flags)
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
      deh_log("Could not find MBF21 bit mnemonic %s\n", strval);
  }

  return value;
}

uint64_t deh_stringToMBF21MobjFlags(char *strval)
{
  return deh_stringToFlags(strval, deh_mobjflags_mbf21);
}

uint64_t deh_stringToMobjFlags(char *strval)
{
  return deh_stringToFlags(strval, deh_mobjflags);
}

//---------------------------------------------------------------------------
// To be on the safe, compatible side, we manually convert DEH bitflags
// to prboom types - POPE
//---------------------------------------------------------------------------
static uint64_t getConvertedDEHBits(uint64_t bits) {
  static const uint64_t bitMap[32] = {
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
    MF_DROPPED, // 17 Disappearing weapon - Dropped, not spawned (like an ammo clip) I have not had much success in using this one.
    MF_SHADOW, // 18 Partial invisibility - Drawn like a spectre.
    MF_NOBLOOD, // 19 Puffs (vs. bleeds) - If hit will spawn bullet puffs instead of blood splats.
    MF_CORPSE, // 20 Sliding helpless - Will slide down steps when dead.
    MF_INFLOAT, // 21 No auto levelling - float but not to target height (?)
    MF_COUNTKILL, // 22 Affects kill % - counted as a killable enemy and affects percentage kills on level summary.
    MF_COUNTITEM, // 23 Affects item % - affects percentage items gathered on level summary.
    MF_SKULLFLY, // 24 Running - special handling for flying skulls.
    MF_NOTDMATCH, // 25 Not in deathmatch - do not spawn in deathmatch (like keys)
    MF_TRANSLATION1, // 26 Color 1 (grey / red)
    MF_TRANSLATION2, // 27 Color 2 (brown / red)
    // Convert bit 28 to MF_TOUCHY, not (MF_TRANSLATION1|MF_TRANSLATION2)
    // fixes bug #1576151 (part 1)
    MF_TOUCHY, // 28 - explodes on contact (MBF)
    MF_BOUNCES, // 29 - bounces off walls and floors (MBF)
    MF_FRIEND, // 30 - friendly monster helps players (MBF)
    MF_TRANSLUCENT // e6y: Translucency via dehacked/bex doesn't work without it
  };
  int i;
  uint64_t shiftBits = bits;
  uint64_t convertedBits = 0;
  for (i=0; i<32; i++) {
    if (shiftBits & 0x1) convertedBits |= bitMap[i];
    shiftBits >>= 1;
  }
  return convertedBits;
}

//---------------------------------------------------------------------------
// See usage below for an explanation of this function's existence - POPE
//---------------------------------------------------------------------------
static void setMobjInfoValue(int mobjInfoIndex, int keyIndex, uint64_t value) {
  mobjinfo_t *mi;
  if (mobjInfoIndex >= num_mobj_types || mobjInfoIndex < 0) return;
  mi = &mobjinfo[mobjInfoIndex];
  switch (keyIndex) {
    case DEH_MOBJINFO_DOOMEDNUM:    mi->doomednum = (int)value; return;
    case DEH_MOBJINFO_SPAWNSTATE:   mi->spawnstate = (int)value; return;
    case DEH_MOBJINFO_SPAWNHEALTH:  mi->spawnhealth = (int)value; return;
    case DEH_MOBJINFO_SEESTATE:     mi->seestate = (int)value; return;
    case DEH_MOBJINFO_SEESOUND:     mi->seesound = (int)value; return;
    case DEH_MOBJINFO_REACTIONTIME: mi->reactiontime = (int)value; return;
    case DEH_MOBJINFO_ATTACKSOUND:  mi->attacksound = (int)value; return;
    case DEH_MOBJINFO_PAINSTATE:    mi->painstate = (int)value; return;
    case DEH_MOBJINFO_PAINCHANCE:   mi->painchance = (int)value; return;
    case DEH_MOBJINFO_PAINSOUND:    mi->painsound = (int)value; return;
    case DEH_MOBJINFO_MELEESTATE:   mi->meleestate = (int)value; return;
    case DEH_MOBJINFO_MISSILESTATE: mi->missilestate = (int)value; return;
    case DEH_MOBJINFO_DEATHSTATE:   mi->deathstate = (int)value; return;
    case DEH_MOBJINFO_XDEATHSTATE:  mi->xdeathstate = (int)value; return;
    case DEH_MOBJINFO_DEATHSOUND:   mi->deathsound = (int)value; return;
    case DEH_MOBJINFO_SPEED:        mi->speed = (int)value; return;
    case DEH_MOBJINFO_RADIUS:       mi->radius = (int)value; return;
    case DEH_MOBJINFO_HEIGHT:       mi->height = (int)value; return;
    case DEH_MOBJINFO_MASS:         mi->mass = (int)value; return;
    case DEH_MOBJINFO_DAMAGE:       mi->damage = (int)value; return;
    case DEH_MOBJINFO_ACTIVESOUND:  mi->activesound = (int)value; return;
    case DEH_MOBJINFO_FLAGS:        mi->flags = value; return;
    // e6y
    // Correction of wrong processing of "Respawn frame" entry.
    // There is no more synch on http://www.doomworld.com/sda/dwdemo/w303-115.zip
    // (with correction in PIT_CheckThing)
    case DEH_MOBJINFO_FLAGS2:
      if (prboom_comp[PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY].state)
      {
        mi->raisestate = (int)value;
        return;
      }
      break;
    case DEH_MOBJINFO_RAISESTATE:
      if (!prboom_comp[PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY].state)
      {
        mi->raisestate = (int)value;
        return;
      }
      break;
    case DEH_MOBJINFO_DROPPEDITEM: // make it base zero (deh is 1-based)
      mi->droppeditem = dsda_TranslateDehMobjIndex((int)value) - 1;
      return;

    // mbf21
    // custom groups count from the end of the vanilla list
    // -> no concern for clashes
    case DEH_MOBJINFO_INFIGHTING_GROUP:
      mi->infighting_group = (int)(value);
      if (mi->infighting_group < 0)
      {
        I_Error("Infighting groups must be >= 0 (check your dehacked)");
        return;
      }
      mi->infighting_group = mi->infighting_group + IG_END;
      return;
    case DEH_MOBJINFO_PROJECTILE_GROUP:
      mi->projectile_group = (int)(value);
      if (mi->projectile_group < 0)
        mi->projectile_group = PG_GROUPLESS;
      else
        mi->projectile_group = mi->projectile_group + PG_END;
      return;
    case DEH_MOBJINFO_SPLASH_GROUP:
      mi->splash_group = (int)(value);
      if (mi->splash_group < 0)
      {
        I_Error("Splash groups must be >= 0 (check your dehacked)");
        return;
      }
      mi->splash_group = mi->splash_group + SG_END;
      return;
    case DEH_MOBJINFO_RIPSOUND: mi->ripsound = (int)value; return;
    case DEH_MOBJINFO_ALTSPEED: mi->altspeed = (int)value; return;
    case DEH_MOBJINFO_MELEERANGE: mi->meleerange = (int)value; return;

    // misc
    case DEH_MOBJINFO_BLOODCOLOR: mi->bloodcolor = V_BloodColor((int)value); return;

    default: return;
  }
}

// ====================================================================
// deh_procThing
// Purpose: Handle DEH Thing block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
// Ty 8/27/98 - revised to also allow mnemonics for
// bit masks for monster attributes
//

static void deh_procThing(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  int internal_index;
  int ix;
  char *strval;
  dsda_deh_mobjinfo_t deh_mobjinfo;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Thing line: '%s'\n", inbuffer);

  // killough 8/98: allow hex numbers in input:
  ix = sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("count=%d, Thing %d\n", ix, indexnum);

  // Note that the mobjinfo[] array is base zero, but object numbers
  // in the dehacked file start with one.  Grumble.
  internal_index = dsda_TranslateDehMobjIndex(indexnum) - 1;

  deh_mobjinfo = dsda_GetDehMobjInfo(internal_index);

  // now process the stuff
  // Note that for Things we can look up the key and use its offset
  // in the array of key strings as an int offset in the structure

  // get a line until a blank or end of file--it's not
  // blank now because it has our incoming key in it
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    int bGetData;

    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);  // toss the end of line

    // killough 11/98: really bail out on blank lines (break != continue)
    if (!*inbuffer) break;  // bail out with blank line between sections

    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData)
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    for (ix = 0; deh_mobjinfo_fields[ix]; ix++) {
      if (deh_strcasecmp(key, deh_mobjinfo_fields[ix])) continue;

      if (!deh_strcasecmp(key, "MBF21 Bits")) {
        if (bGetData == 1)
        {
          value = deh_translate_bits(value, deh_mobjflags_mbf21);
        }
        else
        {
          value = deh_stringToMBF21MobjFlags(strval);
        }

        deh_mobjinfo.info->flags2 = value;
      }
      else if (deh_strcasecmp(key, "Bits")) {
        // standard value set

        // The old code here was the cause of a DEH-related bug in prboom.
        // When the mobjinfo_t.flags member was graduated to an int64, this
        // code was caught unawares and was indexing each property of the
        // mobjinfo as if it were still an int32. This caused sets of the
        // "raisestate" member to partially overwrite the "flags" member,
        // thus screwing everything up and making most DEH patches result in
        // unshootable enemy types. Moved to a separate function above
        // and stripped of all hairy struct address indexing. - POPE
        setMobjInfoValue(internal_index, ix, value);
      }
      else if (bGetData == 1)
      { // proff
        // bit set
        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        // No more desync on HACX demos.

        value = getConvertedDEHBits(value);
        deh_mobjinfo.info->flags = value;
        *deh_mobjinfo.edited_bits = true; //e6y: changed by DEH
      }
      else
      {
        value = deh_stringToMobjFlags(strval);

        // Don't worry about conversion -- simply print values
        deh_log("Bits = 0x%08lX%08lX\n",
                (unsigned long)(value>>32) & 0xffffffff,
                (unsigned long)value & 0xffffffff);
        deh_mobjinfo.info->flags = value; // e6y
        *deh_mobjinfo.edited_bits = true; //e6y: changed by DEH
      }

      switch (ix)
      {
        case DEH_MOBJINFO_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Obituary is empty\n");
            break;
          }

          deh_log("Setting obituary for thing %d\n", indexnum);
          deh_mobjinfo.info->obituary = Z_Strdup(stripped_string);
          break;
        }

        case DEH_MOBJINFO_MELEE_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Melee obituary is empty\n");
            break;
          }

          deh_log("Setting melee obituary for thing %d\n", indexnum);
          deh_mobjinfo.info->obituary_melee = Z_Strdup(stripped_string);
          break;
        }

        case DEH_MOBJINFO_SELF_OBITUARY:
        {
          char *stripped_string = ptr_lstrip(strval);
          size_t len = strlen(stripped_string);

          if (len < 1)
          {
            deh_log("Self obituary is empty\n");
            break;
          }

          deh_log("Setting self obituary for thing %d\n", indexnum);
          deh_mobjinfo.info->obituary_self = Z_Strdup(stripped_string);
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

const deh_block deh_block_thing = { "Thing", deh_procThing };
