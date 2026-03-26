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
//	DSDA HUD Component Base
//

#include <math.h>

#include "base.h"

static char big_digit_lump[9];
static char med_digit_lump[9];
static const char* big_digit_lump_format;
static const char* med_digit_lump_format;
static const char* big_digit_percent_lump;
static int big_digit_spacing;
static int med_digit_spacing;
static int big_digit_percent_spacing;
static int big_digit_percent_width;

int dsda_HudComponentY(int y_offset, int vpt, double ratio) {
  int dsda_ExHudVerticalOffset(void);

  int y = 0;
  int vpt_align;

  if (ratio)
    y_offset = (int)(y_offset * ratio);

  vpt_align = vpt & VPT_ALIGN_MASK;
  if (BOTTOM_ALIGNMENT(vpt_align)) {
    y = 200;
    y_offset = -y_offset;

    y -= dsda_ExHudVerticalOffset();
  }

  return y + y_offset;
}

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (exhud_font.line_height != 8)
      ratio = (double) exhud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &exhud_font, CR_GRAY, vpt);
}

void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (hud_font.line_height != 8)
      ratio = (double) hud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &hud_font, CR_GRAY, vpt);
}

void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt) {
  int x, y;

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, 0);

  component->x = x;
  component->y = y;
  component->vpt = vpt;

  // Big numbers
  big_digit_lump_format = raven ? "FONTB%.1d" : "STTNUM%.1d";
  big_digit_spacing     = raven ? 12 : R_NumPatchWidth(W_GetNumForName("STTNUM0"));

  // Percent
  big_digit_percent_lump    = raven ? "FONTB05" : "STTPRCNT";
  big_digit_percent_width   = R_NumPatchWidth(W_GetNumForName(big_digit_percent_lump));
  big_digit_percent_spacing = raven ? 4 : 0;

  // Med numbers (raven)
  med_digit_lump_format = raven ? "IN%.1d" : "STTNUM%.1d";
  med_digit_spacing     = raven ? 8 : 0;
}

int P_ManaPercent(player_t *player, int mana)
{
  if (!player->ammo[mana])
    return 0;

  return player->ammo[mana] * 100 / MAX_MANA;
}

int dsda_ManaColorBig(player_t* player, int mana) {
  int mana_percent;

  mana_percent = P_ManaPercent(player, mana);

  if (mana_percent < hud_ammo_red)
    return dsda_tc_stbar_ammo_bad;
  else if (mana_percent < hud_ammo_yellow)
    return dsda_tc_stbar_ammo_warning;
  else if (mana_percent < 100)
    return dsda_tc_stbar_ammo_ok;
  else
    return dsda_tc_stbar_ammo_full;
}

int dsda_AmmoColorBig(player_t* player) {
  int ammo_percent;

  ammo_percent = P_AmmoPercent(player, player->readyweapon);

  if (ammo_percent < hud_ammo_red)
    return dsda_tc_stbar_ammo_bad;
  else if (ammo_percent < hud_ammo_yellow)
    return dsda_tc_stbar_ammo_warning;
  else if (ammo_percent < 100)
    return dsda_tc_stbar_ammo_ok;
  else
    return dsda_tc_stbar_ammo_full;
}

static void dsda_DrawBigPercent(int x, int y, int cm, int vpt) {
  extern int sts_pct_always_gray;
  int color = sts_pct_always_gray ? CR_GRAY : cm;
  int flags = (sts_colored_numbers || sts_pct_always_gray) ? VPT_COLOR : VPT_NONE;

  // Add extra space for raven
  x += big_digit_percent_spacing;

  if (raven)
    V_DrawShadowedNamePatch(x, y, big_digit_percent_lump, color, vpt | flags);
  else
    V_DrawNamePatch(x, y, big_digit_percent_lump, color, vpt | flags);
}

static void dsda_DrawBigDigit(int x, int y, int delta_x, int cm, int vpt, int digit) {
  int flags = sts_colored_numbers ? VPT_COLOR : VPT_NONE;

  if (digit > 9 || digit < 0)
    return;

  if (raven)
  {
    int lump;

    snprintf(big_digit_lump, sizeof(big_digit_lump), big_digit_lump_format, digit + 16);
    lump = W_GetNumForName(big_digit_lump);
    x += delta_x / 2 - R_NumPatchWidth(lump) / 2;
    V_DrawShadowedNamePatch(x, y, big_digit_lump, cm, vpt | flags);
  }
  else
  {
    snprintf(big_digit_lump, sizeof(big_digit_lump), big_digit_lump_format, digit);
    V_DrawNamePatch(x, y, big_digit_lump, cm, vpt | flags);
  }
}

static int digit_mod[6] = { 1, 10, 100, 1000, 10000, 100000 };
static int digit_div[6] = { 1,  1,  10,  100,  1000,  10000 };

void dsda_DrawBigNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align, int percent) {
  int i;
  int digit, any_digit;
  int delta_x = big_digit_spacing;

  if (count > 5)
    return;

  any_digit = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
    {
      dsda_DrawBigDigit(x, y, delta_x, cm, vpt, digit);
      x += delta_x;
    }
    else {
      if (right_align)
        x += delta_x;
    }
  }
  if (percent)
    dsda_DrawBigPercent(x, y, cm, vpt);
}

int dsda_GetBigNumberWidth(int count, int n, int right_align, int percent)
{
  int i;
  int digit;
  int any_digit;
  int width;
  int delta_x = big_digit_spacing;

  if (count > 5)
    return 0;

  any_digit = 0;
  width = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
      width += delta_x;
    else if (right_align)
      width += delta_x;
  }

  if (percent)
  {
    // Add extra space for raven
    width += big_digit_percent_width + big_digit_percent_spacing;
  }

  return width;
}

static void dsda_DrawMedDigit(int x, int y, int delta_x, int cm, int vpt, int digit) {
  int flags = sts_colored_numbers ? VPT_COLOR : VPT_NONE;

  if (!raven)
    return;

  if (digit > 9 || digit < 0)
    return;

  snprintf(med_digit_lump, sizeof(med_digit_lump), med_digit_lump_format, digit);
  V_DrawNamePatch(x, y, med_digit_lump, cm, vpt | flags);
}

void dsda_DrawMedNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align) {
  int i;
  int digit, any_digit;
  int delta_x = med_digit_spacing;

  if (!raven)
    return;

  if (count > 5)
    return;

  any_digit = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
    {
      dsda_DrawMedDigit(x, y, delta_x, cm, vpt, digit);
      x += delta_x;
    }
    else {
      if (right_align)
        x += delta_x;
    }
  }
}

int dsda_GetMedNumberWidth(int count, int n, int right_align)
{
  int i;
  int digit;
  int any_digit;
  int width;
  int delta_x = med_digit_spacing;

  if (!raven)
    return 0;

  if (count > 5)
    return 0;

  any_digit = 0;
  width = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
      width += delta_x;
    else if (right_align)
      width += delta_x;
  }

  return width;
}

void dsda_DrawBasicText(dsda_text_t* component) {
  HUlib_drawTextLine(&component->text, false, false, false);
}

void dsda_DrawYellowText(dsda_text_t* component, dboolean yellow) {
  HUlib_drawTextLine(&component->text, yellow, false, false);
}

void dsda_DrawBasicShadowedText(dsda_text_t* component) {
  HUlib_drawTextLine(&component->text, false, true, false);
}

void dsda_DrawYellowShadowedText(dsda_text_t* component, dboolean yellow) {
  HUlib_drawTextLine(&component->text, yellow, true, false);
}

void dsda_RefreshHudText(dsda_text_t* component) {
  const char* s;

  HUlib_clearTextLine(&component->text);

  s = component->msg;
  while (*s) HUlib_addCharToTextLine(&component->text, *(s++));
}

void dsda_RefreshHudTextWrapped(dsda_text_t* component, int centered, int max_lines)
{
  const char* s;

  HUlib_clearTextLine(&component->text);

  s = component->msg;
  HUlib_WrapStringToTextLines(&component->text, s, centered, max_lines);
}
