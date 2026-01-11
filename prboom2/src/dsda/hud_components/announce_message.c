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
  dboolean center;
} local_component_t;

static local_component_t* local;

static void dsda_RefreshHudTextAnnounceWrapped(dsda_text_t *component, int centered)
{
  char *msg = component->msg;
  char *split;
  char saved;
  dboolean title_trunc;

  HUlib_clearTextLine(&component->text);

  if (!msg || !*msg)
    return;

  split = strchr(msg, '\n');
  if (!split)
  {
    HUlib_WrapStringToTextLines(&component->text, msg, centered, 2);
    return;
  }

  saved = *split;
  *split = '\0';

  // Title - 2 lines only
  title_trunc = HUlib_WrapStringToTextLines(&component->text, msg, centered, 2);

  *split = saved;

  // Even if the title got ellipsized and returned early.
  // Only add newline if we aren't already at a fresh line.
  if (component->text.len > 0 && component->text.l[component->text.len - 1] != '\n')
    HUlib_addCharToTextLine(&component->text, '\n');

  // Author prints after title - 1 line
  // Allowance of 3 lines total (with title)
  HUlib_WrapStringToTextLines(&component->text, split + 1, centered, 3);
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  char* HU_AnnounceMessage(void);
  char* HU_AuthorMessage(void);

  const char* title_msg  = HU_AnnounceMessage();
  const char* author_msg = HU_AuthorMessage();

  if (!title_msg || !*title_msg)
  {
    if (max_size)
      str[0] = '\0';
    return;
  }

  if (author_msg && *author_msg)
  {
    snprintf(
      str,
      max_size,
      "%s%s\n%sby %s",
      dsda_TextColor(dsda_tc_hud_announce_message),
      title_msg,
      dsda_TextColor(dsda_tc_hud_announce_author),
      author_msg
    );
  }
  else
  {
    snprintf(
      str,
      max_size,
      "%s%s",
      dsda_TextColor(dsda_tc_hud_announce_message),
      title_msg
    );
  }
}

void dsda_InitAnnounceMessageHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : true;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateAnnounceMessageHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudTextAnnounceWrapped(&local->component, local->center);

  if (local->center)
    HUlib_setTextXCenter(&local->component.text);
}

void dsda_DrawAnnounceMessageHC(void* data) {
  local = data;

  dsda_DrawBasicShadowedText(&local->component);
}
