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

#define HU_COLOR 0x30

char HUlib_Color(int cm)
{
  return HU_COLOR + cm;
}

char* HU_ColorFromConfig(int config)
{
  static char buf[3];

  int cm = dsda_IntConfig(config);

  buf[0] = '\x1b';
  buf[1] = HUlib_Color(cm);
  buf[2] = '\0';
  return buf;
}

char* HU_ColorFromValue(int value)
{
  static char buf[3];

  buf[0] = '\x1b';
  buf[1] = HUlib_Color(value);
  buf[2] = '\0';
  return buf;
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
// text line Ellipsis
//
////////////////////////////////////////////////////////

static int HUlib_CharWidth(const hu_textline_t *l, const patchnum_t *font, unsigned char c)
{
  if (c == ' ') return l->space_width;
  if (c == '\t') return HU_MAXLINELENGTH;
  c = toupper(c);
  if (c >= l->sc && c <= 127) return font[c - l->sc].width + l->kerning;
  return l->space_width;
}

// width of the last line (after the last '\n'), ignoring ESC+param
static int HUlib_LastLineWidthPx(const hu_textline_t *l, const patchnum_t *font)
{
  int w = 0;
  int start = 0;

  for (int i = 0; i < l->len; i++)
    if (l->l[i] == '\n')
      start = i + 1;

  for (int i = start; i < l->len; i++)
  {
    unsigned char c = l->l[i];

    if (c == '\x1b')
    {
      if (i + 1 < l->len) i++; // skip param
      continue;
    }

    w += HUlib_CharWidth(l, font, c);
  }

  return w;
}

// Removes last visible character (and any dangling ESC pair) from the text buffer
static int HUlib_TrimLastVisible(hu_textline_t *l, const patchnum_t *font)
{
  if (l->len <= 0) return 0;

  // Don't delete past a newline; ellipsis only applies to the current last line
  if (l->l[l->len - 1] == '\n') return 0;

  // If last byte is an ESC param and the byte before it is ESC, remove both
  if (l->len >= 2 && l->l[l->len - 2] == '\x1b')
  {
    l->len -= 2;
    l->l[l->len] = '\0';
    l->linelen = 0;
    for (int i = l->len - 1; i >= 0 && l->l[i] != '\n'; --i) l->linelen++;
    return 0;
  }

  // Normal char
  {
    unsigned char c = l->l[l->len - 1];
    int removed = HUlib_CharWidth(l, font, c);

    l->len--;
    l->l[l->len] = '\0';

    l->linelen = 0;
    for (int i = l->len - 1; i >= 0 && l->l[i] != '\n'; --i) l->linelen++;

    return removed;
  }
}

static void HUlib_AppendEllipsis(hu_textline_t *l, const patchnum_t *font, int max_px, dboolean force)
{
  int cur, dotw, dots;
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

  cur = HUlib_LastLineWidthPx(l, font);
  dotw = HUlib_CharWidth(l, font, '.');
  dots = 3 * dotw;

  if (!force && cur <= max_px)
    return;

  // Ensure room for "..."
  cap = (int)sizeof(l->l);
  while (l->len + 3 >= cap && l->len > 0)
  {
    if (HUlib_TrimLastVisible(l, font) == 0 && l->len > 0 && l->l[l->len - 1] == '\n')
      break;
  }

  cur = HUlib_LastLineWidthPx(l, font);
  while (l->len > 0 && l->l[l->len - 1] != '\n' && cur + dots > max_px)
  {
    int removed = HUlib_TrimLastVisible(l, font);
    cur -= removed;
    if (cur < 0) cur = 0;
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

// Returns the maximum "right edge" reached by the line, in pixels,
// matching the draw overflow test: x + width - leftoffset.
static int HUlib_SingleLineMaxRightEdgePx(const hu_textline_t *l, const patchnum_t *font)
{
  int x = l->x;
  int max_right = x;

  for (int i = 0; i < l->len; i++)
  {
    unsigned char c = toupper(l->l[i]);

    if (c == '\n')
      break; // single-line only

    if (c == '\t')
    {
      x = x - x % 80 + 80;
      if (x > max_right) max_right = x;
      continue;
    }

    if (c == '\x1b') // ESC + param
    {
      if (++i < l->len)
      {
        unsigned char p = l->l[i];

        if (p >= HU_COLOR && p < HU_COLOR + CR_HUD_LIMIT)
        {
          // color change: no x change
        }
        else if (p < HU_COLOR)
        {
          // x-offset
          x += p;
          if (x > max_right) max_right = x;
        }
      }
      continue;
    }

    if (c == ' ')
    {
      x += l->space_width;
      if (x > max_right) max_right = x;
      continue;
    }

    if (c >= l->sc && c <= 127)
    {
      const patchnum_t *p = &font[c - l->sc];
      int right_edge = x + p->width + l->kerning - p->leftoffset;
      if (right_edge > max_right) max_right = right_edge;
      x += p->width + l->kerning;
      continue;
    }

    // unknown char -> treat as space
    x += l->space_width;
    if (x > max_right) max_right = x;
  }

  return max_right;
}

// Apply ellipsis "only" for single-line strings, by mutating the buffer.
static void HUlib_AutoEllipsisSingleLine(hu_textline_t *l, const patchnum_t *font, int right)
{
  const int max_px = right - l->x;

  if (max_px <= 0)
    return;

  // If it already fits, do nothing.
  if (HUlib_SingleLineMaxRightEdgePx(l, font) <= right)
    return;

  // Mutate the buffer so "..." fits.
  HUlib_AppendEllipsis(l, font, max_px, true);
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
  unsigned char c;
  int oc = l->cm; //jff 2/17/98 remember default color
  int y;          // killough 1/18/98 -- support multiple lines

  // Choose which font to use (Hexen Yellow Message)
  const patchnum_t* font = yellow ? l->fy : l->f;

  const int sw = HUlib_UsableWidth();
  const int right = l->x + sw;

  dboolean allow_auto_ellipsis = (memchr(l->l, '\n', l->len) == NULL);

  // If this is a single-line message and it would overflow, mutate the buffer once
  // to end with "..." that actually fits. Wrapped/multiline strings already contain '\n'
  // and should not get draw-time ellipsis.
  if (allow_auto_ellipsis)
    HUlib_AutoEllipsisSingleLine(l, font, right);

  // draw the new stuff

  x = l->x;
  y = l->y;
  for (i=0;i<l->len;i++)
  {
    c = toupper(l->l[i]); //jff insure were not getting a cheap toupper conv.

    if (c=='\n')         // killough 1/18/98 -- support multiple lines
    {
      x = l->x;
      y += l->line_height;
    }
    else if (c=='\t')    // killough 1/23/98 -- support tab stops
      x=x-x%80+80;
    else if (c=='\x1b')  //jff 2/17/98 escape code for color change
    {                    //jff 3/26/98 changed to actual escape char
      if (++i < l->len)
      {
        if (l->l[i] >= HU_COLOR && l->l[i] < HU_COLOR + CR_HUD_LIMIT)
          l->cm = l->l[i] - HU_COLOR;
        else if (l->l[i] < HU_COLOR)
          x += l->l[i];
      }
    }
    else  if (c != ' ' && c >= l->sc && c <= 127)
    {
      w = font[c - l->sc].width + l->kerning;
      if (x+w-font[c - l->sc].leftoffset > right)
        break;
      // killough 1/18/98 -- support multiple lines:
      // CPhipps - patch drawing updated
      if (shadow)
        V_DrawMenuNumPatch(x, y, font[c - l->sc].lumpnum, l->cm, VPT_COLOR | l->flags);
      else
        V_DrawNumPatch(x, y, font[c - l->sc].lumpnum, l->cm, VPT_COLOR | l->flags);
      x += w;
    }
    else
    {
      x += l->space_width;
      if (x >= right)
      break;
    }
  }
  l->cm = oc; //jff 2/17/98 restore original color

  // draw the cursor if requested
  if (drawcursor && x + font['_' - l->sc].width + l->kerning <= right)
  {
    // killough 1/18/98 -- support multiple lines
    // CPhipps - patch drawing updated
    if (shadow)
      V_DrawMenuNumPatch(x, y, font['_' - l->sc].lumpnum, CR_DEFAULT, VPT_NONE | l->flags);
    else
      V_DrawNumPatch(x, y, font['_' - l->sc].lumpnum, CR_DEFAULT, VPT_NONE | l->flags);
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

static int HUlib_LineWidthRaw(const hu_textline_t* l, const char* s)
{
  const patchnum_t* font = l->f;

  int x = 0;      // advance position
  int minx = 0;   // leftmost covered pixel relative to line start
  int maxx = 0;   // rightmost covered pixel relative to line start
  dboolean any = false;

  while (*s && *s != '\n')
  {
    unsigned char c = *s++;

    if (c == '\x1b')
    {
      if (*s) s++;
      continue;
    }

    if (c == ' ')
    {
      x += l->space_width;
      continue;
    }

    if (c == '\t')
    {
      x += 80;
      continue;
    }

    c = toupper(c);
    if (c >= l->sc && c <= 127)
    {
      const patchnum_t *p = &font[c - l->sc];

      // Glyph covers [x - leftoffset, x + width - leftoffset)
      int gx0 = x - p->leftoffset;
      int gx1 = x + p->width + l->kerning - p->leftoffset;

      if (!any)
      {
        minx = gx0;
        maxx = gx1;
        any = true;
      }
      else
      {
        if (gx0 < minx) minx = gx0;
        if (gx1 > maxx) maxx = gx1;
      }

      x += p->width + l->kerning;
    }
    else
    {
      x += l->space_width;
    }
  }

  if (!any) return 0;
  return maxx - minx;
}

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
  char outbuf[sizeof(t->l)];
  int out = 0;
  const char *p = t->l;

  const int full_w = HUlib_ScreenWidth();
  const int pad_each = HUlib_ScreenPaddingEach();
  const int usable_w = full_w - 2 * pad_each;

  const int margin = (full_w - BASE_WIDTH) / 2;
  const int base_left = (full_w > BASE_WIDTH) ? -margin : 0;

  // shift x position so extra width is split both sides
  t->x = base_left + pad_each;

  while (*p && out < (int)sizeof(outbuf) - 1)
  {
    int line_w = HUlib_LineWidthRaw(t, p);
    int indent = (usable_w - line_w) / 2;
    if (indent < 0) indent = 0;

    while (indent > 0 && out < (int)sizeof(((hu_textline_t*)0)->l) - 3)
    {
      int chunk = indent;
      if (chunk > HU_COLOR - 1) chunk = HU_COLOR - 1; // 47
      outbuf[(out)++] = '\x1b';
      outbuf[(out)++] = (char)chunk;
      indent -= chunk;
    }

    // Skip existing *position* ESC offsets that we previously injected
    // (but keep color changes: param >= HU_COLOR).
    while (p[0] == '\x1b' && p[1] && p[1] < HU_COLOR)
      p += 2;

    while (*p && *p != '\n' && out < (int)sizeof(outbuf) - 1)
      outbuf[out++] = *p++;

    if (*p == '\n' && out < (int)sizeof(outbuf) - 1)
      outbuf[out++] = *p++;
  }

  outbuf[out] = '\0';
  strncpy(t->l, outbuf, sizeof(t->l));
  t->l[sizeof(t->l) - 1] = '\0';
  t->len = (int)strlen(t->l);

  // best-effort linelen
  t->linelen = 0;
  for (int i = t->len - 1; i >= 0 && t->l[i] != '\n'; )
  {
    if (i >= 1 && t->l[i-1] == '\x1b') { i -= 2; continue; }
    t->linelen++;
    i--;
  }
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
  int w = 0;

  // start after last newline
  int start = 0;
  for (int i = 0; i < l->len; i++)
    if (l->l[i] == '\n')
      start = i + 1;

  for (int i = start; i < l->len; i++)
  {
    unsigned char c = l->l[i];

    if (c == '\x1b')
    {
      if (i + 1 < l->len)
        i++;
      continue;
    }
    if (c == ' ')
    {
      w += l->space_width;
      continue;
    }
    if (c == '\t')
    {
      w += HU_MAXLINELENGTH;
      continue;
    }

    c = toupper(c);
    if (c != ' ' && c >= l->sc && c <= 127)
      w += font[c - l->sc].width + l->kerning;
    else
      w += l->space_width;
  }

  return w;
}

static int HUlib_wrap_wordMaxRightEdgePx(const hu_textline_t *l, const patchnum_t *font,
                                    const char *s, const char *end, int x_start)
{
  int x = x_start;
  int max_right = x_start;

  while (s < end)
  {
    unsigned char c = *s++;

    if (c == '\x1b' && s < end) { s++; continue; } // skip ESC+param

    if (c == ' ')
    {
      x += l->space_width;
      if (x > max_right) max_right = x;
      continue;
    }

    if (c == '\t')
    {
      x += 80;
      if (x > max_right) max_right = x;
      continue;
    }

    c = toupper(c);
    if (c >= l->sc && c <= 127)
    {
      const patchnum_t *p = &font[c - l->sc];
      int right_edge = x + p->width + l->kerning - p->leftoffset;
      if (right_edge > max_right) max_right = right_edge;
      x += p->width + l->kerning;
    }
    else
    {
      x += l->space_width;
      if (x > max_right) max_right = x;
    }
  }

  return max_right;
}

static int HUlib_CountLines(const hu_textline_t *l)
{
  int lines = 1;
  for (int i = 0; i < l->len; i++)
    if (l->l[i] == '\n')
      lines++;
  return lines;
}

#define WRAP_ELLIPSIS(tag) do { \
  lprintf(LO_DEBUG, "WRAP_LOG(%s)\n", tag); \
  HUlib_AppendEllipsis(l, font, max_px, true); \
  return true; \
} while (0)

// Wrap-aware string append into a hu_textline_t.
dboolean HUlib_WrapStringToTextLines(hu_textline_t *l, const char *s, dboolean centered, int max_lines)
{
  const patchnum_t *font = l->f;
  const int usable = HUlib_UsableWidth();
  int max_px = usable - (centered ? 0 : l->x);
  const char *wptr;

  // leading newline padding should not count toward max_lines
  // count leading '\n' padding lines so they don't consume max_lines
  int pad_lines = 0;
  dboolean started_content = false;

  if (max_lines <= 0) max_lines = 1;

  while (*s)
  {
    int lines_now = HUlib_CountLines(l);
    int content_lines = lines_now - pad_lines;
  
    if (content_lines < 1) content_lines = 1;

    // Enforce limit ONLY on content lines (not padding)
    if (started_content && content_lines > max_lines)
    {
      WRAP_ELLIPSIS("content_lines>max");
    }

    // explicit newline
    if (*s == '\n')
    {
      if (!started_content)
      {
        // Treat leading newlines as padding: keep them, but don't count them
        if (!HUlib_addCharToTextLine(l, *s++))
        {
          WRAP_ELLIPSIS("add_fail_leading_newline");
        }
        pad_lines++;          // NEW
        continue;
      }

      // After content starts, newline DOES count toward content_lines
      if (content_lines >= max_lines)
      {
        WRAP_ELLIPSIS("newline_at_limit");
      }

      if (!HUlib_addCharToTextLine(l, *s++))
      {
        WRAP_ELLIPSIS("add_fail_newline");
      }
      continue;
    }

    // ESC sequences: copy through (still not "content" by itself)
    if (*s == '\x1b' && s[1])
    {
      if (!HUlib_addCharToTextLine(l, *s++) || !HUlib_addCharToTextLine(l, *s++))
      {
        WRAP_ELLIPSIS("add_fail_escape");
      }
      continue;
    }

    // skip leading spaces on a line
    if (*s == ' ' && l->linelen == 0)
    {
      s++;
      continue;
    }

    started_content = true;

    if (*s == ' ')
    {
      int cur = HUlib_wrap_textWidthFromLineStart(l);
      if (cur + l->space_width > max_px)
      {
        if (content_lines >= max_lines)
        {
          WRAP_ELLIPSIS("space_wrap_at_limit");
        }
        HUlib_addCharToTextLine(l, '\n');
      }
      else
      {
        if (!HUlib_addCharToTextLine(l, ' '))
        {
          WRAP_ELLIPSIS("add_fail_space");
        }
      }
      s++;
      continue;
    }

    // measure next word width
    wptr = s;
    while (*wptr && *wptr != ' ' && *wptr != '\n')
    {
      if (*wptr == '\x1b' && wptr[1]) { wptr += 2; continue; }
      wptr++;
    }

    // wrap if needed before placing the word
    if (l->linelen != 0)
    {
      int x_cur = HUlib_wrap_textWidthFromLineStart(l);
      int max_right = HUlib_wrap_wordMaxRightEdgePx(l, font, s, wptr, x_cur);

      if (l->linelen != 0 && max_right > max_px)
      {
        if (content_lines >= max_lines) WRAP_ELLIPSIS("word_wrap_at_limit");
        HUlib_addCharToTextLine(l, '\n');
      }
    }

    // append the word
    while (s < wptr)
    {
      if (*s == '\x1b' && s[1])
      {
        if (!HUlib_addCharToTextLine(l, *s++) || !HUlib_addCharToTextLine(l, *s++))
        {
          WRAP_ELLIPSIS("add_fail_word_escape");
        }
      }
      else
      {
        if (!HUlib_addCharToTextLine(l, *s++))
        {
          WRAP_ELLIPSIS("add_fail_word");
        }
      }
    }
  }

  return false;
}
