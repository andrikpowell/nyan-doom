//
// Copyright(C) 2024 by Andrik Powell
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
//	NYAN Map Title Message HUD Component
//

#include "base.h"

#include "announce_message.h"

typedef struct {
  dsda_text_t component;
  dsda_text_t author;
  dboolean center;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateAuthorText(char* str, size_t max_size) {
  char* HU_AuthorMessage(void);

  char* message;

  message = HU_AuthorMessage();

  if (message)
    snprintf(
      str,
      max_size,
      "\n%sby %s",
      dsda_TextColor(dsda_tc_hud_announce_author),
      message
    );
  else
    str[0] = '\0';
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  char* HU_AnnounceMessage(void);

  char* message;

  message = HU_AnnounceMessage();

  if (message)
    snprintf(
      str,
      max_size,
      "%s%s",
      dsda_TextColor(dsda_tc_hud_announce_message),
      message
    );
  else
    str[0] = '\0';
}

void dsda_InitAnnounceMessageHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : true;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
  dsda_InitBlockyHC(&local->author, x_offset, y_offset + 1, vpt);
}

void dsda_UpdateAnnounceMessageHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  dsda_UpdateAuthorText(local->author.msg, sizeof(local->author.msg));
  dsda_RefreshHudText(&local->author);

  if (local->center)
  {
    HUlib_setTextXCenter(&local->component.text);
    HUlib_setTextXCenter(&local->author.text);
  }
}

void dsda_DrawAnnounceMessageHC(void* data) {
  local = data;

  dsda_DrawBasicShadowedText(&local->component);
  dsda_DrawBasicShadowedText(&local->author);
}
