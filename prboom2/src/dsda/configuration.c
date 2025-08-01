//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Config
//

#include <string.h>

#include "am_map.h"
#include "doomdef.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "g_overflow.h"
#include "gl_struct.h"
#include "lprintf.h"
#include "r_main.h"
#include "r_segs.h"
#include "st_stuff.h"
#include "s_sound.h"
#include "s_random.h"
#include "smooth.h"
#include "v_video.h"
#include "z_zone.h"
#include "st_stuff.h"
#include "g_game.h"
#include "m_menu.h"

#include "dsda/args.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/input.h"
#include "dsda/skill_info.h"
#include "dsda/stretch.h"
#include "dsda/utility.h"
#include "dsda/animate.h"

#include "configuration.h"

typedef union {
  int v_int;
  char* v_string;
} dsda_config_value_t;

typedef union {
  int v_int;
  const char* v_string;
} dsda_config_default_t;

typedef struct {
  const char* name;
  dsda_config_identifier_t id;
  dsda_config_type_t type;
  int lower_limit;
  int upper_limit;
  dsda_config_default_t default_value;
  int* int_binding;
  int flags;
  int strict_value;
  void (*onUpdate)(void);
  dsda_config_value_t transient_value;
  dsda_config_value_t persistent_value;
} dsda_config_t;

#define CONF_STRICT  0x01
#define CONF_EVEN    0x02
#define CONF_FEATURE 0x04

#define CONF_BOOL(x) dsda_config_int, 0, 1, { x }
#define CONF_COLOR(x) dsda_config_int, 0, 255, { x }
#define CONF_BYTE(x) dsda_config_int, 0, 255, { x }
#define CONF_STRING(x) dsda_config_string, 0, 0, { .v_string = x }
#define CONF_CR(x) dsda_config_int, 0, CR_HUD_LIMIT - 1, { x }
#define CONF_WEAPON(x) dsda_config_int, 0, 9, { x }

#define NOT_STRICT 0, 0
#define STRICT_INT(x) CONF_FEATURE | CONF_STRICT, x

extern int dsda_input_profile;
extern int weapon_preferences[2][NUMWEAPONS + 1];
extern int demo_smoothturns;
extern int demo_smoothturnsfactor;
extern int sts_colored_numbers;
extern int sts_pct_always_gray;
extern int sts_traditional_keys;
extern int sts_blink_keys;
extern int full_sounds;

void I_Init2(void);
void M_ChangeDemoSmoothTurns(void);
void M_ChangeSkyMode(void);
void M_ChangeMessages(void);
void S_ResetSfxVolume(void);
void I_ResetMusicVolume(void);
void M_ChangeAllowFog(void);
void gld_ResetShadowParameters(void);
void gld_MultisamplingInit(void);
void M_ChangeFOV(void);
void I_InitMouse(void);
void AccelChanging(void);
void G_UpdateMouseSensitivity(void);
void dsda_InitGameController(void);
void M_ChangeSpeed(void);
void M_ChangeShorttics(void);
void I_InitSoundParams(void);
void S_Init(void);
void M_ChangeMIDIPlayer(void);
void HU_InitCrosshair(void);
void HU_InitThresholds(void);
void dsda_InitAutoKeyFrames(void);
void dsda_SetupStretchParams(void);
void dsda_InitCommandHistory(void);
void dsda_InitQuickstartCache(void);
void dsda_InitParallelSFXFilter(void);
void M_ChangeMapMultisamling(void);
void M_ChangeMapTextured(void);
void AM_InitParams(void);
void AM_initPlayerTrail(void);
void gld_ResetAutomapTransparency(void);
void M_ChangeVideoMode(void);
void M_ChangeUncappedFrameRate(void);
void M_ChangeFullScreen(void);
void R_SetViewSize(void);
void M_ChangeApplyPalette(void);
void M_ChangeStretch(void);
void M_ChangeAspectRatio(void);
void dsda_RefreshLinearSky(void);
void deh_changeCompTranslucency(void);
void deh_changeBonusFlash(void);
void deh_changeColoredBlood(void);
void dsda_InitGameControllerParameters(void);
void dsda_InitExHud(void);
void dsda_UpdateFreeText(void);
void dsda_ResetAirControl(void);
void dsda_AlterGameFlags(void);
void dsda_RefreshPistolStart(void);
void dsda_RefreshAlwaysPistolStart(void);
void S_ToggleRandomMusic(void);
void P_UpdateTranMap(void);

void dsda_TrackConfigFeatures(void) {
  if (!demorecording)
    return;

  if (R_PartialView() && dsda_IntConfig(dsda_config_exhud))
    dsda_TrackFeature(uf_exhud);

  if (R_FullView() && dsda_IntConfig(dsda_config_hud_displayed))
    dsda_TrackFeature(uf_advhud);

  if (dsda_IntConfig(dsda_config_game_speed) > 100)
    dsda_TrackFeature(uf_speedup);

  if (dsda_IntConfig(dsda_config_game_speed) < 100)
    dsda_TrackFeature(uf_slowdown);

  if (dsda_IntConfig(dsda_config_coordinate_display) || dsda_IntConfig(dsda_config_map_coordinates))
    dsda_TrackFeature(uf_coordinates);

  if (dsda_IntConfig(dsda_config_weapon_attack_alignment))
    dsda_TrackFeature(uf_weaponalignment);

  if (dsda_IntConfig(dsda_config_command_display))
    dsda_TrackFeature(uf_commanddisplay);

  if (dsda_IntConfig(dsda_config_hudadd_crosshair))
    dsda_TrackFeature(uf_crosshair);

  if (dsda_IntConfig(dsda_config_hudadd_crosshair_target))
    dsda_TrackFeature(uf_crosshaircolor);

  if (dsda_IntConfig(dsda_config_hudadd_crosshair_lock_target))
    dsda_TrackFeature(uf_crosshairlock);

  if (!dsda_IntConfig(dsda_config_palette_ondamage))
    dsda_TrackFeature(uf_painpalette);

  if (!dsda_IntConfig(dsda_config_palette_onbonus))
    dsda_TrackFeature(uf_bonuspalette);

  if (!dsda_IntConfig(dsda_config_palette_onpowers))
    dsda_TrackFeature(uf_powerpalette);

  if (dsda_IntConfig(dsda_config_gl_health_bar))
    dsda_TrackFeature(uf_healthbar);

  if (dsda_IntConfig(dsda_config_sts_blink_keys))
    dsda_TrackFeature(uf_blink_keys);

  if ((dsda_IntConfig(dsda_config_fuzzmode) > 0) && V_IsSoftwareMode())
    dsda_TrackFeature(uf_fuzz);

  if (dsda_IntConfig(dsda_config_movement_strafe50))
    dsda_TrackFeature(uf_alwayssr50);

  if (dsda_IntConfig(dsda_config_max_player_corpse) != 32)
    dsda_TrackFeature(uf_maxplayercorpse);

  if (dsda_IntConfig(dsda_config_hide_weapon))
    dsda_TrackFeature(uf_hideweapon);

  if (dsda_IntConfig(dsda_config_show_alive_monsters))
    dsda_TrackFeature(uf_showalive);

  if (dsda_IntConfig(dsda_config_map_textured) || dsda_IntConfig(dsda_config_show_minimap))
    dsda_TrackFeature(uf_advanced_map);

  if (dsda_IntConfig(dsda_config_translucent_sprites) > 1)
    dsda_TrackFeature(uf_vanillatrans);

  if (dsda_IntConfig(dsda_config_translucent_ghosts))
    dsda_TrackFeature(uf_ghosttrans);
}

// TODO: migrate all kinds of stuff from M_Init

// TODO: automatically go through strict list
void dsda_UpdateStrictMode(void) {
  I_Init2(); // side effect of realtic clock rate
  M_ChangeSpeed(); // side effect of always sr50
  dsda_InitAutoKeyFrames();
  M_ChangeSkyMode(); // affected by mouselook setting
  HU_InitCrosshair();
  M_ChangeApplyPalette();
  M_ChangeMapTextured();
  dsda_RefreshExHudCoordinateDisplay();
  dsda_RefreshExHudCommandDisplay();
  dsda_RefreshExHudMinimap();
  dsda_TrackConfigFeatures();
}

dsda_config_t dsda_config[dsda_config_count] = {
  [dsda_config_game_speed] = {
    "game_speed", dsda_config_game_speed,
    dsda_config_int, 3, 10000, { 100 }, NULL, STRICT_INT(100), I_Init2
  },
  [dsda_config_default_complevel] = {
    "default_compatibility_level", dsda_config_default_complevel,
    dsda_config_int, 0, mbf21_compatibility, { mbf21_compatibility }
  },
  [dsda_config_default_skill] = {
    "default_skill", dsda_config_default_skill,
    dsda_config_int, 1, 5, { 4 }
  },
  [dsda_config_vanilla_keymap] = {
    "vanilla_keymap", dsda_config_vanilla_keymap,
    CONF_BOOL(0)
  },
  [nyan_config_menu_play_demo] = {
    "nyan_play_demos_in_menu", nyan_config_menu_play_demo,
    CONF_BOOL(1)
  },
  [dsda_config_menu_background] = {
    "menu_background", dsda_config_menu_background,
    dsda_config_int, 0, 2, { 1 }
  },
  [dsda_config_process_priority] = {
    "process_priority", dsda_config_process_priority,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_max_player_corpse] = {
    "max_player_corpse", dsda_config_max_player_corpse,
    dsda_config_int, -1, INT_MAX, { 32 }, NULL, STRICT_INT(32)
  },
  [dsda_config_input_profile] = {
    "input_profile", dsda_config_input_profile,
    dsda_config_int, 0, DSDA_INPUT_PROFILE_COUNT - 1, { 0 }, &dsda_input_profile
  },
  [dsda_config_weapon_choice_1] = {
    "weapon_choice_1", dsda_config_weapon_choice_1,
    CONF_WEAPON(6), &weapon_preferences[0][0]
  },
  [dsda_config_weapon_choice_2] = {
    "weapon_choice_2", dsda_config_weapon_choice_2,
    CONF_WEAPON(9), &weapon_preferences[0][1]
  },
  [dsda_config_weapon_choice_3] = {
    "weapon_choice_3", dsda_config_weapon_choice_3,
    CONF_WEAPON(4), &weapon_preferences[0][2]
  },
  [dsda_config_weapon_choice_4] = {
    "weapon_choice_4", dsda_config_weapon_choice_4,
    CONF_WEAPON(3), &weapon_preferences[0][3]
  },
  [dsda_config_weapon_choice_5] = {
    "weapon_choice_5", dsda_config_weapon_choice_5,
    CONF_WEAPON(2), &weapon_preferences[0][4]
  },
  [dsda_config_weapon_choice_6] = {
    "weapon_choice_6", dsda_config_weapon_choice_6,
    CONF_WEAPON(8), &weapon_preferences[0][5]
  },
  [dsda_config_weapon_choice_7] = {
    "weapon_choice_7", dsda_config_weapon_choice_7,
    CONF_WEAPON(5), &weapon_preferences[0][6]
  },
  [dsda_config_weapon_choice_8] = {
    "weapon_choice_8", dsda_config_weapon_choice_8,
    CONF_WEAPON(7), &weapon_preferences[0][7]
  },
  [dsda_config_weapon_choice_9] = {
    "weapon_choice_9", dsda_config_weapon_choice_9,
    CONF_WEAPON(1), &weapon_preferences[0][8]
  },
  [dsda_config_flashing_hom] = {
    "flashing_hom", dsda_config_flashing_hom,
    CONF_BOOL(0)
  },
  [dsda_config_demo_smoothturns] = {
    "demo_smoothturns", dsda_config_demo_smoothturns,
    CONF_BOOL(0), &demo_smoothturns,
    NOT_STRICT, M_ChangeDemoSmoothTurns
  },
  [dsda_config_demo_smoothturnsfactor] = {
    "demo_smoothturnsfactor", dsda_config_demo_smoothturnsfactor,
    dsda_config_int, 1, SMOOTH_PLAYING_MAXFACTOR, { 6 }, &demo_smoothturnsfactor,
    NOT_STRICT, M_ChangeDemoSmoothTurns
  },
  [dsda_config_weapon_attack_alignment] = {
    "weapon_attack_alignment", dsda_config_weapon_attack_alignment,
    dsda_config_int, 0, 3, { 0 }, NULL, STRICT_INT(0)
  },
  [dsda_config_sts_colored_numbers] = {
    "sts_colored_numbers", dsda_config_sts_colored_numbers,
    CONF_BOOL(0), &sts_colored_numbers
  },
  [dsda_config_sts_pct_always_gray] = {
    "sts_pct_always_gray", dsda_config_sts_pct_always_gray,
    CONF_BOOL(0), &sts_pct_always_gray
  },
  [dsda_config_hud_animated_count] = {
    "hud_animated_count", dsda_config_hud_animated_count,
    CONF_BOOL(0), NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_sts_traditional_keys] = {
    "sts_traditional_keys", dsda_config_sts_traditional_keys,
    CONF_BOOL(0), &sts_traditional_keys
  },
  [dsda_config_sts_blink_keys] = {
    "sts_blink_keys", dsda_config_sts_blink_keys,
    CONF_BOOL(0), &sts_blink_keys, STRICT_INT(0),
  },
  [dsda_config_sts_solid_bg_color] = {
    "sts_solid_bg_color", dsda_config_sts_solid_bg_color,
    CONF_BOOL(0), NULL, NOT_STRICT, ST_SetResolution
  },
  [dsda_config_strict_mode] = {
    "dsda_strict_mode", dsda_config_strict_mode,
    CONF_BOOL(1), NULL, NOT_STRICT, dsda_UpdateStrictMode
  },
  [dsda_config_vertmouse] = {
    "movement_vertmouse", dsda_config_vertmouse,
    CONF_BOOL(0)
  },
  [dsda_config_freelook] = {
    "allow_freelook", dsda_config_freelook,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeSkyMode
  },
  [dsda_config_autorun] = {
    "autorun", dsda_config_autorun,
    CONF_BOOL(1)
  },
  [dsda_config_show_messages] = {
    "show_messages", dsda_config_show_messages,
    CONF_BOOL(1), NULL, NOT_STRICT, M_ChangeMessages
  },
  [dsda_config_stats_format] = {
    "dsda_stats_format", dsda_config_stats_format,
    dsda_config_int, 0, 5, { 0 }, NULL, NOT_STRICT
  },
  [dsda_config_command_display] = {
    "dsda_command_display", dsda_config_command_display,
    CONF_BOOL(0), NULL, STRICT_INT(0), dsda_RefreshExHudCommandDisplay
  },
  [dsda_config_coordinate_display] = {
    "dsda_coordinate_display", dsda_config_coordinate_display,
    CONF_BOOL(0), NULL, STRICT_INT(0), dsda_RefreshExHudCoordinateDisplay
  },
  [dsda_config_show_fps] = {
    "dsda_show_fps", dsda_config_show_fps,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_RefreshExHudFPS
  },
  [dsda_config_show_minimap] = {
    "dsda_show_minimap", dsda_config_show_minimap,
    CONF_BOOL(0), NULL, STRICT_INT(0), dsda_RefreshExHudMinimap
  },
  [dsda_config_show_level_splits] = {
    "dsda_show_level_splits", dsda_config_show_level_splits,
    CONF_BOOL(1), NULL, NOT_STRICT, dsda_RefreshExHudLevelSplits
  },
  [dsda_config_exhud] = {
    "dsda_exhud", dsda_config_exhud,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT, dsda_InitExHud
  },
  [nyan_config_hud_berserk] = {
    "nyan_berserk_icon", nyan_config_hud_berserk,
    dsda_config_int, BERSERK_ICON_OFF, BERSERK_ICON_ON,
    { BERSERK_ICON_ON }, (int*) &berserk_icon
  },
  [nyan_config_hud_armoricon] = {
    "nyan_armor_icon", nyan_config_hud_armoricon,
    dsda_config_int, ARMOR_ICON_OFF, ARMOR_ICON_2,
    { ARMOR_ICON_1 }, (int*) &armor_icon
  },
  [nyan_config_ex_status_widget] = {
    "nyan_ex_status_widget", nyan_config_ex_status_widget,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_armor] = {
    "nyan_ex_status_armor", nyan_config_ex_status_armor,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_berserk] = {
    "nyan_ex_status_berserk", nyan_config_ex_status_berserk,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_areamap] = {
    "nyan_ex_status_areamap", nyan_config_ex_status_areamap,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_backpack] = {
    "nyan_ex_status_backpack", nyan_config_ex_status_backpack,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_radsuit] = {
    "nyan_ex_status_radsuit", nyan_config_ex_status_radsuit,
    CONF_BOOL(1), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_invis] = {
    "nyan_ex_status_invis", nyan_config_ex_status_invis,
    CONF_BOOL(1), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_liteamp] = {
    "nyan_ex_status_liteamp", nyan_config_ex_status_liteamp,
    CONF_BOOL(1), NULL, CONF_FEATURE | NOT_STRICT
  },
  [nyan_config_ex_status_invuln] = {
    "nyan_ex_status_invuln", nyan_config_ex_status_invuln,
    CONF_BOOL(1), NULL, CONF_FEATURE | NOT_STRICT
  },
  [dsda_config_free_text] = {
    "dsda_free_text", dsda_config_free_text,
    CONF_STRING(""), NULL, NOT_STRICT, dsda_UpdateFreeText
  },
  [dsda_config_mute_sfx] = {
    "dsda_mute_sfx", dsda_config_mute_sfx,
    CONF_BOOL(0), NULL, NOT_STRICT, S_ResetSfxVolume
  },
  [dsda_config_mute_music] = {
    "dsda_mute_music", dsda_config_mute_music,
    CONF_BOOL(0), NULL, NOT_STRICT, I_ResetMusicVolume
  },
  [dsda_config_mute_unfocused_window] = {
    "dsda_mute_unfocused_window", dsda_config_mute_unfocused_window,
    CONF_BOOL(0), NULL, NOT_STRICT, S_ResetVolume
  },
  [dsda_config_cheat_codes] = {
    "dsda_cheat_codes", dsda_config_cheat_codes,
    CONF_BOOL(1), NULL, STRICT_INT(0)
  },
  [dsda_config_organize_failed_demos] = {
    "dsda_organize_failed_demos", dsda_config_organize_failed_demos,
    CONF_BOOL(0)
  },
  [nyan_config_full_menu_fade] = {
    "nyan_full_menu_fade", nyan_config_full_menu_fade,
    CONF_BOOL(1)
  },
  [nyan_config_gradual_menu_fade] = {
    "nyan_gradual_menu_fade", nyan_config_gradual_menu_fade,
    CONF_BOOL(1)
  },
  [nyan_config_skip_default_demos] = {
   "nyan_skip_default_demos", nyan_config_skip_default_demos,
   CONF_BOOL(1), NULL, STRICT_INT(0)
  },
  [nyan_config_skip_default_text] = {
   "nyan_skip_default_text", nyan_config_skip_default_text,
   CONF_BOOL(1), NULL, STRICT_INT(0)
  },
  [nyan_config_ignore_default_map_names] = {
   "nyan_ignore_default_map_names", nyan_config_ignore_default_map_names,
   CONF_BOOL(1), NULL, NOT_STRICT
  },
  [nyan_config_play_random_music] = {
    "nyan_play_random_music", nyan_config_play_random_music,
    CONF_BOOL(0), NULL, NOT_STRICT, S_ToggleRandomMusic
   },
  [nyan_config_enable_widescreen_lumps] = {
   "nyan_enable_widescreen_lumps", nyan_config_enable_widescreen_lumps,
   CONF_BOOL(1), NULL, NOT_STRICT, N_ReloadAnimateLumps
  },
  [nyan_config_enable_animate_lumps] = {
   "nyan_enable_animate_lumps", nyan_config_enable_animate_lumps,
   CONF_BOOL(1), NULL, NOT_STRICT, N_ReloadAnimateLumps
  },
  [dsda_config_script_0] = {
    "dsda_script_0", dsda_config_script_0,
    CONF_STRING("")
  },
  [dsda_config_script_1] = {
    "dsda_script_1", dsda_config_script_1,
    CONF_STRING("")
  },
  [dsda_config_script_2] = {
    "dsda_script_2", dsda_config_script_2,
    CONF_STRING("")
  },
  [dsda_config_script_3] = {
    "dsda_script_3", dsda_config_script_3,
    CONF_STRING("")
  },
  [dsda_config_script_4] = {
    "dsda_script_4", dsda_config_script_4,
    CONF_STRING("")
  },
  [dsda_config_script_5] = {
    "dsda_script_5", dsda_config_script_5,
    CONF_STRING("")
  },
  [dsda_config_script_6] = {
    "dsda_script_6", dsda_config_script_6,
    CONF_STRING("")
  },
  [dsda_config_script_7] = {
    "dsda_script_7", dsda_config_script_7,
    CONF_STRING("")
  },
  [dsda_config_script_8] = {
    "dsda_script_8", dsda_config_script_8,
    CONF_STRING("")
  },
  [dsda_config_script_9] = {
    "dsda_script_9", dsda_config_script_9,
    CONF_STRING("")
  },
  [dsda_config_overrun_spechit_warn] = {
    "overrun_spechit_warn", dsda_config_overrun_spechit_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_SPECHIT].warn
  },
  [dsda_config_overrun_spechit_emulate] = {
    "overrun_spechit_emulate", dsda_config_overrun_spechit_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_SPECHIT].emulate
  },
  [dsda_config_overrun_reject_warn] = {
    "overrun_reject_warn", dsda_config_overrun_reject_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_REJECT].warn
  },
  [dsda_config_overrun_reject_emulate] = {
    "overrun_reject_emulate", dsda_config_overrun_reject_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_REJECT].emulate
  },
  [dsda_config_overrun_intercept_warn] = {
    "overrun_intercept_warn", dsda_config_overrun_intercept_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_INTERCEPT].warn
  },
  [dsda_config_overrun_intercept_emulate] = {
    "overrun_intercept_emulate", dsda_config_overrun_intercept_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_INTERCEPT].emulate
  },
  [dsda_config_overrun_playeringame_warn] = {
    "overrun_playeringame_warn", dsda_config_overrun_playeringame_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_PLAYERINGAME].warn
  },
  [dsda_config_overrun_playeringame_emulate] = {
    "overrun_playeringame_emulate", dsda_config_overrun_playeringame_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_PLAYERINGAME].emulate
  },
  [dsda_config_overrun_donut_warn] = {
    "overrun_donut_warn", dsda_config_overrun_donut_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_DONUT].warn
  },
  [dsda_config_overrun_donut_emulate] = {
    "overrun_donut_emulate", dsda_config_overrun_donut_emulate,
    CONF_BOOL(0), &overflows[OVERFLOW_DONUT].emulate
  },
  [dsda_config_overrun_missedbackside_warn] = {
    "overrun_missedbackside_warn", dsda_config_overrun_missedbackside_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_MISSEDBACKSIDE].warn
  },
  [dsda_config_overrun_missedbackside_emulate] = {
    "overrun_missedbackside_emulate", dsda_config_overrun_missedbackside_emulate,
    CONF_BOOL(0), &overflows[OVERFLOW_MISSEDBACKSIDE].emulate
  },
  [dsda_config_comperr_zerotag] = {
  "comperr_zerotag", dsda_config_comperr_zerotag,
  CONF_BOOL(0), &default_comperr[comperr_zerotag]
  },
  [dsda_config_comperr_passuse] = {
    "comperr_passuse", dsda_config_comperr_passuse,
    CONF_BOOL(0), &default_comperr[comperr_passuse]
  },
  [dsda_config_comperr_hangsolid] = {
    "comperr_hangsolid", dsda_config_comperr_hangsolid,
    CONF_BOOL(0), &default_comperr[comperr_hangsolid]
  },
  [dsda_config_comperr_blockmap] = {
    "comperr_blockmap", dsda_config_comperr_blockmap,
    CONF_BOOL(0), &default_comperr[comperr_blockmap]
  },
  [dsda_config_mapcolor_back] = {
    "mapcolor_back", dsda_config_mapcolor_back,
    CONF_COLOR(247), &mapcolor.back
  },
  [dsda_config_mapcolor_grid] = {
    "mapcolor_grid", dsda_config_mapcolor_grid,
    CONF_COLOR(104), &mapcolor.grid
  },
  [dsda_config_mapcolor_wall] = {
    "mapcolor_wall", dsda_config_mapcolor_wall,
    CONF_COLOR(23), &mapcolor.wall
  },
  [dsda_config_mapcolor_fchg] = {
    "mapcolor_fchg", dsda_config_mapcolor_fchg,
    CONF_COLOR(55), &mapcolor.fchg
  },
  [dsda_config_mapcolor_cchg] = {
    "mapcolor_cchg", dsda_config_mapcolor_cchg,
    CONF_COLOR(215), &mapcolor.cchg
  },
  [dsda_config_mapcolor_clsd] = {
    "mapcolor_clsd", dsda_config_mapcolor_clsd,
    CONF_COLOR(208), &mapcolor.clsd
  },
  [dsda_config_mapcolor_rkey] = {
    "mapcolor_rkey", dsda_config_mapcolor_rkey,
    CONF_COLOR(175), &mapcolor.rkey
  },
  [dsda_config_mapcolor_bkey] = {
    "mapcolor_bkey", dsda_config_mapcolor_bkey,
    CONF_COLOR(204), &mapcolor.bkey
  },
  [dsda_config_mapcolor_ykey] = {
    "mapcolor_ykey", dsda_config_mapcolor_ykey,
    CONF_COLOR(231), &mapcolor.ykey
  },
  [dsda_config_mapcolor_rdor] = {
    "mapcolor_rdor", dsda_config_mapcolor_rdor,
    CONF_COLOR(175), &mapcolor.rdor
  },
  [dsda_config_mapcolor_bdor] = {
    "mapcolor_bdor", dsda_config_mapcolor_bdor,
    CONF_COLOR(204), &mapcolor.bdor
  },
  [dsda_config_mapcolor_ydor] = {
    "mapcolor_ydor", dsda_config_mapcolor_ydor,
    CONF_COLOR(231), &mapcolor.ydor
  },
  [dsda_config_mapcolor_tele] = {
    "mapcolor_tele", dsda_config_mapcolor_tele,
    CONF_COLOR(119), &mapcolor.tele
  },
  [dsda_config_mapcolor_secr] = {
    "mapcolor_secr", dsda_config_mapcolor_secr,
    CONF_COLOR(252), &mapcolor.secr
  },
  [dsda_config_mapcolor_revsecr] = {
    "mapcolor_revsecr", dsda_config_mapcolor_revsecr,
    CONF_COLOR(112), &mapcolor.revsecr
  },
  [dsda_config_mapcolor_tagfinder] = {
    "mapcolor_tagfinder", dsda_config_mapcolor_tagfinder,
    CONF_COLOR(252), &mapcolor.tagfinder
  },
  [dsda_config_mapcolor_exit] = {
    "mapcolor_exit", dsda_config_mapcolor_exit,
    CONF_COLOR(0), &mapcolor.exit
  },
  [dsda_config_mapcolor_exitsecr] = {
    "mapcolor_exitsecr", dsda_config_mapcolor_exitsecr,
    CONF_COLOR(0), &mapcolor.exitsecr
  },
  [dsda_config_mapcolor_unsn] = {
    "mapcolor_unsn", dsda_config_mapcolor_unsn,
    CONF_COLOR(104), &mapcolor.unsn
  },
  [dsda_config_mapcolor_flat] = {
    "mapcolor_flat", dsda_config_mapcolor_flat,
    CONF_COLOR(88), &mapcolor.flat
  },
  [dsda_config_mapcolor_sprt] = {
    "mapcolor_sprt", dsda_config_mapcolor_sprt,
    CONF_COLOR(112), &mapcolor.sprt
  },
  [dsda_config_mapcolor_item] = {
    "mapcolor_item", dsda_config_mapcolor_item,
    CONF_COLOR(231), &mapcolor.item
  },
  [dsda_config_mapcolor_hair] = {
    "mapcolor_hair", dsda_config_mapcolor_hair,
    CONF_COLOR(208), &mapcolor.hair
  },
  [dsda_config_mapcolor_sngl] = {
    "mapcolor_sngl", dsda_config_mapcolor_sngl,
    CONF_COLOR(208), &mapcolor.sngl
  },
  [dsda_config_mapcolor_me] = {
    "mapcolor_me", dsda_config_mapcolor_me,
    CONF_COLOR(112), &mapcolor.me
  },
  [dsda_config_mapcolor_enemy] = {
    "mapcolor_enemy", dsda_config_mapcolor_enemy,
    CONF_COLOR(177), &mapcolor.enemy
  },
  [dsda_config_mapcolor_frnd] = {
    "mapcolor_frnd", dsda_config_mapcolor_frnd,
    CONF_COLOR(112), &mapcolor.frnd
  },
  [dsda_config_mapcolor_trail_1] = {
    "mapcolor_trail_1", dsda_config_mapcolor_trail_1,
    CONF_COLOR(80), &mapcolor.trail_1
  },
  [dsda_config_mapcolor_trail_2] = {
    "mapcolor_trail_2", dsda_config_mapcolor_trail_2,
    CONF_COLOR(100), &mapcolor.trail_2
  },
  [dsda_config_mapcolor_heretic_back] = {
    "mapcolor_heretic_back", dsda_config_mapcolor_heretic_back,
    CONF_COLOR(96), &mapcolor_heretic.back
  },
  [dsda_config_mapcolor_heretic_grid] = {
    "mapcolor_heretic_grid", dsda_config_mapcolor_heretic_grid,
    CONF_COLOR(40), &mapcolor_heretic.grid
  },
  [dsda_config_mapcolor_heretic_wall] = {
    "mapcolor_heretic_wall", dsda_config_mapcolor_heretic_wall,
    CONF_COLOR(104), &mapcolor_heretic.wall
  },
  [dsda_config_mapcolor_heretic_fchg] = {
    "mapcolor_heretic_fchg", dsda_config_mapcolor_heretic_fchg,
    CONF_COLOR(90), &mapcolor_heretic.fchg
  },
  [dsda_config_mapcolor_heretic_cchg] = {
    "mapcolor_heretic_cchg", dsda_config_mapcolor_heretic_cchg,
    CONF_COLOR(80), &mapcolor_heretic.cchg
  },
  [dsda_config_mapcolor_heretic_clsd] = {
    "mapcolor_heretic_clsd", dsda_config_mapcolor_heretic_clsd,
    CONF_COLOR(94), &mapcolor_heretic.clsd
  },
  [dsda_config_mapcolor_heretic_gkey] = {
    "mapcolor_heretic_gkey", dsda_config_mapcolor_heretic_gkey,
    CONF_COLOR(220), &mapcolor_heretic.rkey
  },
  [dsda_config_mapcolor_heretic_bkey] = {
    "mapcolor_heretic_bkey", dsda_config_mapcolor_heretic_bkey,
    CONF_COLOR(197), &mapcolor_heretic.bkey
  },
  [dsda_config_mapcolor_heretic_ykey] = {
    "mapcolor_heretic_ykey", dsda_config_mapcolor_heretic_ykey,
    CONF_COLOR(144), &mapcolor_heretic.ykey
  },
  [dsda_config_mapcolor_heretic_gdor] = {
    "mapcolor_heretic_gdor", dsda_config_mapcolor_heretic_gdor,
    CONF_COLOR(220), &mapcolor_heretic.rdor
  },
  [dsda_config_mapcolor_heretic_bdor] = {
    "mapcolor_heretic_bdor", dsda_config_mapcolor_heretic_bdor,
    CONF_COLOR(197), &mapcolor_heretic.bdor
  },
  [dsda_config_mapcolor_heretic_ydor] = {
    "mapcolor_heretic_ydor", dsda_config_mapcolor_heretic_ydor,
    CONF_COLOR(144), &mapcolor_heretic.ydor
  },
  [dsda_config_mapcolor_heretic_tele] = {
    "mapcolor_heretic_tele", dsda_config_mapcolor_heretic_tele,
    CONF_COLOR(223), &mapcolor_heretic.tele
  },
  [dsda_config_mapcolor_heretic_secr] = {
    "mapcolor_heretic_secr", dsda_config_mapcolor_heretic_secr,
    CONF_COLOR(175), &mapcolor_heretic.secr
  },
  [dsda_config_mapcolor_heretic_revsecr] = {
    "mapcolor_heretic_revsecr", dsda_config_mapcolor_heretic_revsecr,
    CONF_COLOR(223), &mapcolor_heretic.revsecr
  },
  [dsda_config_mapcolor_heretic_tagfinder] = {
    "mapcolor_heretic_tagfinder", dsda_config_mapcolor_heretic_tagfinder,
    CONF_COLOR(175), &mapcolor_heretic.tagfinder
  },
  [dsda_config_mapcolor_heretic_exit] = {
    "mapcolor_heretic_exit", dsda_config_mapcolor_heretic_exit,
    CONF_COLOR(0), &mapcolor_heretic.exit
  },
  [dsda_config_mapcolor_heretic_exitsecr] = {
    "mapcolor_heretic_exitsecr", dsda_config_mapcolor_heretic_exitsecr,
    CONF_COLOR(0), &mapcolor_heretic.exitsecr
  },
  [dsda_config_mapcolor_heretic_unsn] = {
    "mapcolor_heretic_unsn", dsda_config_mapcolor_heretic_unsn,
    CONF_COLOR(43), &mapcolor_heretic.unsn
  },
  [dsda_config_mapcolor_heretic_flat] = {
    "mapcolor_heretic_flat", dsda_config_mapcolor_heretic_flat,
    CONF_COLOR(74), &mapcolor_heretic.flat
  },
  [dsda_config_mapcolor_heretic_sprt] = {
    "mapcolor_heretic_sprt", dsda_config_mapcolor_heretic_sprt,
    CONF_COLOR(224), &mapcolor_heretic.sprt
  },
  [dsda_config_mapcolor_heretic_item] = {
    "mapcolor_heretic_item", dsda_config_mapcolor_heretic_item,
    CONF_COLOR(144), &mapcolor_heretic.item
  },
  [dsda_config_mapcolor_heretic_hair] = {
    "mapcolor_heretic_hair", dsda_config_mapcolor_heretic_hair,
    CONF_COLOR(40), &mapcolor_heretic.hair
  },
  [dsda_config_mapcolor_heretic_sngl] = {
    "mapcolor_heretic_sngl", dsda_config_mapcolor_heretic_sngl,
    CONF_COLOR(32), &mapcolor_heretic.sngl
  },
  [dsda_config_mapcolor_heretic_me] = {
    "mapcolor_heretic_me", dsda_config_mapcolor_heretic_me,
    CONF_COLOR(220), &mapcolor_heretic.me
  },
  [dsda_config_mapcolor_heretic_enemy] = {
    "mapcolor_heretic_enemy", dsda_config_mapcolor_heretic_enemy,
    CONF_COLOR(160), &mapcolor_heretic.enemy
  },
  [dsda_config_mapcolor_heretic_frnd] = {
    "mapcolor_heretic_frnd", dsda_config_mapcolor_heretic_frnd,
    CONF_COLOR(224), &mapcolor_heretic.frnd
  },
  [dsda_config_mapcolor_heretic_trail_1] = {
    "mapcolor_heretic_trail_1", dsda_config_mapcolor_heretic_trail_1,
    CONF_COLOR(33), &mapcolor_heretic.trail_1
  },
  [dsda_config_mapcolor_heretic_trail_2] = {
    "mapcolor_heretic_trail_2", dsda_config_mapcolor_heretic_trail_2,
    CONF_COLOR(15), &mapcolor_heretic.trail_2
  },
  [dsda_config_mapcolor_hexen_back] = {
    "mapcolor_hexen_back", dsda_config_mapcolor_hexen_back,
    CONF_COLOR(81), &mapcolor_hexen.back
  },
  [dsda_config_mapcolor_hexen_grid] = {
    "mapcolor_hexen_grid", dsda_config_mapcolor_hexen_grid,
    CONF_COLOR(40), &mapcolor_hexen.grid
  },
  [dsda_config_mapcolor_hexen_wall] = {
    "mapcolor_hexen_wall", dsda_config_mapcolor_hexen_wall,
    CONF_COLOR(96), &mapcolor_hexen.wall
  },
  [dsda_config_mapcolor_hexen_fchg] = {
    "mapcolor_hexen_fchg", dsda_config_mapcolor_hexen_fchg,
    CONF_COLOR(112), &mapcolor_hexen.fchg
  },
  [dsda_config_mapcolor_hexen_cchg] = {
    "mapcolor_hexen_cchg", dsda_config_mapcolor_hexen_cchg,
    CONF_COLOR(80), &mapcolor_hexen.cchg
  },
  [dsda_config_mapcolor_hexen_clsd] = {
    "mapcolor_hexen_clsd", dsda_config_mapcolor_hexen_clsd,
    CONF_COLOR(86), &mapcolor_hexen.clsd
  },
  [dsda_config_mapcolor_hexen_key] = {
    "mapcolor_hexen_key", dsda_config_mapcolor_hexen_key,
    CONF_COLOR(179), &mapcolor_hexen.ykey
  },
  [dsda_config_mapcolor_hexen_puzzle] = {
    "mapcolor_hexen_puzzle", dsda_config_mapcolor_hexen_puzzle,
    CONF_COLOR(163), &mapcolor_hexen.bkey
  },
  [dsda_config_mapcolor_hexen_dor] = {
    "mapcolor_hexen_dor", dsda_config_mapcolor_hexen_dor,
    CONF_COLOR(179), &mapcolor_hexen.ydor
  },
  [dsda_config_mapcolor_hexen_tele] = {
    "mapcolor_hexen_tele", dsda_config_mapcolor_hexen_tele,
    CONF_COLOR(215), &mapcolor_hexen.tele
  },
  [dsda_config_mapcolor_hexen_tagfinder] = {
    "mapcolor_hexen_tagfinder", dsda_config_mapcolor_hexen_tagfinder,
    CONF_COLOR(175), &mapcolor_hexen.tagfinder
  },
  [dsda_config_mapcolor_hexen_exit] = {
    "mapcolor_hexen_exit", dsda_config_mapcolor_hexen_exit,
    CONF_COLOR(177), &mapcolor_hexen.exit
  },
  [dsda_config_mapcolor_hexen_unsn] = {
    "mapcolor_hexen_unsn", dsda_config_mapcolor_hexen_unsn,
    CONF_COLOR(43), &mapcolor_hexen.unsn
  },
  [dsda_config_mapcolor_hexen_flat] = {
    "mapcolor_hexen_flat", dsda_config_mapcolor_hexen_flat,
    CONF_COLOR(84), &mapcolor_hexen.flat
  },
  [dsda_config_mapcolor_hexen_sprt] = {
    "mapcolor_hexen_sprt", dsda_config_mapcolor_hexen_sprt,
    CONF_COLOR(216), &mapcolor_hexen.sprt
  },
  [dsda_config_mapcolor_hexen_item] = {
    "mapcolor_hexen_item", dsda_config_mapcolor_hexen_item,
    CONF_COLOR(230), &mapcolor_hexen.item
  },
  [dsda_config_mapcolor_hexen_hair] = {
    "mapcolor_hexen_hair", dsda_config_mapcolor_hexen_hair,
    CONF_COLOR(40), &mapcolor_hexen.hair
  },
  [dsda_config_mapcolor_hexen_sngl] = {
    "mapcolor_hexen_sngl", dsda_config_mapcolor_hexen_sngl,
    CONF_COLOR(32), &mapcolor_hexen.sngl
  },
  [dsda_config_mapcolor_hexen_me] = {
    "mapcolor_hexen_me", dsda_config_mapcolor_hexen_me,
    CONF_COLOR(157), &mapcolor_hexen.me
  },
  [dsda_config_mapcolor_hexen_enemy] = {
    "mapcolor_hexen_enemy", dsda_config_mapcolor_hexen_enemy,
    CONF_COLOR(176), &mapcolor_hexen.enemy
  },
  [dsda_config_mapcolor_hexen_frnd] = {
    "mapcolor_hexen_frnd", dsda_config_mapcolor_hexen_frnd,
    CONF_COLOR(216), &mapcolor_hexen.frnd
  },
  [dsda_config_mapcolor_hexen_trail_1] = {
    "mapcolor_hexen_trail_1", dsda_config_mapcolor_hexen_trail_1,
    CONF_COLOR(33), &mapcolor_hexen.trail_1
  },
  [dsda_config_mapcolor_hexen_trail_2] = {
    "mapcolor_hexen_trail_2", dsda_config_mapcolor_hexen_trail_2,
    CONF_COLOR(15), &mapcolor_hexen.trail_2
  },
  [dsda_config_gl_blend_animations] = {
  "gl_blend_animations", dsda_config_gl_blend_animations,
  CONF_BOOL(0), &gl_blend_animations
  },
  [dsda_config_gl_skymode] = {
    "gl_skymode", dsda_config_gl_skymode,
    dsda_config_int, skytype_auto, skytype_count - 1, { skytype_auto }, NULL,
    NOT_STRICT, M_ChangeSkyMode
  },
  [dsda_config_gl_render_multisampling] = {
    "gl_render_multisampling", dsda_config_gl_render_multisampling,
    dsda_config_int, 0, 8, { 0 }, NULL, CONF_EVEN, 0, gld_MultisamplingInit
  },
  [dsda_config_gl_render_fov] = {
    "gl_render_fov", dsda_config_gl_render_fov,
    dsda_config_int, 20, 160, { 90 }, &gl_render_fov, NOT_STRICT, M_ChangeFOV
  },
  [dsda_config_gl_health_bar] = {
    "gl_health_bar", dsda_config_gl_health_bar,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [dsda_config_gl_usevbo] = {
    "gl_usevbo", dsda_config_gl_usevbo,
    CONF_BOOL(1), NULL, NOT_STRICT
  },
  [dsda_config_use_mouse] = {
    "use_mouse", dsda_config_use_mouse,
    CONF_BOOL(1), NULL, NOT_STRICT, I_InitMouse
  },
  [dsda_config_mouse_sensitivity_horiz] = {
    "mouse_sensitivity_horiz", dsda_config_mouse_sensitivity_horiz,
    dsda_config_int, 0, INT_MAX, { 10 }, NULL, NOT_STRICT, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_sensitivity_vert] = {
    "mouse_sensitivity_vert", dsda_config_mouse_sensitivity_vert,
    dsda_config_int, 0, INT_MAX, { 1 }, NULL, NOT_STRICT, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_acceleration] = {
    "dsda_mouse_acceleration", dsda_config_mouse_acceleration,
    dsda_config_int, 0, INT_MAX, { 0 }, NULL, NOT_STRICT, AccelChanging
  },
  [dsda_config_mouse_sensitivity_mlook] = {
    "mouse_sensitivity_mlook", dsda_config_mouse_sensitivity_mlook,
    dsda_config_int, 0, INT_MAX, { 10 }, NULL, NOT_STRICT, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_stutter_correction] = {
    "mouse_stutter_correction", dsda_config_mouse_stutter_correction,
    CONF_BOOL(1)
  },
  [dsda_config_mouse_doubleclick_as_use] = {
    "mouse_doubleclick_as_use", dsda_config_mouse_doubleclick_as_use,
    CONF_BOOL(0)
  },
  [dsda_config_mouse_carrytics] = {
    "mouse_carrytics", dsda_config_mouse_carrytics,
    CONF_BOOL(1)
  },
  [dsda_config_movement_mouseinvert] = {
    "movement_mouseinvert", dsda_config_movement_mouseinvert,
    CONF_BOOL(0)
  },
  [dsda_config_movement_mousestrafedivisor] = {
    "movement_mousestrafedivisor", dsda_config_movement_mousestrafedivisor,
    dsda_config_int, 1, INT_MAX, { 4 }, NULL, NOT_STRICT, G_UpdateMouseSensitivity
  },
  [dsda_config_fine_sensitivity] = {
    "dsda_fine_sensitivity", dsda_config_fine_sensitivity,
    dsda_config_int, 0, 99, { 0 }, NULL, NOT_STRICT, G_UpdateMouseSensitivity
  },
  [dsda_config_use_game_controller] = {
    "use_game_controller", dsda_config_use_game_controller,
    dsda_config_int, 0, 2, { 0 }, NULL, NOT_STRICT, dsda_InitGameController
  },
  [dsda_config_deh_change_cheats] = {
    "dsda_deh_change_cheats", dsda_config_deh_change_cheats,
    CONF_BOOL(1)
  },
  [dsda_config_movement_strafe50] = {
    "movement_strafe50", dsda_config_movement_strafe50,
    CONF_BOOL(0), NULL, STRICT_INT(0), M_ChangeSpeed
  },
  [dsda_config_movement_strafe50onturns] = {
    "movement_strafe50onturns", dsda_config_movement_strafe50onturns,
    CONF_BOOL(0), NULL, STRICT_INT(0), M_ChangeSpeed
  },
  [dsda_config_movement_shorttics] = {
    "movement_shorttics", dsda_config_movement_shorttics,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeShorttics
  },
  [dsda_config_screenshot_dir] = {
    "screenshot_dir", dsda_config_screenshot_dir,
    CONF_STRING("")
  },
  [dsda_config_startup_delay_ms] = {
    "startup_delay_ms", dsda_config_startup_delay_ms,
    dsda_config_int, 0, 1000, { 0 }
  },
  [dsda_config_pitched_sounds] = {
    "pitched_sounds", dsda_config_pitched_sounds,
    CONF_BOOL(0), NULL, NOT_STRICT, I_InitSoundParams
  },
  [dsda_config_full_sounds] = {
    "full_sounds", dsda_config_full_sounds,
    CONF_BOOL(0), &full_sounds
  },
  [dsda_config_snd_samplerate] = {
    "snd_samplerate", dsda_config_snd_samplerate,
    dsda_config_int, 11025, 48000, { 44100 }, NULL, NOT_STRICT, I_InitSoundParams
  },
  [dsda_config_snd_samplecount] = {
    "snd_samplecount", dsda_config_snd_samplecount,
    dsda_config_int, 0, 8192, { 0 }, NULL, NOT_STRICT, I_InitSoundParams
  },
  [dsda_config_sfx_volume] = {
    "sfx_volume", dsda_config_sfx_volume,
    dsda_config_int, 0, 15, { 8 }, NULL, NOT_STRICT, S_ResetSfxVolume
  },
  [dsda_config_music_volume] = {
    "music_volume", dsda_config_music_volume,
    dsda_config_int, 0, 15, { 8 }, NULL, NOT_STRICT, I_ResetMusicVolume
  },
  [dsda_config_mus_pause_opt] = {
    "mus_pause_opt", dsda_config_mus_pause_opt,
    dsda_config_int, 0, 2, { 1 }
  },
  [dsda_config_snd_channels] = {
    "snd_channels", dsda_config_snd_channels,
    dsda_config_int, 1, MAX_CHANNELS, { 32 }, NULL, NOT_STRICT, S_Init
  },
  [dsda_config_snd_midiplayer] = {
    "snd_midiplayer", dsda_config_snd_midiplayer,
    CONF_STRING("fluidsynth"), NULL, NOT_STRICT, M_ChangeMIDIPlayer
  },
  [dsda_config_snd_mididev] = {
    "snd_mididev", dsda_config_snd_mididev,
    CONF_STRING("")
  },
  [dsda_config_snd_soundfont] = {
    "snd_soundfont", dsda_config_snd_soundfont,
    CONF_STRING("")
  },
  [dsda_config_mus_fluidsynth_chorus] = {
    "mus_fluidsynth_chorus", dsda_config_mus_fluidsynth_chorus,
    CONF_BOOL(0)
  },
  [dsda_config_mus_fluidsynth_reverb] = {
    "mus_fluidsynth_reverb", dsda_config_mus_fluidsynth_reverb,
    CONF_BOOL(0)
  },
  [dsda_config_mus_fluidsynth_gain] = {
    "mus_fluidsynth_gain", dsda_config_mus_fluidsynth_gain,
    dsda_config_int, 0, 1000, { 50 }
  },
  [dsda_config_mus_fluidsynth_chorus_depth] = {
    "mus_fluidsynth_chorus_depth", dsda_config_mus_fluidsynth_chorus_depth,
    dsda_config_int, 0, 10000, { 500 }
  },
  [dsda_config_mus_fluidsynth_chorus_level] = {
    "mus_fluidsynth_chorus_level", dsda_config_mus_fluidsynth_chorus_level,
    dsda_config_int, 0, 1000, { 35 }
  },
  [dsda_config_mus_fluidsynth_reverb_damp] = {
    "mus_fluidsynth_reverb_damp", dsda_config_mus_fluidsynth_reverb_damp,
    dsda_config_int, 0, 1000, { 40 }
  },
  [dsda_config_mus_fluidsynth_reverb_level] = {
    "mus_fluidsynth_reverb_level", dsda_config_mus_fluidsynth_reverb_level,
    dsda_config_int, 0, 1000, { 15 }
  },
  [dsda_config_mus_fluidsynth_reverb_width] = {
    "mus_fluidsynth_reverb_width", dsda_config_mus_fluidsynth_reverb_width,
    dsda_config_int, 0, 10000, { 400 }
  },
  [dsda_config_mus_fluidsynth_reverb_room_size] = {
    "mus_fluidsynth_reverb_room_size", dsda_config_mus_fluidsynth_reverb_room_size,
    dsda_config_int, 0, 1000, { 60 }
  },
  [dsda_config_mus_opl_gain] = {
    "mus_opl_gain", dsda_config_mus_opl_gain,
    dsda_config_int, 0, 1000, { 50 }
  },
  [dsda_config_mus_opl_opl3mode] = {
    "mus_opl_opl3mode", dsda_config_mus_opl_opl3mode,
    CONF_BOOL(1)
  },
  [dsda_config_mus_portmidi_reset_type] = {
    "mus_portmidi_reset_type", dsda_config_mus_portmidi_reset_type,
    CONF_STRING("gm") // none, gs, gm, gm2, xg
  },
  [dsda_config_mus_portmidi_reset_delay] = {
    "mus_portmidi_reset_delay", dsda_config_mus_portmidi_reset_delay,
    dsda_config_int, 0, 2000, { 0 }
  },
  [dsda_config_mus_portmidi_filter_sysex] = {
    "mus_portmidi_filter_sysex", dsda_config_mus_portmidi_filter_sysex,
    dsda_config_int, 0, 1, { 1 }
  },
  [dsda_config_mus_portmidi_reverb_level] = {
    "mus_portmidi_reverb_level", dsda_config_mus_portmidi_reverb_level,
    dsda_config_int, -1, 127, { -1 }
  },
  [dsda_config_mus_portmidi_chorus_level] = {
    "mus_portmidi_chorus_level", dsda_config_mus_portmidi_chorus_level,
    dsda_config_int, -1, 127, { -1 }
  },
  [dsda_config_cap_soundcommand] = {
    "cap_soundcommand", dsda_config_cap_soundcommand,
    CONF_STRING("ffmpeg -f s16le -ar %s -ac 2 -i - -c:a libopus -y temp_a.nut")
  },
  [dsda_config_cap_videocommand] = {
    "cap_videocommand", dsda_config_cap_videocommand,
    CONF_STRING("ffmpeg -f rawvideo -pix_fmt rgb24 -r %r -s %wx%h -i - -c:v libx264 -y temp_v.nut")
  },
  [dsda_config_cap_muxcommand] = {
    "cap_muxcommand", dsda_config_cap_muxcommand,
    CONF_STRING("ffmpeg -i temp_v.nut -i temp_a.nut -c copy -y %f")
  },
  [dsda_config_cap_tempfile1] = {
    "cap_tempfile1", dsda_config_cap_tempfile1,
    CONF_STRING("temp_a.nut")
  },
  [dsda_config_cap_tempfile2] = {
    "cap_tempfile2", dsda_config_cap_tempfile2,
    CONF_STRING("temp_v.nut")
  },
  [dsda_config_cap_remove_tempfiles] = {
    "cap_remove_tempfiles", dsda_config_cap_remove_tempfiles,
    CONF_BOOL(1)
  },
  [dsda_config_cap_wipescreen] = {
    "cap_wipescreen", dsda_config_cap_wipescreen,
    CONF_BOOL(0)
  },
  [dsda_config_cap_fps] = {
    "cap_fps", dsda_config_cap_fps,
    dsda_config_int, 16, 300, { 60 }
  },
  [dsda_config_hudadd_crosshair_color] = {
    "hudadd_crosshair_color", dsda_config_hudadd_crosshair_color,
    CONF_CR(3)
  },
  [dsda_config_hudadd_crosshair_target_color] = {
    "hudadd_crosshair_target_color", dsda_config_hudadd_crosshair_target_color,
    CONF_CR(9), NULL, STRICT_INT(9)
  },
  [dsda_config_hud_displayed] = {
    "hud_displayed", dsda_config_hud_displayed,
    CONF_BOOL(0), NULL, CONF_FEATURE | NOT_STRICT, R_SetViewSize
  },
  [dsda_config_hudadd_secretarea] = {
    "hudadd_secretarea", dsda_config_hudadd_secretarea,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_hudadd_demoprogressbar] = {
    "hudadd_demoprogressbar", dsda_config_hudadd_demoprogressbar,
    CONF_BOOL(1)
  },
  [dsda_config_hudadd_crosshair_scale] = {
    "hudadd_crosshair_scale", dsda_config_hudadd_crosshair_scale,
    CONF_BOOL(0), NULL, NOT_STRICT, HU_InitCrosshair
  },
  [dsda_config_hudadd_crosshair_health] = {
    "hudadd_crosshair_health", dsda_config_hudadd_crosshair_health,
    CONF_BOOL(0), NULL, NOT_STRICT, HU_InitCrosshair
  },
  [dsda_config_hudadd_crosshair_target] = {
    "hudadd_crosshair_target", dsda_config_hudadd_crosshair_target,
    CONF_BOOL(0), NULL, STRICT_INT(0), HU_InitCrosshair
  },
  [dsda_config_hudadd_crosshair_lock_target] = {
    "hudadd_crosshair_lock_target", dsda_config_hudadd_crosshair_lock_target,
    CONF_BOOL(0), NULL, STRICT_INT(0), HU_InitCrosshair
  },
  [dsda_config_hudadd_crosshair] = {
    "hudadd_crosshair", dsda_config_hudadd_crosshair,
    dsda_config_int, 0, HU_CROSSHAIRS - 1, { 0 }, NULL, CONF_FEATURE | NOT_STRICT, HU_InitCrosshair
  },
  [dsda_config_hud_health_red] = {
    "hud_health_red", dsda_config_hud_health_red,
    dsda_config_int, 0, 200, { 25 }, NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_hud_health_yellow] = {
    "hud_health_yellow", dsda_config_hud_health_yellow,
    dsda_config_int, 0, 200, { 50 }, NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_hud_health_green] = {
    "hud_health_green", dsda_config_hud_health_green,
    dsda_config_int, 0, 200, { 100 }, NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_hud_ammo_red] = {
    "hud_ammo_red", dsda_config_hud_ammo_red,
    dsda_config_int, 0, 100, { 25 }, NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_hud_ammo_yellow] = {
    "hud_ammo_yellow", dsda_config_hud_ammo_yellow,
    dsda_config_int, 0, 100, { 50 }, NULL, NOT_STRICT, HU_InitThresholds
  },
  [dsda_config_cycle_ghost_colors] = {
    "dsda_cycle_ghost_colors", dsda_config_cycle_ghost_colors,
    CONF_BOOL(0)
  },
  [dsda_config_auto_key_frame_active] = {
    "dsda_auto_key_frame_active", dsda_config_auto_key_frame_active,
    CONF_BOOL(1), NULL, STRICT_INT(0), dsda_InitAutoKeyFrames
  },
  [dsda_config_auto_key_frame_interval] = {
    "dsda_auto_key_frame_interval", dsda_config_auto_key_frame_interval,
    dsda_config_int, 1, 600, { 1 }, NULL, STRICT_INT(1), dsda_InitAutoKeyFrames
  },
  [dsda_config_auto_key_frame_depth] = {
    "dsda_auto_key_frame_depth", dsda_config_auto_key_frame_depth,
    dsda_config_int, 0, 600, { 60 }, NULL, STRICT_INT(0), dsda_InitAutoKeyFrames
  },
  [dsda_config_auto_key_frame_timeout] = {
    "dsda_auto_key_frame_timeout", dsda_config_auto_key_frame_timeout,
    dsda_config_int, 0, 25, { 10 }, NULL, STRICT_INT(0), dsda_InitAutoKeyFrames
  },
  [dsda_config_auto_save] = {
    "dsda_config_auto_save", dsda_config_auto_save,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [dsda_config_ex_text_scale_x] = {
    "ex_text_scale_x", dsda_config_ex_text_scale_x,
    dsda_config_int, 0, 4000, { 0 }, NULL, NOT_STRICT, dsda_SetupStretchParams
  },
  [dsda_config_ex_text_ratio_y] = {
    "ex_text_ratio_y", dsda_config_ex_text_ratio_y,
    dsda_config_int, 0, 200, { 0 }, NULL, NOT_STRICT, dsda_SetupStretchParams
  },
  [dsda_config_wipe_at_full_speed] = {
    "dsda_wipe_at_full_speed", dsda_config_wipe_at_full_speed,
    CONF_BOOL(1), NULL, STRICT_INT(0)
  },
  [dsda_config_show_demo_attempts] = {
    "dsda_show_demo_attempts", dsda_config_show_demo_attempts,
    CONF_BOOL(1)
  },
  [dsda_config_hide_horns] = {
    "dsda_hide_horns", dsda_config_hide_horns,
    CONF_BOOL(0)
  },
  [dsda_config_hide_weapon] = {
    "dsda_hide_weapon", dsda_config_hide_weapon,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [dsda_config_organized_saves] = {
    "dsda_organized_saves", dsda_config_organized_saves,
    CONF_BOOL(1)
  },
  [dsda_config_command_history_size] = {
    "dsda_command_history_size", dsda_config_command_history_size,
    dsda_config_int, 1, 20, { 10 }, NULL, STRICT_INT(0), dsda_InitCommandHistory
  },
  [dsda_config_hide_empty_commands] = {
    "dsda_hide_empty_commands", dsda_config_hide_empty_commands,
    CONF_BOOL(1), NULL, STRICT_INT(0), dsda_InitCommandHistory
  },
  [dsda_config_skip_quit_prompt] = {
    "dsda_skip_quit_prompt", dsda_config_skip_quit_prompt,
    CONF_BOOL(0)
  },
  [dsda_config_show_split_data] = {
    "dsda_show_split_data", dsda_config_show_split_data,
    CONF_BOOL(1)
  },
  [dsda_config_player_name] = {
    "dsda_player_name", dsda_config_player_name,
    CONF_STRING("Anonymous")
  },
  [dsda_config_quickstart_cache_tics] = {
    "dsda_quickstart_cache_tics", dsda_config_quickstart_cache_tics,
    dsda_config_int, 0, 35, { 0 }, NULL, NOT_STRICT, dsda_InitQuickstartCache
  },
  [dsda_config_death_use_action] = {
    "dsda_death_use_action", dsda_config_death_use_action,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_allow_jumping] = {
    "dsda_allow_jumping", dsda_config_allow_jumping,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_ResetAirControl
  },
  [dsda_config_disable_horiz_autoaim] = {
    "dsda_disable_horiz_autoaim", dsda_config_disable_horiz_autoaim,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [dsda_config_limit_removing] = {
    "dsda_limit_removing", dsda_config_limit_removing,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_AlterGameFlags
  },
  [dsda_config_always_pistol_start] = {
    "dsda_always_pistol_start", dsda_config_always_pistol_start,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_RefreshAlwaysPistolStart
  },
  [dsda_config_pistol_start] = {
    "dsda_pistol_start", dsda_config_pistol_start,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_RefreshPistolStart
  },
  [dsda_config_respawn_monsters] = {
    "dsda_respawn_monsters", dsda_config_respawn_monsters,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_AlterGameFlags
  },
  [dsda_config_fast_monsters] = {
    "dsda_fast_monsters", dsda_config_fast_monsters,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_AlterGameFlags
  },
  [dsda_config_no_monsters] = {
    "dsda_no_monsters", dsda_config_no_monsters,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_AlterGameFlags
  },
  [dsda_config_coop_spawns] = {
    "dsda_coop_spawns", dsda_config_coop_spawns,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_AlterGameFlags
  },
  [dsda_config_skill_spawn_filter] = {
    "dsda_skill_spawn_filter", dsda_config_skill_spawn_filter,
    dsda_config_int, 0, 2, { 1 },
  },
  [dsda_config_skill_coop_spawns] = {
    "dsda_skill_coop_spawns", dsda_config_skill_coop_spawns,
    CONF_BOOL(0), &cskill_coop_spawns, NOT_STRICT
  },
  [dsda_config_skill_ammo_factor] = {
    "dsda_skill_ammo_factor", dsda_config_skill_ammo_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_ammo_factor
  },
  [dsda_config_skill_damage_factor] = {
    "dsda_skill_damage_factor", dsda_config_skill_damage_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_damage_factor
  },
  [dsda_config_skill_armor_factor] = {
    "dsda_skill_armor_factor", dsda_config_skill_armor_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_armor_factor
  },
  [dsda_config_skill_health_factor] = {
    "dsda_skill_health_factor", dsda_config_skill_health_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_health_factor
  },
  [dsda_config_skill_monster_health_factor] = {
    "dsda_skill_monster_health_factor", dsda_config_skill_monster_health_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_monster_hp_factor
  },
  [dsda_config_skill_friend_health_factor] = {
    "dsda_skill_friend_health_factor", dsda_config_skill_friend_health_factor,
    dsda_config_int, 0, 4, { 1 }, &cskill_friend_hp_factor
  },
  [dsda_config_skill_respawn_monsters] = {
    "dsda_skill_respawn_monsters", dsda_config_skill_respawn_monsters,
    CONF_BOOL(0), &cskill_respawn, NOT_STRICT
  },
  [dsda_config_skill_respawn_time] = {
    "dsda_skill_respawn_time", dsda_config_skill_respawn_time,
    dsda_config_int, 1, 32, { 12 }, &cskill_respawn_time
  },
  [dsda_config_skill_fast_monsters] = {
    "dsda_skill_fast_monsters", dsda_config_skill_fast_monsters,
    CONF_BOOL(0), &cskill_fast_monsters, NOT_STRICT
  },
  [dsda_config_skill_aggressive_monsters] = {
    "dsda_skill_aggressive_monsters", dsda_config_skill_aggressive_monsters,
    CONF_BOOL(0), &cskill_aggressive, NOT_STRICT
  },
  [dsda_config_skill_no_monsters] = {
    "dsda_skill_no_monsters", dsda_config_skill_no_monsters,
    CONF_BOOL(0), &cskill_no_monsters, NOT_STRICT
  },
  [dsda_config_skill_easy_brain] = {
    "dsda_skill_easy_brain", dsda_config_skill_easy_brain,
    CONF_BOOL(0), &cskill_easy_brain, NOT_STRICT
  },
  [dsda_config_skill_auto_use_health] = {
    "dsda_skill_auto_use_health", dsda_config_skill_auto_use_health,
    CONF_BOOL(0), &cskill_auto_use_hp, NOT_STRICT
  },
  [dsda_config_skill_no_pain] = {
    "dsda_skill_no_pain", dsda_config_skill_no_pain,
    CONF_BOOL(0), &cskill_no_pain, NOT_STRICT
  },
  [dsda_config_skill_easy_key] = {
    "dsda_skill_easy_key", dsda_config_skill_easy_key,
    CONF_BOOL(0), &cskill_easy_key, NOT_STRICT
  },
  [dsda_config_parallel_sfx_active] = {
    "dsda_parallel_sfx_active", dsda_config_parallel_sfx_active,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_InitParallelSFXFilter
  },
  [dsda_config_parallel_sfx_limit] = {
    "dsda_parallel_sfx_limit", dsda_config_parallel_sfx_limit,
    dsda_config_int, 1, 32, { 32 }, NULL, NOT_STRICT, dsda_InitParallelSFXFilter
  },
  [dsda_config_parallel_sfx_window] = {
    "dsda_parallel_sfx_window", dsda_config_parallel_sfx_window,
    dsda_config_int, 1, 32, { 1 }, NULL, NOT_STRICT, dsda_InitParallelSFXFilter
  },
  [dsda_config_movement_toggle_sfx] = {
    "dsda_movement_toggle_sfx", dsda_config_movement_toggle_sfx,
    CONF_BOOL(0)
  },
  [dsda_config_quicksave_sfx] = {
    "dsda_quicksave_sfx", dsda_config_quicksave_sfx,
    CONF_BOOL(1)
  },
  [dsda_config_switch_when_ammo_runs_out] = {
    "dsda_switch_when_ammo_runs_out", dsda_config_switch_when_ammo_runs_out,
    CONF_BOOL(1)
  },
  [dsda_config_switch_weapon_on_pickup] = {
    "dsda_switch_weapon_on_pickup", dsda_config_switch_weapon_on_pickup,
    CONF_BOOL(1), NULL, STRICT_INT(1)
  },
  [dsda_config_ssg_on_arms] = {
    "dsda_ssg_on_arms", dsda_config_ssg_on_arms,
    CONF_BOOL(1)
  },
  [dsda_config_viewbob] = {
    "dsda_viewbob_pct", dsda_config_viewbob,
    dsda_config_int, 0, 4, { 4 }
  },
  [dsda_config_weaponbob] = {
    "dsda_weaponbob_pct", dsda_config_weaponbob,
    dsda_config_int, 0, 4, { 4 }
  },
  [dsda_config_quake_intensity] = {
    "dsda_quake_intensity", dsda_config_quake_intensity,
    dsda_config_int, 0, 100, { 100 }
  },
  [dsda_config_fuzzmode] = {
    "dsda_fuzzmode", dsda_config_fuzzmode,
    dsda_config_int, 0, 2, { 0 }, NULL, STRICT_INT(0)
  },
  [dsda_config_multiple_area_maps] = {
    "dsda_multiple_area_maps", dsda_config_multiple_area_maps,
    CONF_BOOL(1), NULL, STRICT_INT(0)
  },
  [dsda_config_demo_end_quit] = {
    "dsda_demo_end_quit", dsda_config_demo_end_quit,
    CONF_BOOL(1)
  },
  [dsda_config_map_blinking_locks] = {
    "map_blinking_locks", dsda_config_map_blinking_locks,
    CONF_BOOL(0), NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_secret_after] = {
    "map_secret_after", dsda_config_map_secret_after,
    CONF_BOOL(0), NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_coordinates] = {
    "map_coordinates", dsda_config_map_coordinates,
    CONF_BOOL(1), NULL, STRICT_INT(0), dsda_RefreshMapCoordinates
  },
  [dsda_config_map_totals] = {
    "map_totals", dsda_config_map_totals,
    CONF_BOOL(1), NULL, NOT_STRICT, dsda_RefreshMapTotals
  },
  [dsda_config_map_time] = {
    "map_time", dsda_config_map_time,
    CONF_BOOL(1), NULL, NOT_STRICT, dsda_RefreshMapTime
  },
  [dsda_config_map_title] = {
    "map_title", dsda_config_map_title,
    CONF_BOOL(1), NULL, NOT_STRICT, dsda_RefreshMapTitle
  },
  [dsda_config_map_title_author_cycle] = {
    "map_title_author_cycle", dsda_config_map_title_author_cycle,
    CONF_BOOL(1), NULL, NOT_STRICT
  },
  [dsda_config_map_trail] = {
    "map_trail", dsda_config_map_trail,
    CONF_BOOL(0), NULL, STRICT_INT(0), AM_initPlayerTrail
  },
  [dsda_config_map_trail_collisions] = {
    "map_trail_collisions", dsda_config_map_trail_collisions,
    CONF_BOOL(0), NULL, STRICT_INT(0), AM_initPlayerTrail
  },
  [dsda_config_map_trail_size] = {
    "map_trail_size", dsda_config_map_trail_size,
    dsda_config_int, 0, 350, { 105 }, NULL, STRICT_INT(0), AM_initPlayerTrail
  },
  [dsda_config_automap_overlay] = {
    "automap_overlay", dsda_config_automap_overlay,
    dsda_config_int, 0, 2, { 0 }, &automap_overlay
  },
  [dsda_config_automap_rotate] = {
    "automap_rotate", dsda_config_automap_rotate,
    CONF_BOOL(0), &automap_rotate
  },
  [dsda_config_automap_follow] = {
    "automap_follow", dsda_config_automap_follow,
    CONF_BOOL(1), &automap_follow
  },
  [dsda_config_automap_grid] = {
    "automap_grid", dsda_config_automap_grid,
    CONF_BOOL(0), &automap_grid
  },
  [dsda_config_map_grid_size] = {
    "map_grid_size", dsda_config_map_grid_size,
    dsda_config_int, 8, 256, { 128 }, NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_pan_speed] = {
    "map_pan_speed", dsda_config_map_pan_speed,
    dsda_config_int, 1, 32, { 16 }, NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_scroll_speed] = {
    "map_scroll_speed", dsda_config_map_scroll_speed,
    dsda_config_int, 1, 32, { 32 }, NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_wheel_zoom] = {
    "map_wheel_zoom", dsda_config_map_wheel_zoom,
    CONF_BOOL(1), NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_use_multisamling] = {
    "map_use_multisampling", dsda_config_map_use_multisamling,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeMapMultisamling
  },
  [dsda_config_map_textured] = {
    "map_textured", dsda_config_map_textured,
    CONF_BOOL(1), NULL, STRICT_INT(0), M_ChangeMapTextured
  },
  [dsda_config_map_textured_trans] = {
    "map_textured_trans", dsda_config_map_textured_trans,
    dsda_config_int, 0, 100, { 100 }, NULL, STRICT_INT(0), gld_ResetAutomapTransparency
  },
  [dsda_config_map_textured_overlay_trans] = {
    "map_textured_overlay_trans", dsda_config_map_textured_overlay_trans,
    dsda_config_int, 0, 100, { 66 }, NULL, STRICT_INT(0), gld_ResetAutomapTransparency
  },
  [dsda_config_map_lines_overlay_trans] = {
    "map_lines_overlay_trans", dsda_config_map_lines_overlay_trans,
    dsda_config_int, 0, 100, { 100 }, NULL, NOT_STRICT, gld_ResetAutomapTransparency
  },
  [dsda_config_map_things_appearance] = {
    "map_things_appearance", dsda_config_map_things_appearance,
    dsda_config_int, 0, map_things_appearance_max - 1, { map_things_appearance_max - 1 },
    NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_map_things_nice] = {
    "dsda_map_things_nice", dsda_config_map_things_nice,
    dsda_config_int, 0, 1, { 1 }, NULL, NOT_STRICT, AM_InitParams
  },
  [dsda_config_videomode] = {
    "videomode", dsda_config_videomode,
    CONF_STRING("Software"), NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_screen_resolution] = {
    "screen_resolution", dsda_config_screen_resolution,
    CONF_STRING("640x480"), NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_custom_resolution] = {
    "custom_resolution", dsda_config_custom_resolution,
    CONF_STRING("")
  },
  [dsda_config_use_fullscreen] = {
    "use_fullscreen", dsda_config_use_fullscreen,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeFullScreen
  },
  [dsda_config_exclusive_fullscreen] = {
    "exclusive_fullscreen", dsda_config_exclusive_fullscreen,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_render_vsync] = {
    "render_vsync", dsda_config_render_vsync,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_uncapped_framerate] = {
    "uncapped_framerate", dsda_config_uncapped_framerate,
    CONF_BOOL(1), NULL, NOT_STRICT, M_ChangeUncappedFrameRate
  },
  [dsda_config_fps_limit] = {
    "dsda_fps_limit", dsda_config_fps_limit,
    dsda_config_int, 0, 1000, { 0 }
  },
  [dsda_config_background_fps_limit] = {
    "dsda_background_fps_limit", dsda_config_background_fps_limit,
    dsda_config_int, 0, 1000, { 35 }
  },
  [dsda_config_usegamma] = {
    "usegamma", dsda_config_usegamma,
    dsda_config_int, 0, 4, { 0 }, &usegamma, NOT_STRICT, M_ChangeApplyPalette
  },
  [dsda_config_screenblocks] = {
    "screenblocks", dsda_config_screenblocks,
    dsda_config_int, 3, 11, { 10 }, NULL, CONF_FEATURE | NOT_STRICT, R_SetViewSize
  },
  [dsda_config_sdl_video_window_pos] = {
    "sdl_video_window_pos", dsda_config_sdl_video_window_pos,
    CONF_STRING("")
  },
  [dsda_config_sdl_video_display_index] = {
    "sdl_video_display_index", dsda_config_sdl_video_display_index,
    dsda_config_int, 0, 10, { 0 }, NULL, NOT_STRICT
  },
  [dsda_config_palette_ondamage] = {
    "palette_ondamage", dsda_config_palette_ondamage,
    CONF_BOOL(1), NULL, STRICT_INT(1), M_ChangeApplyPalette
  },
  [dsda_config_palette_onbonus] = {
    "palette_onbonus", dsda_config_palette_onbonus,
    CONF_BOOL(1), NULL, STRICT_INT(1), M_ChangeApplyPalette
  },
  [dsda_config_palette_onpowers] = {
    "palette_onpowers", dsda_config_palette_onpowers,
    CONF_BOOL(1), NULL, STRICT_INT(1), M_ChangeApplyPalette
  },
  [dsda_config_enhanced_liteamp] = {
    "enhanced_liteamp", dsda_config_enhanced_liteamp,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [dsda_config_colored_borderbox] = {
    "dsda_colored_borderbox", dsda_config_colored_borderbox,
    CONF_BOOL(1)
  },
  [dsda_config_render_wipescreen] = {
    "render_wipescreen", dsda_config_render_wipescreen,
    CONF_BOOL(1), NULL, STRICT_INT(1)
  },
  [dsda_config_render_screen_multiply] = {
    "render_screen_multiply", dsda_config_render_screen_multiply,
    dsda_config_int, 1, 5, { 1 }, NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_integer_scaling] = {
    "integer_scaling", dsda_config_integer_scaling,
    CONF_BOOL(0), NULL, NOT_STRICT, M_ChangeVideoMode
  },
  [dsda_config_render_aspect] = {
    "render_aspect", dsda_config_render_aspect,
    dsda_config_int, 0, 4, { 0 }, NULL, NOT_STRICT, M_ChangeAspectRatio
  },
  [dsda_config_render_doom_lightmaps] = {
    "render_doom_lightmaps", dsda_config_render_doom_lightmaps,
    CONF_BOOL(0)
  },
  [dsda_config_fake_contrast_mode] = {
    "fake_contrast_mode", dsda_config_fake_contrast_mode,
    dsda_config_int, FAKE_CONTRAST_MODE_OFF, FAKE_CONTRAST_MODE_SMOOTH,
    { FAKE_CONTRAST_MODE_ON }, (int*) &fake_contrast_mode
  },
  [dsda_config_render_stretch_hud] = {
    "render_stretch_hud", dsda_config_render_stretch_hud,
    dsda_config_int, patch_stretch_not_adjusted, patch_stretch_fit_to_width, { patch_stretch_doom_format },
    NULL, NOT_STRICT, M_ChangeStretch
  },
  [dsda_config_render_patches_scalex] = {
    "render_patches_scalex", dsda_config_render_patches_scalex,
    dsda_config_int, 0, 16, { 0 }
  },
  [dsda_config_render_patches_scaley] = {
    "render_patches_scaley", dsda_config_render_patches_scaley,
    dsda_config_int, 0, 16, { 0 }
  },
  [dsda_config_render_stretchsky] = {
    "render_stretchsky", dsda_config_render_stretchsky,
    CONF_BOOL(1)
  },
  [dsda_config_render_linearsky] = {
    "render_linearsky", dsda_config_render_linearsky,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_RefreshLinearSky
  },
  [dsda_config_aspect_ratio_correction] = {
    "aspect_ratio_correction", dsda_config_aspect_ratio_correction,
    CONF_BOOL(1), NULL, NOT_STRICT
  },
  [dsda_config_gl_fade_mode] = {
    "gl_fade_mode", dsda_config_gl_fade_mode,
    dsda_config_int, 0, 1, { 0 }
  },
  [dsda_config_tran_filter_pct] = {
    "tran_filter_pct", dsda_config_tran_filter_pct,
    dsda_config_int, 0, 100, { 66 }, NULL, STRICT_INT(66), P_UpdateTranMap
  },
  [dsda_config_translucent_sprites] = {
    "boom_translucent_sprites", dsda_config_translucent_sprites,
    dsda_config_int, 0, 2, { 1 }, NULL, STRICT_INT(1), deh_changeCompTranslucency
  },
  [dsda_config_translucent_missiles] = {
    "boom_translucent_missiles", dsda_config_translucent_missiles,
    CONF_BOOL(1), NULL, STRICT_INT(1), deh_changeCompTranslucency
  },
  [dsda_config_translucent_powerups] = {
    "boom_translucent_powerups", dsda_config_translucent_powerups,
    CONF_BOOL(1), NULL, STRICT_INT(1), deh_changeCompTranslucency
  },
  [dsda_config_translucent_effects] = {
    "boom_translucent_effects", dsda_config_translucent_effects,
    CONF_BOOL(1), NULL, STRICT_INT(1), deh_changeCompTranslucency
  },
  [dsda_config_translucent_ghosts] = {
    "translucent_ghosts", dsda_config_translucent_ghosts,
    CONF_BOOL(0), NULL, STRICT_INT(0)
  },
  [nyan_config_item_bonus_flash] = {
    "nyan_item_bonus_flash", nyan_config_item_bonus_flash,
    CONF_BOOL(0), NULL, NOT_STRICT, deh_changeBonusFlash
  },
  [nyan_config_colored_blood] = {
    "nyan_colored_blood", nyan_config_colored_blood,
    dsda_config_int, 0, 2, { 0 }, NULL, NOT_STRICT, deh_changeColoredBlood
  },
  [nyan_config_loading_disk] = {
    "nyan_loading_disk", nyan_config_loading_disk,
    dsda_config_int, 0, 2, { 0 }, NULL, NOT_STRICT
  },
  [dsda_config_show_alive_monsters] = { // never persisted
    "show_alive_monsters", dsda_config_show_alive_monsters,
    dsda_config_int, 0, 2, { 0 }, NULL, STRICT_INT(0)
  },
  [dsda_config_left_analog_deadzone] = {
    "left_analog_deadzone", dsda_config_left_analog_deadzone,
    dsda_config_int, 0, 16384, { 6556 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_right_analog_deadzone] = {
    "right_analog_deadzone", dsda_config_right_analog_deadzone,
    dsda_config_int, 0, 16384, { 6556 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_left_trigger_deadzone] = {
    "left_trigger_deadzone", dsda_config_left_trigger_deadzone,
    dsda_config_int, 0, 16384, { 6556 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_right_trigger_deadzone] = {
    "right_trigger_deadzone", dsda_config_right_trigger_deadzone,
    dsda_config_int, 0, 16384, { 6556 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_left_analog_sensitivity_x] = {
    "left_analog_sensitivity_x", dsda_config_left_analog_sensitivity_x,
    dsda_config_int, 0, 16384, { 100 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_left_analog_sensitivity_y] = {
    "left_analog_sensitivity_y", dsda_config_left_analog_sensitivity_y,
    dsda_config_int, 0, 16384, { 100 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_right_analog_sensitivity_x] = {
    "right_analog_sensitivity_x", dsda_config_right_analog_sensitivity_x,
    dsda_config_int, 0, 16384, { 1536 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_right_analog_sensitivity_y] = {
    "right_analog_sensitivity_y", dsda_config_right_analog_sensitivity_y,
    dsda_config_int, 0, 16384, { 768 }, NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_analog_look_acceleration] = {
    "analog_look_acceleration", dsda_config_analog_look_acceleration,
    dsda_config_int, 0, INT_MAX, { 0 }, NULL, NOT_STRICT, AccelChanging
  },
  [dsda_config_swap_analogs] = {
    "swap_analogs", dsda_config_swap_analogs,
    CONF_BOOL(0), NULL, NOT_STRICT, dsda_InitGameControllerParameters
  },
  [dsda_config_invert_analog_look] = {
    "invert_analog_look", dsda_config_invert_analog_look,
    CONF_BOOL(0),
  },
  [nyan_config_show_endoom] = {
    "show_endoom", nyan_config_show_endoom,
    dsda_config_int, 0, 2, { 1 }
  },
  [nyan_config_type_endoom] = {
    "type_endoom", nyan_config_type_endoom,
    dsda_config_int, 0, 1, { 1 }
  },
  [dsda_config_ansi_endoom] = {
    "ansi_endoom", dsda_config_ansi_endoom,
    dsda_config_int, 1, 2, { 1 }
  },
  [dsda_config_quit_sounds] = {
    "quit_sounds", dsda_config_quit_sounds,
    CONF_BOOL(0),
  },
  [dsda_config_announce_map] = {
    "announce_map", dsda_config_announce_map,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_extra_level_brightness] = {
    "extra_level_brightness", dsda_config_extra_level_brightness,
    dsda_config_int, 0, 4, { 0 },  &extra_brightness, STRICT_INT(0)
  },
  [dsda_config_playback_mouse_controls] = {
    "playback_mouse_controls", dsda_config_playback_mouse_controls,
    CONF_BOOL(1)
  },
};

static void dsda_PersistIntConfig(dsda_config_t* conf) {
  conf->persistent_value.v_int = conf->transient_value.v_int;
}

static void dsda_PersistStringConfig(dsda_config_t* conf) {
  if (conf->persistent_value.v_string)
    Z_Free(conf->persistent_value.v_string);

  conf->persistent_value.v_string = Z_Strdup(conf->transient_value.v_string);
}

static void dsda_ConstrainIntConfig(dsda_config_t* conf) {
  if (conf->transient_value.v_int > conf->upper_limit)
    conf->transient_value.v_int = conf->upper_limit;
  else if (conf->transient_value.v_int < conf->lower_limit) {
    if (conf->transient_value.v_int == -1)
      conf->transient_value.v_int = conf->default_value.v_int;
    else
      conf->transient_value.v_int = conf->lower_limit;
  }

  if (conf->flags & CONF_EVEN && (conf->transient_value.v_int % 2))
    conf->transient_value.v_int = conf->default_value.v_int;
}

static void dsda_PropagateIntConfig(dsda_config_t* conf) {
  if (conf->int_binding)
    *conf->int_binding = dsda_IntConfig(conf->id);
}

// No side effects
static void dsda_InitIntConfig(dsda_config_t* conf, int value, dboolean persist) {
  conf->transient_value.v_int = value;

  dsda_ConstrainIntConfig(conf);
  if (persist)
    dsda_PersistIntConfig(conf);
  dsda_PropagateIntConfig(conf);
}

// No side effects
static void dsda_InitStringConfig(dsda_config_t* conf, const char* value, dboolean persist) {
  if (conf->transient_value.v_string)
    Z_Free(conf->transient_value.v_string);

  conf->transient_value.v_string = Z_Strdup(value);
  if (persist)
    dsda_PersistStringConfig(conf);
}

// No side effects
void dsda_RevertIntConfig(dsda_config_identifier_t id) {
  dsda_config[id].transient_value.v_int = dsda_config[id].persistent_value.v_int;
}

int dsda_MaxConfigLength(void) {
  int length = 0;

  int i;

  for (i = 1; i < dsda_config_count; ++i) {
    dsda_config_t* conf;

    conf = &dsda_config[i];

    if (strlen(conf->name) > length)
      length = strlen(conf->name);
  }

  return length;
}

void dsda_InitConfig(void) {
  int i;

  for (i = 1; i < dsda_config_count; ++i) {
    dsda_config_t* conf;

    conf = &dsda_config[i];

    if (conf->type == dsda_config_int)
      dsda_InitIntConfig(conf, conf->default_value.v_int, true);
    else if (conf->type == dsda_config_string)
      dsda_InitStringConfig(conf, conf->default_value.v_string, true);
  }
}

dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param) {
  int id;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int && !string_param)
      dsda_InitIntConfig(conf, int_param, true);
    else if (conf->type == dsda_config_string && string_param)
      dsda_InitStringConfig(conf, string_param, true);

    return true;
  }

  return false;
}

void dsda_WriteConfig(dsda_config_identifier_t id, int key_length, FILE* file) {
  dsda_config_t* conf;

  conf = &dsda_config[id];

  if (conf->type == dsda_config_int)
    fprintf(file, "%-*s %i\n", key_length, conf->name, conf->persistent_value.v_int);
  else if (conf->type == dsda_config_string)
    fprintf(file, "%-*s \"%s\"\n", key_length, conf->name, conf->persistent_value.v_string);
}

static void dsda_ParseConfigArg(int arg_id, dboolean persist) {
  dsda_arg_t* arg;

  arg = dsda_Arg(arg_id);
  if (arg->found) {
    int i;

    for (i = 0; i < arg->count; ++i) {
      int id;
      dsda_config_t* conf;
      char* pair;
      char** key_value;

      pair = Z_Strdup(arg->value.v_string_array[i]);
      key_value = dsda_SplitString(pair, "=");
      if (!key_value[0] || !key_value[1])
        I_Error("Invalid config variable assignment \"%s\" (use key=value)", pair);

      id = dsda_ConfigIDByName(key_value[0]);
      if (!id)
        I_Error("Unknown config variable \"%s\"", key_value[0]);

      conf = &dsda_config[id];
      if (conf->type == dsda_config_int) {
        int value;

        if (sscanf(key_value[1], "%i", &value) != 1)
          I_Error("Config variable \"%s\" requires an integer value", key_value[0]);

        dsda_InitIntConfig(conf, value, persist);
      }
      else {
        dsda_InitStringConfig(conf, key_value[1], persist);
      }

      Z_Free(pair);
      Z_Free(key_value);
    }
  }
}

void dsda_ApplyAdHocConfiguration(void) {
  dsda_arg_t* arg;

  dsda_ParseConfigArg(dsda_arg_update, true);
  dsda_ParseConfigArg(dsda_arg_assign, false);

  arg = dsda_Arg(dsda_arg_game_speed);
  if (arg->found)
    dsda_ReadConfig("game_speed", NULL, arg->value.v_int);
}

int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, !dsda_config[id].transient_value.v_int, persist);
}

int dsda_IncrementIntConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int + 1, persist);
}

int dsda_DecrementIntConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int - 1, persist);
}

int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist) {
  int value;

  value = dsda_config[id].transient_value.v_int + 1;

  if (value > dsda_config[id].upper_limit)
    value = dsda_config[id].lower_limit;

  return dsda_UpdateIntConfig(id, value, persist);
}

int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist) {
  dsda_config[id].transient_value.v_int = value;

  dsda_ConstrainIntConfig(&dsda_config[id]);

  if (persist)
    dsda_PersistIntConfig(&dsda_config[id]);

  dsda_PropagateIntConfig(&dsda_config[id]);

  if (dsda_config[id].onUpdate)
    dsda_config[id].onUpdate();

  if (dsda_config[id].flags & CONF_FEATURE)
    dsda_TrackConfigFeatures();

  return dsda_IntConfig(id);
}

const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist) {
  if (dsda_config[id].transient_value.v_string)
    Z_Free(dsda_config[id].transient_value.v_string);

  dsda_config[id].transient_value.v_string = Z_Strdup(value);

  if (persist)
    dsda_PersistStringConfig(&dsda_config[id]);

  if (dsda_config[id].onUpdate)
    dsda_config[id].onUpdate();

  return dsda_StringConfig(id);
}

// No callbacks, to avoid recursion cases
const char* dsda_HackStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist) {
  if (dsda_config[id].transient_value.v_string)
    Z_Free(dsda_config[id].transient_value.v_string);

  dsda_config[id].transient_value.v_string = Z_Strdup(value);

  if (persist)
    dsda_PersistStringConfig(&dsda_config[id]);

  return dsda_StringConfig(id);
}

int dsda_IntConfig(dsda_config_identifier_t id) {
  dboolean dsda_StrictMode(void);

  if (dsda_config[id].flags & CONF_STRICT && dsda_StrictMode())
    return dsda_config[id].strict_value;

  return dsda_config[id].transient_value.v_int;
}

dboolean dsda_IsStrictConfig(dsda_config_identifier_t id) {
  return dsda_config[id].flags & CONF_STRICT;
}

int dsda_TransientIntConfig(dsda_config_identifier_t id) {
  return dsda_config[id].transient_value.v_int;
}

int dsda_LowerLimitConfig(dsda_config_identifier_t id) {
  return dsda_config[id].lower_limit;
}

int dsda_UpperLimitConfig(dsda_config_identifier_t id) {
  return dsda_config[id].upper_limit;
}

const char* dsda_StringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].transient_value.v_string;
}

char* dsda_ConfigSummary(const char* name) {
  int id;
  char* summary = NULL;
  size_t length;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int) {
      length = snprintf(NULL, 0,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
    }
    else if (conf->type == dsda_config_string) {
      length = snprintf(NULL, 0,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
    }

    return summary;
  }

  return NULL;
}

int dsda_ConfigIDByName(const char* name) {
  int i;

  for (i = 1; i < dsda_config_count; ++i)
    if (!strcmp(name, dsda_config[i].name))
      return i;

  return 0;
}

dsda_config_type_t dsda_ConfigType(dsda_config_identifier_t id) {
  return dsda_config[id].type;
}
