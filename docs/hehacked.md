# HeHackEd

Nyan Doom supports Hehacked patches for Heretic.

Hehacked is more complicated to support compared to Dehacked, as all patches currently use the header of `Heretic version = 10` regardless of Heretic version.

While Nyan Doom does support stacking Hehacked patches, mixing different version patches is NOT supported. (for more information, see the `HeHackEd Versioning` section below)

The following Vanilla Hehacked features are supported:
- Ammo
- Frame
- Sound
- Sprite *(via Text)*
- Text
- Thing
- Weapon

## HeHackEd Versioning

Nyan Doom supports Hehacked patches for Heretic `1.0`, `1.2`, and `1.3`. In order to get around the lack of version detection, Nyan Doom uses an auto-detect system to "vote" on which version a hehacked patch is most likely for.

Modders can also force a version via the `-hhever` argument to specify/override which Heretic version.
- Example: `-hhever 1.2`

Modders can also use a `HHEVER` lump to specify an exact version.
- Example: `HHEVER` text lump with the value `1.0`

>Note that Heretic Hehacked versions adjust the state and frame tables... So mixing and matching multiple Hehacked patches is NOT supported. Once a version has been selected, that version is "locked" for the rest of the loaded patches.

## DSDhacked Support

Nyan Doom supports unlimited states, frames, sprites, sounds, things via DSDhacked.
It also supports sprites named `SP##` and sounds named `DSFRE###` just like in Doom.

For more info about DSDhacked, take a look at this page here:
[DSDhacked](dsdhacked.md)

## REX (Raven Extended) Support

Nyan Doom also includes support for these Boom style REX blocks:
- `[MUSIC]`
- `[CODEPTR]`
- `[SOUNDS]`
- `[SPRITES]`
- `[STRINGS]`

## REX Strings

To add support for the REX `[STRINGS]` block, names had to be assigned to certain text strings. Many of these names were named after names given in the Heretic source code, as well as some named after ZDoom string names.

Nyan Doom supports the following string mnemonics:

## Level Name Strings
> Note that map names in heretic use the following format: "E#M#:" + 2 spaces + "map name". Heretic is hardcoded to remove the first 7 characters when displaying the map on the intermission screen. 

| String Name | String Default |
|-|-|
| HHUSTR_E1M1 | E1M1:  THE DOCKS |
| HHUSTR_E1M2 | E1M2:  THE DUNGEONS |
| HHUSTR_E1M3 | E1M3:  THE GATEHOUSE |
| HHUSTR_E1M4 | E1M4:  THE GUARD TOWER |
| HHUSTR_E1M5 | E1M5:  THE CITADEL |
| HHUSTR_E1M6 | E1M6:  THE CATHEDRAL |
| HHUSTR_E1M7 | E1M7:  THE CRYPTS |
| HHUSTR_E1M8 | E1M8:  HELL'S MAW |
| HHUSTR_E1M9 | E1M9:  THE GRAVEYARD |
| HHUSTR_E2M1 | E2M1:  THE CRATER |
| HHUSTR_E2M2 | E2M2:  THE LAVA PITS |
| HHUSTR_E2M3 | E2M3:  THE RIVER OF FIRE |
| HHUSTR_E2M4 | E2M4:  THE ICE GROTTO |
| HHUSTR_E2M5 | E2M5:  THE CATACOMBS |
| HHUSTR_E2M6 | E2M6:  THE LABYRINTH |
| HHUSTR_E2M7 | E2M7:  THE GREAT HALL |
| HHUSTR_E2M8 | E2M8:  THE PORTALS OF CHAOS |
| HHUSTR_E2M9 | E2M9:  THE GLACIER |
| HHUSTR_E3M1 | E3M1:  THE STOREHOUSE |
| HHUSTR_E3M2 | E3M2:  THE CESSPOOL |
| HHUSTR_E3M3 | E3M3:  THE CONFLUENCE |
| HHUSTR_E3M4 | E3M4:  THE AZURE FORTRESS |
| HHUSTR_E3M5 | E3M5:  THE OPHIDIAN LAIR |
| HHUSTR_E3M6 | E3M6:  THE HALLS OF FEAR |
| HHUSTR_E3M7 | E3M7:  THE CHASM |
| HHUSTR_E3M8 | E3M8:  D'SPARIL'S KEEP |
| HHUSTR_E3M9 | E3M9:  THE AQUIFER |
| HHUSTR_E4M1 | E4M1:  CATAFALQUE |
| HHUSTR_E4M2 | E4M2:  BLOCKHOUSE |
| HHUSTR_E4M3 | E4M3:  AMBULATORY |
| HHUSTR_E4M4 | E4M4:  SEPULCHER |
| HHUSTR_E4M5 | E4M5:  GREAT STAIR |
| HHUSTR_E4M6 | E4M6:  HALLS OF THE APOSTATE |
| HHUSTR_E4M7 | E4M7:  RAMPARTS OF PERDITION |
| HHUSTR_E4M8 | E4M8:  SHATTERED BRIDGE |
| HHUSTR_E4M9 | E4M9:  MAUSOLEUM |
| HHUSTR_E5M1 | E5M1:  OCHRE CLIFFS |
| HHUSTR_E5M2 | E5M2:  RAPIDS |
| HHUSTR_E5M3 | E5M3:  QUAY |
| HHUSTR_E5M4 | E5M4:  COURTYARD |
| HHUSTR_E5M5 | E5M5:  HYDRATYR |
| HHUSTR_E5M6 | E5M6:  COLONNADE |
| HHUSTR_E5M7 | E5M7:  FOETID MANSE |
| HHUSTR_E5M8 | E5M8:  FIELD OF JUDGEMENT |
| HHUSTR_E5M9 | E5M9:  SKEIN OF D'SPARIL |

## Story Text strings

| String Name | String Default | Notes |
|-|-|-|
| HE1TEXT | with the destruction of the iron... | |
| HE2TEXT | the mighty maulotaurs have proved... | |
| HE3TEXT | the death of d'sparil has loosed... | |
| HE4TEXT | you thought you would return to your... | |
| HE5TEXT | as the final maulotaur bellows his... | |
| BGFLATE1 | FLOOR25 | Episode 1 Background Flat |
| BGFLATE2 | FLATHUH1 | Episode 2 Background Flat |
| BGFLATE3 | FLTWAWA2 | Episode 3 Background Flat |
| BGFLATE4 | FLOOR28 | Episode 4 Background Flat |
| BGFLATE5 | FLOOR08 | Episode 5 Background Flat |

## Intermission Strings

| String Name | String Default | Notes |
|-|-|-|
| WI_BGFLAT | FLOOR16 | Intermission Flat |
| WI_FINISHED | FINISHED | |
| WI_ENTERING | NOW ENTERING: | |
| TXT_IMKILLS | KILLS | Singleplayer |
| TXT_IMITEMS | ITEMS | Singleplayer |
| TXT_IMSECRETS | SECRETS | Singleplayer |
| TXT_IMTIME | TIME | Singleplayer |
| SCORE_TOTAL | TOTAL | Single + Multiplayer |
| SCORE_VICTIMS | VICTIMS | Multiplayer |
| SCORE_BONUS | BONUS | Multiplayer |
| SCORE_SECRET | SECRET | Multiplayer |
| TXT_KILLERS1 | K | Multiplayer? |
| TXT_KILLERS2 | I | |
| TXT_KILLERS3 | L | Note: This string is used twice |
| TXT_KILLERS4 | E | |
| TXT_KILLERS5 | R | |
| TXT_KILLERS6 | S | |

## Menu Strings
| String Name | String Default |
|-|-|
| | |
|**Main Menu** | |
| MNU_NEWGAME | NEW GAME |
| MNU_OPTIONS | OPTIONS |
| MNU_GAMEFILES | GAME FILES |
| MNU_INFO | INFO |
| MNU_QUITGAME | QUIT GAME |
| | |
|**Sub Menus** | |
| MNU_LOADGAME | LOAD GAME |
| MNU_SAVEGAME | SAVE GAME |
| MNU_SFXVOL | SFX VOLUME |
| MNU_MUSICVOL | MUSIC VOLUME |
| | |
|**Episodes** | |
| MNU_COTD | CITY OF THE DAMNED |
| MNU_HELLSMAW | HELL'S MAW |
| MNU_DOME | THE DOME OF D'SPARIL |
| MNU_OSSUARY | THE OSSUARY |
| MNU_DEMESNE | THE STAGNANT DEMESNE |
| | |
|**Skills** | |
| MNU_WETNURSE | THOU NEEDETH A WET-NURSE |
| MNU_YELLOWBELLIES | YELLOWBELLIES-R-US |
| MNU_BRINGEST | BRINGEST THEM ONETH |
| MNU_SMITE | THOU ART A SMITE-MEISTER |
| MNU_BLACKPLAGUE | BLACK PLAGUE POSSESSES THEE |

## Message Strings

| String Name | String Default |
|-|-|
| AMSTR_FOLLOWON | FOLLOW MODE ON |
| AMSTR_FOLLOWOFF | FOLLOW MODE OFF |
| GGSAVED | GAME SAVED |
| RAVENQUITMSG | ARE YOU SURE YOU WANT TO QUIT? |
| ENDGAME | ARE YOU SURE YOU WANT TO END THE GAME? |
| MSGON | MESSAGES ON |
| MSGOFF | MESSAGES OFF |
| SWSTRING | ONLY AVAILABLE IN THE REGISTERED VERSION |
| TXT_NEEDYELLOWKEY | YOU NEED A YELLOW KEY TO OPEN THIS DOOR |
| TXT_NEEDGREENKEY | YOU NEED A GREEN KEY TO OPEN THIS DOOR |
| TXT_NEEDBLUEKEY | YOU NEED A BLUE KEY TO OPEN THIS DOOR |

## Pickup Strings
| String Name | String Default |
|-|-|
| | |
|**Keys** | |
| TXT_GOTYELLOWKEY | YELLOW KEY |
| TXT_GOTGREENKEY | GREEN KEY |
| TXT_GOTBLUEKEY | BLUE KEY |
| | |
|**Pickups** | |
| TXT_ITEMHEALTH | CRYSTAL VIAL |
| TXT_ITEMSHIELD1 | SILVER SHIELD |
| TXT_ITEMSHIELD2 | ENCHANTED SHIELD |
| TXT_ITEMBAGOFHOLDING | BAG OF HOLDING |
| TXT_ITEMSUPERMAP | MAP SCROLL |
| | |
|**Artifacts** | |
| TXT_ARTIHEALTH | QUARTZ FLASK |
| TXT_ARTIFLY | WINGS OF WRATH |
| TXT_ARTIINVULNERABILITY | RING OF INVINCIBILITY |
| TXT_ARTITOMEOFPOWER | TOME OF POWER |
| TXT_ARTIINVISIBILITY | SHADOWSPHERE |
| TXT_ARTIEGG | MORPH OVUM |
| TXT_ARTISUPERHEALTH | MYSTIC URN |
| TXT_ARTITORCH | TORCH |
| TXT_ARTIFIREBOMB | TIME BOMB OF THE ANCIENTS |
| TXT_ARTITELEPORT | CHAOS DEVICE |
| | |
|**Ammo Pickups** | |
| TXT_AMMOGOLDWAND1 | WAND CRYSTAL |
| TXT_AMMOGOLDWAND2 | CRYSTAL GEODE |
| TXT_AMMOCROSSBOW1 | ETHEREAL ARROWS |
| TXT_AMMOCROSSBOW2 | QUIVER OF ETHEREAL ARROWS |
| TXT_AMMOBLASTER1 | CLAW ORB |
| TXT_AMMOBLASTER2 | ENERGY ORB |
| TXT_AMMOSKULLROD1 | LESSER RUNES |
| TXT_AMMOSKULLROD2 | GREATER RUNES |
| TXT_AMMOPHOENIXROD1 | FLAME ORB |
| TXT_AMMOPHOENIXROD2 | INFERNO ORB |
| TXT_AMMOMACE1 | MACE SPHERES |
| TXT_AMMOMACE2 | PILE OF MACE SPHERES |
| | |
|**Weapon Pickups** | |
| TXT_WPNCROSSBOW | ETHEREAL CROSSBOW |
| TXT_WPNBLASTER | DRAGON CLAW |
| TXT_WPNSKULLROD | HELLSTAFF |
| TXT_WPNPHOENIXROD | PHOENIX ROD |
| TXT_WPNMACE | FIREMACE |
| TXT_WPNGAUNTLETS | GAUNTLETS OF THE NECROMANCER |

## Message Strings - Cheats
| String Name | String Default | Notes |
|-|-|-|
| TXT_CHEATGODON | GOD MODE ON |
| TXT_CHEATGODOFF | GOD MODE OFF |
| TXT_CHEATNOCLIPON | NO CLIPPING ON |
| TXT_CHEATNOCLIPOFF | NO CLIPPING OFF |
| TXT_CHEATWEAPONS | ALL WEAPONS |
| TXT_CHEATPOWERON |POWER ON |
| TXT_CHEATPOWEROFF | POWER OFF |
| TXT_CHEATHEALTH | FULL HEALTH |
| TXT_CHEATKEYS | ALL KEYS |
| TXT_CHEATSOUNDON | SOUND DEBUG ON | Nyan Doom does not use this |
| TXT_CHEATSOUNDOFF | SOUND DEBUG OFF | Nyan Doom does not use this |
| TXT_CHEATTICKERON | TICKER ON | Nyan Doom does not use this |
| TXT_CHEATTICKEROFF | TICKER OFF | Nyan Doom does not use this |
| TXT_CHEATARTIFACTS1 | CHOOSE AN ARTIFACT ( A - J ) |
| TXT_CHEATARTIFACTS2 | HOW MANY ( 1 - 9 ) |
| TXT_CHEATARTIFACTS3 | YOU GOT IT |
| TXT_CHEATARTIFACTSFAIL | BAD INPUT |
| TXT_CHEATWARP | LEVEL WARP |
| TXT_CHEATCHICKENON | CHICKEN ON |
| TXT_CHEATCHICKENOFF | CHICKEN OFF |
| TXT_CHEATMASSACRE | MASSACRE |
| TXT_CHEATIDDQD | TRYING TO CHEAT, EH?  NOW YOU DIE! |
| TXT_CHEATIDKFA | CHEATER - YOU DON'T DESERVE WEAPONS |

## Extra (Non-Hehacked) Nyan Doom Strings
Many of these strings are Doom compatable strings.

`MNU_CHOOSESKILL`/`TXT_CHEATFLIGHTON`/`TXT_CHEATFLIGHTOFF` are unique in that they are not editable strings in Hehacked.

| String Name | String Default |
|-|-|
| PRESSKEY | press a key. |
| PRESSYN | press y or n. |
| GAMMALVL0 | Gamma correction OFF |
| GAMMALVL1 | Gamma correction level 1 |
| GAMMALVL2 | Gamma correction level 2 |
| GAMMALVL3 | Gamma correction level 3 |
| GAMMALVL4 | Gamma correction level 4 |
| STSTR_MUS | Music Change |
| STSTR_NOMUS | IMPOSSIBLE SELECTION |
| MNU_CHOOSESKILL | CHOOSE SKILL LEVEL: |
| TXT_CHEATFLIGHTON | FLIGHT ON |
| TXT_CHEATFLIGHTOFF | FLIGHT OFF |
| TXT_SHOWFPSON | SHOW FPS ON |
| TXT_SHOWFPSOFF | SHOW FPS OFF |
| AMSTR_GRIDON | GRID ON |
| AMSTR_GRIDOFF | GRID OFF |
| AMSTR_MARKEDSPOT | Marked Spot |
| AMSTR_MARKSCLEARED | All Marks Cleared |
