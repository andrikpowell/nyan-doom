//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// DStrings.h

//---------------------------------------------------------------------------
//
// P_inter.c
//
//---------------------------------------------------------------------------

// Keys

#define HERETIC_TXT_GOTBLUEKEY			"BLUE KEY"
#define HERETIC_TXT_GOTYELLOWKEY		"YELLOW KEY"
#define HERETIC_TXT_GOTGREENKEY			"GREEN KEY"

// Artifacts

#define HERETIC_TXT_ARTIHEALTH			"QUARTZ FLASK"
#define HERETIC_TXT_ARTIFLY				"WINGS OF WRATH"
#define HERETIC_TXT_ARTIINVULNERABILITY	"RING OF INVINCIBILITY"
#define HERETIC_TXT_ARTITOMEOFPOWER		"TOME OF POWER"
#define HERETIC_TXT_ARTIINVISIBILITY	"SHADOWSPHERE"
#define HERETIC_TXT_ARTIEGG				"MORPH OVUM"
#define HERETIC_TXT_ARTISUPERHEALTH		"MYSTIC URN"
#define HERETIC_TXT_ARTITORCH			"TORCH"
#define HERETIC_TXT_ARTIFIREBOMB		"TIME BOMB OF THE ANCIENTS"
#define HERETIC_TXT_ARTITELEPORT		"CHAOS DEVICE"

// Artifact Descriptions

#define HERETIC_ARTI_DESC_A	"Temporarily Grants Invulnerability" // Ring of Invincibility
#define HERETIC_ARTI_DESC_B	"Temporarily Obscures Presence" // Shadowsphere
#define HERETIC_ARTI_DESC_C	"Heals 25% Health" // Quartz Flask
#define HERETIC_ARTI_DESC_D	"Heals 100% Health" // Mystic Urn
#define HERETIC_ARTI_DESC_E	"Temporarily Buffs All Weapons" // Tome of Power
#define HERETIC_ARTI_DESC_F	"Temporarily Lights Up the Surroundings" // Torch
#define HERETIC_ARTI_DESC_G	"A Magical Bomb That Explodes After a Few Seconds" // Time Bomb of the Ancients
#define HERETIC_ARTI_DESC_H	"Transforms Enemies Into Chickens" // Morph Ovum
#define HERETIC_ARTI_DESC_I	"Temporarily Grants Flight" // Wings of Wrath
#define HERETIC_ARTI_DESC_J	"Instantly Teleport to the Start of the Map" // Chaos Device

// Items

#define HERETIC_TXT_ITEMHEALTH			"CRYSTAL VIAL"
#define HERETIC_TXT_ITEMBAGOFHOLDING	"BAG OF HOLDING"
#define HERETIC_TXT_ITEMSHIELD1			"SILVER SHIELD"
#define HERETIC_TXT_ITEMSHIELD2			"ENCHANTED SHIELD"
#define HERETIC_TXT_ITEMSUPERMAP		"MAP SCROLL"

// Ammo

#define HERETIC_TXT_AMMOGOLDWAND1		"WAND CRYSTAL"
#define HERETIC_TXT_AMMOGOLDWAND2		"CRYSTAL GEODE"
#define HERETIC_TXT_AMMOMACE1			"MACE SPHERES"
#define HERETIC_TXT_AMMOMACE2			"PILE OF MACE SPHERES"
#define HERETIC_TXT_AMMOCROSSBOW1		"ETHEREAL ARROWS"
#define HERETIC_TXT_AMMOCROSSBOW2		"QUIVER OF ETHEREAL ARROWS"
#define HERETIC_TXT_AMMOBLASTER1		"CLAW ORB"
#define HERETIC_TXT_AMMOBLASTER2		"ENERGY ORB"
#define HERETIC_TXT_AMMOSKULLROD1		"LESSER RUNES"
#define HERETIC_TXT_AMMOSKULLROD2		"GREATER RUNES"
#define HERETIC_TXT_AMMOPHOENIXROD1		"FLAME ORB"
#define HERETIC_TXT_AMMOPHOENIXROD2		"INFERNO ORB"

// Weapons

#define HERETIC_TXT_WPNMACE				"FIREMACE"
#define HERETIC_TXT_WPNCROSSBOW			"ETHEREAL CROSSBOW"
#define HERETIC_TXT_WPNBLASTER			"DRAGON CLAW"
#define HERETIC_TXT_WPNSKULLROD			"HELLSTAFF"
#define HERETIC_TXT_WPNPHOENIXROD		"PHOENIX ROD"
#define HERETIC_TXT_WPNGAUNTLETS		"GAUNTLETS OF THE NECROMANCER"

//---------------------------------------------------------------------------
//
// SB_bar.c
//
//---------------------------------------------------------------------------

#define HERETIC_TXT_CHEATGODON			"GOD MODE ON"
#define HERETIC_TXT_CHEATGODOFF			"GOD MODE OFF"
#define HERETIC_TXT_CHEATNOCLIPON		"NO CLIPPING ON"
#define HERETIC_TXT_CHEATNOCLIPOFF		"NO CLIPPING OFF"
#define HERETIC_TXT_CHEATWEAPONS		"ALL WEAPONS"
#define HERETIC_TXT_CHEATFLIGHTON		"FLIGHT ON"
#define HERETIC_TXT_CHEATFLIGHTOFF		"FLIGHT OFF"
#define HERETIC_TXT_CHEATPOWERON		"POWER ON"
#define HERETIC_TXT_CHEATPOWEROFF		"POWER OFF"
#define HERETIC_TXT_CHEATHEALTH			"FULL HEALTH"
#define HERETIC_TXT_CHEATKEYS			"ALL KEYS"
#define HERETIC_TXT_CHEATSOUNDON		"SOUND DEBUG ON"
#define HERETIC_TXT_CHEATSOUNDOFF		"SOUND DEBUG OFF"
#define HERETIC_TXT_CHEATTICKERON		"TICKER ON"
#define HERETIC_TXT_CHEATTICKEROFF		"TICKER OFF"
#define HERETIC_TXT_CHEATARTIFACTS1		"CHOOSE AN ARTIFACT ( A - J )"
#define HERETIC_TXT_CHEATARTIFACTS2		"HOW MANY ( 1 - 9 )"
#define HERETIC_TXT_CHEATARTIFACTS3		"YOU GOT IT"
#define HERETIC_TXT_CHEATARTIFACTSFAIL	"BAD INPUT"
#define HERETIC_TXT_CHEATWARP			"LEVEL WARP"
#define HERETIC_TXT_CHEATSCREENSHOT		"SCREEN SHOT"
#define HERETIC_TXT_CHEATCHICKENON		"CHICKEN ON"
#define HERETIC_TXT_CHEATCHICKENOFF		"CHICKEN OFF"
#define HERETIC_TXT_SHOWFPSON			"SHOW FPS ON"	// [crispy]
#define HERETIC_TXT_SHOWFPSOFF			"SHOW FPS OFF"	// [crispy]
#define HERETIC_TXT_CHEATMASSACRE		"MASSACRE"
#define HERETIC_TXT_CHEATIDDQD			"TRYING TO CHEAT, EH?  NOW YOU DIE!"
#define HERETIC_TXT_CHEATIDKFA			"CHEATER - YOU DON'T DESERVE WEAPONS"

//---------------------------------------------------------------------------
//
// P_doors.c
//
//---------------------------------------------------------------------------

#define HERETIC_TXT_NEEDBLUEKEY			"YOU NEED A BLUE KEY TO OPEN THIS DOOR"
#define HERETIC_TXT_NEEDGREENKEY		"YOU NEED A GREEN KEY TO OPEN THIS DOOR"
#define HERETIC_TXT_NEEDYELLOWKEY		"YOU NEED A YELLOW KEY TO OPEN THIS DOOR"

//---------------------------------------------------------------------------
//
// G_game.c
//
//---------------------------------------------------------------------------

#define HERETIC_TXT_GAMESAVED			"GAME SAVED"

//---------------------------------------------------------------------------
//
// AM_map.c
//
//---------------------------------------------------------------------------

#define HERETIC_AMSTR_FOLLOWON		"FOLLOW MODE ON"
#define HERETIC_AMSTR_FOLLOWOFF		"FOLLOW MODE OFF"

#define HERETIC_AMSTR_GRIDON		"Grid ON"
#define HERETIC_AMSTR_GRIDOFF		"Grid OFF"

#define HERETIC_AMSTR_MARKEDSPOT	"Marked Spot"
#define HERETIC_AMSTR_MARKSCLEARED	"All Marks Cleared"

#define HERETIC_AUTOPAGE			"AUTOPAGE"

//---------------------------------------------------------------------------
//
// m_menu.c
//
//---------------------------------------------------------------------------

#define HERETIC_SWSTRING		"ONLY AVAILABLE IN THE REGISTERED VERSION"
#define HERETIC_RAVENQUITMSG	"ARE YOU SURE YOU WANT TO QUIT?"
#define HERETIC_ENDGAME			"ARE YOU SURE YOU WANT TO END THE GAME?"

#define HERETIC_MSGON			"MESSAGES ON"
#define HERETIC_MSGOFF			"MESSAGES OFF"

#define HERETIC_MNU_NEW_GAME	"NEW GAME"
#define HERETIC_MNU_OPTIONS		"OPTIONS"
#define HERETIC_MNU_GAME_FILES	"GAME FILES"
#define HERETIC_MNU_INFO		"INFO"
#define HERETIC_MNU_QUIT_GAME	"QUIT GAME"
#define HERETIC_MNU_SAVE_GAME	"SAVE GAME"
#define HERETIC_MNU_LOAD_GAME	"LOAD GAME"
#define HERETIC_MNU_MUSIC_VOL	"MUSIC VOLUME"
#define HERETIC_MNU_SFX_VOL		"SFX VOLUME"
#define HERETIC_MNU_CHOOSESKILL "CHOOSE SKILL LEVEL:"

#define HERETIC_EPISODE_1		"CITY OF THE DAMNED"
#define HERETIC_EPISODE_2		"HELL'S MAW"
#define HERETIC_EPISODE_3		"THE DOME OF D'SPARIL"
#define HERETIC_EPISODE_4		"THE OSSUARY"
#define HERETIC_EPISODE_5		"THE STAGNANT DEMESNE"

//---------------------------------------------------------------------------
//
// level_names.c
//
//---------------------------------------------------------------------------

// EPISODE 1 - THE CITY OF THE DAMNED
#define HERETIC_HUSTR_E1M1 "E1M1:  THE DOCKS"
#define HERETIC_HUSTR_E1M2 "E1M2:  THE DUNGEONS"
#define HERETIC_HUSTR_E1M3 "E1M3:  THE GATEHOUSE"
#define HERETIC_HUSTR_E1M4 "E1M4:  THE GUARD TOWER"
#define HERETIC_HUSTR_E1M5 "E1M5:  THE CITADEL"
#define HERETIC_HUSTR_E1M6 "E1M6:  THE CATHEDRAL"
#define HERETIC_HUSTR_E1M7 "E1M7:  THE CRYPTS"
#define HERETIC_HUSTR_E1M8 "E1M8:  HELL'S MAW"
#define HERETIC_HUSTR_E1M9 "E1M9:  THE GRAVEYARD"

// EPISODE 2 - HELL'S MAW
#define HERETIC_HUSTR_E2M1 "E2M1:  THE CRATER"
#define HERETIC_HUSTR_E2M2 "E2M2:  THE LAVA PITS"
#define HERETIC_HUSTR_E2M3 "E2M3:  THE RIVER OF FIRE"
#define HERETIC_HUSTR_E2M4 "E2M4:  THE ICE GROTTO"
#define HERETIC_HUSTR_E2M5 "E2M5:  THE CATACOMBS"
#define HERETIC_HUSTR_E2M6 "E2M6:  THE LABYRINTH"
#define HERETIC_HUSTR_E2M7 "E2M7:  THE GREAT HALL"
#define HERETIC_HUSTR_E2M8 "E2M8:  THE PORTALS OF CHAOS"
#define HERETIC_HUSTR_E2M9 "E2M9:  THE GLACIER"

// EPISODE 3 - THE DOME OF D'SPARIL
#define HERETIC_HUSTR_E3M1 "E3M1:  THE STOREHOUSE"
#define HERETIC_HUSTR_E3M2 "E3M2:  THE CESSPOOL"
#define HERETIC_HUSTR_E3M3 "E3M3:  THE CONFLUENCE"
#define HERETIC_HUSTR_E3M4 "E3M4:  THE AZURE FORTRESS"
#define HERETIC_HUSTR_E3M5 "E3M5:  THE OPHIDIAN LAIR"
#define HERETIC_HUSTR_E3M6 "E3M6:  THE HALLS OF FEAR"
#define HERETIC_HUSTR_E3M7 "E3M7:  THE CHASM"
#define HERETIC_HUSTR_E3M8 "E3M8:  D'SPARIL'S KEEP"
#define HERETIC_HUSTR_E3M9 "E3M9:  THE AQUIFER"

// EPISODE 4: THE OSSUARY
#define HERETIC_HUSTR_E4M1 "E4M1:  CATAFALQUE"
#define HERETIC_HUSTR_E4M2 "E4M2:  BLOCKHOUSE"
#define HERETIC_HUSTR_E4M3 "E4M3:  AMBULATORY"
#define HERETIC_HUSTR_E4M4 "E4M4:  SEPULCHER"
#define HERETIC_HUSTR_E4M5 "E4M5:  GREAT STAIR"
#define HERETIC_HUSTR_E4M6 "E4M6:  HALLS OF THE APOSTATE"
#define HERETIC_HUSTR_E4M7 "E4M7:  RAMPARTS OF PERDITION"
#define HERETIC_HUSTR_E4M8 "E4M8:  SHATTERED BRIDGE"
#define HERETIC_HUSTR_E4M9 "E4M9:  MAUSOLEUM"

// EPISODE 5: THE STAGNANT DEMESNE
#define HERETIC_HUSTR_E5M1 "E5M1:  OCHRE CLIFFS"
#define HERETIC_HUSTR_E5M2 "E5M2:  RAPIDS"
#define HERETIC_HUSTR_E5M3 "E5M3:  QUAY"
#define HERETIC_HUSTR_E5M4 "E5M4:  COURTYARD"
#define HERETIC_HUSTR_E5M5 "E5M5:  HYDRATYR"
#define HERETIC_HUSTR_E5M6 "E5M6:  COLONNADE"
#define HERETIC_HUSTR_E5M7 "E5M7:  FOETID MANSE"
#define HERETIC_HUSTR_E5M8 "E5M8:  FIELD OF JUDGEMENT"
#define HERETIC_HUSTR_E5M9 "E5M9:  SKEIN OF D'SPARIL"

// EPISODE 6: Extra
#define HERETIC_HUSTR_E6M1 "E6M1:  UNKNOWN"
#define HERETIC_HUSTR_E6M2 "E6M2:  UNKNOWN"
#define HERETIC_HUSTR_E6M3 "E6M3:  UNKNOWN"
#define HERETIC_HUSTR_E6M4 "E6M4:  UNKNOWN"
#define HERETIC_HUSTR_E6M5 "E6M5:  UNKNOWN"
#define HERETIC_HUSTR_E6M6 "E6M6:  UNKNOWN"
#define HERETIC_HUSTR_E6M7 "E6M7:  UNKNOWN"
#define HERETIC_HUSTR_E6M8 "E6M8:  UNKNOWN"
#define HERETIC_HUSTR_E6M9 "E6M9:  UNKNOWN"

//---------------------------------------------------------------------------
//
// F_finale.c
//
//---------------------------------------------------------------------------

#define HERETIC_E1TEXT	"with the destruction of the iron\n"\
					"liches and their minions, the last\n"\
					"of the undead are cleared from this\n"\
					"plane of existence.\n\n"\
					"those creatures had to come from\n"\
					"somewhere, though, and you have the\n"\
					"sneaky suspicion that the fiery\n"\
					"portal of hell's maw opens onto\n"\
					"their home dimension.\n\n"\
					"to make sure that more undead\n"\
					"(or even worse things) don't come\n"\
					"through, you'll have to seal hell's\n"\
					"maw from the other side. of course\n"\
					"this means you may get stuck in a\n"\
					"very unfriendly world, but no one\n"\
					"ever said being a Heretic was easy!"

#define HERETIC_E2TEXT "the mighty maulotaurs have proved\n"\
					"to be no match for you, and as\n"\
					"their steaming corpses slide to the\n"\
					"ground you feel a sense of grim\n"\
					"satisfaction that they have been\n"\
					"destroyed.\n\n"\
					"the gateways which they guarded\n"\
					"have opened, revealing what you\n"\
					"hope is the way home. but as you\n"\
					"step through, mocking laughter\n"\
					"rings in your ears.\n\n"\
					"was some other force controlling\n"\
					"the maulotaurs? could there be even\n"\
					"more horrific beings through this\n"\
					"gate? the sweep of a crystal dome\n"\
					"overhead where the sky should be is\n"\
					"certainly not a good sign...."

#define HERETIC_E3TEXT	"the death of d'sparil has loosed\n"\
					"the magical bonds holding his\n"\
					"creatures on this plane, their\n"\
					"dying screams overwhelming his own\n"\
					"cries of agony.\n\n"\
					"your oath of vengeance fulfilled,\n"\
					"you enter the portal to your own\n"\
					"world, mere moments before the dome\n"\
					"shatters into a million pieces.\n\n"\
					"but if d'sparil's power is broken\n"\
					"forever, why don't you feel safe?\n"\
					"was it that last shout just before\n"\
					"his death, the one that sounded\n"\
					"like a curse? or a summoning? you\n"\
					"can't really be sure, but it might\n"\
					"just have been a scream.\n\n"\
					"then again, what about the other\n"\
					"serpent riders?"

#define HERETIC_E4TEXT		"you thought you would return to your\n"\
					"own world after d'sparil died, but\n"\
					"his final act banished you to his\n"\
					"own plane. here you entered the\n"\
					"shattered remnants of lands\n"\
					"conquered by d'sparil. you defeated\n"\
					"the last guardians of these lands,\n"\
					"but now you stand before the gates\n"\
					"to d'sparil's stronghold. until this\n"\
					"moment you had no doubts about your\n"\
					"ability to face anything you might\n"\
					"encounter, but beyond this portal\n"\
					"lies the very heart of the evil\n"\
					"which invaded your world. d'sparil\n"\
					"might be dead, but the pit where he\n"\
					"was spawned remains. now you must\n"\
					"enter that pit in the hopes of\n"\
					"finding a way out. and somewhere,\n"\
					"in the darkest corner of d'sparil's\n"\
					"demesne, his personal bodyguards\n"\
					"await your arrival ..."

#define HERETIC_E5TEXT		"as the final maulotaur bellows his\n"\
					"death-agony, you realize that you\n"\
					"have never come so close to your own\n"\
					"destruction. not even the fight with\n"\
					"d'sparil and his disciples had been\n"\
					"this desperate. grimly you stare at\n"\
					"the gates which open before you,\n"\
					"wondering if they lead home, or if\n"\
					"they open onto some undreamed-of\n"\
					"horror. you find yourself wondering\n"\
					"if you have the strength to go on,\n"\
					"if nothing but death and pain await\n"\
					"you. but what else can you do, if\n"\
					"the will to fight is gone? can you\n"\
					"force yourself to continue in the\n"\
					"face of such despair? do you have\n"\
					"the courage? you find, in the end,\n"\
					"that it is not within you to\n"\
					"surrender without a fight. eyes\n"\
					"wide, you go to meet your fate."

//
// Heretic Text Screen Backgrounds
//

#define HERETIC_BGFLATE1 "FLOOR25"
#define HERETIC_BGFLATE2 "FLATHUH1"
#define HERETIC_BGFLATE3 "FLTWAWA2"
#define HERETIC_BGFLATE4 "FLOOR28"
#define HERETIC_BGFLATE5 "FLOOR08"

//
// killers
//

#define HERETIC_IN_KILLERS1 "K"
#define HERETIC_IN_KILLERS2 "I"
#define HERETIC_IN_KILLERS3 "L"
#define HERETIC_IN_KILLERS4 "E"
#define HERETIC_IN_KILLERS5 "R"
#define HERETIC_IN_KILLERS6 "S"

//
// Heretic Intermission
//

#define HERETIC_IN_BGFLAT	"FLOOR16"
#define HERETIC_IN_FINISHED	"FINISHED"
#define HERETIC_IN_ENTERING "NOW ENTERING:"
#define HERETIC_IN_KILLS	"KILLS"
#define HERETIC_IN_ITEMS	"ITEMS"
#define HERETIC_IN_SECRETS	"SECRETS"
#define HERETIC_IN_TIME		"TIME"
#define HERETIC_IN_TOTAL	"TOTAL"
#define HERETIC_IN_VICTIMS	"VICTIMS"

#define HERETIC_COOP_BONUS	"BONUS"
#define HERETIC_COOP_SECRET	"SECRET"

//
// Heretic Skills
//

#define HERETIC_SKILL_1		"THOU NEEDETH A WET-NURSE"
#define HERETIC_SKILL_2		"YELLOWBELLIES-R-US"
#define HERETIC_SKILL_3		"BRINGEST THEM ONETH"
#define HERETIC_SKILL_4		"THOU ART A SMITE-MEISTER"
#define HERETIC_SKILL_5		"BLACK PLAGUE POSSESSES THEE"

//
// Heretic Obituaries
//

#define OB_DSPARIL1HIT		"%o was chewed up by D'Sparil's serpent."
#define OB_DSPARIL2HIT		"%o was smacked down by D'Sparil."
#define OB_HERETICIMPHIT	"%o was hacked by a gargoyle."
#define OB_IRONLICHHIT		"%o got up-close and personal with an ironlich."
#define OB_BONEKNIGHTHIT	"%o was slain by an undead warrior."
#define OB_MINOTAURHIT		"%o was pulped by a Maulotaur."
#define OB_WIZARDHIT		"%o was palpated by a wizard."

#define OB_CHICKEN			"%o was pecked to death."
#define OB_BEAST			"%o was charred by a weredragon."
#define OB_CLINK			"%o was slashed by a sabreclaw."
#define OB_DSPARIL1			"%o was scalded by D'Sparil's serpent."
#define OB_DSPARIL2			"%o was no match for D'Sparil."
#define OB_HERETICIMP		"%o was scarred by a gargoyle."
#define OB_IRONLICH			"%o was devastated by an ironlich."
#define OB_BONEKNIGHT		"%o was axed by an undead warrior."
#define OB_MINOTAUR			"%o was blasted into cinders by a Maulotaur."
#define OB_MUMMY			"%o was smashed by a golem."
#define OB_MUMMYLEADER		"%o was shrieked to death by a nitrogolem."
#define OB_SNAKE			"%o was rattled by an ophidian."
#define OB_WIZARD			"%o was cursed by a wizard."

// Normal Weapons
#define OB_MPSTAFF			"%o got staffed by %k."
#define OB_MPGAUNTLETS		"%o got a shock from %k's gauntlets."
#define OB_MPGOLDWAND		"%o waved goodbye to %k's elven wand."
#define OB_MPCROSSBOW		"%o was pegged by %k's ethereal crossbow."
#define OB_MPBLASTER		"%o was blasted a new one by %k's dragon claw."
#define OB_MPSKULLROD		"%o got sent down under by %k's hellstaff."
#define OB_MPPHOENIXROD		"%o was scorched to cinders by %k's phoenix rod."
#define OB_MPMACE			"%o was bounced by %k's firemace."

// Tome Powered Weapons
#define OB_MPPSTAFF			"%o got clapped by %k's charged staff."
#define OB_MPPGAUNTLETS		"%o was bled dry by %k's gauntlets."
#define OB_MPPGOLDWAND		"%o was assaulted by %k's elven wand."
#define OB_MPPCROSSBOW		"%o was shafted by %k's ethereal crossbow."
#define OB_MPPBLASTER		"%o was ripped apart by %k's dragon claw."
#define OB_MPPSKULLROD		"%k poured the hellstaff on %o."
#define OB_MPPPHOENIXROD	"%o was burned down by %k's phoenix staff."
#define OB_MPPMACE			"%o was squished by %k's giant mace sphere."
