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
//	HHE Text
//

#include "doomstat.h"
#include "doomdef.h"
#include "dstrings.h"  // to get the initial values

#include "deh/func.h"
#include "deh/strings.h"
#include "heretic/hhe/text.h"
#include "heretic/hhe/sprites.h"

#include "dsda/sprite.h"
#include "dsda/music.h"
#include "dsda/sfx.h"

//
// Here are the strings and offset we DO NOT support
// But we can use them to determine versions
//

//     Offsets                 String
//       v1.0   v1.2   v1.3

// These are strings that Crispy supports, but we don't.
// Keeping this so that can detect hhe version
const hhe_string_t crispystrings[] =
{
  // lumps (let's not support replacing these at this time)
    { {   228,   228,   228 }, "PLAYPAL" },
    { {  1964,  2364,  2364 }, "FONTA_S" },
    { {  1964,  2364,  2364 }, "FONTA_S" },
    { {  2028,  2428,  2428 }, "FONTA59" },
    { {  2036,  2504,  2504 }, "PAUSED" },
    { {  2072,  2540,  2540 }, "ADVISOR" },
    { {  2080,  2548,  2548 }, "TITLE" },
    { {  2088,  2556,  2556 }, "demo1" },
    { {  2096,  2564,  2564 }, "CREDIT" },
    { {  2104,  2572,  2572 }, "demo2" },
    { {  2112,  2580,  2580 }, "ORDER" },
    { {  2120,  2588,  2588 }, "demo3" },
    { {     0,  3128,  3128 }, "EXTENDED" }, // huh
    { {  2740,  3140,  3140 }, "LOADING" },
    { {  3908,  4132,  4132 }, "SKY1" },
    { {  3916,  4140,  4140 }, "SKY2" },
    { {  3924,  4148,  4148 }, "SKY3" },
    { {  5664,  7516,  7516 }, "FONTA_S" },
    { {  5704,  7524,  7524 }, "PLAYPAL" },
    { {  5672,  7532,  7532 }, "FINAL1" },
    { {  5680,  7540,  7540 }, "FINAL2" },
    { {  5688,  7548,  7548 }, "E2PAL" },
    { {  5696,  7556,  7556 }, "E2END" },
    { {  7884,  7564,  7564 }, "TITLE" },
    { {  5712,  7572,  7572 }, "ORDER" },
    { {     0,  7580,  7580 }, "CREDIT" },
    { {  6768,  8636,  8636 }, "PLAYPAL" },
    { {  6776,  8644,  8644 }, "MAPE1" },
    { {  6784,  8652,  8652 }, "MAPE2" },
    { {  6792,  8660,  8660 }, "MAPE3" },
    { {  6800,  8668,  8668 }, "IN_X" },
    { {  6808,  8676,  8676 }, "IN_YAH" },
    { {  6816,  8684,  8684 }, "FONTB16" },
    { {  6824,  8692,  8692 }, "FONTB_S" },
    { {  6832,  8700,  8700 }, "FONTB13" },
    { {  6840,  8708,  8708 }, "FONTB15" },
    { {  6848,  8716,  8716 }, "FONTB05" },
    { {  6856,  8724,  8724 }, "FACEA0" },
    { {  6864,  8732,  8732 }, "FACEB0" },
    { {  7392,  9448,  9448 }, "M_SKL00" },
    { {  7400,  9456,  9456 }, "FONTA_S" },
    { {  7408,  9464,  9464 }, "FONTB_S" },
    { {  7572,  9476,  9476 }, "M_SLCTR1" },
    { {  7584,  9488,  9488 }, "M_SLCTR2" },
    { {  7596,  9500,  9500 }, "M_HTIC" },
    { {  7652,  9556,  9556 }, "M_FSLOT" },
    { {  7792,  9720,  9720 }, "PLAYPAL" },
    { {     0,  9812,  9812 }, "TITLE" },
    { {  7892,  9820,  9820 }, "M_SLDLT" },
    { {  7900,  9828,  9828 }, "M_SLDMD1" },
    { {  7912,  9840,  9840 }, "M_SLDMD2" },
    { {  7924,  9852,  9852 }, "M_SLDRT" },
    { {  7932,  9860,  9860 }, "M_SLDKB" },
    { { 10148, 12076, 12076 }, "END" },
    { { 10236, 12164, 12164 }, "texture2" },
    { { 10444, 12372, 12372 }, "PLAYPAL" },
    { { 10596, 12488, 12488 }, "PNAMES" },
    { { 10604, 12496, 12496 }, "TEXTURE1" },
    { { 10616, 12508, 12508 }, "TEXTURE2" },
    { { 10628, 12520, 12520 }, "S_END" },
    { { 10636, 12528, 12528 }, "S_START" },
    { { 10728, 12620, 12620 }, "F_START" },
    { { 10736, 12628, 12628 }, "F_END" },
    { { 10744, 12636, 12636 }, "COLORMAP" },
    { { 10948, 12772, 12772 }, "TINTTAB" },
    { { 10984, 12780, 12780 }, "FLOOR04" }, // stbar bg - shareware
    { { 10992, 12788, 12788 }, "FLAT513" }, // stbar bg
    { { 11000, 12796, 12796 }, "bordt" },
    { { 11008, 12804, 12804 }, "bordb" },
    { { 11016, 12812, 12812 }, "bordl" },
    { { 11024, 12820, 12820 }, "bordr" },
    { { 11032, 12828, 12828 }, "bordtl" },
    { { 11040, 12836, 12836 }, "bordtr" },
    { { 11048, 12844, 12844 }, "bordbr" },
    { { 11056, 12852, 12852 }, "bordbl" },
    { { 11164, 12960, 12960 }, "F_SKY1" },
    { { 12120, 13484, 13484 }, "LTFACE" },
    { { 12128, 13492, 13492 }, "RTFACE" },
    { { 12136, 13500, 13500 }, "BARBACK" },
    { { 12144, 13508, 13508 }, "INVBAR" },
    { { 12152, 13516, 13516 }, "CHAIN" },
    { { 12160, 13524, 13524 }, "STATBAR" },
    { { 12168, 13532, 13532 }, "LIFEBAR" },
    { { 12176, 13540, 13540 }, "LIFEGEM2" },
    { { 12188, 13552, 13552 }, "LIFEGEM0" },
    { { 12200, 13564, 13564 }, "LTFCTOP" },
    { { 12208, 13572, 13572 }, "RTFCTOP" },
    { { 12224, 13580, 13580 }, "SELECTBOX" },
    { { 12236, 13592, 13592 }, "INVGEML1" },
    { { 12248, 13604, 13604 }, "INVGEML2" },
    { { 12260, 13616, 13616 }, "INVGEMR1" },
    { { 12272, 13628, 13628 }, "INVGEMR2" },
    { { 12284, 13640, 13640 }, "BLACKSQ" },
    { { 12292, 13648, 13648 }, "ARMCLEAR" },
    { { 12304, 13660, 13660 }, "CHAINBACK" },
    { { 12316, 13672, 13672 }, "IN0" },
    { { 12320, 13676, 13676 }, "NEGNUM" },
    { { 12328, 13684, 13684 }, "FONTB16" },
    { { 12336, 13692, 13692 }, "SMALLIN0" },
    { { 12348, 13704, 13704 }, "PLAYPAL" },
    { { 12356, 13712, 13712 }, "SPINBK0" },
    { { 12364, 13720, 13720 }, "SPFLY0" },
    { { 12372, 13728, 13728 }, "LAME" },
    { { 12472, 13828, 13828 }, "GOD1" },
    { { 12480, 13836, 13836 }, "GOD2" },
    { { 12488, 13844, 13844 }, "useartia" },
    { { 12500, 13856, 13856 }, "ykeyicon" },
    { { 12512, 13868, 13868 }, "gkeyicon" },
    { { 12524, 13880, 13880 }, "bkeyicon" },
    { { 12216, 13892, 13892 }, "ARTIBOX" },

    // Animated lumps
    { { 10088, 12016, 12016 }, "FLTWAWA1" },
    { { 10100, 12028, 12028 }, "FLTFLWW1" },
    { { 10112, 12040, 12040 }, "FLTLAVA1" },
    { { 10124, 12052, 12052 }, "FLATHUH1" },
    { { 10136, 12064, 12064 }, "FLTSLUD1" },

    // Messages we don't use
    { {  1924,  2324,  2324 }, "GREEN:  " },
    { {  1936,  2336,  2336 }, "YELLOW:  " },
    { {  1948,  2348,  2348 }, "RED:  " },
    { {  1956,  2356,  2356 }, "BLUE:  " },
    { {  1972,  2372,  2372 }, "-MESSAGE SENT-" },
    { {  1988,  2388,  2388 }, "THERE ARE NO OTHER PLAYERS IN THE GAME!" },
    { {     0,  3752,  3752 }, "PLAYER 1 LEFT THE GAME" },
    { {  3736,  4196,  4196 }, "NET GAME" },
    { {  3748,  4208,  4208 }, "SAVE GAME" },
    { {  7040,  8908,  8908 }, ":" }, // maybe we can add this :)
    { {  7300,  9204,  9204 }, "END GAME" },
    { {  7312,  9216,  9216 }, "MESSAGES : " },
    { {  7660,  9564,  9564 }, "ON" },
    { {  7664,  9568,  9568 }, "OFF" },
    { {  7324,  9228,  9228 }, "MOUSE SENSITIVITY" },
    { {  7344,  9248,  9248 }, "MORE..." },
    { {  7352,  9256,  9256 }, "SCREEN SIZE" },
    { {  7488,  9368,  9368 }, "DO YOU WANT TO QUICKSAVE THE GAME NAMED" },
    { {  7528,  9408,  9408 }, "DO YOU WANT TO QUICKLOAD THE GAME NAMED" },
    { {  7568,  9472,  9472 }, "?" }, // maybe we can add this :)
    { {     0,  9572,  9572 }, "YOU CAN'T START A NEW GAME IN NETPLAY!" },
    { {     0,  9612,  9612 }, "YOU CAN'T LOAD A GAME IN NETPLAY!" },
    { {  7800,  9728,  9728 }, "QUICKSAVING...." },
    { {  7816,  9744,  9744 }, "QUICKLOADING...." },
    { {  7836,  9764,  9764 }, "CHOOSE A QUICKSAVE SLOT" },
    { {  7860,  9788,  9788 }, "CHOOSE A QUICKLOAD SLOT" },
    { {  9016, 10944, 10944 }, "SCREEN SHOT" },

    // Console stuff
    { {  2304,  2696,  2696 }, "Exited from HERETIC.\n" },
    { {  2412,  2800,  2800 }, "c:\\heretic.cd" },
    { {  2528,  2916,  2916 }, "Playing demo %s.lmp.\n" },
    { {  2592,  2980,  2980 }, "V_Init: allocate screens.\n" },
    { {  2620,  3008,  3008 }, "M_LoadDefaults: Load system defaults.\n" },
    { {  2660,  3048,  3048 }, "Z_Init: Init zone memory allocation daemon.\n" },
    { {  2708,  3096,  3096 }, "W_Init: Init WADfiles.\n" },
    { {  2748,  3148,  3148 }, "DeathMatch..." },
    { {  2764,  3164,  3164 }, "No Monsters..." },
    { {  2780,  3180,  3180 }, "Respawning..." },
    { {  2796,  3196,  3196 }, "Warp to Episode %d, Map %d, Skill %d " },
    { {  2836,  3236,  3236 }, "MN_Init: Init menu system.\n" },
    { {  2864,  3264,  3264 }, "R_Init: Init Heretic refresh daemon." },
    { {  2904,  3304,  3304 }, "Loading graphics" },
    { {  2924,  3324,  3324 }, "P_Init: Init Playloop state." },
    { {  2956,  3356,  3356 }, "Init game engine." },
    { {  2976,  3376,  3376 }, "I_Init: Setting up machine state.\n" },
    { {  3012,  3412,  3412 }, "D_CheckNetGame: Checking network game status.\n" },
    { {  3060,  3460,  3460 }, "Checking network game status." },
    { {  3092,  3492,  3492 }, "SB_Init: Loading patches.\n" },
    { {  3508,  3932,  3932 }, "Network game synchronization aborted." },
    { {     0,  3972,  3972 }, "Different DOOM versions cannot play a net game!" },
    { {  3760,  4220,  4220 }, "Only %i deathmatch spots, 4 required" },
    { {  3800,  4260,  4260 }, "version %i" },
    { {  3828,  4372,  4372 }, "c:\\heretic.cd\\hticsav%d.hsg" },
    { {  3856,  4400,  4400 }, "hticsav%d.hsg" },
    { {  7604,  9508,  9508 }, "c:\\heretic.cd\\hticsav%d.hsg" },
    { {  7632,  9536,  9536 }, "hticsav%d.hsg" },
    { { 10756, 12648, 12648 }, "\nR_InitTextures " },
    { { 10776, 12668, 12668 }, "R_InitFlats\n" },
    { { 10792, 12684, 12684 }, "R_InitSpriteLumps " },
    { { 11064, 12860, 12860 }, "R_InitData " },
    { { 11076, 12872, 12872 }, "R_InitPointToAngle\n" },
    { { 11096, 12892, 12892 }, "R_InitTables " },
    { { 11112, 12908, 12908 }, "R_InitPlanes\n" },
    { { 11128, 12924, 12924 }, "R_InitLightTables " },
    { { 11148, 12944, 12944 }, "R_InitSkyMap\n" },
    { { 12380, 13736, 13736 }, "*** SOUND DEBUG INFO ***" },
    { { 12408, 13764, 13764 }, "NAME" },
    { { 12416, 13772, 13772 }, "MO.T" },
    { { 12424, 13780, 13780 }, "MO.X" },
    { { 12432, 13788, 13788 }, "MO.Y" },
    { { 12440, 13796, 13796 }, "ID" },
    { { 12444, 13800, 13800 }, "PRI" },
    { { 12448, 13804, 13804 }, "DIST" },
    { { 12456, 13812, 13812 }, "------" },
    { { 12464, 13820, 13820 }, "%s" },
    { { 12468, 13824, 13824 }, "%d" },

    // No idea...
    { {  2732,  3120,  3120 }, "E2M1" },
};

int hhe_numcrispystrings = sizeof(crispystrings) / sizeof(crispystrings[0]);

// [Crispy] String offsets that are valid but we don't support.
static const int unsupported_strings_1_0[] =
{
        0,     4,    64,   104,   160,   200,   220,   236,
      244,   252,   272,   288,   296,   316,   332,   372,
      436,   500,   504,   536,   544,   560,   576,   584,
      592,   612,   640,   664,   708,   712,   744,   764,
      808,   820,   828,   840,   876,   884,   908,   952,
      992,  1028,  1036,  1048,  1088,  1128,  1160,  1192,
     1212,  1912,  2044,  2056,  2068,  2128,  2140,  2168,
     2184,  2196,  2212,  2228,  2240,  2252,  2260,  2264,
     2284,  2292,  2296,  2300,  2328,  2340,  2352,  2364,
     2372,  2384,  2388,  2404,  2428,  2436,  2444,  2464,
     2496,  2508,  2520,  2552,  2564,  2572,  2584,  3120,
     3128,  3140,  3184,  3220,  3248,  3252,  3256,  3280,
     3304,  3320,  3352,  3380,  3400,  3432,  3464,  3548,
     3600,  3624,  3664,  3696,  3812,  3872,  3932,  3940,
     3976,  3996,  6872,  6896,  7648,  7696,  7940,  7964,
     7968,  7992,  8020,  8028,  8052,  8056,  8076,  8088,
     8104,  8116,  8128,  8136,  8148,  8164,  8180,  8192,
     8204,  8220,  8232,  8248,  8264,  8276,  8292,  8308,
     8320,  8328,  8340,  8352,  8364,  8376,  8392,  8408,
     8424,  8436,  8448,  8460,  8472,  8488,  8504,  8520,
     8536,  8548,  8560,  8572,  8584,  8596,  8608,  8612,
     8624,  8648,  8660,  8668,  8680,  8708,  8720,  8728,
     8740,  8752,  8764,  8788,  8800,  8812,  8824,  8848,
     8860,  8864,  8868,  8876,  8888,  8896,  8916,  8944,
     8948,  8960,  8964,  8968,  8980,  9148,  9172,  9212,
     9216,  9220,  9820,  9860,  9892,  9940,  9972, 10012,
    10036, 10040, 10052, 10080, 10152, 10192, 10248, 10284,
    10320, 10360, 10392, 10452, 10488, 10508, 10556, 10644,
    10684, 10812, 10844, 10880, 10912, 10956, 11172, 11200,
    11232, 11272, 11312, 11348, 11380, 11404, 11436, 11492,
    11548, 11616, 11684, 11748, 11792, 11840, 11896, 11936,
    11980, 12028, 12072, 12908, 12924, 12956, 12960, 12968,
    12976, 13020, 13048, 13076, 13104, 13136, 13168, 13196,
    13240, 13272, 13292, 13296, 13308, 13312, 13320, 13324,
    13364, 13408, 13460, 13492, 13516, 13560, 13612, 13664,
    13700, 13744, 13796, 13848, 13884, 13940, 13996, 14040,
    14084, 14140, 14148, 14156, 14164, 14184, 14192, 14204,
    14208, 14212, 14256, 14272, 14284, 14296, 14300, 14312,
    14320, 14324, 14348, 14356, 14360, 14372, 14380, 14392,
    14432, 14440, 14444, 14472, 14496, 14516, 14536, 14548,
    14560, 14572, 14580, 14588, 14596, 14604, 14612, 14620,
    14636, 14660, 14704, 14740, 14748, 14756, 14760, 14768,
       -1,
};

static const int unsupported_strings_1_2[] =
{
        0,     4,    64,   104,   160,   200,   220,   236,
      244,   252,   272,   288,   296,   316,   332,   372,
      436,   500,   504,   536,   544,   560,   576,   584,
      592,   612,   640,   664,   708,   712,   744,   756,
      776,   820,   832,   840,   852,   888,   896,   920,
      964,  1004,  1040,  1048,  1060,  1100,  1140,  1172,
     1204,  1224,  2312,  2436,  2448,  2464,  2480,  2492,
     2512,  2524,  2536,  2596,  2608,  2636,  2652,  2656,
     2676,  2684,  2688,  2720,  2732,  2744,  2752,  2764,
     2772,  2776,  2792,  2816,  2824,  2832,  2852,  2884,
     2896,  2908,  2940,  2952,  2960,  2972,  3520,  3528,
     3540,  3584,  3620,  3648,  3652,  3656,  3680,  3704,
     3720,  3776,  3804,  3824,  3856,  3888,  4020,  4044,
     4084,  4116,  4156,  4272,  4288,  4296,  4332,  4352,
     4428,  4432,  8740,  8764,  9552,  9868,  9888,  9900,
     9916,  9928,  9940,  9948,  9960,  9976,  9992, 10004,
    10016, 10032, 10044, 10060, 10076, 10088, 10104, 10120,
    10132, 10140, 10152, 10164, 10176, 10188, 10204, 10220,
    10236, 10248, 10260, 10272, 10284, 10300, 10316, 10332,
    10348, 10360, 10372, 10384, 10396, 10408, 10420, 10424,
    10436, 10460, 10472, 10480, 10492, 10520, 10532, 10540,
    10552, 10564, 10576, 10600, 10612, 10624, 10636, 10660,
    10672, 10676, 10700, 10704, 10728, 10756, 10764, 10788,
    10792, 10796, 10804, 10816, 10824, 10844, 10872, 10876,
    10888, 10892, 10896, 10908, 11076, 11100, 11140, 11144,
    11148, 11748, 11788, 11820, 11868, 11900, 11940, 11964,
    11968, 11980, 12008, 12080, 12120, 12176, 12212, 12248,
    12288, 12320, 12380, 12400, 12448, 12536, 12576, 12704,
    12736, 12968, 13000, 13024, 13080, 13136, 13204, 13272,
    13336, 13380, 13428, 14272, 14288, 14320, 14324, 14332,
    14340, 14384, 14412, 14440, 14468, 14500, 14532, 14560,
    14604, 14636, 14656, 14696, 14740, 14792, 14824, 14848,
    14892, 14944, 14996, 15032, 15076, 15128, 15180, 15216,
    15272, 15328, 15372, 15416, 15472, 15480, 15488, 15496,
    15516, 15524, 15536, 15540, 15544, 15588, 15604, 15616,
    15628, 15632, 15644, 15652, 15656, 15680, 15688, 15692,
    15704, 15712, 15724, 15764, 15772, 15776, 15804, 15828,
    15848, 15868, 15880, 15892, 15904, 15912, 15920, 15928,
    15936,    -1,
};

static const int unsupported_strings_1_3[] =
{
        0,     4,    64,   104,   160,   200,   220,   236,
      244,   252,   272,   288,   296,   316,   332,   372,
      436,   500,   504,   536,   544,   560,   576,   584,
      592,   612,   640,   664,   708,   712,   744,   756,
      776,   820,   832,   840,   852,   888,   896,   920,
      964,  1004,  1040,  1048,  1060,  1100,  1140,  1172,
     1204,  1224,  2312,  2436,  2448,  2464,  2480,  2492,
     2512,  2524,  2536,  2596,  2608,  2636,  2652,  2656,
     2676,  2684,  2688,  2720,  2732,  2744,  2752,  2764,
     2772,  2776,  2792,  2816,  2824,  2832,  2852,  2884,
     2896,  2908,  2940,  2952,  2960,  2972,  3520,  3528,
     3540,  3584,  3620,  3648,  3652,  3656,  3680,  3704,
     3720,  3776,  3804,  3824,  3856,  3888,  4020,  4044,
     4084,  4116,  4156,  4272,  4288,  4296,  4332,  4352,
     4428,  4432,  8740,  8764,  9552,  9868,  9888,  9900,
     9916,  9928,  9940,  9948,  9960,  9976,  9992, 10004,
    10016, 10032, 10044, 10060, 10076, 10088, 10104, 10120,
    10132, 10140, 10152, 10164, 10176, 10188, 10204, 10220,
    10236, 10248, 10260, 10272, 10284, 10300, 10316, 10332,
    10348, 10360, 10372, 10384, 10396, 10408, 10420, 10424,
    10436, 10460, 10472, 10480, 10492, 10520, 10532, 10540,
    10552, 10564, 10576, 10600, 10612, 10624, 10636, 10660,
    10672, 10676, 10700, 10704, 10728, 10756, 10764, 10788,
    10792, 10796, 10804, 10816, 10824, 10844, 10872, 10876,
    10888, 10892, 10896, 10908, 11076, 11100, 11140, 11144,
    11148, 11748, 11788, 11820, 11868, 11900, 11940, 11964,
    11968, 11980, 12008, 12080, 12120, 12176, 12212, 12248,
    12288, 12320, 12380, 12400, 12448, 12536, 12576, 12704,
    12736, 12968, 13000, 13024, 13080, 13136, 13204, 13272,
    13336, 13380, 13428, 14272, 14288, 14320, 14324, 14332,
    14340, 14384, 14412, 14440, 14468, 14500, 14532, 14560,
    14604, 14636, 14656, 14696, 14740, 14792, 14824, 14848,
    14892, 14944, 14996, 15032, 15076, 15128, 15180, 15216,
    15272, 15328, 15372, 15416, 15472, 15480, 15488, 15496,
    15516, 15524, 15536, 15540, 15544, 15588, 15604, 15616,
    15628, 15632, 15644, 15652, 15656, 15680, 15688, 15692,
    15704, 15712, 15724, 15764, 15772, 15776, 15804, 15828,
    15848, 15868, 15880, 15892, 15904, 15912, 15920, 15928,
    15936,    -1,
};

const int *unsupported_strings[] =
{
    unsupported_strings_1_0,
    unsupported_strings_1_2,
    unsupported_strings_1_3,
};

// Stuff for renaming sprites
//
// yaaaay - heretic why

static const hhe_sprite_str_t *HHE_FindSpriteStringByOffset(int offset)
{
  size_t i;

  if (offset == 0)
    return NULL;

  for (i = 0; i < hhe_numsprites; ++i)
    if (hhe_sprites[i].offsets[deh_hhe_version] == offset)
      return &hhe_sprites[i];

  return NULL;
}

dboolean HHE_RenameSprite(int spritenum, const char *newname, const char *context, int offset)
{
  char name[5];
  int j;

  if (spritenum < 0)
    return false;

  // Need at least 4 bytes.
  for (j = 0; j < 4; ++j)
  {
    unsigned char c = (unsigned char)newname[j];
    if (c == '\0')
      return false;
    name[j] = (char)toupper(c);
  }
  name[4] = '\0';

  if (context)
    deh_log("Sprite rename (%s): idx=%d old=%s new=%s\n",
            context, spritenum, sprnames[spritenum] ? sprnames[spritenum] : "(null)", name);
  else if (offset)
    deh_log("Sprite rename (offset %d): idx=%d old=%s new=%s\n",
            offset, spritenum, sprnames[spritenum] ? sprnames[spritenum] : "(null)", name);
  else
    deh_log("Sprite rename: idx=%d old=%s new=%s\n",
            spritenum, sprnames[spritenum] ? sprnames[spritenum] : "(null)", name);

  sprnames[spritenum] = Z_Strdup(name);
  return true;
}

// TEXT - Hehacked block name = "Text"
// Usage: Text offset length
// Unlike Dehacked, Hehacked uses the specific offset (WHY??)
// Hehacked allows a bit of adjustment to the length

// flag to skip included hhe-style text, used with INCLUDE NOTEXT directive
dboolean hhe_includenotext = false;

// ====================================================================
// hhe_procText
// Purpose: Handle HHE Text block
// Notes:   We look things up in the current information and if found
//          we replace it.  At the same time we write the new and
//          improved REX syntax to the log file for future use.
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procText(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char *repl = NULL; // replacement string (instead of the comparing two strings) - Heretic
  int offset, repl_len;  // Heretic - Yay, to parse Hehacked, we have to deal with stupid offsets... :/
  dboolean found = false;  // to allow early exit once found

  // In HHE, these are (orig_offset, replacement_length).
  if (sscanf(line, "%s %d %d", key, &offset, &repl_len) != 3)
  {
    deh_log("Bad HHE Text header: '%s'\n", line);
    return;
  }
  
  // Ty 04/11/98 - Included file may have NOTEXT skip flag set
  if (hhe_includenotext) // flag to skip included hhe-style text
  {
    int c, totlen = 0;

    deh_log("Skipped text block because of notext directive\n");

    // HHE Text blocks are length-based.
    // Even when skipping text, we must consume repl_len characters
    // to keep the file stream in sync.
    while (totlen < repl_len && (c = dehfgetc(fpin)) != EOF)
      if (c != '\r')
        ++totlen;

    // Ty 05/17/98 - don't care if this fails
    return; // ************** Early return
  }

  deh_log("Processing HHE Text (key=%s, offset=%d, repl_len=%d, hhever=%d)\n", key, offset, repl_len, deh_hhe_version);

  repl = Z_Malloc((size_t)repl_len + 1);

  // HHE uses length-based text blocks, not line-based.
  {
    int c, totlen = 0;

    while (totlen < repl_len && (c = dehfgetc(fpin)) != EOF)
      if (c != '\r')
        repl[totlen++] = (char)c;

    repl[totlen] = '\0';
  }

  // ------------------------------------------------------------
  // 1) Sprite renames (by offset table)
  //    Only attempt if this block is exactly 4 chars long.
  // ------------------------------------------------------------
  if (repl_len == 4 && offset != 0)
  {
    const hhe_sprite_str_t *sprite_rename = HHE_FindSpriteStringByOffset(offset);

    if (sprite_rename)
    {
      int i = dsda_GetDehSpriteIndex(sprite_rename->default_string);

      if (HHE_RenameSprite(i, repl, sprite_rename->default_string, offset))
        found = true;
      else // bad sprite name
        deh_log("Bad sprite name in HHE Text block (offset %d): '%.*s'\n", offset, repl_len, repl);
    }
  }

  // 2) NOTE - hehacked doesn't store sfx names, so we replace sfx in procSound
  // 3) NOTE - hehacked cannot replace music names

  if (!found) // Nothing we want to handle here--see if strings can deal with it.
  {
    found = hhe_procStringSub(NULL, NULL, offset, repl);
    if (!found)
      deh_log("Unknown string offset %d (hhever=%d): '%s'\n",
              offset, deh_hhe_version, repl);
  }

  Z_Free(repl);
}

const deh_block hhe_block_text = { "Text", hhe_procText };
