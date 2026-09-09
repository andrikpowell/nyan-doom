/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:  heads-up text and input code
 *
 *-----------------------------------------------------------------------------
 */

#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "r_draw.h"
#include "lprintf.h"

#include "dsda/configuration.h"
#include "dsda/stretch.h"
#include "dsda/hud_components/base.h"

#define HU_COLOR 0x30
#define HU_COLOR_ORIG   (HU_COLOR - 1)  // 47
#define HU_XOFF_MAX     (HU_COLOR - 2)  // 46

char HUlib_Color(int cm)
{
  return HU_COLOR + cm;
}

char HUlib_ColorReset(void)
{
  return HU_COLOR_ORIG;
}

static char hu_color_str[CR_LIMIT][3];
static char hu_orig_str[3];

void HU_InitColorStrings(void)
{
  int i;
  for (i = 0; i < CR_HUD_LIMIT; i++)
  {
    hu_color_str[i][0] = '\x1b';
    hu_color_str[i][1] = HUlib_Color(i); // HU_COLOR + i
    hu_color_str[i][2] = '\0';
  }
}

const char *HU_ColorReset(void)
{
  hu_orig_str[0] = '\x1b';
  hu_orig_str[1] = HU_COLOR_ORIG;
  hu_orig_str[2] = '\0';

  return hu_orig_str;
}

char* HU_ColorFromConfig(int config)
{
  int cm = dsda_IntConfig(config);
  if (cm >= CR_HUD_LIMIT) cm = CR_HUD_LIMIT - 1;
  return hu_color_str[cm];
}

char* HU_ColorFromValue(int cm)
{
  if (cm >= CR_HUD_LIMIT) cm = CR_HUD_LIMIT - 1;
  return hu_color_str[cm];
}

////////////////////////////////////////////////////////
//
// Get font string width
//
////////////////////////////////////////////////////////

int HU_FontStringWidth(const dsda_font_t* f, const char* string)
{
  int i, c, w = 0;
  for (i = 0; string[i]; i++)
    w += (c = toupper((unsigned char)string[i]) - HU_FONTSTART) < 0 || c >= HU_FONTSIZE ?
      f->space_width : f->font[c].width;
  return w;
}

////////////////////////////////////////////////////////
//
// Text Screen Boundaries
//
////////////////////////////////////////////////////////

static int HUlib_ScreenWidth(void)
{
  stretch_param_t *params = dsda_StretchParams(VPT_STRETCH);
  float screenwidth_sml = (SCREENWIDTH) * 320.0f / params->video->width;
  return (int)(screenwidth_sml + 0.5f); // round up
}

static int HUlib_ScreenPaddingEach(void)
{
  const float padding_total = 0.05f; // 5% total = 2.5% each side
  int screenwidth_sml = HUlib_ScreenWidth();
  int padding_each;

  // Remove padding from non-widescreen (4:3 - 320-wide)
  if (screenwidth_sml <= 320)
    return 0;

  padding_each = (int)(screenwidth_sml * (padding_total * 0.5f) + 0.5f);
  return padding_each;
}

static int HUlib_UsableWidth(void)
{
  int screenwidth_sml = HUlib_ScreenWidth();
  int padding = HUlib_ScreenPaddingEach() * 2;
  int usable = screenwidth_sml - padding;
  if (usable < 1) usable = 1;
  return usable;
}

////////////////////////////////////////////////////////
//
// Basic text line widget
//
////////////////////////////////////////////////////////

//
// HUlib_clearTextLine()
//
// Blank the internal text line in a hu_textline_t widget
//
// Passed a hu_textline_t, returns nothing
//
void HUlib_clearTextLine(hu_textline_t* t)
{
  t->linelen =         // killough 1/23 98: support multiple lines
    t->len = 0;
  t->l[0] = 0;
}

//
// HUlib_initTextLine()
//
// Initialize a hu_textline_t widget. Set the position, font, start char
// of the font, and color range to be used.
//
// Passed a hu_textline_t, and the values used to initialize
// Returns nothing
//
void HUlib_initTextLine(hu_textline_t* t, int x, int y,
      const dsda_font_t* f, int cm, enum patch_translation_e flags )
  //jff 2/16/98 add color range parameter
{
  t->x = x;
  t->y = y;
  t->f = f->font;
  t->fy = yellow_hud_font.font; // Hexen - Yellow font
  t->sc = f->start;
  t->cm = cm;
  t->flags = flags;
  t->line_height = f->line_height;
  t->space_width = f->space_width;
  t->kerning = f->kerning;
  t->fade_alpha = 100;
  t->alignment = HU_ALIGN_LEFT;
  HUlib_clearTextLine(t);
}

//
// HUlib_addCharToTextLine()
//
// Adds a character at the end of the text line in a hu_textline_t widget
//
// Passed the hu_textline_t and the char to add
// Returns false if already at length limit, true if the character added
//
dboolean HUlib_addCharToTextLine
( hu_textline_t*  t,
  char      ch )
{
  // killough 1/23/98 -- support multiple lines
  if (t->linelen == HU_MAXLINELENGTH)
    return false;
  else
  {
    t->linelen++;
    if (ch == '\n')
      t->linelen=0;

    t->l[t->len++] = ch;
    t->l[t->len] = 0;
    return true;
  }
}

////////////////////////////////////////////////////////
//
// Char + ESC stuff
//
////////////////////////////////////////////////////////

typedef enum
{
  HU_END,
  HU_NEWLINE,
  HU_TAB,
  HU_ESC,   // stuff like x-offset / color control
  HU_CHAR   // actual character (like "K")
} hu_chartype_t;

typedef struct
{
  hu_chartype_t type;
  unsigned char ch;
  unsigned char esc;
} hu_char_t;

typedef struct
{
  int has_newline;
  int rendered_lines;          // lines that would actually draw
  int last_line_width_px;      // ignores ESC sequences
  int singleline_max_right_px; // matches draw overflow test
} hu_charinfo_t;

// Set color
static dboolean HU_EscIsColor(unsigned char p)
{
  return (p >= HU_COLOR && p < HU_COLOR + CR_HUD_LIMIT);
}

// This is required for multi-coloured lines
// (to reset back to the main colour)
static dboolean HU_EscIsColorReset(unsigned char p)
{
  return (p == HU_COLOR_ORIG);
}

static dboolean HU_EscIsXOffset(unsigned char p)
{
  // signed for negative X offsets
  int v = (signed char)p;

  // 0 means "do nothing"
  if (v == 0)
    return false;

  // Ignore color reset
  if (HU_EscIsColorReset(p))
    return false;

  return (v >= -HU_XOFF_MAX && v <= HU_XOFF_MAX);
}

////////////////////////////////////////////////////////
//
// Text scan helpers
//
////////////////////////////////////////////////////////

static int HUlib_AdvanceX(const hu_textline_t *l, const patchnum_t *font, unsigned char ch)
{
  if (ch == ' ') return l->space_width;

  if (ch >= l->sc && ch <= 127)
    return font[ch - l->sc].width + l->kerning;

  return l->space_width;
}

static int HUlib_CharRightEdgePx(const hu_textline_t *l, const patchnum_t *font, int x, unsigned char ch)
{
  if (ch >= l->sc && ch <= 127)
    return x + font[ch - l->sc].width + l->kerning - font[ch - l->sc].leftoffset;

  // spaces / unknowns don't have leftoffset behavior
  return x + HUlib_AdvanceX(l, font, ch);
}

////////////////////////////////////////////////////////
//
// Text ESC Token System
//
////////////////////////////////////////////////////////

static hu_char_t HU_GetNextChar(const char *s, int len, int *i)
{
  hu_char_t nextch;
  unsigned char c;

  nextch.type = HU_END;
  nextch.ch = 0;
  nextch.esc = 0;

  if (*i >= len)
    return nextch;

  c = (unsigned char)s[*i];
  nextch.ch = (unsigned char)toupper(c);  //jff insure were not getting a cheap toupper conv.

  if (c == '\n')         // killough 1/18/98 -- support multiple lines
  {
    nextch.type = HU_NEWLINE;
    (*i)++;
    return nextch;
  }

  if (c == '\t')    // killough 1/23/98 -- support tab stops
  {
    nextch.type = HU_TAB;
    (*i)++;
    return nextch;
  }

  if (c=='\x1b')       //jff 2/17/98 escape code for color change
  {                    //jff 3/26/98 changed to actual escape char
    (*i)++;

    if (*i >= len)
    {
      nextch.type = HU_END;
      return nextch;
    }

    nextch.type = HU_ESC;
    nextch.esc = (unsigned char)s[*i];
    (*i)++;
    return nextch;
  }

  nextch.type = HU_CHAR;
  (*i)++;
  return nextch;
}

static void HUlib_GetCharInfo(const hu_textline_t *l, const patchnum_t *font, hu_charinfo_t *info)
{
  hu_chartype_t last_type = HU_END;
  int i = 0;
  int line_x = 0;
  int screen_x = l->x;
  int lines = 1;

  info->has_newline = 0;
  info->singleline_max_right_px = l->x;

  for (;;)
  {
    hu_char_t nextch = HU_GetNextChar(l->l, l->len, &i);
    int advance;

    if (nextch.type == HU_END)
      break;

    last_type = nextch.type;

    if (nextch.type == HU_NEWLINE)
    {
      info->has_newline = 1;
      lines++;
      line_x = 0;
      screen_x = l->x;
      continue;
    }

    if (nextch.type == HU_TAB)
    {
      line_x = line_x - line_x % 80 + 80;
      screen_x = screen_x - screen_x % 80 + 80;
    }
    else if (nextch.type == HU_ESC)
    {
      if (!HU_EscIsXOffset(nextch.esc))
        continue;

      advance = (signed char)nextch.esc;
      line_x += advance;
      screen_x += advance;
    }
    else
    {
      int right_edge = HUlib_CharRightEdgePx(l, font, screen_x, nextch.ch);

      if (right_edge > info->singleline_max_right_px)
        info->singleline_max_right_px = right_edge;

      advance = HUlib_AdvanceX(l, font, nextch.ch);
      line_x += advance;
      screen_x += advance;
    }

    if (screen_x > info->singleline_max_right_px)
      info->singleline_max_right_px = screen_x;
  }

  if (last_type == HU_NEWLINE)
    lines--;

  info->rendered_lines = MAX(lines, 1);
  info->last_line_width_px = line_x;
}

static int HUlib_LineStartX(const hu_textline_t *l, const char *line)
{
  int x = 0;
  int minx = 0;
  int maxx = 0;
  int i = 0;
  int len;
  dboolean any = false;

  if (l->alignment == HU_ALIGN_LEFT)
    return l->x;

  len = (int)strlen(line);

  for (;;)
  {
    hu_char_t nextch = HU_GetNextChar(line, len, &i);

    if (nextch.type == HU_END || nextch.type == HU_NEWLINE)
      break;

    // Alignment ignores ESC sequences
    if (nextch.type == HU_ESC)
      continue;

    // Preserve the original alignment tab behavior
    if (nextch.type == HU_TAB)
    {
      x += 80;
      continue;
    }

    if (nextch.ch == ' ')
    {
      x += l->space_width;
      continue;
    }

    if (nextch.ch >= l->sc && nextch.ch <= 127)
    {
      const patchnum_t *p = &l->f[nextch.ch - l->sc];
      int left = x - p->leftoffset;
      int right = left + p->width + l->kerning;

      if (!any)
      {
        minx = left;
        maxx = right;
        any = true;
      }
      else
      {
        minx = MIN(minx, left);
        maxx = MAX(maxx, right);
      }

      x += p->width + l->kerning;
    }
    else
    {
      x += l->space_width;
    }
  }

  if (!any)
    return l->x;

  if (l->alignment == HU_ALIGN_CENTER)
  {
    int indent = (HUlib_UsableWidth() - (maxx - minx)) / 2;
    return l->x + MAX(indent, 0);
  }

  return l->x - maxx;
}

////////////////////////////////////////////////////////
//
// Text line Ellipsis
//
////////////////////////////////////////////////////////

static void HUlib_Ellipsis_RecountLineLen(hu_textline_t *l)
{
  int i;

  l->linelen = 0;

  for (i = l->len - 1; i >= 0 && l->l[i] != '\n'; --i)
    l->linelen++;
}

#define NO_ELLIPSIS_TRIM 0

// Removes last visible character (and any dangling ESC pair) from the text buffer
static int HUlib_Ellipsis_TrimLastVisible(hu_textline_t *l, const patchnum_t *font)
{
  int ellipsis_trim_px;

  // Don't delete before buffer
  if (l->len <= 0)
    return NO_ELLIPSIS_TRIM;

  // Don't delete past a newline, ellipsis only applies to the last line
  if (l->l[l->len - 1] == '\n')
    return NO_ELLIPSIS_TRIM;

  // If last 2 bytes are ESC + param, remove both
  if (l->len >= 2 && l->l[l->len - 2] == '\x1b')
  {
    l->len -= 2;
    l->l[l->len] = '\0';
    HUlib_Ellipsis_RecountLineLen(l);
    return NO_ELLIPSIS_TRIM;
  }

  // Delete normal character
  ellipsis_trim_px = HUlib_AdvanceX(l, font, (unsigned char)toupper((unsigned char)l->l[l->len - 1]));

  l->len--;
  l->l[l->len] = '\0';
  HUlib_Ellipsis_RecountLineLen(l);

  return ellipsis_trim_px;
}

static void HUlib_AppendEllipsis(hu_textline_t *l, const patchnum_t *font, int max_px)
{
  int line_width_px;
  int dotw, dots;
  int cap;

  // Only strip trailing '\n' if we're truly on an empty last line
  while (l->len > 0 && l->l[l->len - 1] == '\n')
  {
    // If this newline is the only separator (example: title and author),
    // stripping it will collapse lines. So only strip repeated newlines
    if (l->len >= 2 && l->l[l->len - 2] != '\n')
      break;

    l->len--;
    l->l[l->len] = '\0';
  }

  HUlib_Ellipsis_RecountLineLen(l);

  // Compute current last-line width ONCE
  {
    hu_charinfo_t info;
    HUlib_GetCharInfo(l, font, &info);
    line_width_px = info.last_line_width_px;
  }

  dotw = HUlib_AdvanceX(l, font, '.');
  dots = 3 * dotw;

  // Ensure room for "..."
  cap = (int)sizeof(l->l);
  while (l->len + 3 >= cap && l->len > 0)
  {
    // If we trim an ESC pair removed==0, cur stays correct
    line_width_px -= HUlib_Ellipsis_TrimLastVisible(l, font);
    if (line_width_px < 0) line_width_px = 0;

    // If we hit a newline, stop
    if (l->len > 0 && l->l[l->len - 1] == '\n')
      break;
  }

  // Trim until the last line with "..." fits
  while (l->len > 0 && l->l[l->len - 1] != '\n' && line_width_px + dots > max_px)
  {
    line_width_px -= HUlib_Ellipsis_TrimLastVisible(l, font);
    if (line_width_px < 0) line_width_px = 0;
  }

  if (l->len + 3 < (int)sizeof(l->l))
  {
    l->l[l->len++] = '.';
    l->l[l->len++] = '.';
    l->l[l->len++] = '.';
    l->l[l->len] = '\0';
    l->linelen += 3;
  }
}
// Apply ellipsis "only" for single-line strings, by mutating the buffer.
static void HUlib_AppendEllipsis_SingleLine(hu_textline_t *l, const patchnum_t *font, int right)
{
  const int max_px = right - l->x;
  hu_charinfo_t info;

  // If at the right of screen, return
  if (max_px <= 0)
    return;

  HUlib_GetCharInfo(l, font, &info);

  // Skip if multiple lines.
  if (info.has_newline)
    return;

  // If it already fits to right of screen, do nothing.
  if (info.singleline_max_right_px <= right)
    return;

  // Mutate the buffer so "..." fits.
  HUlib_AppendEllipsis(l, font, max_px);
}

static void HUlib_ForceNextLine(const hu_textline_t* l, int *i, int *x, int *y)
{
  // Find newline
  while (*i < l->len && l->l[*i] != '\n')
    (*i)++;

  if (*i < l->len && l->l[*i] == '\n')
  {
    // Consume newline and move pen exactly once
    (*i)++;                 // now points to first char of next line
    *x = HUlib_LineStartX(l, l->l + *i);
    *y += l->line_height;
  }
  else
  {
    // No newline ahead - nothing else to draw
    *i = l->len;
  }
}

////////////////////////////////////////////////////////
//
// draw text line
//
////////////////////////////////////////////////////////

//
// HUlib_drawTextLine()
//
// Draws a hu_textline_t widget
//
// Passed the hu_textline_t and flag whether to draw a cursor
// Returns nothing
//
void HUlib_drawTextLine
( hu_textline_t* l,
  dboolean yellow,
  dboolean shadow,
  dboolean drawcursor )
{

  int     i;
  int     w;
  int     x;
  int     y; // killough 1/18/98 -- support multiple lines

  // Color stuff
  unsigned char c;
  int oc = l->cm; //jff 2/17/98 remember default color
  int base_cm = oc;
  dboolean base_set = false;
  int fade_alpha = l->fade_alpha;

  // Choose which font to use (Hexen Yellow Message)
  const patchnum_t* font = yellow ? l->fy : l->f;

  // Get right boundry for textwrap
  const int sw = HUlib_UsableWidth();
  const int right = l->x + sw;

  // Get char info
  hu_charinfo_t info;
  dboolean has_newline = false;
  dboolean allow_auto_ellipsis = false;

  // if any '\n' exists, treat as multi-line
  HUlib_GetCharInfo(l, font, &info);
  has_newline = info.has_newline;
  allow_auto_ellipsis = !has_newline;

  // If this is a single-line message and overflows, mutate the buffer once
  // to end with "..." that actually fits.
  //
  // Wrapped/multiline strings already contain '\n'
  // and should not get draw-time ellipsis.
  if (allow_auto_ellipsis)
    HUlib_AppendEllipsis_SingleLine(l, font, right);

  // fade stuff
  fade_alpha = CLAMP(fade_alpha, 0, 100);
  if (fade_alpha == 0) return;

  // draw the new stuff

  x = HUlib_LineStartX(l, l->l);
  y = l->y;
  i = 0;

  for (;;)
  {
    hu_char_t nextch = HU_GetNextChar(l->l, l->len, &i);
    int type = nextch.type;

    if (type == HU_END)
      break;

    if (type == HU_NEWLINE)
    {
      x = HUlib_LineStartX(l, l->l + i);
      y += l->line_height;
      continue;
    }

    if (type == HU_TAB)
    {
      x = x - x % 80 + 80;
      continue;
    }

    if (type == HU_ESC)
    {
      unsigned char p = nextch.esc;

      // Reset to base color
      if (HU_EscIsColorReset(p))
      {
        l->cm = base_cm;
        continue;
      }

      // Color change (allow multiple inline)
      if (HU_EscIsColor(p))
      {
        // First color code in the string becomes color "base"
        if (!base_set)
        {
          base_cm = p - HU_COLOR;
          base_set = true;
        }
        l->cm = p - HU_COLOR;
        continue;
      }

      // x-offset
      if (HU_EscIsXOffset(p))
      {
        x += (signed char)p;
      }

      continue;
    }

    // Draw character
    c = nextch.ch;

    if (c == ' ')
    {
      x += l->space_width;
      if (x >= right)
      {
        if (has_newline) // multi-line
        {
          HUlib_ForceNextLine(l, &i, &x, &y);
          continue;
        }
        break; // single-line
      }
      continue;
    }

    if (c >= l->sc && c <= 127)
    {
      w = font[c - l->sc].width + l->kerning;
      if (x + w - font[c - l->sc].leftoffset > right)
      {
        if (has_newline) // multi-line
        {
          HUlib_ForceNextLine(l, &i, &x, &y);
          continue;
        }
        break; // single-line
      }

      if (shadow)
        V_DrawMenuFadeNumPatch(x, y, font[c - l->sc].lumpnum, l->cm, fade_alpha, VPT_COLOR | l->flags);
      else
        V_DrawFadeNumPatch(x, y, font[c - l->sc].lumpnum, l->cm, fade_alpha, VPT_COLOR | l->flags);

      x += w;
      continue;
    }

    // unknown -> draw space
    x += l->space_width;

    if (x >= right)
    {
      if (has_newline) // multi-line
      {
        HUlib_ForceNextLine(l, &i, &x, &y);
        continue;
      }
      break; // single-line
    }
  }

  l->cm = oc;

  // draw the cursor if requested
  if (drawcursor)
  {
    int under_idx = '_' - l->sc;

    if (x + font[under_idx].width + l->kerning <= right)
    {
      // killough 1/18/98 -- support multiple lines
      // CPhipps - patch drawing updated
      if (shadow)
        V_DrawMenuNumPatch(x, y, font[under_idx].lumpnum, CR_DEFAULT, VPT_NONE | l->flags);
      else
        V_DrawNumPatch(x, y, font[under_idx].lumpnum, CR_DEFAULT, VPT_NONE | l->flags);
    }
  }
}

void HUlib_drawOffsetTextLine(hu_textline_t* l, dboolean yellow, dboolean shadow, int offset)
{
  int old_y;

  old_y = l->y;
  l->y += offset;
  HUlib_drawTextLine(l, yellow, shadow, false);
  l->y = old_y;
}

////////////////////////////////////////////////////////
//
// Centered text lines
//
////////////////////////////////////////////////////////


//
// HUlib_setTextXCenter()
//
// Centering a hu_textline_t
//
// Passed the hu_textline_t
// Returns nothing
//
void HUlib_setTextXCenter(hu_textline_t* t)
{
  const int full_w   = HUlib_ScreenWidth();
  const int pad_each = HUlib_ScreenPaddingEach();
  const int margin    = (full_w - BASE_WIDTH) / 2;
  const int base_left = (full_w > BASE_WIDTH) ? -margin : 0;

  // shift x position so extra width is split both sides
  t->x = base_left + pad_each;
  t->alignment = HU_ALIGN_CENTER;
}

////////////////////////////////////////////////////////
//
// Right aligned text
//
////////////////////////////////////////////////////////

void HUlib_setTextXRightAlign(hu_textline_t *t)
{
  t->alignment = HU_ALIGN_RIGHT;
}


////////////////////////////////////////////////////////
//
// Multiple wrapped text lines
//
////////////////////////////////////////////////////////

// Compute current line width from the last '\n' to end.
// Ignores ESC color sequences: '\x1b' + one byte.
static int HUlib_wrap_textWidthFromLineStart(const hu_textline_t *l)
{
  const patchnum_t *font = l->f;
  int i;
  int cur = 0;
  hu_char_t nextch;

  // start after last newline
  int start = 0;
  for (i = 0; i < l->len; i++)
    if (l->l[i] == '\n')
      start = i + 1;

  i = start;
  for (;;)
  {
    nextch = HU_GetNextChar(l->l, l->len, &i);
    if (nextch.type == HU_END || nextch.type == HU_NEWLINE)
      break;

    if (nextch.type == HU_TAB)
    {
      cur = cur - cur % 80 + 80;
      continue;
    }

    if (nextch.type == HU_ESC)
    {
      if (HU_EscIsXOffset(nextch.esc))
        cur += (signed char)nextch.esc;
      continue;
    }

    // HU_CHAR
    cur += HUlib_AdvanceX(l, font, nextch.ch);
  }

  return cur;
}

static int HUlib_wrap_wordMaxRightEdgePx(const hu_textline_t *l, const patchnum_t *font,
                                    const char *s, const char *end, int x_start)
{
  int x = x_start;
  int max_right = x_start;

  int i = 0;
  int len = (int)(end - s);
  if (len < 0) len = 0;

  for (;;)
  {
    hu_char_t nextch = HU_GetNextChar(s, len, &i);

    if (nextch.type == HU_END || nextch.type == HU_NEWLINE)
      break;

    if (nextch.type == HU_ESC)
    {
      if (HU_EscIsXOffset(nextch.esc))
      {
        x += (signed char)nextch.esc;
        if (x > max_right) max_right = x;
      }
      continue;
    }

    if (nextch.type == HU_TAB)
    {
      x = x - x % 80 + 80;
      if (x > max_right) max_right = x;
      continue;
    }

    // HU_CHAR
    {
      unsigned char c = nextch.ch; // already uppercased by HU_GetNextChar()

      if (c == ' ')
      {
        x += l->space_width;
        if (x > max_right) max_right = x;
        continue;
      }

      // normal char / unknown (CharRightEdgePx handles it via AdvanceX)
      {
        int right_edge = HUlib_CharRightEdgePx(l, font, x, c);
        if (right_edge > max_right) max_right = right_edge;
        x += HUlib_AdvanceX(l, font, c);
      }
    }
  }

  return max_right;
}

#define WRAP_ELLIPSIS(tag) do { \
  lprintf(LO_DEBUG, "WRAP_LOG(%s)\n", tag); \
  HUlib_AppendEllipsis(l, font, max_px); \
  return true; \
} while (0)

// Wrap-aware string append into a hu_textline_t.
// Returns true if it ellipsized early
dboolean HUlib_WrapStringToTextLines(hu_textline_t *l, const char *s, dboolean centered, int max_lines)
{
  const patchnum_t *font = l->f;
  const int usable = HUlib_UsableWidth();
  int max_px = usable - (centered ? 0 : l->x);
  int cur_px;
  int len;
  int i = 0;

  // leading newline padding should not count toward max_lines
  // count leading '\n' padding lines so they don't consume max_lines
  int pad_lines = 0;
  dboolean started_content = false;
  int lines_now = 1;

  if (max_lines <= 0) max_lines = 1;

  // Track total lines in-buffer
  {
    int i = 0;
    while (i < l->len)
    {
      hu_char_t nextch = HU_GetNextChar(l->l, l->len, &i);
      if (nextch.type == HU_END)
        break;
      if (nextch.type == HU_NEWLINE)
        lines_now++;
    }
  }

  // Track current line advance width
  cur_px = HUlib_wrap_textWidthFromLineStart(l);

  len = (int)strlen(s);

  while (i < len)
  {
    int content_lines = lines_now - pad_lines;
    int word_start;
    int word_end;
    int i_next;
    hu_char_t nextch;

    if (content_lines < 1)
      content_lines = 1;

    // Enforce limit ONLY on content lines (not padding)
    if (started_content && content_lines > max_lines)
      WRAP_ELLIPSIS("content_lines>max");

    i_next = i;
    nextch = HU_GetNextChar(s, len, &i_next);

    if (nextch.type == HU_END)
      break;

    // explicit newline
    if (nextch.type == HU_NEWLINE)
    {
      i = i_next;

      if (!started_content)
      {
        // Treat leading newlines as padding: keep them, but don't count them
        if (!HUlib_addCharToTextLine(l, '\n'))
          WRAP_ELLIPSIS("add_fail_leading_newline");

        pad_lines++;
        lines_now++;
        cur_px = 0;
        continue;
      }

      // After content starts, newline DOES count toward content_lines
      if (content_lines >= max_lines)
        WRAP_ELLIPSIS("newline_at_limit");

      if (!HUlib_addCharToTextLine(l, '\n'))
        WRAP_ELLIPSIS("add_fail_newline");

      lines_now++;
      cur_px = 0;
      continue;
    }

    // ESC (only XOffset affects cur_px)
    if (nextch.type == HU_ESC)
    {
      i = i_next;

      if (!HUlib_addCharToTextLine(l, '\x1b') || !HUlib_addCharToTextLine(l, (char)nextch.esc))
        WRAP_ELLIPSIS("add_fail_escape");

      if (HU_EscIsXOffset(nextch.esc))
        cur_px += (signed char)nextch.esc;

      continue;
    }

    // TAB
    if (nextch.type == HU_TAB)
    {
      i = i_next;

      if (!HUlib_addCharToTextLine(l, '\t'))
        WRAP_ELLIPSIS("add_fail_tab");

      cur_px = cur_px - cur_px % 80 + 80;
      continue;
    }

    // HU_CHAR
    if (nextch.type == HU_CHAR)
    {
      // Skip leading spaces on a line
      if (nextch.ch == ' ' && l->linelen == 0)
      {
        i = i_next;
        continue;
      }

      started_content = true;

      if (nextch.ch == ' ')
      {
        i = i_next;

        if (cur_px + l->space_width > max_px)
        {
          if (content_lines >= max_lines)
            WRAP_ELLIPSIS("space_wrap_at_limit");

          if (!HUlib_addCharToTextLine(l, '\n'))
            WRAP_ELLIPSIS("add_fail_wrap_newline");

          lines_now++;
          cur_px = 0;
        }
        else
        {
          if (!HUlib_addCharToTextLine(l, ' '))
            WRAP_ELLIPSIS("add_fail_space");

          cur_px += l->space_width;
        }

        continue;
      }

      // Else consume a "word"
      word_start = i;
      word_end = i;

      {
        int j = i;
        for (; j < len; )
        {
          int j_next = j;
          hu_char_t nextch = HU_GetNextChar(s, len, &j_next);

          if (nextch.type == HU_END || nextch.type == HU_NEWLINE || nextch.type == HU_TAB)
            break;

          if (nextch.type == HU_CHAR && nextch.ch == ' ')
            break;

          j = j_next;
          word_end = j;
        }
      }

      // wrap if needed before placing the word
      if (l->linelen != 0)
      {
        int max_right = HUlib_wrap_wordMaxRightEdgePx(l, font, s + word_start, s + word_end, cur_px);
        if (max_right > max_px)
        {
          if (content_lines >= max_lines)
            WRAP_ELLIPSIS("word_wrap_at_limit");

          if (!HUlib_addCharToTextLine(l, '\n'))
            WRAP_ELLIPSIS("add_fail_wrap_newline");

          lines_now++;
          cur_px = 0;
        }
      }

      // append the word
      {
        int wi = word_start;
        while (wi < word_end)
        {
          int wi_next = wi;
          hu_char_t nextch = HU_GetNextChar(s, len, &wi_next);

          if (nextch.type == HU_END || nextch.type == HU_NEWLINE)
            break;

          if (nextch.type == HU_ESC)
          {
            if (!HUlib_addCharToTextLine(l, '\x1b') || !HUlib_addCharToTextLine(l, (char)nextch.esc))
              WRAP_ELLIPSIS("add_fail_word_escape");

            if (HU_EscIsXOffset(nextch.esc))
              cur_px += (signed char)nextch.esc;

            wi = wi_next;
            continue;
          }

          if (nextch.type == HU_TAB)
          {
            if (!HUlib_addCharToTextLine(l, '\t'))
              WRAP_ELLIPSIS("add_fail_tab");

            cur_px = cur_px - cur_px % 80 + 80;

            wi = wi_next;
            continue;
          }

          if (nextch.type == HU_CHAR)
          {
            if (!HUlib_addCharToTextLine(l, (char)nextch.ch))
              WRAP_ELLIPSIS("add_fail_word");

            cur_px += HUlib_AdvanceX(l, font, nextch.ch);

            wi = wi_next;
            continue;
          }

          wi = wi_next;
        }
      }

      i = word_end;
      continue;
    }

    i = i_next;
  }

  return false;
}

////////////////////////////////////////////////////////
//
// Bottom Align wrapped text lines
//
////////////////////////////////////////////////////////

void HUlib_AdjustBottomOffset_MultiLine(hu_textline_t *t, int y_offset, double ratio, int vpt)
{
  int lines;
  hu_charinfo_t info;

  if (!BOTTOM_ALIGNMENT(t->flags & VPT_ALIGN_MASK))
    return;

  HUlib_GetCharInfo(t, t->f, &info);
  lines = info.rendered_lines;
  if (lines < 1) lines = 1;

  // Alter Y coordinate from the original offset each update
  t->y = dsda_HudComponentY(y_offset, vpt, ratio) - (lines - 1) * t->line_height;
}
