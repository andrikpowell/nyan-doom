//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Settings
//

#include <string.h>

#include "doomstat.h"
#include "m_menu.h"
#include "e6y.h"
#include "r_things.h"
#include "w_wad.h"
#include "g_game.h"
#include "gl_struct.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_video.h"

#include "dsda/args.h"
#include "dsda/build.h"
#include "dsda/configuration.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/key_frame.h"
#include "dsda/map_format.h"
#include "dsda/utility.h"
#include "dsda/skip.h"

#include "settings.h"

int dsda_skip_next_wipe;

void dsda_InitSettings(void) {
  void G_UpdateMouseSensitivity(void);
  void dsda_InitQuickstartCache(void);
  void dsda_InitParallelSFXFilter(void);
  void gld_ResetAutomapTransparency(void);

  dsda_UpdateStrictMode();
  G_UpdateMouseSensitivity();
  dsda_InitQuickstartCache();
  dsda_InitParallelSFXFilter();
  gld_ResetAutomapTransparency();
}

static int dsda_ParseGameversLump(void) {
    int num = W_CheckNumForName("GAMEVERS");

    if (num != LUMP_NOT_FOUND)
    {
      int count;
      char* lump;
      char** lines;
      char comp[9] = { 0 };
      char limit[9] = { 0 };

      lump = W_ReadLumpToString(num);
      lines = dsda_SplitString(lump, "\n\r");

      count = sscanf(lines[0], "%8s %8s", comp, limit);
      if (count > 2)
          I_Error("Too many GAMEVERS arguments!");

      if (!strcasecmp("1.2", comp))
          complvl = 0;
      else if (!strcasecmp("1.666", comp))
          complvl = 1;
      else if (!strcasecmp("1.9", comp))
          complvl = 2;
      else if (!strcasecmp("ultimate", comp))
          complvl = 3;
      else if (!strcasecmp("final", comp))
          complvl = 4;
      
      if (!strcasecmp("nolimits", comp)  || !strcasecmp("limit", comp) ||
          !strcasecmp("nolimits", limit) || !strcasecmp("limit", limit))
          limitremoving_lmp = true;
    }

    if (complvl != -1)
      return true;

    return false;
}

static int dsda_WadCompatibilityLevel(void) {
  static int last_numwadfiles = -1;

  // This might be called before all wads are loaded
  if (numwadfiles != last_numwadfiles) {
      int num, gamever = false;
      const char *lumps, *limit;

      last_numwadfiles = numwadfiles;
      num = W_CheckNumForName("COMPLVL");

      if (num != LUMP_NOT_FOUND) {
          int length;
          const char* data;

          length = W_LumpLength(num);
          data = W_LumpByNum(num);

          if (length == 7 && !strncasecmp("vanilla", data, 7)) {
            gamever = dsda_ParseGameversLump();

            if (complvl == -1) {
                if (gamemode == commercial)
                      if (gamemission == pack_plut || gamemission == pack_tnt)
                          complvl = 4;
                      else
                          complvl = 2;
                  else
                      complvl = 3;
            }
          }
          else if (length == 4 && !strncasecmp("boom", data, 4))
              complvl = 9;
          else if (length == 3 && !strncasecmp("mbf", data, 3))
              complvl = 11;
          else if (length == 5 && !strncasecmp("mbf21", data, 5))
              complvl = 21;

          lumps = (gamever ? "COMPLVL and GAMEVERS" : "COMPLVL");
          limit = (limitremoving_lmp ? " (limit-removing)" : "");

          lprintf(LO_INFO, "Detected %s lump: %i%s\n", lumps, complvl, limit);
      }
}

  return complvl;
}

int dsda_CompatibilityLevel(void) {
  int level;
  dsda_arg_t* complevel_arg;
  dsda_arg_t* lr_arg;

  if (raven) return doom_12_compatibility;

  if (doom_v11) return doom_12_compatibility;

  if (map_format.zdoom) return mbf21_compatibility;

  complevel_arg = dsda_Arg(dsda_arg_complevel);

  // Set Limit-Removing from arg
  lr_arg = dsda_Arg(dsda_arg_limitremoving);
  if ((lr_arg->count || limitremoving_arg) &&
      (complevel_arg->value.v_int < boom_compatibility_compatibility))
  {
    dsda_UpdateIntConfig(dsda_config_limit_removing, 1, true);
    limitremoving = true;
  }

  if (complevel_arg->count)
    return complevel_arg->value.v_int;

  level = dsda_WadCompatibilityLevel();

  if (level >= 0)
    return level;

  return UNSPECIFIED_COMPLEVEL;
}

void dsda_SetTas(dboolean t) {
  dsda_UpdateIntConfig(dsda_config_strict_mode, !t, true);
}

int dsda_ViewBob(void) {
  return dsda_IntConfig(dsda_config_viewbob);
}

int dsda_WeaponBob(void) {
  return dsda_IntConfig(dsda_config_weaponbob);
}

dboolean dsda_CameraMode(void) {
  return (players[consoleplayer].cheats & CF_CAMERA);
}

dboolean dsda_ShowMessages(void) {
  return dsda_IntConfig(dsda_config_show_messages);
}

dboolean dsda_AutoRun(void) {
  return dsda_IntConfig(dsda_config_autorun);
}

dboolean dsda_MouseLook(void) {
  return dsda_IntConfig(dsda_config_freelook);
}

dboolean dsda_VertMouse(void) {
  return dsda_IntConfig(dsda_config_vertmouse);
}

dboolean dsda_StrictMode(void) {
  return dsda_IntConfig(dsda_config_strict_mode) && demorecording;
}

dboolean dsda_MuteSfx(void) {
  return dsda_IntConfig(dsda_config_mute_sfx) ||
         (!I_WindowFocused() && dsda_IntConfig(dsda_config_mute_unfocused_window));
}

dboolean dsda_MuteMusic(void) {
  return dsda_IntConfig(dsda_config_mute_music) ||
         (!I_WindowFocused() && dsda_IntConfig(dsda_config_mute_unfocused_window));
}

dboolean dsda_ProcessCheatCodes(void) {
  return dsda_IntConfig(dsda_config_cheat_codes);
}

dboolean dsda_CycleGhostColors(void) {
  return dsda_IntConfig(dsda_config_cycle_ghost_colors);
}

dboolean dsda_AlwaysSR50(void) {
  return dsda_IntConfig(dsda_config_movement_strafe50);
}

dboolean dsda_HideHorns(void) {
  return dsda_IntConfig(dsda_config_hide_horns);
}

dboolean dsda_HideWeapon(void) {
  return dsda_IntConfig(dsda_config_hide_weapon);
}

dboolean dsda_SwitchWhenAmmoRunsOut(void) {
  return dsda_IntConfig(dsda_config_switch_when_ammo_runs_out);
}

dboolean dsda_SkipQuitPrompt(void) {
  return dsda_IntConfig(dsda_config_skip_quit_prompt) || dsda_SkipMode();
}

dboolean dsda_PlayQuicksaveSFX(void) {
  return dsda_IntConfig(dsda_config_quicksave_sfx);
}

dboolean dsda_DetailedQuicksave(void) {
  return dsda_IntConfig(dsda_config_quicksave_sfx);
}

dboolean dsda_TrackSplits(void) {
  return demorecording || (demoplayback && dsda_Flag(dsda_arg_track_playback));
}

dboolean dsda_ShowSplitData(void) {
  return dsda_IntConfig(dsda_config_show_split_data);
}

dboolean dsda_CommandDisplay(void) {
  return dsda_IntConfig(dsda_config_command_display) || dsda_BuildMode();
}

dboolean dsda_CoordinateDisplay(void) {
  return dsda_IntConfig(dsda_config_coordinate_display);
}

dboolean dsda_ShowFPS(void) {
  return dsda_IntConfig(dsda_config_show_fps);
}

dboolean dsda_ShowMinimap(void) {
  return dsda_IntConfig(dsda_config_show_minimap);
}

dboolean dsda_ShowLevelSplits(void) {
  return dsda_IntConfig(dsda_config_show_level_splits);
}

dboolean dsda_ShowDemoAttempts(void) {
  return dsda_IntConfig(dsda_config_show_demo_attempts) && demorecording;
}

dboolean dsda_MapCoordinates(void) {
  return dsda_IntConfig(dsda_config_map_coordinates);
}

dboolean dsda_MapTotals(void) {
  return dsda_IntConfig(dsda_config_map_totals);
}

dboolean dsda_MapTime(void) {
  return dsda_IntConfig(dsda_config_map_time);
}

dboolean dsda_MapTitle(void) {
  return dsda_IntConfig(dsda_config_map_title);
}

dboolean dsda_PainPalette(void) {
  return dsda_IntConfig(dsda_config_palette_ondamage);
}

dboolean dsda_PickupPalette(void) {
  return dsda_IntConfig(dsda_config_palette_onbonus);
}

dboolean dsda_PowerPalette(void) {
  return dsda_IntConfig(dsda_config_palette_onpowers);
}

dboolean dsda_EffectPalette(void) {
  if (!allow_incompatibility || dsda_StrictMode()) return true;

  return dsda_IntConfig(dsda_config_palette_oneffects);
}

dboolean dsda_PainPaletteReduced(void) {
  if (!allow_incompatibility || dsda_StrictMode()) return false;

  return dsda_IntConfig(dsda_config_palette_ondamage) > 1;
}

dboolean dsda_PickupPaletteReduced(void) {
  if (!allow_incompatibility || dsda_StrictMode()) return false;

  return dsda_IntConfig(dsda_config_palette_onbonus) > 1;;
}

dboolean dsda_EffectPaletteReduced(void) {
  if (!allow_incompatibility || dsda_StrictMode()) return false;

  return dsda_IntConfig(dsda_config_palette_oneffects) > 1;
}

dboolean dsda_ShowHealthBars(void) {
  return dsda_IntConfig(dsda_config_gl_health_bar);
}

dboolean dsda_WipeAtFullSpeed(void) {
  return dsda_IntConfig(dsda_config_wipe_at_full_speed);
}

int dsda_ShowAliveMonsters(void) {
  return dsda_IntConfig(dsda_config_show_alive_monsters);
}

dboolean dsda_ShowAutomapKeys(void) {
  return dsda_IntConfig(dsda_config_map_show_keys) && allow_incompatibility;
}

dboolean dsda_DisableHorizAutoaim(void) {
  return dsda_IntConfig(dsda_config_disable_horiz_autoaim) && allow_incompatibility;
}

int dsda_TranslucencyPercent(void) {
  if (!allow_incompatibility) return 66;

  return dsda_IntConfig(dsda_config_tran_filter_pct);
}

int dsda_MenuTranslucency(void) {
  return dsda_IntConfig(dsda_config_menu_tran_filter);
}

int dsda_MenuTranslucencyPercent(void) {
  return dsda_IntConfig(dsda_config_menu_tran_filter_pct);
}

int dsda_ShadowTranslucency(void) {
  return dsda_IntConfig(dsda_config_shadow_tran_filter);
}

int dsda_ShadowTranslucencyPercent(void) {
  return dsda_IntConfig(dsda_config_shadow_tran_filter_pct);
}

int dsda_ExHudTranslucency(void) {
  return dsda_IntConfig(dsda_config_ex_text_tran_filter);
}

int dsda_ExHudTranslucencyPercent(void) {
  return dsda_IntConfig(dsda_config_ex_text_tran_filter_pct);
}

int dsda_ShowDataDisk(void) {
  return dsda_IntConfig(nyan_config_loading_disk);
}

int dsda_reveal_map;

int dsda_RevealAutomap(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_reveal_map;
}

void dsda_ResetRevealMap(void) {
  dsda_reveal_map = 0;
}

int dsda_GameSpeed(void) {
  return dsda_IntConfig(dsda_config_game_speed);
}

void dsda_UpdateGameSpeed(int value) {
  dsda_UpdateIntConfig(dsda_config_game_speed, value, true);
}

void dsda_SkipNextWipe(void) {
  dsda_skip_next_wipe = 1;
}

// In raven, strict mode does not affect this setting
dboolean dsda_RenderWipeScreen(void) {
  return raven ? dsda_TransientIntConfig(dsda_config_render_wipescreen) :
                 dsda_IntConfig(dsda_config_render_wipescreen);
}

dboolean dsda_PendingSkipWipe(void) {
  return dsda_skip_next_wipe || !dsda_RenderWipeScreen();
}

dboolean dsda_SkipWipe(void) {
  if (dsda_skip_next_wipe) {
    dsda_skip_next_wipe = 0;
    return true;
  }

  return !dsda_RenderWipeScreen() || raven;
}

dboolean dsda_MultipleAreaMaps(void) {
  return dsda_IntConfig(dsda_config_multiple_area_maps) && !dsda_StrictMode() && allow_incompatibility;
}

static dboolean game_controller_used;
static dboolean mouse_used;

dboolean dsda_AllowGameController(void) {
  return !dsda_StrictMode() || !mouse_used;
}

dboolean dsda_AllowMouse(void) {
  return !dsda_StrictMode() || !game_controller_used;
}

void dsda_WatchGameControllerEvent(void) {
  game_controller_used = true;

  if (mouse_used)
    dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_WatchMouseEvent(void) {
  mouse_used = true;

  if (game_controller_used)
    dsda_TrackFeature(uf_mouse_and_controller);
}

void dsda_LiftInputRestrictions(void) {
  game_controller_used = false;
  mouse_used = false;
}
