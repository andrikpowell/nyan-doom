//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Extended HUD
//

#include <stdio.h>

#include "am_map.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "lprintf.h"
#include "m_file.h"
#include "r_main.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/console.h"
#include "dsda/global.h"
#include "dsda/hud_components.h"
#include "dsda/render_stats.h"
#include "dsda/settings.h"
#include "dsda/utility.h"
#include "dsda/utility/string_view.h"

#include "exhud.h"

typedef struct {
  void (*init)(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
  void (*update)(void* data);
  void (*draw)(void* data);
  const char* name;
  const int default_vpt;
  const dboolean strict;
  const dboolean off_by_default;
  const dboolean intermission;
  const dboolean not_level;
  dboolean on;
  dboolean initialized;
  void* data;
} exhud_component_t;

typedef enum {
  exhud_ammo_text,
  exhud_armor_text,
  exhud_big_ammo,
  exhud_big_ammo_text,
  exhud_big_mana,
  exhud_big_armor,
  exhud_big_armor_text,
  exhud_big_artifact,
  exhud_big_artifact_bar,
  exhud_doomguy_face,
  exhud_loading_disk,
  exhud_status_widget,
  exhud_status_timers,
  exhud_big_health,
  exhud_big_health_text,
  exhud_composite_time,
  exhud_health_text,
  exhud_keys,
  exhud_ready_ammo_text,
  exhud_speed_text,
  exhud_stat_totals,
  exhud_tracker,
  exhud_weapon_text,
  exhud_render_stats,
  exhud_fps,
  exhud_attempts,
  exhud_local_time,
  exhud_coordinate_display,
  exhud_line_display,
  exhud_command_display,
  exhud_event_split,
  exhud_level_splits,
  exhud_color_test,
  exhud_free_text,
  exhud_artifact_desc,
  exhud_message,
  exhud_announce_message,
  exhud_secret_message,
  exhud_target_health,
  exhud_map_coordinates,
  exhud_map_time,
  exhud_map_title,
  exhud_map_totals,
  exhud_minimap,
  exhud_component_count,
} exhud_component_id_t;

exhud_component_t components_template[exhud_component_count] = {
  [exhud_ammo_text] = {
    dsda_InitAmmoTextHC,
    dsda_UpdateAmmoTextHC,
    dsda_DrawAmmoTextHC,
    "ammo_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_armor_text] = {
    dsda_InitArmorTextHC,
    dsda_UpdateArmorTextHC,
    dsda_DrawArmorTextHC,
    "armor_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_big_ammo] = {
    dsda_InitBigAmmoHC,
    dsda_UpdateBigAmmoHC,
    dsda_DrawBigAmmoHC,
    "big_ammo",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_big_ammo_text] = {
    dsda_InitBigAmmoTextHC,
    dsda_UpdateBigAmmoTextHC,
    dsda_DrawBigAmmoTextHC,
    "big_ammo_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_big_mana] = {
    dsda_InitBigManaHC,
    dsda_UpdateBigManaHC,
    dsda_DrawBigManaHC,
    "big_mana",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_big_armor] = {
    dsda_InitBigArmorHC,
    dsda_UpdateBigArmorHC,
    dsda_DrawBigArmorHC,
    "big_armor",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_big_armor_text] = {
    dsda_InitBigArmorTextHC,
    dsda_UpdateBigArmorTextHC,
    dsda_DrawBigArmorTextHC,
    "big_armor_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_big_artifact] = {
    dsda_InitBigArtifactHC,
    dsda_UpdateBigArtifactHC,
    dsda_DrawBigArtifactHC,
    "big_artifact",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_big_artifact_bar] = {
    dsda_InitBigArtifactBarHC,
    dsda_UpdateBigArtifactBarHC,
    dsda_DrawBigArtifactBarHC,
    "big_artifact_bar",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_doomguy_face] = {
    dsda_InitDoomguyFaceHC,
    dsda_UpdateDoomguyFaceHC,
    dsda_DrawDoomguyFaceHC,
    "doomguy_face",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_loading_disk] = {
    dsda_InitLoadingDiskHC,
    dsda_UpdateLoadingDiskHC,
    dsda_DrawLoadingDiskHC,
    "loading_disk",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_status_widget] = {
    dsda_InitStatusIconsHC,
    dsda_UpdateStatusIconsHC,
    dsda_DrawStatusIconsHC,
    "status_widget",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_status_timers] = {
    dsda_InitStatusTimersHC,
    dsda_UpdateStatusTimersHC,
    dsda_DrawStatusTimersHC,
    "status_timers",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_big_health] = {
    dsda_InitBigHealthHC,
    dsda_UpdateBigHealthHC,
    dsda_DrawBigHealthHC,
    "big_health",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_big_health_text] = {
    dsda_InitBigHealthTextHC,
    dsda_UpdateBigHealthTextHC,
    dsda_DrawBigHealthTextHC,
    "big_health_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_composite_time] = {
    dsda_InitCompositeTimeHC,
    dsda_UpdateCompositeTimeHC,
    dsda_DrawCompositeTimeHC,
    "composite_time",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_health_text] = {
    dsda_InitHealthTextHC,
    dsda_UpdateHealthTextHC,
    dsda_DrawHealthTextHC,
    "health_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_keys] = {
    dsda_InitKeysHC,
    dsda_UpdateKeysHC,
    dsda_DrawKeysHC,
    "keys",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET | VPT_EX_TRANS,
  },
  [exhud_ready_ammo_text] = {
    dsda_InitReadyAmmoTextHC,
    dsda_UpdateReadyAmmoTextHC,
    dsda_DrawReadyAmmoTextHC,
    "ready_ammo_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_speed_text] = {
    dsda_InitSpeedTextHC,
    dsda_UpdateSpeedTextHC,
    dsda_DrawSpeedTextHC,
    "speed_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_stat_totals] = {
    dsda_InitStatTotalsHC,
    dsda_UpdateStatTotalsHC,
    dsda_DrawStatTotalsHC,
    "stat_totals",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_tracker] = {
    dsda_InitTrackerHC,
    dsda_UpdateTrackerHC,
    dsda_DrawTrackerHC,
    "tracker",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .strict = true,
  },
  [exhud_weapon_text] = {
    dsda_InitWeaponTextHC,
    dsda_UpdateWeaponTextHC,
    dsda_DrawWeaponTextHC,
    "weapon_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_render_stats] = {
    dsda_InitRenderStatsHC,
    dsda_UpdateRenderStatsHC,
    dsda_DrawRenderStatsHC,
    "render_stats",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_fps] = {
    dsda_InitFPSHC,
    dsda_UpdateFPSHC,
    dsda_DrawFPSHC,
    "fps",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .off_by_default = true,
  },
  [exhud_attempts] = {
    dsda_InitAttemptsHC,
    dsda_UpdateAttemptsHC,
    dsda_DrawAttemptsHC,
    "attempts",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_local_time] = {
    dsda_InitLocalTimeHC,
    dsda_UpdateLocalTimeHC,
    dsda_DrawLocalTimeHC,
    "local_time",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_coordinate_display] = {
    dsda_InitCoordinateDisplayHC,
    dsda_UpdateCoordinateDisplayHC,
    dsda_DrawCoordinateDisplayHC,
    "coordinate_display",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_line_display] = {
    dsda_InitLineDisplayHC,
    dsda_UpdateLineDisplayHC,
    dsda_DrawLineDisplayHC,
    "line_display",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_command_display] = {
    dsda_InitCommandDisplayHC,
    dsda_UpdateCommandDisplayHC,
    dsda_DrawCommandDisplayHC,
    "command_display",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .strict = true,
    .off_by_default = true,
    .intermission = true,
  },
  [exhud_event_split] = {
    dsda_InitEventSplitHC,
    dsda_UpdateEventSplitHC,
    dsda_DrawEventSplitHC,
    "event_split",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_level_splits] = {
    dsda_InitLevelSplitsHC,
    dsda_UpdateLevelSplitsHC,
    dsda_DrawLevelSplitsHC,
    "level_splits",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
    .intermission = true,
    .not_level = true,
  },
  [exhud_color_test] = {
    dsda_InitColorTestHC,
    dsda_UpdateColorTestHC,
    dsda_DrawColorTestHC,
    "color_test",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_free_text] = {
    dsda_InitFreeTextHC,
    dsda_UpdateFreeTextHC,
    dsda_DrawFreeTextHC,
    "free_text",
    .default_vpt = VPT_EX_TEXT | VPT_EX_TRANS,
  },
  [exhud_artifact_desc] = {
    dsda_InitArtifactDescHC,
    dsda_UpdateArtifactDescHC,
    dsda_DrawArtifactDescHC,
    "artifact_desc",
  },
  [exhud_message] = {
    dsda_InitMessageHC,
    dsda_UpdateMessageHC,
    dsda_DrawMessageHC,
    "message",
  },
  [exhud_announce_message] = {
    dsda_InitAnnounceMessageHC,
    dsda_UpdateAnnounceMessageHC,
    dsda_DrawAnnounceMessageHC,
    "announce_message",
  },
  [exhud_secret_message] = {
    dsda_InitSecretMessageHC,
    dsda_UpdateSecretMessageHC,
    dsda_DrawSecretMessageHC,
    "secret_message",
  },
  [exhud_target_health] = {
    dsda_InitTargetHealthHC,
    dsda_UpdateTargetHealthHC,
    dsda_DrawTargetHealthHC,
    "target_health",
    .strict = true,
    .off_by_default = true,
  },
  [exhud_map_coordinates] = {
    dsda_InitMapCoordinatesHC,
    dsda_UpdateMapCoordinatesHC,
    dsda_DrawMapCoordinatesHC,
    "map_coordinates",
    .strict = true,
  },
  [exhud_map_time] = {
    dsda_InitMapTimeHC,
    dsda_UpdateMapTimeHC,
    dsda_DrawMapTimeHC,
    "map_time",
  },
  [exhud_map_title] = {
    dsda_InitMapTitleHC,
    dsda_UpdateMapTitleHC,
    dsda_DrawMapTitleHC,
    "map_title",
  },
  [exhud_map_totals] = {
    dsda_InitMapTotalsHC,
    dsda_UpdateMapTotalsHC,
    dsda_DrawMapTotalsHC,
    "map_totals",
  },
  [exhud_minimap] = {
    dsda_InitMinimapHC,
    dsda_UpdateMinimapHC,
    dsda_DrawMinimapHC,
    "minimap",
    .default_vpt = VPT_EX_TEXT,
    .off_by_default = true,
  },
};

typedef struct {
  const char* name;
  dboolean status_bar;
  dboolean allow_offset;
  const char* hud_string;  // optional full HUD name
  dboolean loaded;
  dboolean cleared;
  exhud_component_t components[exhud_component_count];
  int y_offset[VPT_ALIGN_MAX];
} dsda_hud_container_t;

typedef enum {
  hud_ex,
  hud_off,
  hud_full,
  hud_map,
  hud_null,
} dsda_hud_variant_t;

static dsda_hud_container_t containers[] = {
  [hud_ex]   = { "ex", true, true },
  [hud_off]  = { "off", true, true },
  [hud_full] = { "full", false, true }, // legacy hud - 0
  [hud_map] = { "map", true, false },
  [hud_null] = { NULL }
};

static dsda_hud_container_t* container;
static exhud_component_t* components;

// full HUD variants: "doom full", "doom full 1", "doom full 2", etc.
static dsda_hud_container_t *full_containers = NULL;
static int full_container_count = 0;

static void dsda_InitContainer(dsda_hud_container_t *c, const char *name, dboolean status_bar, dboolean allow_offset)
{
  int i;
  memset(c, 0, sizeof(*c));
  c->name = name;
  c->status_bar = status_bar;
  c->allow_offset = allow_offset;
  c->hud_string = NULL;
  c->cleared = false;
  for (i = 0; i < VPT_ALIGN_MAX; ++i)
    c->y_offset[i] = 0;
}

#define DSDA_MAX_FULLHUD_MAX 16

static dsda_hud_container_t* dsda_GetFullContainer(int index)
{
  int old_count;

  if (index < 1 || index > DSDA_MAX_FULLHUD_MAX)
    I_Error("full hud index %d out of range (1-%d)", index, DSDA_MAX_FULLHUD_MAX);

  // add to hud slots
  if (index >= full_container_count) {
    old_count = full_container_count;
    full_container_count = index + 1;

    if (!full_containers)
      full_containers = Z_Malloc(sizeof(*full_containers) * full_container_count);
    else
      full_containers = Z_Realloc(full_containers, sizeof(*full_containers) * full_container_count);

    // init new added slots
    for (; old_count < full_container_count; ++old_count)
      dsda_InitContainer(&full_containers[old_count], "full", false, true);
  }

  return &full_containers[index];
}

int dsda_show_render_stats;

int dsda_ExHudVerticalOffset(void) {
  if (container && container->status_bar)
    return g_st_height;

  return 0;
}

static void dsda_TurnComponentOn(int id) {
  if (!components[id].initialized)
    return;

  components[id].on = true;
}

static void dsda_TurnComponentOff(int id) {
  components[id].on = false;
}

static void dsda_InitializeComponent(int id, int x, int y, int vpt, int* args, int arg_count) {
  components[id].initialized = true;
  components[id].init(x, y, vpt | components[id].default_vpt,
                      args, arg_count, &components[id].data);

  if (components[id].off_by_default)
    dsda_TurnComponentOff(id);
  else
    dsda_TurnComponentOn(id);
}

static int dsda_AlignmentToVPT(const char* alignment) {
  if (!strcmp(alignment, "bottom_left"))
    return VPT_ALIGN_LEFT_BOTTOM;
  else if (!strcmp(alignment, "bottom_right"))
    return VPT_ALIGN_RIGHT_BOTTOM;
  else if (!strcmp(alignment, "bottom_center"))
    return VPT_ALIGN_CENTER_BOTTOM;
  else if (!strcmp(alignment, "top_left"))
    return VPT_ALIGN_LEFT_TOP;
  else if (!strcmp(alignment, "top_right"))
    return VPT_ALIGN_RIGHT_TOP;
  else if (!strcmp(alignment, "top_center"))
    return VPT_ALIGN_CENTER_TOP;
  else if (!strcmp(alignment, "top"))
    return VPT_ALIGN_TOP;
  else if (!strcmp(alignment, "bottom"))
    return VPT_ALIGN_BOTTOM;
  else if (!strcmp(alignment, "center"))
    return VPT_ALIGN_CENTER;
  else if (!strcmp(alignment, "left"))
    return VPT_ALIGN_LEFT;
  else if (!strcmp(alignment, "right"))
    return VPT_ALIGN_RIGHT;
  else if (!strcmp(alignment, "none"))
    return VPT_STRETCH;
  else
    return -1;
}

static int dsda_ParseHUDConfig(char** hud_config, int line_i) {
  int i;
  int count;
  dboolean found;
  const char* line;
  char command[64];
  char args[64];

  for (++line_i; hud_config[line_i]; ++line_i) {
    line = hud_config[line_i];

    if (line[0] == '#' || line[0] == '/' || line[0] == '!' || !line[0])
      continue;

    count = sscanf(line, "%63s %63[^\n\r]", command, args);

    if (count < 1)
      I_Error("Invalid hud definition \"%s\"", line);

    // clearhud is unique
    if (!strncmp(command, "clearhud", sizeof(command)))
    {
      if (count != 1)
        I_Error("Invalid hud definition \"%s\"", line);

      if (container == &containers[hud_full])
        I_Error("\"clearhud\" is not supported for legacy full hud");

      found = true;
      container->cleared = true;
      continue;
    }

    if (count != 2)
      I_Error("Invalid hud definition \"%s\"", line);

    // The start of another definition
    if (!strncmp(command, "doom", sizeof(command)) ||
        !strncmp(command, "heretic", sizeof(command)) ||
        !strncmp(command, "hexen", sizeof(command)))
      break;

    found = false;

    for (i = 0; i < exhud_component_count; ++i)
      if (!strncmp(command, components[i].name, sizeof(command))) {
        int x, y;
        int vpt;
        int component_args[7] = { 0 };
        char alignment[16];

        found = true;

        count = sscanf(args, "%d %d %15s %d %d %d %d %d %d %d", &x, &y, alignment,
                        &component_args[0], &component_args[1],
                        &component_args[2], &component_args[3],
                        &component_args[4], &component_args[5],
                        &component_args[6]);
        if (count < 3)
          I_Error("Invalid hud component args \"%s\"", line);

        vpt = dsda_AlignmentToVPT(alignment);
        if (vpt < 0)
          I_Error("Invalid hud component alignment \"%s\"", line);

        dsda_InitializeComponent(i, x, y, vpt, component_args, count - 3);
      }

    if (!strncmp(command, "add_offset", sizeof(command))) {
      int offset;
      int vpt;
      char alignment[16];

      found = true;

      if (!container->allow_offset)
        I_Error("The %s config does not support add_offset", container->name);

      count = sscanf(args, "%d %15s", &offset, alignment);
      if (count != 2)
        I_Error("Invalid hud offset \"%s\"", line);

      vpt = dsda_AlignmentToVPT(alignment);
      if (vpt < 0) {
        I_Error("Invalid hud offset alignment \"%s\"", line);
        vpt = 0; // TODO: remove after I_Error marked noreturn
      }

      container->y_offset[vpt] = offset;

      if (BOTTOM_ALIGNMENT(vpt))
        container->y_offset[vpt] = -container->y_offset[vpt];
    }

    if (!found)
      I_Error("Invalid hud component \"%s\"", line);
  }

  // roll back the line that wasn't part of this config
  return line_i - 1;
}

static const char *dsda_ParseHUDName(const char *line)
{
  dsda_string_view_t sv;
  dsda_string_view_t cur;
  char name[64] = { 0 };

  dsda_InitStringView(&sv, line, strlen(line));
  cur = sv;

  if (!dsda_SplitStringViewAfterChar(&cur, '"', NULL, &cur) ||
      !dsda_SplitStringViewBeforeChar(&cur, '"', &cur, NULL))
    return NULL;

  memcpy(name, cur.string, MIN(sizeof(name) - 1, cur.size));
  return Z_Strdup(name);
}

static void dsda_ParseHUDConfigs(char** hud_config) {
  const char* line;
  int line_i;
  const char* target_format;
  char hud_variant[32];
  int full_index = 0;
  int header_count;

  target_format = hexen ? "hexen %31s %d" : heretic ? "heretic %31s %d" : "doom %31s %d";

  for (line_i = 0; hud_config[line_i]; ++line_i)
  {
    line = hud_config[line_i];

    // try new hud # format
    full_index = 0;
    header_count = sscanf(line, target_format, hud_variant, &full_index);

    // If that failed, try legacy full hud
    if (header_count < 1)
    {
      const char *legacy_format = hexen ? "hexen %31s" : heretic ? "heretic %31s" : "doom %31s";
      header_count = sscanf(line, legacy_format, hud_variant);
      full_index = 0;
    }

    // Behavior for full (up to 16 huds)
    if (header_count >= 1)
    {
      if (!strcmp(hud_variant, "full"))
      {
        dsda_hud_container_t *full_container;
        const char *display_name = NULL;

        // legacy
        if (header_count == 1)
        {
          full_container = &containers[hud_full];
        }
        // full #
        else
        {
          display_name = dsda_ParseHUDName(line);

          if (full_index < 1 || full_index > DSDA_MAX_FULLHUD_MAX)
            I_Error("full hud index %d out of range (1-%d)", full_index, DSDA_MAX_FULLHUD_MAX);

          full_container = dsda_GetFullContainer(full_index);
        }

        // Reset hud
        dsda_InitContainer(full_container, "full", false, true);

        full_container->loaded = true;
        full_container->hud_string = display_name;

        components = full_container->components;
        memcpy(components, components_template, sizeof(components_template));

        container = full_container;
        line_i = dsda_ParseHUDConfig(hud_config, line_i);
        continue;
      }

      // Behavior for ex/off/map
      for (container = containers; container->name; container++) {
        if (!strncmp(container->name, hud_variant, sizeof(hud_variant))) {
          if (container->loaded)
            break;

          container->loaded = true;
          components = container->components;
          memcpy(components, components_template, sizeof(components_template));

          line_i = dsda_ParseHUDConfig(hud_config, line_i);

          break;
        }
      }
    }
  }
}

static int dsda_GetGameHudConfig(void)
{
  return raven ? hexen ? dsda_config_hexen_full_hud : dsda_config_heretic_full_hud : dsda_config_doom_full_hud;
}

static int dsda_FullHudIndex(void)
{
  int config_full_hud = dsda_GetGameHudConfig();
  int idx = dsda_IntConfig(config_full_hud);

  idx = CLAMP(idx, 0, DSDA_MAX_FULLHUD_MAX);

  return idx;
}

static dboolean dsda_FullHudLoaded(int idx)
{
  if (idx == 0)
    return containers[hud_full].loaded;

  if (idx < 1 || idx >= full_container_count)
    return false;

  return full_containers[idx].loaded && !full_containers[idx].cleared;
}

static int dsda_FindNextLoadedFullHud(int cur)
{
  int i;

  cur = CLAMP(cur, 0, DSDA_MAX_FULLHUD_MAX);

  for (i = 1; i <= DSDA_MAX_FULLHUD_MAX + 1; ++i)
  {
    int idx = (cur + i) % (DSDA_MAX_FULLHUD_MAX + 1);  // cycles 0..16
  
    if (dsda_FullHudLoaded(idx))
      return idx;
  }

  return cur;
}

static dsda_hud_container_t* dsda_ActiveFullContainer(void)
{
  int idx = dsda_FullHudIndex();
  int i;

  // HUD 0 = legacy
  if (idx == 0)
  {
    if (containers[hud_full].loaded)
      return &containers[hud_full];

    // no legacy: fall back to first loaded numbered hud
    for (i = 1; i < full_container_count; ++i)
      if (full_containers[i].loaded && !full_containers[i].cleared)
        return &full_containers[i];

    return &containers[hud_full];
  }

  // HUD 1-16
  if (idx < full_container_count && full_containers[idx].loaded && !full_containers[idx].cleared)
    return &full_containers[idx];

  // HUD 1-16 - missing or cleared? move on to next hud
  else
  {
    int next = dsda_FindNextLoadedFullHud(idx);

    if (next > 0 && next < full_container_count)
    {
      int config_full_hud = dsda_GetGameHudConfig();
      dsda_UpdateIntConfig(config_full_hud, next, true);
      return &full_containers[next];
    }
  }

  // fallback to legacy
  if (containers[hud_full].loaded)
    return &containers[hud_full];

  // fallback - shouldn't reach here
  return &containers[hud_full];
}

static void dsda_LoadHUDConfig(void) {
  DO_ONCE
    char* hud_config = NULL;
    char** lines;
    dsda_arg_t* arg;
    int lump;
    int length = 0;

    arg = dsda_Arg(dsda_arg_hud);
    if (arg->found)
    {
      length = M_ReadFileToString(arg->value.v_string, &hud_config);
    }
    else
    {
      lump = -1;
      while ((lump = W_ListNumFromName("NYANHUD", lump)) >= 0) {
        if (!hud_config) {
          hud_config = W_ReadLumpToString(lump);
          length = W_LumpLength(lump);
        }
        else {
          hud_config = Z_Realloc(hud_config, length + W_LumpLength(lump) + 2);
          hud_config[length++] = '\n'; // in case the file didn't end in a new line
          W_ReadLump(lump, hud_config + length);
          length += W_LumpLength(lump);
          hud_config[length] = '\0';
        }
      }
    }

    if (hud_config) {
      lines = dsda_SplitString(hud_config, "\n\r");

      if (lines) {
        dsda_ParseHUDConfigs(lines);

        Z_Free(lines);
      }

      Z_Free(hud_config);
    }
  END_ONCE
}

static dboolean dsda_HideHUD(void) {
  return dsda_Flag(dsda_arg_nodraw) ||
         (R_FullView() && !dsda_IntConfig(dsda_config_hud_displayed));
}

dboolean dsda_FullExHudOn(void) {
  return !dsda_HideHUD() && R_FullView();
}

static dboolean dsda_HUDActive(void) {
  return container && container->loaded;
}

static void dsda_ResetActiveHUD(void) {
  container = NULL;
  components = NULL;
}

static void dsda_UpdateActiveHUD(void) {
  container = R_FullView() ? dsda_ActiveFullContainer() :
              dsda_IntConfig(dsda_config_exhud) ? &containers[hud_ex] :
              &containers[hud_off];

  if (container->loaded)
    components = container->components;
  else
    dsda_ResetActiveHUD();
}

static void dsda_ResetOffsets(void) {
  void dsda_UpdateExTextOffset(enum patch_translation_e flags, int offset);
  void dsda_ResetExTextOffsets(void);

  int i;

  dsda_ResetExTextOffsets();

  for (i = 0; i < VPT_ALIGN_MAX; ++i)
    if (container->y_offset[i])
      dsda_UpdateExTextOffset(i, container->y_offset[i]);
}

static void dsda_RefreshHUD(void) {
  if (!dsda_HUDActive())
    return;

  dsda_ResetOffsets();

  if (dsda_show_render_stats)
    dsda_TurnComponentOn(exhud_render_stats);

  dsda_RefreshExHudFPS();
  dsda_RefreshExHudMinimap();
  dsda_RefreshExHudLevelSplits();
  dsda_RefreshExHudTargetHealth();
  dsda_RefreshExHudCoordinateDisplay();
  dsda_RefreshExHudCommandDisplay();
  dsda_RefreshExHudFreeText();
  dsda_RefreshExHudStatusWidget();
  dsda_RefreshExHudTimerWidget();
  dsda_RefreshMapCoordinates();
  dsda_RefreshMapTotals();
  dsda_RefreshMapTime();
  dsda_RefreshMapTitle();

  if (in_game && gamestate == GS_LEVEL)
    dsda_UpdateExHud();
}

void dsda_InitExHud(void) {
  dsda_ResetActiveHUD();

  if (dsda_HideHUD())
    return;

  dsda_LoadHUDConfig();
  dsda_UpdateActiveHUD();
  dsda_RefreshHUD();
}

void dsda_CycleFullHud(void)
{
  int config_full_hud = raven ? hexen ? dsda_config_hexen_full_hud : dsda_config_heretic_full_hud : dsda_config_doom_full_hud;
  int cur = dsda_FullHudIndex();
  int next = dsda_FindNextLoadedFullHud(cur);
  dsda_hud_container_t *c;

  dsda_UpdateIntConfig(config_full_hud, next, true);
  dsda_UpdateActiveHUD();
  dsda_RefreshHUD();

  c = dsda_ActiveFullContainer();

  // Legacy full
  if (c == &containers[hud_full]) {
    doom_printf("HUD Legacy");
  }
  // full with name
  else if (c->hud_string && *c->hud_string) {
    doom_printf("HUD %i: %s", next, c->hud_string);
  }
  // full without name
  else {
    doom_printf("HUD %i", next);
  }
}

static void dsda_UpdateComponents(exhud_component_t* update_components) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      update_components[i].on &&
      !update_components[i].not_level &&
      (!update_components[i].strict || !dsda_StrictMode())
    )
      update_components[i].update(update_components[i].data);
}

void dsda_UpdateExHud(void) {
  if (automap_stbar) {
    if (containers[hud_map].loaded)
      dsda_UpdateComponents(containers[hud_map].components);

    return;
  }

  if (!dsda_HUDActive())
    return;

  dsda_UpdateComponents(components);
}

static void dsda_DrawComponents(exhud_component_t* draw_components) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      draw_components[i].on &&
      !draw_components[i].not_level &&
      (!draw_components[i].strict || !dsda_StrictMode())
    )
      draw_components[i].draw(draw_components[i].data);
}

int global_patch_top_offset;

void dsda_DrawExHud(void) {
  global_patch_top_offset = M_ConsoleOpen() ? dsda_ConsoleHeight() : 0;

  if (automap_stbar) {
    if (containers[hud_map].loaded)
      dsda_DrawComponents(containers[hud_map].components);
  }
  else if (dsda_HUDActive())
    dsda_DrawComponents(components);

  global_patch_top_offset = 0;
}

void dsda_DrawExIntermission(void) {
  int i;

  if (!dsda_HUDActive())
    return;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      components[i].on &&
      components[i].intermission &&
      (!components[i].strict || !dsda_StrictMode())
    )
      components[i].draw(components[i].data);
}

dboolean dsda_CheckBigArtifactBar(void) {
  return (!dsda_HideHUD() && (components[exhud_big_artifact_bar].on == true));
}

void dsda_ToggleRenderStats(void) {
  dsda_show_render_stats = !dsda_show_render_stats;

  if (!dsda_HUDActive())
    return;

  if (components[exhud_render_stats].on && !dsda_show_render_stats)
    dsda_TurnComponentOff(exhud_render_stats);
  else if (!components[exhud_render_stats].on && dsda_show_render_stats) {
    dsda_BeginRenderStats();
    dsda_TurnComponentOn(exhud_render_stats);
  }
}

static void dsda_BasicRefresh(dboolean (*show_component)(void), exhud_component_id_t id) {
  if (!dsda_HUDActive())
    return;

  if (show_component())
    dsda_TurnComponentOn(id);
  else
    dsda_TurnComponentOff(id);
}

static void dsda_BasicMapRefresh(dboolean (*show_component)(void), exhud_component_id_t id) {
  exhud_component_t* old_components;

  if (!dsda_HUDActive())
    return;

  old_components = components;
  components = containers[hud_map].components;

  if (show_component())
    dsda_TurnComponentOn(id);
  else
    dsda_TurnComponentOff(id);

  components = old_components;
}

dboolean dsda_CheckExHudKeys(void) {
  return (!dsda_HideHUD() && (components[exhud_keys].on == true));
}

void dsda_RefreshExHudFPS(void) {
  dsda_BasicRefresh(dsda_ShowFPS, exhud_fps);
}

void dsda_RefreshExHudMinimap(void) {
  if (!dsda_HUDActive())
    return;

  if (dsda_ShowMinimap()) {
    dsda_TurnComponentOn(exhud_minimap);

    // Need to update the component before calling AM_Start
    if (components[exhud_minimap].initialized)
      components[exhud_minimap].update(components[exhud_minimap].data);

    if (in_game && gamestate == GS_LEVEL)
      AM_Start(AM_OPEN_MINIMAP);
  }
  else
    dsda_TurnComponentOff(exhud_minimap);
}

void dsda_RefreshExHudLevelSplits(void) {
  dsda_BasicRefresh(dsda_ShowLevelSplits, exhud_level_splits);
}

void dsda_RefreshExHudCoordinateDisplay(void) {
  if (!dsda_HUDActive())
    return;

  if (dsda_CoordinateDisplay()) {
    dsda_TurnComponentOn(exhud_coordinate_display);
    dsda_TurnComponentOn(exhud_line_display);
  }
  else {
    dsda_TurnComponentOff(exhud_coordinate_display);
    dsda_TurnComponentOff(exhud_line_display);
  }
}

void dsda_RefreshExHudTargetHealth(void) {
  dsda_BasicRefresh(dsda_TargetHealth, exhud_target_health);
}

void dsda_RefreshExHudCommandDisplay(void) {
  dsda_BasicRefresh(dsda_CommandDisplay, exhud_command_display);
}

void dsda_RefreshExHudFreeText(void) {
  dsda_BasicRefresh(dsda_FreeTextShown, exhud_free_text);
}

void dsda_RefreshExHudStatusWidget(void) {
  dsda_BasicRefresh(dsda_StatusWidget, exhud_status_widget);
}

void dsda_RefreshExHudTimerWidget(void) {
  dsda_BasicRefresh(dsda_TimerWidget, exhud_status_timers);
}

void dsda_RefreshMapCoordinates(void) {
  dsda_BasicMapRefresh(dsda_MapCoordinates, exhud_map_coordinates);
}

void dsda_RefreshMapTotals(void) {
  dsda_BasicMapRefresh(dsda_MapTotals, exhud_map_totals);
}

void dsda_RefreshMapTime(void) {
  dsda_BasicMapRefresh(dsda_MapTime, exhud_map_time);
}

void dsda_RefreshMapTitle(void) {
  dsda_BasicMapRefresh(dsda_MapTitle, exhud_map_title);
}
