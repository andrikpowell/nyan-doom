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
//	HHE [STRINGS] Parser
//

#include "doomstat.h"
#include "doomdef.h"
#include "dstrings.h"  // to get the initial values

#include "deh/func.h"
#include "heretic/hhe/text.h"
#include "heretic/hhe/strings.h"
#include "deh/strings.h"

#include "heretic/hhe/rex_strings.h"

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above.  We will use
// strdups to set these new values that we read from the file, orphaning
// the original value set above.

//     Offsets                 String (Pointer)      REX String lookup
//       v1.0   v1.2   v1.3

hhe_strs hhe_strlookup[] = {
  // map names
    { {  1240,  1252,  1252 }, &s_HERETIC_HUSTR_E1M1, "HHUSTR_E1M1" },
    { {  1260,  1272,  1272 }, &s_HERETIC_HUSTR_E1M2, "HHUSTR_E1M2" },
    { {  1280,  1292,  1292 }, &s_HERETIC_HUSTR_E1M3, "HHUSTR_E1M3" },
    { {  1304,  1316,  1316 }, &s_HERETIC_HUSTR_E1M4, "HHUSTR_E1M4" },
    { {  1328,  1340,  1340 }, &s_HERETIC_HUSTR_E1M5, "HHUSTR_E1M5" },
    { {  1348,  1360,  1360 }, &s_HERETIC_HUSTR_E1M6, "HHUSTR_E1M6" },
    { {  1372,  1384,  1384 }, &s_HERETIC_HUSTR_E1M7, "HHUSTR_E1M7" },
    { {  1392,  1404,  1404 }, &s_HERETIC_HUSTR_E1M8, "HHUSTR_E1M8" },
    { {  1412,  1424,  1424 }, &s_HERETIC_HUSTR_E1M9, "HHUSTR_E1M9" },
    { {  1436,  1448,  1448 }, &s_HERETIC_HUSTR_E2M1, "HHUSTR_E2M1" },
    { {  1456,  1468,  1468 }, &s_HERETIC_HUSTR_E2M2, "HHUSTR_E2M2" },
    { {  1480,  1492,  1492 }, &s_HERETIC_HUSTR_E2M3, "HHUSTR_E2M3" },
    { {  1508,  1520,  1520 }, &s_HERETIC_HUSTR_E2M4, "HHUSTR_E2M4" },
    { {  1532,  1544,  1544 }, &s_HERETIC_HUSTR_E2M5, "HHUSTR_E2M5" },
    { {  1556,  1568,  1568 }, &s_HERETIC_HUSTR_E2M6, "HHUSTR_E2M6" },
    { {  1580,  1592,  1592 }, &s_HERETIC_HUSTR_E2M7, "HHUSTR_E2M7" },
    { {  1604,  1616,  1616 }, &s_HERETIC_HUSTR_E2M8, "HHUSTR_E2M8" },
    { {  1632,  1644,  1644 }, &s_HERETIC_HUSTR_E2M9, "HHUSTR_E2M9" },
    { {  1652,  1664,  1664 }, &s_HERETIC_HUSTR_E3M1, "HHUSTR_E3M1" },
    { {  1676,  1688,  1688 }, &s_HERETIC_HUSTR_E3M2, "HHUSTR_E3M2" },
    { {  1696,  1708,  1708 }, &s_HERETIC_HUSTR_E3M3, "HHUSTR_E3M3" },
    { {  1720,  1732,  1732 }, &s_HERETIC_HUSTR_E3M4, "HHUSTR_E3M4" },
    { {  1748,  1760,  1760 }, &s_HERETIC_HUSTR_E3M5, "HHUSTR_E3M5" },
    { {  1776,  1788,  1788 }, &s_HERETIC_HUSTR_E3M6, "HHUSTR_E3M6" },
    { {  1804,  1816,  1816 }, &s_HERETIC_HUSTR_E3M7, "HHUSTR_E3M7" },
    { {  1824,  1836,  1836 }, &s_HERETIC_HUSTR_E3M8, "HHUSTR_E3M8" },
    { {  1848,  1860,  1860 }, &s_HERETIC_HUSTR_E3M9, "HHUSTR_E3M9" },
    { {     0,  1880,  1880 }, &s_HERETIC_HUSTR_E4M1, "HHUSTR_E4M1" },
    { {     0,  1900,  1900 }, &s_HERETIC_HUSTR_E4M2, "HHUSTR_E4M2" },
    { {     0,  1920,  1920 }, &s_HERETIC_HUSTR_E4M3, "HHUSTR_E4M3" },
    { {     0,  1940,  1940 }, &s_HERETIC_HUSTR_E4M4, "HHUSTR_E4M4" },
    { {     0,  1960,  1960 }, &s_HERETIC_HUSTR_E4M5, "HHUSTR_E4M5" },
    { {     0,  1980,  1980 }, &s_HERETIC_HUSTR_E4M6, "HHUSTR_E4M6" },
    { {     0,  2012,  2012 }, &s_HERETIC_HUSTR_E4M7, "HHUSTR_E4M7" },
    { {     0,  2044,  2044 }, &s_HERETIC_HUSTR_E4M8, "HHUSTR_E4M8" },
    { {     0,  2068,  2068 }, &s_HERETIC_HUSTR_E4M9, "HHUSTR_E4M9" },
    { {     0,  2088,  2088 }, &s_HERETIC_HUSTR_E5M1, "HHUSTR_E5M1" },
    { {     0,  2108,  2108 }, &s_HERETIC_HUSTR_E5M2, "HHUSTR_E5M2" },
    { {     0,  2124,  2124 }, &s_HERETIC_HUSTR_E5M3, "HHUSTR_E5M3" },
    { {     0,  2136,  2136 }, &s_HERETIC_HUSTR_E5M4, "HHUSTR_E5M4" },
    { {     0,  2156,  2156 }, &s_HERETIC_HUSTR_E5M5, "HHUSTR_E5M5" },
    { {     0,  2172,  2172 }, &s_HERETIC_HUSTR_E5M6, "HHUSTR_E5M6" },
    { {     0,  2192,  2192 }, &s_HERETIC_HUSTR_E5M7, "HHUSTR_E5M7" },
    { {     0,  2212,  2212 }, &s_HERETIC_HUSTR_E5M8, "HHUSTR_E5M8" },
    { {     0,  2240,  2240 }, &s_HERETIC_HUSTR_E5M9, "HHUSTR_E5M9" },

  // Extra episode strings
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M1, "HHUSTR_E6M1" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M2, "HHUSTR_E6M2" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M3, "HHUSTR_E6M3" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M4, "HHUSTR_E6M4" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M5, "HHUSTR_E6M5" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M6, "HHUSTR_E6M6" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M7, "HHUSTR_E6M7" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M8, "HHUSTR_E6M8" },
    { {     0,     0,     0 }, &s_HERETIC_HUSTR_E6M9, "HHUSTR_E6M9" },

  // Automap
    { {  1868,  2268,  2268 }, &s_HERETIC_AUTOPAGE, "AUTOPAGE" },
    { {  1880,  2280,  2280 }, &s_HERETIC_AMSTR_FOLLOWON, "AMSTR_FOLLOWON" },
    { {  1896,  2296,  2296 }, &s_HERETIC_AMSTR_FOLLOWOFF, "AMSTR_FOLLOWOFF" },

    //{ {  3748,  4208,  4208 }, "SAVE GAME" },  // I don't know what this is...
    { {  3896,  4416,  4416 }, &s_HERETIC_TXT_GAMESAVED, "GGSAVED" },

  // Story screen
    { {  4016,  4456,  4456 }, &s_HERETIC_E1TEXT, "HE1TEXT" },
    { {  4536,  4976,  4976 }, &s_HERETIC_E2TEXT, "HE2TEXT" },
    { {  5068,  5508,  5508 }, &s_HERETIC_E3TEXT, "HE3TEXT" },
    { {     0,  6072,  6072 }, &s_HERETIC_E4TEXT, "HE4TEXT" },
    { {     0,  6780,  6780 }, &s_HERETIC_E5TEXT, "HE5TEXT" },
    { {  5632,  7468,  7468 }, &s_HERETIC_BGFLATE1, "BGFLATE1" },
    { {  5640,  7476,  7476 }, &s_HERETIC_BGFLATE2, "BGFLATE2" },
    { {  5652,  7488,  7488 }, &s_HERETIC_BGFLATE3, "BGFLATE3" },
    { {     0,  7500,  7500 }, &s_HERETIC_BGFLATE4, "BGFLATE4" },
    { {     0,  7508,  7508 }, &s_HERETIC_BGFLATE5, "BGFLATE5" },

    { {  6744,  8612,  8612 }, &s_HERETIC_IN_KILLERS1, "TXT_KILLERS1" },
    { {  6748,  8616,  8616 }, &s_HERETIC_IN_KILLERS2, "TXT_KILLERS2" },
    { {  6752,  8620,  8620 }, &s_HERETIC_IN_KILLERS3, "TXT_KILLERS3" },
    { {  6756,  8624,  8624 }, &s_HERETIC_IN_KILLERS4, "TXT_KILLERS4" },
    { {  6760,  8628,  8628 }, &s_HERETIC_IN_KILLERS5, "TXT_KILLERS5" },
    { {  6764,  8632,  8632 }, &s_HERETIC_IN_KILLERS6, "TXT_KILLERS6" },

  // Intermission
    { {  6940,  8808,  8808 }, &s_HERETIC_IN_BGFLAT, "WI_BGFLAT" },
    { {  6948,  8816,  8816 }, &s_HERETIC_IN_FINISHED, "WI_FINISHED" },
    { {  6960,  8828,  8828 }, &s_HERETIC_IN_ENTERING, "WI_ENTERING" },
    { {  6976,  8844,  8844 }, &s_HERETIC_IN_KILLS, "TXT_IMKILLS" },
    { {  6984,  8852,  8852 }, &s_HERETIC_IN_ITEMS, "TXT_IMITEMS" },
    { {  6992,  8860,  8860 }, &s_HERETIC_IN_SECRETS, "TXT_IMSECRETS" },
    { {  7000,  8868,  8868 }, &s_HERETIC_IN_TIME, "TXT_IMTIME" },
    { {  7024,  8892,  8892 }, &s_HERETIC_IN_TOTAL, "SCORE_TOTAL" },
    { {  7032,  8900,  8900 }, &s_HERETIC_IN_VICTIMS, "SCORE_VICTIMS" },
    { {  7008,  8876,  8876 }, &s_HERETIC_COOP_BONUS, "SCORE_BONUS" },
    { {  7016,  8884,  8884 }, &s_HERETIC_COOP_SECRET, "SCORE_SECRET" },

  // Main Menu
    { {  7044,  8912,  8912 }, &s_HERETIC_MNU_NEW_GAME, "MNU_NEWGAME" },
    { {  7056,  8924,  8924 }, &s_HERETIC_MNU_OPTIONS, "MNU_OPTIONS" },
    { {  7064,  8932,  8932 }, &s_HERETIC_MNU_GAME_FILES, "MNU_GAMEFILES" },
    { {  7076,  8944,  8944 }, &s_HERETIC_MNU_INFO, "MNU_INFO" },
    { {  7084,  8952,  8952 }, &s_HERETIC_MNU_QUIT_GAME, "MNU_QUITGAME" },

  // Episode Strings
    { {  7096,  8964,  8964 }, &s_HERETIC_EPISODE_1, "MNU_COTD" },
    { {  7116,  8984,  8984 }, &s_HERETIC_EPISODE_2, "MNU_HELLSMAW" },
    { {  7128,  8996,  8996 }, &s_HERETIC_EPISODE_3, "MNU_DOME" },
    { {     0,  9020,  9020 }, &s_HERETIC_EPISODE_4, "MNU_OSSUARY" },
    { {     0,  9032,  9032 }, &s_HERETIC_EPISODE_5, "MNU_DEMESNE" },
  
  // Game Files Menu
    { {  7152,  9056,  9056 }, &s_HERETIC_MNU_LOAD_GAME, "MNU_LOADGAME" },
    { {  7164,  9068,  9068 }, &s_HERETIC_MNU_SAVE_GAME, "MNU_SAVEGAME" },

  // Skill Strings
    { {  7176,  9080,  9080 }, &s_HERETIC_SKILL_1, "MNU_WETNURSE" },
    { {  7204,  9108,  9108 }, &s_HERETIC_SKILL_2, "MNU_YELLOWBELLIES" },
    { {  7224,  9128,  9128 }, &s_HERETIC_SKILL_3, "MNU_BRINGEST" },
    { {  7244,  9148,  9148 }, &s_HERETIC_SKILL_4, "MNU_SMITE" },
    { {  7272,  9176,  9176 }, &s_HERETIC_SKILL_5, "MNU_BLACKPLAGUE" },

  // More Menu Stuff
    { {  7364,  9268,  9268 }, &s_HERETIC_MNU_SFX_VOL, "MNU_SFXVOL" },
    { {  7376,  9280,  9280 }, &s_HERETIC_MNU_MUSIC_VOL, "MNU_MUSICVOL" },
    { {  7416,  9296,  9296 }, &s_HERETIC_RAVENQUITMSG, "RAVENQUITMSG" },
    { {  7448,  9328,  9328 }, &s_HERETIC_ENDGAME, "ENDGAME" },
    { {  7668,  9648,  9648 }, &s_HERETIC_MSGON, "MSGON" },
    { {  7680,  9660,  9660 }, &s_HERETIC_MSGOFF, "MSGOFF" },
    { {  7748,  9676,  9676 }, &s_HERETIC_SWSTRING, "SWSTRING" },
  
    { {  9028, 10956, 10956 }, &s_HERETIC_TXT_NEEDBLUEKEY, "TXT_NEEDBLUEKEY" },
    { {  9068, 10996, 10996 }, &s_HERETIC_TXT_NEEDYELLOWKEY, "TXT_NEEDYELLOWKEY" },
    { {  9108, 11036, 11036 }, &s_HERETIC_TXT_NEEDGREENKEY, "TXT_NEEDGREENKEY" },
  
    { {  9244, 11172, 11172 }, &s_HERETIC_TXT_ITEMHEALTH, "TXT_ITEMHEALTH" },
    { {  9260, 11188, 11188 }, &s_HERETIC_TXT_ITEMSHIELD1, "TXT_ITEMSHIELD1" },
    { {  9276, 11204, 11204 }, &s_HERETIC_TXT_ITEMSHIELD2, "TXT_ITEMSHIELD2" },
    { {  9296, 11224, 11224 }, &s_HERETIC_TXT_ITEMBAGOFHOLDING, "TXT_ITEMBAGOFHOLDING" },
    { {  9312, 11240, 11240 }, &s_HERETIC_TXT_ITEMSUPERMAP, "TXT_ITEMSUPERMAP" },
  
    { {  9324, 11252, 11252 }, &s_HERETIC_TXT_GOTBLUEKEY, "TXT_GOTBLUEKEY" },
    { {  9336, 11264, 11264 }, &s_HERETIC_TXT_GOTYELLOWKEY, "TXT_GOTYELLOWKEY" },
    { {  9348, 11276, 11276 }, &s_HERETIC_TXT_GOTGREENKEY, "TXT_GOTGREENKEY" },
  
    { {  9360, 11288, 11288 }, &s_HERETIC_TXT_ARTIHEALTH, "TXT_ARTIHEALTH" },
    { {  9376, 11304, 11304 }, &s_HERETIC_TXT_ARTIFLY, "TXT_ARTIFLY" },
    { {  9392, 11320, 11320 }, &s_HERETIC_TXT_ARTIINVULNERABILITY, "TXT_ARTIINVULNERABILITY" },
    { {  9416, 11344, 11344 }, &s_HERETIC_TXT_ARTITOMEOFPOWER, "TXT_ARTITOMEOFPOWER" },
    { {  9432, 11360, 11360 }, &s_HERETIC_TXT_ARTIINVISIBILITY, "TXT_ARTIINVISIBILITY" },
    { {  9448, 11376, 11376 }, &s_HERETIC_TXT_ARTIEGG, "TXT_ARTIEGG" },
    { {  9460, 11388, 11388 }, &s_HERETIC_TXT_ARTISUPERHEALTH, "TXT_ARTISUPERHEALTH" },
    { {  9472, 11400, 11400 }, &s_HERETIC_TXT_ARTITORCH, "TXT_ARTITORCH" },
    { {  9480, 11408, 11408 }, &s_HERETIC_TXT_ARTIFIREBOMB, "TXT_ARTIFIREBOMB" },
    { {  9508, 11436, 11436 }, &s_HERETIC_TXT_ARTITELEPORT, "TXT_ARTITELEPORT" },
  
  // Ammo strings
    { {  9524, 11452, 11452 }, &s_HERETIC_TXT_AMMOGOLDWAND1, "TXT_AMMOGOLDWAND1" },
    { {  9540, 11468, 11468 }, &s_HERETIC_TXT_AMMOGOLDWAND2, "TXT_AMMOGOLDWAND2" },
    { {  9556, 11484, 11484 }, &s_HERETIC_TXT_AMMOMACE1, "TXT_AMMOMACE1" },
    { {  9572, 11500, 11500 }, &s_HERETIC_TXT_AMMOMACE2, "TXT_AMMOMACE2" },
    { {  9596, 11524, 11524 }, &s_HERETIC_TXT_AMMOCROSSBOW1, "TXT_AMMOCROSSBOW1" },
    { {  9612, 11540, 11540 }, &s_HERETIC_TXT_AMMOCROSSBOW2, "TXT_AMMOCROSSBOW2" },
    { {  9640, 11568, 11568 }, &s_HERETIC_TXT_AMMOBLASTER1, "TXT_AMMOBLASTER1" },
    { {  9652, 11580, 11580 }, &s_HERETIC_TXT_AMMOBLASTER2, "TXT_AMMOBLASTER2" },
    { {  9664, 11592, 11592 }, &s_HERETIC_TXT_AMMOSKULLROD1, "TXT_AMMOSKULLROD1" },
    { {  9680, 11608, 11608 }, &s_HERETIC_TXT_AMMOSKULLROD2, "TXT_AMMOSKULLROD2" },
    { {  9696, 11624, 11624 }, &s_HERETIC_TXT_AMMOPHOENIXROD1, "TXT_AMMOPHOENIXROD1" },
    { {  9708, 11636, 11636 }, &s_HERETIC_TXT_AMMOPHOENIXROD2, "TXT_AMMOPHOENIXROD2" },
  
  // Weapon strings
    { {  9720, 11648, 11648 }, &s_HERETIC_TXT_WPNMACE, "TXT_WPNMACE" },
    { {  9732, 11660, 11660 }, &s_HERETIC_TXT_WPNCROSSBOW, "TXT_WPNCROSSBOW" },
    { {  9752, 11680, 11680 }, &s_HERETIC_TXT_WPNBLASTER, "TXT_WPNBLASTER" },
    { {  9764, 11692, 11692 }, &s_HERETIC_TXT_WPNSKULLROD, "TXT_WPNSKULLROD" },
    { {  9776, 11704, 11704 }, &s_HERETIC_TXT_WPNPHOENIXROD, "TXT_WPNPHOENIXROD" },
    { {  9788, 11716, 11716 }, &s_HERETIC_TXT_WPNGAUNTLETS, "TXT_WPNGAUNTLETS" },
  
  // Cheat Strings
    { { 12536, 13900, 13900 }, &s_HERETIC_TXT_CHEATGODON, "TXT_CHEATGODON" },
    { { 12548, 13912, 13912 }, &s_HERETIC_TXT_CHEATGODOFF, "TXT_CHEATGODOFF" },
    { { 12564, 13928, 13928 }, &s_HERETIC_TXT_CHEATNOCLIPON, "TXT_CHEATNOCLIPON" },
    { { 12580, 13944, 13944 }, &s_HERETIC_TXT_CHEATNOCLIPOFF, "TXT_CHEATNOCLIPOFF" },
    { { 12596, 13960, 13960 }, &s_HERETIC_TXT_CHEATWEAPONS, "TXT_CHEATWEAPONS" },
    { { 12608, 13972, 13972 }, &s_HERETIC_TXT_CHEATPOWEROFF, "TXT_CHEATPOWEROFF" },
    { { 12620, 13984, 13984 }, &s_HERETIC_TXT_CHEATPOWERON, "TXT_CHEATPOWERON" },
    { { 12632, 13996, 13996 }, &s_HERETIC_TXT_CHEATHEALTH, "TXT_CHEATHEALTH" },
    { { 12644, 14008, 14008 }, &s_HERETIC_TXT_CHEATKEYS, "TXT_CHEATKEYS" },
    { { 12656, 14020, 14020 }, &s_HERETIC_TXT_CHEATSOUNDON, "TXT_CHEATSOUNDON" },
    { { 12672, 14036, 14036 }, &s_HERETIC_TXT_CHEATSOUNDOFF, "TXT_CHEATSOUNDOFF" },
    { { 12688, 14052, 14052 }, &s_HERETIC_TXT_CHEATTICKERON, "TXT_CHEATTICKERON" },
    { { 12700, 14064, 14064 }, &s_HERETIC_TXT_CHEATTICKEROFF, "TXT_CHEATTICKEROFF" },
    { { 12712, 14076, 14076 }, &s_HERETIC_TXT_CHEATARTIFACTS1, "TXT_CHEATARTIFACTS1" },
    { { 12744, 14108, 14108 }, &s_HERETIC_TXT_CHEATARTIFACTS2, "TXT_CHEATARTIFACTS2" },
    { { 12764, 14128, 14128 }, &s_HERETIC_TXT_CHEATARTIFACTS3, "TXT_CHEATARTIFACTS3" },
    { { 12776, 14140, 14140 }, &s_HERETIC_TXT_CHEATARTIFACTSFAIL, "TXT_CHEATARTIFACTSFAIL" },
    { { 12788, 14152, 14152 }, &s_HERETIC_TXT_CHEATWARP, "TXT_CHEATWARP" },
    { { 12800, 14164, 14164 }, &s_HERETIC_TXT_CHEATCHICKENOFF, "TXT_CHEATCHICKENOFF" },
    { { 12812, 14176, 14176 }, &s_HERETIC_TXT_CHEATCHICKENON, "TXT_CHEATCHICKENON" },
    { { 12824, 14188, 14188 }, &s_HERETIC_TXT_CHEATMASSACRE, "TXT_CHEATMASSACRE" },
    { { 12836, 14200, 14200 }, &s_HERETIC_TXT_CHEATIDDQD, "TXT_CHEATIDDQD" },
    { { 12872, 14236, 14236 }, &s_HERETIC_TXT_CHEATIDKFA, "TXT_CHEATIDKFA" },

  // Extra "doom" strings
    { {     0,     0,     0 }, &s_PRESSKEY, "PRESSKEY" },
    { {     0,     0,     0 }, &s_PRESSYN, "PRESSYN" },
    { {     0,     0,     0 }, &s_GAMMALVL0, "GAMMALVL0" },
    { {     0,     0,     0 }, &s_GAMMALVL1, "GAMMALVL1" },
    { {     0,     0,     0 }, &s_GAMMALVL2, "GAMMALVL2" },
    { {     0,     0,     0 }, &s_GAMMALVL3, "GAMMALVL3" },
    { {     0,     0,     0 }, &s_GAMMALVL4, "GAMMALVL4" },
    { {     0,     0,     0 }, &s_STSTR_MUS, "STSTR_MUS" },
    { {     0,     0,     0 }, &s_STSTR_NOMUS, "STSTR_NOMUS" },
    { {     0,     0,     0 }, &savegamename, "SAVEGAMENAME" },  // Ty 05/03/98

  // Extra strings (not in hehacked)
    { {     0,     0,     0,}, &s_HERETIC_MNU_CHOOSESKILL, "MNU_CHOOSESKILL" },
    { {     0,     0,     0 }, &s_HERETIC_TXT_CHEATFLIGHTON, "TXT_CHEATFLIGHTON" },
    { {     0,     0,     0 }, &s_HERETIC_TXT_CHEATFLIGHTOFF, "TXT_CHEATFLIGHTOFF" },
    { {     0,     0,     0 }, &s_HERETIC_TXT_SHOWFPSON, "TXT_SHOWFPSON" },	// [crispy]
    { {     0,     0,     0 }, &s_HERETIC_TXT_SHOWFPSOFF, "TXT_SHOWFPSOFF" },	// [crispy]
    { {     0,     0,     0 }, &s_HERETIC_AMSTR_GRIDON, "AMSTR_GRIDON" },
    { {     0,     0,     0 }, &s_HERETIC_AMSTR_GRIDOFF, "AMSTR_GRIDOFF" },
    { {     0,     0,     0 }, &s_HERETIC_AMSTR_MARKEDSPOT, "AMSTR_MARKEDSPOT" },
    { {     0,     0,     0 }, &s_HERETIC_AMSTR_MARKSCLEARED, "AMSTR_MARKSCLEARED" },

  // Obituaries
    { {     0,     0,     0 }, &s_OB_CRUSH, "OB_CRUSH" },
    { {     0,     0,     0 }, &s_OB_SLIME, "OB_SLIME" },
    { {     0,     0,     0 }, &s_OB_LAVA, "OB_LAVA" },
    { {     0,     0,     0 }, &s_OB_KILLEDSELF, "OB_KILLEDSELF" },
    { {     0,     0,     0 }, &s_OB_VOODOO, "OB_VOODOO" },
    { {     0,     0,     0 }, &s_OB_MONTELEFRAG, "OB_MONTELEFRAG "},
    { {     0,     0,     0 }, &s_OB_DEFAULT, "OB_DEFAULT "},
    { {     0,     0,     0 }, &s_OB_MPDEFAULT, "OB_MPDEFAULT" },

    { {     0,     0,     0 }, &s_OB_DSPARIL1HIT, "OB_DSPARIL1HIT" },
    { {     0,     0,     0 }, &s_OB_DSPARIL2HIT, "OB_DSPARIL2HIT" },
    { {     0,     0,     0 }, &s_OB_HERETICIMPHIT, "OB_HERETICIMPHIT" },
    { {     0,     0,     0 }, &s_OB_IRONLICHHIT, "OB_IRONLICHHIT" },
    { {     0,     0,     0 }, &s_OB_BONEKNIGHTHIT, "OB_BONEKNIGHTHIT" },
    { {     0,     0,     0 }, &s_OB_MINOTAURHIT, "OB_MINOTAURHIT" },
    { {     0,     0,     0 }, &s_OB_WIZARDHIT, "OB_WIZARDHIT" },

    { {     0,     0,     0 }, &s_OB_CHICKEN, "OB_CHICKEN" },
    { {     0,     0,     0 }, &s_OB_BEAST, "OB_BEAST" },
    { {     0,     0,     0 }, &s_OB_CLINK, "OB_CLINK" },
    { {     0,     0,     0 }, &s_OB_DSPARIL1, "OB_DSPARIL1" },
    { {     0,     0,     0 }, &s_OB_DSPARIL2, "OB_DSPARIL2" },
    { {     0,     0,     0 }, &s_OB_HERETICIMP, "OB_HERETICIMP" },
    { {     0,     0,     0 }, &s_OB_IRONLICH, "OB_IRONLICH" },
    { {     0,     0,     0 }, &s_OB_BONEKNIGHT, "OB_BONEKNIGHT" },
    { {     0,     0,     0 }, &s_OB_MINOTAUR, "OB_MINOTAUR" },
    { {     0,     0,     0 }, &s_OB_MUMMY, "OB_MUMMY" },
    { {     0,     0,     0 }, &s_OB_MUMMYLEADER, "OB_MUMMYLEADER" },
    { {     0,     0,     0 }, &s_OB_SNAKE, "OB_SNAKE" },
    { {     0,     0,     0 }, &s_OB_WIZARD, "OB_WIZARD" },

// Normal Weapons
    { {     0,     0,     0 }, &s_OB_MPSTAFF, "OB_MPSTAFF" },
    { {     0,     0,     0 }, &s_OB_MPGAUNTLETS, "OB_MPGAUNTLETS" },
    { {     0,     0,     0 }, &s_OB_MPGOLDWAND, "OB_MPGOLDWAND" },
    { {     0,     0,     0 }, &s_OB_MPCROSSBOW, "OB_MPCROSSBOW" },
    { {     0,     0,     0 }, &s_OB_MPBLASTER, "OB_MPBLASTER" },
    { {     0,     0,     0 }, &s_OB_MPSKULLROD, "OB_MPSKULLROD" },
    { {     0,     0,     0 }, &s_OB_MPPHOENIXROD, "OB_MPPHOENIXROD" },
    { {     0,     0,     0 }, &s_OB_MPMACE, "OB_MPMACE" },

// Tome Powered Weapons
    { {     0,     0,     0 }, &s_OB_MPPSTAFF, "OB_MPPSTAFF" },
    { {     0,     0,     0 }, &s_OB_MPPGAUNTLETS, "OB_MPPGAUNTLETS" },
    { {     0,     0,     0 }, &s_OB_MPPGOLDWAND, "OB_MPPGOLDWAND" },
    { {     0,     0,     0 }, &s_OB_MPPCROSSBOW, "OB_MPPCROSSBOW" },
    { {     0,     0,     0 }, &s_OB_MPPBLASTER, "OB_MPPBLASTER" },
    { {     0,     0,     0 }, &s_OB_MPPSKULLROD, "OB_MPPSKULLROD" },
    { {     0,     0,     0 }, &s_OB_MPPPHOENIXROD, "OB_MPPPHOENIXROD" },
    { {     0,     0,     0 }, &s_OB_MPPMACE, "OB_MPPMACE" },
};

int hhe_numstrlookup = sizeof(hhe_strlookup) / sizeof(hhe_strlookup[0]);

//
// Obituaries
static dboolean hhe_procObituarySub(char *key, char *newstring)
{
  dboolean found = false;
  int actor = -1;

  // parse actor id
  if (sscanf(key, "Obituary_Deh_Actor_%d", &actor) == 1)
  {
    if (actor >= 0 && actor < num_mobj_types)
    {
      // decide which slot
      if (deh_StringEndsWith(key, "_Melee"))
        mobjinfo[actor].obituary_melee = Z_Strdup(newstring);
      else
        mobjinfo[actor].obituary = Z_Strdup(newstring);

      // guess we ignoring "_Self"?

      found = true;
    }
  }

  return found;
}

// ====================================================================
// hhe_procStrings
// Purpose: Handle REX [STRINGS] extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procStrings(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;      // holds the string value of the line
  static size_t maxstrlen = 128; // maximum string length, bumped 128 at
  // a time as needed
  // holds the final result of the string after concatenation
  static char *holdstring = NULL;
  dboolean found = false;  // looking for string continuation

  deh_log("Processing extended string substitution\n");

  if (!holdstring) holdstring = Z_Malloc(maxstrlen * sizeof(*holdstring));

  *holdstring = '\0';  // empty string to start with
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  // Ty 04/24/98 - have to allow inbuffer to start with a blank for
  // the continuations of C1TEXT etc.
  while (!dehfeof(fpin) && *inbuffer)  /* && (*inbuffer != ' ') */
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    if (*inbuffer == '#') continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer && !*holdstring) break;  // killough 11/98
    if (!*holdstring) // first one--get the key
    {
      if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
      {
        deh_log("Bad data pair in '%s'\n", inbuffer);
        continue;
      }
    }
    while (strlen(holdstring) + strlen(inbuffer) > maxstrlen) // Ty03/29/98 - fix stupid error
    {
      // killough 11/98: allocate enough the first time
      maxstrlen = strlen(holdstring) + strlen(inbuffer);
      deh_log("* increased buffer from to %ld for buffer size %d\n",
              (long)maxstrlen, (int)strlen(inbuffer));
      holdstring = Z_Realloc(holdstring, maxstrlen * sizeof(*holdstring));
    }
    // concatenate the whole buffer if continuation or the value iffirst
    strcat(holdstring, ptr_lstrip(((*holdstring) ? inbuffer : strval)));
    rstrip(holdstring);
    // delete any trailing blanks past the backslash
    // note that blanks before the backslash will be concatenated
    // but ones at the beginning of the next line will not, allowing
    // indentation in the file to read well without affecting the
    // string itself.
    if (holdstring[strlen(holdstring) - 1] == '\\')
    {
      holdstring[strlen(holdstring) - 1] = '\0';
      continue; // ready to concatenate
    }
    if (*holdstring) // didn't have a backslash, trap above would catch that
    {
      // go process the current string
      found = hhe_procStringSub(key, NULL, -1, holdstring);  // supply keyand not search string

      // Obituaries
      if (!found)
        hhe_procObituarySub(key, holdstring);

      if (!found)
        deh_log("Invalid string key '%s', substitution skipped.\n", key);

      *holdstring = '\0';  // empty string for the next one
    }
  }
}

static void HHE_String_SuggestOtherVersions(unsigned int offset)
{
    int i, v;

    for (i = 0; i < hhe_numstrlookup; ++i)
    {
        for (v = 0; v < deh_hhe_num_versions; ++v)
        {
            if (hhe_strlookup[i].offsets[v] == offset)
            {
                HHE_SuggestHereticVersion(v);
            }
        }
    }
}

// ====================================================================
// hhe_procStringSub
// Purpose: Common string parsing and handling routine for DEH and REX
// Args:    key       -- place to put the mnemonic for the string if found
//          lookfor   -- original value string to look for
//          newstring -- string to put in its place if found
// Returns: dboolean: True if string found, false if not
//
dboolean hhe_procStringSub(char *key, char *lookfor, int offset, char *newstring)
{
  dboolean found; // loop exit flag
  int i;  // looper

  found = false;

  // Never match offset 0 -> skip offset parsing.
  if (offset == 0)
    offset = -1;

  for (i = 0; i < hhe_numstrlookup; i++)
  {
    // Cache original the first time we touch this entry
    if (hhe_strlookup[i].orig == NULL)
      hhe_strlookup[i].orig = *hhe_strlookup[i].ppstr;

    // Choose match mode:
    if (offset >= 0)
      found = (hhe_strlookup[i].offsets[deh_hhe_version] == offset);
    else if (lookfor)
      found = !stricmp(hhe_strlookup[i].orig, lookfor);
    else
      found = (key && hhe_strlookup[i].lookup && !stricmp(hhe_strlookup[i].lookup, key));

    if (found)
    {
      char *t;
      *hhe_strlookup[i].ppstr = t = Z_Strdup(newstring ? newstring : ""); // orphan originalstring
      found = true;
      // Handle embedded \n's in the incoming string, convert to 0x0a's
      {
        const char *s;
        for (s = *hhe_strlookup[i].ppstr; *s; ++s, ++t)
        {
          if (*s == '\\' && (s[1] == 'n' || s[1] == 'N')) //found one
            ++s, *t = '\n';  // skip one extra for second character
          else
            *t = *s;
        }
        *t = '\0';  // cap off the target string
      }

      // Logging:
      if (offset >= 0)
        deh_log("Assigned offset %d => '%s'\n", offset, newstring ? newstring : "");
      else if (key)
        deh_log("Assigned key %s => '%s'\n", key, newstring ? newstring : "");
      else if (lookfor)
      {
        deh_log("Assigned '%.12s%s' to'%.12s%s' at key %s\n",
                lookfor, (strlen(lookfor) > 12) ? "..." : "",
                newstring ? newstring : "", (newstring && strlen(newstring) > 12) ? "..." :"",
                hhe_strlookup[i].lookup);

        // must have passed an old style string so showREX
        deh_log("*REX FORMAT:\n%s = %s\n*END REX\n",
                hhe_strlookup[i].lookup, dehReformatStr((char *)(newstring ? newstring : "")));
      }

      break;
    }
  }
  if (!found)
  {
    if (offset >= 0) // in offset mode?
    {
      HHE_String_SuggestOtherVersions(offset);
      deh_log("Could not find offset %d (hhever=%d)\n", offset, deh_hhe_version);
    }
    else // REX mode
      deh_log("Could not find '%.12s'\n", key ? key : lookfor);
  }

  return found;
}

const deh_block hhe_block_rex_strings = { "[STRINGS]", hhe_procStrings };
