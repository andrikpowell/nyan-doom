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
//	NYAN Inventory Description Message HUD Component
//

#include "base.h"

#include "artifact_desc.h"
#include "lprintf.h"

typedef struct {
  dsda_text_t component;
  dboolean center;
  int y_offset;
  int vpt;
  double ratio;
} local_component_t;

static local_component_t* local;

static void dsda_RefreshArtifactDescWrapped(dsda_text_t *component, int centered)
{
  char *msg = component->msg;
  char *split;
  char saved;

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

  // Name - 2 lines only
  HUlib_WrapStringToTextLines(&component->text, msg, centered, 2);

  *split = saved;

  // Even if the Name got ellipsized and returned early.
  // Only add newline if we aren't already at a fresh line.
  if (component->text.len > 0 && component->text.l[component->text.len - 1] != '\n')
    HUlib_addCharToTextLine(&component->text, '\n');

  // Desc prints after Name - 1 line
  // Allowance of 4 lines total (with desc)
  HUlib_WrapStringToTextLines(&component->text, split + 1, centered, 4);
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  extern void dsda_GetInventoryDescription(const char** name, const char** desc);
  const char* ArtiName; // 0 = Name
  const char* ArtiDesc; // 1 = Description

  dsda_GetInventoryDescription(&ArtiName, &ArtiDesc);

  //if (!str || max_size == 0) return;

  if (ArtiDesc && *ArtiDesc && ArtiName && *ArtiName)
  {
    snprintf(
      str,
      max_size,
      "%s%s\n%s%s",
      HU_ColorFromValue(CR_GOLD),
      ArtiName,
      HU_ColorFromValue(CR_DEFAULT),
      ArtiDesc
    );
  }
  else if (ArtiName && *ArtiName)
  {
    snprintf(
      str,
      max_size,
      "%s%s",
      HU_ColorFromValue(CR_GOLD),
      ArtiName
    );
  }
  else if (ArtiDesc && *ArtiDesc)
  {
    snprintf(
      str,
      max_size,
      "%s%s",
      HU_ColorFromValue(CR_DEFAULT),
      ArtiDesc
    );
  }
  else
  {
    str[0] = '\0';
  }
}

void dsda_InitArtifactDescHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : true;

  if (!raven) return;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);

  local->vpt = vpt;
  local->y_offset = y_offset;
  local->ratio = (hud_font.line_height != 8) ? (double)hud_font.line_height / 8.0 : 0.0;
}

void dsda_UpdateArtifactDescHC(void* data) {
  local = data;

  if (!raven) return;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshArtifactDescWrapped(&local->component, local->center);

  if (local->center)
    HUlib_setTextXCenter(&local->component.text);

  // Adjust y-offset for multi-line if bottom-aligned
  if (BOTTOM_ALIGNMENT(local->component.text.flags & VPT_ALIGN_MASK))
    HUlib_AdjustBottomOffset_MultiLine(&local->component.text, local->y_offset, local->ratio, local->vpt);
}

void dsda_DrawArtifactDescHC(void* data) {
  local = data;

  if (!raven) return;

  dsda_DrawBasicShadowedText(&local->component);
}
