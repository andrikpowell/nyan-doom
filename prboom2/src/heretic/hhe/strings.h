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
//	HHE Strings
//

#ifndef __HHE_STRINGS__
#define __HHE_STRINGS__

//      Description: This file translates the #defined string constants
//      to named variables to externalize them for deh/bex changes.
//      Should be able to compile with D_FRENCH (for example) and still
//      work (untested).
//
//      Ty 03/22/98 - note that we are keeping the english versions and
//      comments in this file
//      New string names all start with an extra s_ to avoid conflicts,
//      but are otherwise identical to the original including uppercase.
//      This is partly to keep the changes simple and partly for easier
//      identification of the locations in which they're used.
//
//      Printed strings for translation
//

// Artifacts
extern const char *s_HERETIC_TXT_ARTIHEALTH;
extern const char *s_HERETIC_TXT_ARTIFLY;
extern const char *s_HERETIC_TXT_ARTIINVULNERABILITY;
extern const char *s_HERETIC_TXT_ARTITOMEOFPOWER;
extern const char *s_HERETIC_TXT_ARTIINVISIBILITY;
extern const char *s_HERETIC_TXT_ARTIEGG;
extern const char *s_HERETIC_TXT_ARTISUPERHEALTH;
extern const char *s_HERETIC_TXT_ARTITORCH;
extern const char *s_HERETIC_TXT_ARTIFIREBOMB;
extern const char *s_HERETIC_TXT_ARTITELEPORT;

// Items
extern const char *s_HERETIC_TXT_ITEMHEALTH;
extern const char *s_HERETIC_TXT_ITEMBAGOFHOLDING;
extern const char *s_HERETIC_TXT_ITEMSHIELD1;
extern const char *s_HERETIC_TXT_ITEMSHIELD2;
extern const char *s_HERETIC_TXT_ITEMSUPERMAP;

// Ammo
extern const char *s_HERETIC_TXT_AMMOGOLDWAND1;
extern const char *s_HERETIC_TXT_AMMOGOLDWAND2;
extern const char *s_HERETIC_TXT_AMMOMACE1;
extern const char *s_HERETIC_TXT_AMMOMACE2;
extern const char *s_HERETIC_TXT_AMMOCROSSBOW1;
extern const char *s_HERETIC_TXT_AMMOCROSSBOW2;
extern const char *s_HERETIC_TXT_AMMOBLASTER1;
extern const char *s_HERETIC_TXT_AMMOBLASTER2;
extern const char *s_HERETIC_TXT_AMMOSKULLROD1;
extern const char *s_HERETIC_TXT_AMMOSKULLROD2;
extern const char *s_HERETIC_TXT_AMMOPHOENIXROD1;
extern const char *s_HERETIC_TXT_AMMOPHOENIXROD2;

// Weapons
extern const char *s_HERETIC_TXT_WPNMACE;
extern const char *s_HERETIC_TXT_WPNCROSSBOW;
extern const char *s_HERETIC_TXT_WPNBLASTER;
extern const char *s_HERETIC_TXT_WPNSKULLROD;
extern const char *s_HERETIC_TXT_WPNPHOENIXROD;
extern const char *s_HERETIC_TXT_WPNGAUNTLETS;

// cheats
extern const char *s_HERETIC_TXT_CHEATGODON;
extern const char *s_HERETIC_TXT_CHEATGODOFF;
extern const char *s_HERETIC_TXT_CHEATNOCLIPON;
extern const char *s_HERETIC_TXT_CHEATNOCLIPOFF;
extern const char *s_HERETIC_TXT_CHEATWEAPONS;
extern const char *s_HERETIC_TXT_CHEATFLIGHTON;
extern const char *s_HERETIC_TXT_CHEATFLIGHTOFF;
extern const char *s_HERETIC_TXT_CHEATPOWERON;
extern const char *s_HERETIC_TXT_CHEATPOWEROFF;
extern const char *s_HERETIC_TXT_CHEATHEALTH;
extern const char *s_HERETIC_TXT_CHEATKEYS;
extern const char *s_HERETIC_TXT_CHEATSOUNDON;
extern const char *s_HERETIC_TXT_CHEATSOUNDOFF;
extern const char *s_HERETIC_TXT_CHEATTICKERON;
extern const char *s_HERETIC_TXT_CHEATTICKEROFF;
extern const char *s_HERETIC_TXT_CHEATARTIFACTS1;
extern const char *s_HERETIC_TXT_CHEATARTIFACTS2;
extern const char *s_HERETIC_TXT_CHEATARTIFACTS3;
extern const char *s_HERETIC_TXT_CHEATARTIFACTSFAIL;
extern const char *s_HERETIC_TXT_CHEATWARP;
extern const char *s_HERETIC_TXT_CHEATSCREENSHOT;
extern const char *s_HERETIC_TXT_CHEATCHICKENON;
extern const char *s_HERETIC_TXT_CHEATCHICKENOFF;
extern const char *s_HERETIC_TXT_SHOWFPSON;
extern const char *s_HERETIC_TXT_SHOWFPSOFF;
extern const char *s_HERETIC_TXT_CHEATMASSACRE;
extern const char *s_HERETIC_TXT_CHEATIDDQD;
extern const char *s_HERETIC_TXT_CHEATIDKFA;

// keys
extern const char *s_HERETIC_TXT_GOTBLUEKEY;
extern const char *s_HERETIC_TXT_GOTGREENKEY;
extern const char *s_HERETIC_TXT_GOTYELLOWKEY;
extern const char *s_HERETIC_TXT_NEEDBLUEKEY;
extern const char *s_HERETIC_TXT_NEEDGREENKEY;
extern const char *s_HERETIC_TXT_NEEDYELLOWKEY;

// automap
extern const char *s_HERETIC_AUTOPAGE;
extern const char *s_HERETIC_AMSTR_FOLLOWON;
extern const char *s_HERETIC_AMSTR_FOLLOWOFF;
extern const char *s_HERETIC_AMSTR_GRIDON;
extern const char *s_HERETIC_AMSTR_GRIDOFF;
extern const char *s_HERETIC_AMSTR_MARKEDSPOT;
extern const char *s_HERETIC_AMSTR_MARKSCLEARED;

// messages
extern const char *s_HERETIC_TXT_GAMESAVED;
extern const char *s_HERETIC_MSGON;
extern const char *s_HERETIC_MSGOFF;
extern const char *s_HERETIC_SWSTRING;
extern const char *s_HERETIC_RAVENQUITMSG;
extern const char *s_HERETIC_ENDGAME;

// main menu
extern const char *s_HERETIC_MNU_NEW_GAME;
extern const char *s_HERETIC_MNU_OPTIONS;
extern const char *s_HERETIC_MNU_GAME_FILES;
extern const char *s_HERETIC_MNU_INFO;
extern const char *s_HERETIC_MNU_QUIT_GAME;
extern const char *s_HERETIC_MNU_SAVE_GAME;
extern const char *s_HERETIC_MNU_LOAD_GAME;
extern const char *s_HERETIC_MNU_MUSIC_VOL;
extern const char *s_HERETIC_MNU_SFX_VOL;
extern const char *s_HERETIC_MNU_CHOOSESKILL;

// episode names
extern const char *s_HERETIC_EPISODE_1;
extern const char *s_HERETIC_EPISODE_2;
extern const char *s_HERETIC_EPISODE_3;
extern const char *s_HERETIC_EPISODE_4;
extern const char *s_HERETIC_EPISODE_5;

// level names
extern const char *s_HERETIC_HUSTR_E1M1;
extern const char *s_HERETIC_HUSTR_E1M2;
extern const char *s_HERETIC_HUSTR_E1M3;
extern const char *s_HERETIC_HUSTR_E1M4;
extern const char *s_HERETIC_HUSTR_E1M5;
extern const char *s_HERETIC_HUSTR_E1M6;
extern const char *s_HERETIC_HUSTR_E1M7;
extern const char *s_HERETIC_HUSTR_E1M8;
extern const char *s_HERETIC_HUSTR_E1M9;
extern const char *s_HERETIC_HUSTR_E2M1;
extern const char *s_HERETIC_HUSTR_E2M2;
extern const char *s_HERETIC_HUSTR_E2M3;
extern const char *s_HERETIC_HUSTR_E2M4;
extern const char *s_HERETIC_HUSTR_E2M5;
extern const char *s_HERETIC_HUSTR_E2M6;
extern const char *s_HERETIC_HUSTR_E2M7;
extern const char *s_HERETIC_HUSTR_E2M8;
extern const char *s_HERETIC_HUSTR_E2M9;
extern const char *s_HERETIC_HUSTR_E3M1;
extern const char *s_HERETIC_HUSTR_E3M2;
extern const char *s_HERETIC_HUSTR_E3M3;
extern const char *s_HERETIC_HUSTR_E3M4;
extern const char *s_HERETIC_HUSTR_E3M5;
extern const char *s_HERETIC_HUSTR_E3M6;
extern const char *s_HERETIC_HUSTR_E3M7;
extern const char *s_HERETIC_HUSTR_E3M8;
extern const char *s_HERETIC_HUSTR_E3M9;
extern const char *s_HERETIC_HUSTR_E4M1;
extern const char *s_HERETIC_HUSTR_E4M2;
extern const char *s_HERETIC_HUSTR_E4M3;
extern const char *s_HERETIC_HUSTR_E4M4;
extern const char *s_HERETIC_HUSTR_E4M5;
extern const char *s_HERETIC_HUSTR_E4M6;
extern const char *s_HERETIC_HUSTR_E4M7;
extern const char *s_HERETIC_HUSTR_E4M8;
extern const char *s_HERETIC_HUSTR_E4M9;
extern const char *s_HERETIC_HUSTR_E5M1;
extern const char *s_HERETIC_HUSTR_E5M2;
extern const char *s_HERETIC_HUSTR_E5M3;
extern const char *s_HERETIC_HUSTR_E5M4;
extern const char *s_HERETIC_HUSTR_E5M5;
extern const char *s_HERETIC_HUSTR_E5M6;
extern const char *s_HERETIC_HUSTR_E5M7;
extern const char *s_HERETIC_HUSTR_E5M8;
extern const char *s_HERETIC_HUSTR_E5M9;

// Extra maps
extern const char *s_HERETIC_HUSTR_E6M1;
extern const char *s_HERETIC_HUSTR_E6M2;
extern const char *s_HERETIC_HUSTR_E6M3;
extern const char *s_HERETIC_HUSTR_E6M4;
extern const char *s_HERETIC_HUSTR_E6M5;
extern const char *s_HERETIC_HUSTR_E6M6;
extern const char *s_HERETIC_HUSTR_E6M7;
extern const char *s_HERETIC_HUSTR_E6M8;
extern const char *s_HERETIC_HUSTR_E6M9;

// textscreens
extern const char *s_HERETIC_E1TEXT;
extern const char *s_HERETIC_E2TEXT;
extern const char *s_HERETIC_E3TEXT;
extern const char *s_HERETIC_E4TEXT;
extern const char *s_HERETIC_E5TEXT;

// killers
extern const char *s_HERETIC_IN_KILLERS1;
extern const char *s_HERETIC_IN_KILLERS2;
extern const char *s_HERETIC_IN_KILLERS3;
extern const char *s_HERETIC_IN_KILLERS4;
extern const char *s_HERETIC_IN_KILLERS5;
extern const char *s_HERETIC_IN_KILLERS6;

// textscreen backgrounds
extern const char *s_HERETIC_BGFLATE1;
extern const char *s_HERETIC_BGFLATE2;
extern const char *s_HERETIC_BGFLATE3;
extern const char *s_HERETIC_BGFLATE4;
extern const char *s_HERETIC_BGFLATE5;

// Intermission
extern const char *s_HERETIC_IN_BGFLAT;
extern const char *s_HERETIC_IN_FINISHED;
extern const char *s_HERETIC_IN_ENTERING;
extern const char *s_HERETIC_IN_KILLS;
extern const char *s_HERETIC_IN_ITEMS;
extern const char *s_HERETIC_IN_SECRETS;
extern const char *s_HERETIC_IN_TIME;
extern const char *s_HERETIC_IN_TOTAL;
extern const char *s_HERETIC_IN_VICTIMS;

extern const char *s_HERETIC_COOP_BONUS;
extern const char *s_HERETIC_COOP_SECRET;

// skills
extern const char *s_HERETIC_SKILL_1;
extern const char *s_HERETIC_SKILL_2;
extern const char *s_HERETIC_SKILL_3;
extern const char *s_HERETIC_SKILL_4;
extern const char *s_HERETIC_SKILL_5;

// obituaries
extern const char *s_OB_DSPARIL1HIT;
extern const char *s_OB_DSPARIL2HIT;
extern const char *s_OB_HERETICIMPHIT;
extern const char *s_OB_IRONLICHHIT;
extern const char *s_OB_BONEKNIGHTHIT;
extern const char *s_OB_MINOTAURHIT;
extern const char *s_OB_WIZARDHIT;

extern const char *s_OB_CHICKEN;
extern const char *s_OB_BEAST;
extern const char *s_OB_CLINK;
extern const char *s_OB_DSPARIL1;
extern const char *s_OB_DSPARIL2;
extern const char *s_OB_HERETICIMP;
extern const char *s_OB_IRONLICH;
extern const char *s_OB_BONEKNIGHT;
extern const char *s_OB_MINOTAUR;
extern const char *s_OB_MUMMY;
extern const char *s_OB_MUMMYLEADER;
extern const char *s_OB_SNAKE;
extern const char *s_OB_WIZARD;

extern const char *s_OB_MPSTAFF;
extern const char *s_OB_MPGAUNTLETS;
extern const char *s_OB_MPGOLDWAND;
extern const char *s_OB_MPCROSSBOW;
extern const char *s_OB_MPBLASTER;
extern const char *s_OB_MPSKULLROD;
extern const char *s_OB_MPPHOENIXROD;
extern const char *s_OB_MPMACE;

extern const char *s_OB_MPPSTAFF;
extern const char *s_OB_MPPGAUNTLETS;
extern const char *s_OB_MPPGOLDWAND;
extern const char *s_OB_MPPCROSSBOW;
extern const char *s_OB_MPPBLASTER;
extern const char *s_OB_MPPSKULLROD;
extern const char *s_OB_MPPPHOENIXROD;
extern const char *s_OB_MPPMACE;

//
// Hexen stuff (probably shouldn't be here, but meh)
//

// Artifacts
extern const char *s_TXT_ARTIINVULNERABILITY;
extern const char *s_TXT_ARTIHEALTH;
extern const char *s_TXT_ARTISUPERHEALTH;
extern const char *s_TXT_ARTIHEALINGRADIUS;
extern const char *s_TXT_ARTISUMMON;
extern const char *s_TXT_ARTITORCH;
extern const char *s_TXT_ARTIEGG;
extern const char *s_TXT_ARTIFLY;
extern const char *s_TXT_ARTIBLASTRADIUS;
extern const char *s_TXT_ARTIPOISONBAG;
extern const char *s_TXT_ARTITELEPORTOTHER;
extern const char *s_TXT_ARTISPEED;
extern const char *s_TXT_ARTIBOOSTMANA;
extern const char *s_TXT_ARTIBOOSTARMOR;
extern const char *s_TXT_ARTITELEPORT;

// Puzzle artifacts
extern const char *s_TXT_ARTIPUZZSKULL;
extern const char *s_TXT_ARTIPUZZGEMBIG;
extern const char *s_TXT_ARTIPUZZGEMRED;
extern const char *s_TXT_ARTIPUZZGEMGREEN1;
extern const char *s_TXT_ARTIPUZZGEMGREEN2;
extern const char *s_TXT_ARTIPUZZGEMBLUE1;
extern const char *s_TXT_ARTIPUZZGEMBLUE2;
extern const char *s_TXT_ARTIPUZZBOOK1;
extern const char *s_TXT_ARTIPUZZBOOK2;
extern const char *s_TXT_ARTIPUZZSKULL2;
extern const char *s_TXT_ARTIPUZZFWEAPON;
extern const char *s_TXT_ARTIPUZZCWEAPON;
extern const char *s_TXT_ARTIPUZZMWEAPON;
extern const char *s_TXT_ARTIPUZZGEAR;

#endif
