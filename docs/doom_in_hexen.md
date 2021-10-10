## Doom in Hexen

This page tracks support for the "Doom in Hexen" map format and related features, as seen in ZDoom.

### Current Status

Current work is isolated to the initial pass over the level format itself - line and sector specials, necessary changes to internal data structures, etc. Lots of refactoring needs to be done in the engine itself in order to treat different formats correctly without breaking compatibility.

### Features

| Name         | Supported |
| ------------ | --------- |
| Levels       | :warning: |
| Poly Objects | :x:       |
| ACS          | :x:       |
| MAPINFO      | :x:       |
| SNDINFO      | :x:       |
| SNDSEQ       | :x:       |
| ANIMDEFS     | :x:       |

### Things

| Value       | Name                    | Supported          |
| ----------- | ----------------------- | ------------------ |
| 118         | ZBridge                 | :x:                |
| -           | -                       | -                  |
| 888         | MBFHelperDog            | :heavy_check_mark: |
| -           | -                       | -                  |
| 1400-1409   | Sound Sequence          | :x:                |
| 1411        | Sound Sequence Override | :x:                |
| -           | -                       | -                  |
| 1500-1501   | Line Slopes             | :x:                |
| -           | -                       | -                  |
| 1504-1505   | Vertex Slopes           | :x:                |
| -           | -                       | -                  |
| 4001-4004   | Player 5-8 Start        | :x:                |
| 5001        | PointPusher             | :heavy_check_mark: |
| 5002        | PointPuller             | :heavy_check_mark: |
| -           | -                       | -                  |
| 5010        | Pistol                  | :x:                |
| -           | -                       | -                  |
| 5050        | Stalagmite              | :x:                |
| -           | -                       | -                  |
| 5061        | InvisibleBridge32       | :x:                |
| -           | -                       | -                  |
| 5064        | InvisibleBridge16       | :x:                |
| 5065        | InvisibleBridge8        | :x:                |
| -           | -                       | -                  |
| 9001        | MapSpot                 | :x:                |
| -           | -                       | -                  |
| 9013        | MapSpotGravity          | :x:                |
| -           | -                       | -                  |
| 9024        | PatrolPoint             | :x:                |
| 9025        | SecurityCamera          | :x:                |
| 9026        | Spark                   | :x:                |
| 9027        | RedParticleFountain     | :x:                |
| 9028        | GreenParticleFountain   | :x:                |
| 9029        | BlueParticleFountain    | :x:                |
| 9030        | YellowParticleFountain  | :x:                |
| 9031        | PurpleParticleFountain  | :x:                |
| 9032        | BlackParticleFountain   | :x:                |
| 9033        | WhiteParticleFountain   | :x:                |
| -           | -                       | -                  |
| 9037        | BetaSkull               | :x:                |
| 9038        | ColorSetter             | :x:                |
| 9039        | FadeSetter              | :x:                |
| 9040        | MapMarker               | :x:                |
| 9041        | SectorFlagSetter        | :x:                |
| -           | -                       | -                  |
| 9043-9044   | TeleportDest*           | :x:                |
| 9045        | Waterzone               | :x:                |
| 9046        | SecretTrigger           | :x:                |
| 9047        | PatrolSpecial           | :x:                |
| 9048        | SoundEnvironment        | :x:                |
| -           | -                       | -                  |
| 9050-9061   | Stealth*                | :x:                |
| -           | -                       | -                  |
| 9070        | InterpolationPoint      | :x:                |
| 9071        | PathFollower            | :x:                |
| 9072        | MovingCamera            | :x:                |
| 9073        | AimingCamera            | :x:                |
| 9074        | ActorMover              | :x:                |
| 9075        | InterpolationSpecial    | :x:                |
| 9076        | HateTarget              | :x:                |
| 9077        | UpperStackLookOnly      | :x:                |
| 9078        | LowerStackLookOnly      | :x:                |
| -           | -                       | -                  |
| 9080        | SkyViewpoint            | :x:                |
| 9081        | SkyPicker               | :x:                |
| 9082        | SectorSilencer          | :x:                |
| 9083        | SkyCamCompat            | :x:                |
| -           | -                       | -                  |
| 9100        | ScriptedMarine          | :x:                |
| 9101-9111   | Marine*                 | :x:                |
| -           | -                       | -                  |
| 9200        | Decal                   | :x:                |
| -           | -                       | -                  |
| 9300-9303   | PolyObject*             | :x:                |
| -           | -                       | -                  |
| 9500-9503   | Slopes                  | :x:                |
| -           | -                       | -                  |
| 9510-9511   | Copy Planes             | :x:                |
| -           | -                       | -                  |
| 9982        | SecActEyesAboveC        | :x:                |
| 9983        | SecActEyesBelowC        | :x:                |
| -           | -                       | -                  |
| 9988        | CustomSprite            | :x:                |
| 9989        | SecActHitFakeFloor      | :x:                |
| 9990        | InvisibleBridge         | :x:                |
| 9991        | CustomBridge            | :x:                |
| 9992        | SecActEyesSurface       | :x:                |
| 9993        | SecActEyesDive          | :x:                |
| 9994        | SecActUseWall           | :x:                |
| 9995        | SecActUse               | :x:                |
| 9996        | SecActHitCeil           | :x:                |
| 9997        | SecActExit              | :x:                |
| 9998        | SecActEnter             | :x:                |
| 9999        | SecActHitFloor          | :x:                |
| -           | -                       | -                  |
| 14001-14064 | AmbientSound            | :x:                |
| 14065       | Custom AmbientSound     | :x:                |
| 14066       | SoundSequence           | :x:                |
| 14067       | AmbientSoundNoGravity   | :x:                |
| -           | -                       | -                  |
| 14101-14164 | MusicChanger            | :heavy_check_mark: |
| 14165       | Custom MusicChanger     | :x:                |

### Spawn Flags

| Name        | Supported          |
| ----------- | ------------------ |
| Dormant     | :warning:          |
| Translucent | :heavy_check_mark: |
| Invisible   | :heavy_check_mark: |
| Friendly    | :heavy_check_mark: |
| Stand Still | :x:                |

#### Notes
- Some dormant behaviour is still missing.

### Line Specials

| Value | Name                            | Supported          |
| ----- | ------------------------------- | ------------------ |
| 1     | Polyobj_StartLine               | :x:                |
| 2     | Polyobj_RotateLeft              | :x:                |
| 3     | Polyobj_RotateRight             | :x:                |
| 4     | Polyobj_Move                    | :x:                |
| 5     | Polyobj_ExplicitLine            | :x:                |
| 6     | Polyobj_MoveTimes8              | :x:                |
| 7     | Polyobj_DoorSwing               | :x:                |
| 8     | Polyobj_DoorSlide               | :x:                |
| 9     | Line_Horizon                    | :x:                |
| 10    | Door_Close                      | :x:                |
| 11    | Door_Open                       | :x:                |
| 12    | Door_Raise                      | :x:                |
| 13    | Door_LockedRaise                | :x:                |
| 14    | Door_Animated                   | :x:                |
| 15    | Autosave                        | :x:                |
| 16    | Transfer_WallLight              | :x:                |
| 17    | Thing_Raise                     | :x:                |
| 18    | StartConversation               | :x:                |
| 19    | Thing_Stop                      | :x:                |
| 20    | Floor_LowerByValue              | :x:                |
| 21    | Floor_LowerToLowest             | :x:                |
| 22    | Floor_LowerToNearest            | :x:                |
| 23    | Floor_RaiseByValue              | :x:                |
| 24    | Floor_RaiseToHighest            | :x:                |
| 25    | Floor_RaiseToNearest            | :x:                |
| 26    | Stairs_BuildDown                | :x:                |
| 27    | Stairs_BuildUp                  | :x:                |
| 28    | Floor_RaiseAndCrush             | :x:                |
| 29    | Pillar_Build                    | :x:                |
| 30    | Pillar_Open                     | :x:                |
| 31    | Stairs_BuildDownSync            | :x:                |
| 32    | Stairs_BuildUpSync              | :x:                |
| 33    | ForceField                      | :x:                |
| 34    | ClearForceField                 | :x:                |
| 35    | Floor_RaiseByValueTimes8        | :x:                |
| 36    | Floor_LowerByValueTimes8        | :x:                |
| 37    | Floor_MoveToValue               | :x:                |
| 38    | Ceiling_Waggle                  | :x:                |
| 39    | Teleport_ZombieChanger          | :x:                |
| 40    | Ceiling_LowerByValue            | :x:                |
| 41    | Ceiling_RaiseByValue            | :x:                |
| 42    | Ceiling_CrushAndRaise           | :x:                |
| 43    | Ceiling_LowerAndCrush           | :x:                |
| 44    | Ceiling_CrushStop               | :x:                |
| 45    | Ceiling_CrushRaiseAndStay       | :x:                |
| 46    | Floor_CrushStop                 | :x:                |
| 47    | Ceiling_MoveToValue             | :x:                |
| 48    | Sector_Attach3dMidtex           | :x:                |
| 49    | GlassBreak                      | :x:                |
| 50    | ExtraFloor_LightOnly            | :x:                |
| 51    | Sector_SetLink                  | :x:                |
| 52    | Scroll_Wall                     | :x:                |
| 53    | Line_SetTextureOffset           | :x:                |
| 54    | Sector_ChangeFlags              | :x:                |
| 55    | Line_SetBlocking                | :x:                |
| 56    | Line_SetTextureScale            | :x:                |
| 57    | Sector_SetPortal                | :x:                |
| 58    | Sector_CopyScroller             | :heavy_check_mark: |
| 59    | Polyobj_OR_MoveToSpot           | :x:                |
| 60    | Plat_PerpetualRaise             | :x:                |
| 61    | Plat_Stop                       | :x:                |
| 62    | Plat_DownWaitUpStay             | :x:                |
| 63    | Plat_DownByValue                | :x:                |
| 64    | Plat_UpWaitDownStay             | :x:                |
| 65    | Plat_UpByValue                  | :x:                |
| 66    | Floor_LowerInstant              | :x:                |
| 67    | Floor_RaiseInstant              | :x:                |
| 68    | Floor_MoveToValueTimes8         | :x:                |
| 69    | Ceiling_MoveToValueTimes8       | :x:                |
| 70    | Teleport                        | :x:                |
| 71    | Teleport_NoFog                  | :x:                |
| 72    | ThrustThing                     | :x:                |
| 73    | DamageThing                     | :x:                |
| 74    | Teleport_NewMap                 | :x:                |
| 75    | Teleport_EndGame                | :x:                |
| 76    | TeleportOther                   | :x:                |
| 77    | TeleportGroup                   | :x:                |
| 78    | TeleportInSector                | :x:                |
| 79    | Thing_SetConversation           | :x:                |
| 80    | ACS_Execute                     | :x:                |
| 81    | ACS_Suspend                     | :x:                |
| 82    | ACS_Terminate                   | :x:                |
| 83    | ACS_LockedExecute               | :x:                |
| 84    | ACS_ExecuteWithResult           | :x:                |
| 85    | ACS_LockedExecuteDoor           | :x:                |
| 86    | Polyobj_MoveToSpot              | :x:                |
| 87    | Polyobj_Stop                    | :x:                |
| 88    | Polyobj_MoveTo                  | :x:                |
| 89    | Polyobj_OR_MoveTo               | :x:                |
| 90    | Polyobj_OR_RotateLeft           | :x:                |
| 91    | Polyobj_OR_RotateRight          | :x:                |
| 92    | Polyobj_OR_Move                 | :x:                |
| 93    | Polyobj_OR_MoveTimes8           | :x:                |
| 94    | Pillar_BuildAndCrush            | :x:                |
| 95    | FloorAndCeiling_LowerByValue    | :x:                |
| 96    | FloorAndCeiling_RaiseByValue    | :x:                |
| 97    | Ceiling_LowerAndCrushDist       | :x:                |
| 98    | Sector_SetTranslucent           | :x:                |
| 99    | Floor_RaiseAndCrushDoom         | :x:                |
| 100   | Scroll_Texture_Left             | :warning:          |
| 101   | Scroll_Texture_Right            | :warning:          |
| 102   | Scroll_Texture_Up               | :warning:          |
| 103   | Scroll_Texture_Down             | :warning:          |
| 104   | Ceiling_CrushAndRaiseSilentDist | :x:                |
| 105   | Door_WaitRaise                  | :x:                |
| 106   | Door_WaitClose                  | :x:                |
| 107   | Line_SetPortalTarget            | :x:                |
| -     | -                               | -                  |
| 109   | Light_ForceLightning            | :x:                |
| 110   | Light_RaiseByValue              | :x:                |
| 111   | Light_LowerByValue              | :x:                |
| 112   | Light_ChangeToValue             | :x:                |
| 113   | Light_Fade                      | :x:                |
| 114   | Light_Glow                      | :x:                |
| 115   | Light_Flicker                   | :x:                |
| 116   | Light_Strobe                    | :x:                |
| 117   | Light_Stop                      | :x:                |
| 118   | Plane_Copy                      | :x:                |
| 119   | Thing_Damage                    | :x:                |
| 120   | Radius_Quake                    | :x:                |
| 121   | Line_SetIdentification          | :x:                |
| -     | -                               | -                  |
| 125   | Thing_Move                      | :x:                |
| -     | -                               | -                  |
| 127   | Thing_SetSpecial                | :x:                |
| 128   | ThrustThingZ                    | :x:                |
| 129   | UsePuzzleItem                   | :x:                |
| 130   | Thing_Activate                  | :x:                |
| 131   | Thing_Deactivate                | :x:                |
| 132   | Thing_Remove                    | :x:                |
| 133   | Thing_Destroy                   | :x:                |
| 134   | Thing_Projectile                | :x:                |
| 135   | Thing_Spawn                     | :x:                |
| 136   | Thing_ProjectileGravity         | :x:                |
| 137   | Thing_SpawnNoFog                | :x:                |
| 138   | Floor_Waggle                    | :x:                |
| 139   | Thing_SpawnFacing               | :x:                |
| 140   | Sector_ChangeSound              | :x:                |
| -     | -                               | -                  |
| 154   | Teleport_NoStop                 | :x:                |
| -     | -                               | -                  |
| 157   | SetGlobalFogParameter           | :x:                |
| 158   | FS_Execute                      | :x:                |
| 159   | Sector_SetPlaneReflection       | :x:                |
| 160   | Sector_Set3DFloor               | :x:                |
| 161   | Sector_SetContents              | :x:                |
| -     | -                               | -                  |
| 168   | Ceiling_CrushAndRaiseDist       | :x:                |
| 169   | Generic_Crusher2                | :x:                |
| 170   | Sector_SetCeilingScale2         | :x:                |
| 171   | Sector_SetFloorScale2           | :x:                |
| 172   | Plat_UpNearestWaitDownStay      | :x:                |
| 173   | NoiseAlert                      | :x:                |
| 174   | SendToCommunicator              | :x:                |
| 175   | Thing_ProjectileIntercept       | :x:                |
| 176   | Thing_ChangeTID                 | :x:                |
| 177   | Thing_Hate                      | :x:                |
| 178   | Thing_ProjectileAimed           | :x:                |
| 179   | ChangeSkill                     | :x:                |
| 180   | Thing_SetTranslation            | :x:                |
| 181   | Plane_Align                     | :x:                |
| 182   | Line_Mirror                     | :x:                |
| 183   | Line_AlignCeiling               | :x:                |
| 184   | Line_AlignFloor                 | :x:                |
| 185   | Sector_SetRotation              | :x:                |
| 186   | Sector_SetCeilingPanning        | :x:                |
| 187   | Sector_SetFloorPanning          | :x:                |
| 188   | Sector_SetCeilingScale          | :x:                |
| 189   | Sector_SetFloorScale            | :x:                |
| 190   | Static_Init                     | :warning:          |
| 191   | SetPlayerProperty               | :x:                |
| 192   | Ceiling_LowerToHighestFloor     | :x:                |
| 193   | Ceiling_LowerInstant            | :x:                |
| 194   | Ceiling_RaiseInstant            | :x:                |
| 195   | Ceiling_CrushRaiseAndStayA      | :x:                |
| 196   | Ceiling_CrushAndRaiseA          | :x:                |
| 197   | Ceiling_CrushAndRaiseSilentA    | :x:                |
| 198   | Ceiling_RaiseByValueTimes8      | :x:                |
| 199   | Ceiling_LowerByValueTimes8      | :x:                |
| 200   | Generic_Floor                   | :x:                |
| 201   | Generic_Ceiling                 | :x:                |
| 202   | Generic_Door                    | :x:                |
| 203   | Generic_Lift                    | :x:                |
| 204   | Generic_Stairs                  | :x:                |
| 205   | Generic_Crusher                 | :x:                |
| 206   | Plat_DownWaitUpStayLip          | :x:                |
| 207   | Plat_PerpetualRaiseLip          | :x:                |
| 208   | TranslucentLine                 | :x:                |
| 209   | Transfer_Heights                | :heavy_check_mark: |
| 210   | Transfer_FloorLight             | :heavy_check_mark: |
| 211   | Transfer_CeilingLight           | :heavy_check_mark: |
| 212   | Sector_SetColor                 | :x:                |
| 213   | Sector_SetFade                  | :x:                |
| 214   | Sector_SetDamage                | :x:                |
| 215   | Teleport_Line                   | :x:                |
| 216   | Sector_SetGravity               | :x:                |
| 217   | Stairs_BuildUpDoom              | :x:                |
| 218   | Sector_SetWind                  | :heavy_check_mark: |
| 219   | Sector_SetFriction              | :heavy_check_mark: |
| 220   | Sector_SetCurrent               | :heavy_check_mark: |
| 221   | Scroll_Texture_Both             | :heavy_check_mark: |
| 222   | Scroll_Texture_Model            | :heavy_check_mark: |
| 223   | Scroll_Floor                    | :heavy_check_mark: |
| 224   | Scroll_Ceiling                  | :heavy_check_mark: |
| 225   | Scroll_Texture_Offsets          | :warning:          |
| 226   | ACS_ExecuteAlways               | :x:                |
| 227   | PointPush_SetForce              | :heavy_check_mark: |
| 228   | Plat_RaiseAndStayTx0            | :x:                |
| 229   | Thing_SetGoal                   | :x:                |
| 230   | Plat_UpByValueStayTx            | :x:                |
| 231   | Plat_ToggleCeiling              | :x:                |
| 232   | Light_StrobeDoom                | :x:                |
| 233   | Light_MinNeighbor               | :x:                |
| 234   | Light_MaxNeighbor               | :x:                |
| 235   | Floor_TransferTrigger           | :x:                |
| 236   | Floor_TransferNumeric           | :x:                |
| 237   | ChangeCamera                    | :x:                |
| 238   | Floor_RaiseToLowestCeiling      | :x:                |
| 239   | Floor_RaiseByValueTxTy          | :x:                |
| 240   | Floor_RaiseByTexture            | :x:                |
| 241   | Floor_LowerToLowestTxTy         | :x:                |
| 242   | Floor_LowerToHighest            | :x:                |
| 243   | Exit_Normal                     | :x:                |
| 244   | Exit_Secret                     | :x:                |
| 245   | Elevator_RaiseToNearest         | :x:                |
| 246   | Elevator_MoveToFloor            | :x:                |
| 247   | Elevator_LowerToNearest         | :x:                |
| 248   | HealThing                       | :x:                |
| 249   | Door_CloseWaitOpen              | :x:                |
| 250   | Floor_Donut                     | :x:                |
| 251   | FloorAndCeiling_LowerRaise      | :x:                |
| 252   | Ceiling_RaiseToNearest          | :x:                |
| 253   | Ceiling_LowerToLowest           | :x:                |
| 254   | Ceiling_LowerToFloor            | :x:                |
| 255   | Ceiling_CrushRaiseAndStaySilA   | :x:                |
| 256   | Floor_LowerToHighestEE          | :x:                |
| 257   | Floor_RaiseToLowest             | :x:                |
| 258   | Floor_LowerToLowestCeiling      | :x:                |
| 259   | Floor_RaiseToCeiling            | :x:                |
| 260   | Floor_ToCeilingInstant          | :x:                |
| 261   | Floor_LowerByTexture            | :x:                |
| 262   | Ceiling_RaiseToHighest          | :x:                |
| 263   | Ceiling_ToHighestInstant        | :x:                |
| 264   | Ceiling_LowerToNearest          | :x:                |
| 265   | Ceiling_RaiseToLowest           | :x:                |
| 266   | Ceiling_RaiseToHighestFloor     | :x:                |
| 267   | Ceiling_ToFloorInstant          | :x:                |
| 268   | Ceiling_RaiseByTexture          | :x:                |
| 269   | Ceiling_LowerByTexture          | :x:                |
| 270   | Stairs_BuildDownDoom            | :x:                |
| 271   | Stairs_BuildUpDoomSync          | :x:                |
| 272   | Stairs_BuildDownDoomSync        | :x:                |

#### Notes

- Wall scrollers cannot distinguish between top, bottom, and mid textures.
- The static init supports damage, gravity, and skies only.

### Sector Specials

| Value   | Name                     | Supported          |
| ------- | ------------------------ | ------------------ |
| *       | Generalized Effects      | :heavy_check_mark: |
| 1       | Light_Phased             | :heavy_check_mark: |
| 2-4     | LightSequence*           | :grey_question:    |
| -       | -                        | -                  |
| 26-27   | Stairs_Special*          | :grey_question:    |
| -       | -                        | -                  |
| 40-51   | Wind*                    | :heavy_check_mark: |
| -       | -                        | -                  |
| 65      | dLight_Flicker           | :heavy_check_mark: |
| 66      | dLight_StrobeFast        | :heavy_check_mark: |
| 67      | dLight_StrobeSlow        | :heavy_check_mark: |
| 68      | dLight_Strobe_Hurt       | :heavy_check_mark: |
| 69      | dDamage_Hellslime        | :heavy_check_mark: |
| -       | -                        | -                  |
| 71      | dDamage_Nukage           | :heavy_check_mark: |
| 72      | dLight_Glow              | :heavy_check_mark: |
| -       | -                        | -                  |
| 74      | dSector_DoorCloseIn30    | :heavy_check_mark: |
| 75      | dDamage_End              | :heavy_check_mark: |
| 76      | dLight_StrobeSlowSync    | :heavy_check_mark: |
| 77      | dLight_StrobeFastSync    | :heavy_check_mark: |
| 78      | dSector_DoorRaiseIn5Mins | :heavy_check_mark: |
| 79      | dFriction_Low            | :heavy_check_mark: |
| 80      | dDamage_SuperHellslime   | :heavy_check_mark: |
| 81      | dLight_FireFlicker       | :heavy_check_mark: |
| 82      | dDamage_LavaWimpy        | :heavy_check_mark: |
| 83      | dDamage_LavaHefty        | :heavy_check_mark: |
| 84      | dScroll_EastLavaDamage   | :heavy_check_mark: |
| 85      | hDamage_Sludge           | :heavy_check_mark: |
| -       | -                        | -                  |
| 87      | Sector_Outside           | :x:                |
| -       | -                        | -                  |
| 104     | sLight_Strobe_Hurt       | :heavy_check_mark: |
| 105     | sDamage_Hellslime        | :heavy_check_mark: |
| -       | -                        | -                  |
| 115     | Damage_InstantDeath      | :heavy_check_mark: |
| 116     | sDamage_SuperHellslime   | :heavy_check_mark: |
| -       | -                        | -                  |
| 118     | Scroll_StrifeCurrent     | :x:                |
| -       | -                        | -                  |
| 195     | Sector_Hidden            | :heavy_check_mark: |
| 196     | Sector_Heal              | :heavy_check_mark: |
| 197     | Light_OutdoorLightning   | :x:                |
| 198-199 | Light_IndoorLightning*   | :x:                |
| 200     | Sky2                     | :x:                |
| 201-224 | Scroll*                  | :heavy_check_mark: |
| 225-244 | Carry*                   | :heavy_check_mark: |