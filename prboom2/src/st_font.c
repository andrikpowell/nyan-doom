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
//	DSDA ST Fonts
//

#include "doomstat.h"
#include "w_wad.h"
#include "st_font.h"

int sts_colored_numbers;      //jff 2/18/98 control to disable status color changes
int sts_pct_always_gray; // killough 2/21/98: always gray %'s? bug or feature?

// Big Font
static char big_digit_lump[9];
static const char* big_digit_lump_format;
static int big_digit_spacing;

// Big Percent
static const char* big_digit_percent_lump;
static int big_digit_percent_spacing;
static int big_digit_percent_width;

// Big Negative
static const char* big_digit_negative_lump;

// Med Font (Raven)
static char med_digit_lump[9];
static const char* med_digit_lump_format;
static int med_digit_spacing;

// Small Font
static char sml_digit_lump[9];
static const char* sml_digit_lump_format;
static int sml_digit_spacing;

void dsda_InitPatchNumbers(void)
{
  // Big numbers
  big_digit_lump_format = raven ? "FONTB%.1d" : "STTNUM%.1d";
  big_digit_spacing     = raven ? 12 : R_NumPatchWidth(W_GetNumForName("STTNUM0"));

  // Percent
  big_digit_percent_lump    = raven ? "FONTB05" : "STTPRCNT";
  big_digit_percent_width   = R_NumPatchWidth(W_GetNumForName(big_digit_percent_lump));
  big_digit_percent_spacing = raven ? 4 : 0;

  // Negative
  big_digit_negative_lump   = raven ? "FONTB13" : "STTMINUS";

  // Med numbers (raven)
  med_digit_lump_format = raven ? "IN%.1d" : "STTNUM%.1d";
  med_digit_spacing     = raven ? 8 : 0;

  // Small numbers
  sml_digit_lump_format = raven ? "SMALLIN%.1d" : "STYSNUM%.1d";
  sml_digit_spacing     = raven ? 4 : R_NumPatchWidth(W_GetNumForName("STYSNUM0"));
}

static int digit_mod[6] = { 1, 10, 100, 1000, 10000, 100000 };
static int digit_div[6] = { 1,  1,  10,  100,  1000,  10000 };

static int dsda_BigNegativeNumberClamp(int count, int n)
{
  int min;

  if (n >= 0)
    return n;

  min = -(digit_mod[count - 1] - 1);

  if (n < min)
    return min;

  return n;
}

static void dsda_DrawDigitMinus(int x, int y, int delta_x, int cm, int vpt)
{
  int flags = sts_colored_numbers ? VPT_COLOR : VPT_NONE;
  int minus_x;

  minus_x = x - delta_x;
  minus_x += delta_x / 2 - R_NamePatchWidth(big_digit_negative_lump) / 2;

  V_DrawNamePatch(minus_x, y, big_digit_negative_lump, cm, vpt | flags);
}

static void dsda_DrawBigPercent(int x, int y, int cm, int vpt) {
  extern int sts_pct_always_gray;
  int color = sts_pct_always_gray ? CR_GRAY : cm;
  int flags = (sts_colored_numbers || sts_pct_always_gray) ? VPT_COLOR : VPT_NONE;
  int shadow = raven ? SHADOW_ALWAYS_RAVEN : SHADOW_OFF;

  // Add extra space for raven
  x += big_digit_percent_spacing;

  V_DrawShadowedNamePatchAdv(x, y, big_digit_percent_lump, shadow, color, vpt | flags);
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

void dsda_DrawBigNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align, int anchor, int percent) {
  int i;
  int digit, any_digit;
  int delta_x = big_digit_spacing;
  dboolean neg = false;
  int percent_x = x;

  if (count > 5)
    return;

  // Deal with negative numbers (Doom only)
  if (!raven && n < 0)
  {
    neg = true;
    n = dsda_BigNegativeNumberClamp(count, n);
    n = -n;
  }

  if (anchor)
  {
    int total_width = dsda_GetBigNumberWidth(count, neg ? -n : n, right_align, anchor, percent);

    if (anchor >= ANCHOR_RIGHT)
      x -= total_width;
    else if (anchor == ANCHOR_CENTER)
      x -= total_width / 2;
  }

  if (neg)
  {
    dsda_DrawDigitMinus(x + delta_x, y, delta_x, cm, vpt);
    x += delta_x;
  }

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
  {
    int px = (anchor == ANCHOR_PERCENT) ? percent_x : x;
    dsda_DrawBigPercent(px, y, cm, vpt);
  }
}

int dsda_GetBigNumberWidth(int count, int n, int right_align, int anchor, int percent)
{
  int i;
  int digit;
  int any_digit;
  int width;
  int delta_x = big_digit_spacing;
  dboolean neg = false;
  int percent_anchor = (anchor == ANCHOR_PERCENT);

  if (count > 5)
    return 0;

  // Deal with negative numbers (Doom only)
  if (!raven && n < 0)
  {
    neg = true;
    n = dsda_BigNegativeNumberClamp(count, n);
    n = -n;
  }

  any_digit = 0;
  width = 0;

  if (neg)
    width += delta_x;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
      width += delta_x;
    else if (right_align)
      width += delta_x;
  }

  if (percent && !percent_anchor)
  {
    // Add extra space for raven
    width += big_digit_percent_width + big_digit_percent_spacing;
  }

  return width;
}

static void dsda_DrawSmallDigit(int x, int y, int delta_x, int cm, int vpt, int digit) {
  int flags = sts_colored_numbers ? VPT_COLOR : VPT_NONE;

  if (digit > 9 || digit < 0)
    return;

  snprintf(sml_digit_lump, sizeof(sml_digit_lump), sml_digit_lump_format, digit);
  V_DrawNamePatch(x, y, sml_digit_lump, cm, vpt | flags);
}

int dsda_GetSmallNumberWidth(int count, int n, int right_align)
{
  int i;
  int digit;
  int any_digit;
  int width;
  int delta_x = sml_digit_spacing;
  dboolean neg = false;

  if (count > 5)
    return 0;

  // Deal with negative numbers (Doom only)
  if (!raven && n < 0)
  {
    neg = true;
    n = dsda_BigNegativeNumberClamp(count, n);
    n = -n;
  }

  any_digit = 0;
  width = 0;

  if (neg)
    width += delta_x;

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

void dsda_DrawSmallNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align, int anchor) {
  int i;
  int digit, any_digit;
  int delta_x = sml_digit_spacing;
  dboolean neg = false;

  if (count > 5)
    return;

  // Deal with negative numbers (Doom only)
  if (!raven && n < 0)
  {
    neg = true;
    n = dsda_BigNegativeNumberClamp(count, n);
    n = -n;
  }

  if (anchor)
  {
    int total_width = dsda_GetSmallNumberWidth(count, neg ? -n : n, right_align);

    if (anchor >= ANCHOR_RIGHT)
      x -= total_width;
    else if (anchor == ANCHOR_CENTER)
      x -= total_width / 2;
  }

  if (neg)
  {
    dsda_DrawDigitMinus(x + delta_x, y, delta_x, cm, vpt);
    x += delta_x;
  }

  any_digit = 0;

  for (i = count; i > 0; --i) {
    digit = (n % digit_mod[i]) / digit_div[i];
    any_digit |= digit;

    if (any_digit || i == 1)
    {
      dsda_DrawSmallDigit(x, y, delta_x, cm, vpt, digit);
      x += delta_x;
    }
    else {
      if (right_align)
        x += delta_x;
    }
  }
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

