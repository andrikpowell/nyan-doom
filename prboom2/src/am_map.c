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
 * DESCRIPTION:
 *   the automap code
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <float.h>

#include "gl_opengl.h"
#include "doomstat.h"
#include "st_stuff.h"
#include "r_main.h"
#include "p_setup.h"
#include "p_maputl.h"
#include "w_wad.h"
#include "v_video.h"
#include "p_spec.h"
#include "am_map.h"
#include "deh/strings.h"    // Ty 03/27/98 - externalizations
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf
#include "g_game.h"
#include "r_fps.h"
#include "smooth.h"
#include "m_misc.h"
#include "m_bbox.h"
#include "d_main.h"
#include "m_menu.h"

#include "dsda/id_list.h"
#include "dsda/input.h"
#include "dsda/map_format.h"
#include "dsda/messenger.h"
#include "dsda/pause.h"
#include "dsda/settings.h"
#include "dsda/skill_info.h"
#include "dsda/stretch.h"
#include "dsda/utility.h"

mapcolor_t mapcolor = {
  .plyr = { 112, 96, 64, 176 },
};

mapcolor_t mapcolor_heretic = {
  .plyr = { 220, 144, 150, 197 },
};

mapcolor_t mapcolor_hexen = {
  .plyr = { 157, 177, 137, 198, 215, 32, 106, 234 },
};

static mapcolor_t *mapcolor_p;

static void AM_SetColors(void)
{
  mapcolor_p = hexen   ? &mapcolor_hexen   :
               heretic ? &mapcolor_heretic :
                         &mapcolor;
}

typedef struct
{
  mpoint_t a, b;
} mline_t;

typedef struct
{
  int tag;
  fixed_t x, y;
  sector_t *sec;
  line_t *line;
  mline_t *connections;
  int connection_count;
  int connection_max;
} highlight_t;

static highlight_t highlight;

static int map_blinking_locks;
static int map_secret_after;
static int map_grid_size;
static int map_pan_speed;
static int map_scroll_speed;
static int map_wheel_zoom;
static int map_things_hitboxes;
static int map_opengl_nice_things;
int map_textured;
int map_use_multisampling;

static map_things_appearance_t map_things_appearance;

// drawing stuff
#define FB    0

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_SPEED  (dsda_InputActive(dsda_input_speed) ? !dsda_AutoRun() : dsda_AutoRun())
#define F_PANINC  (F_SPEED ? map_pan_speed * 2 : map_pan_speed)
#define F_ZOOMINC  (F_SPEED ? map_scroll_speed * 2 : map_scroll_speed)
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int) ((float)FRACUNIT * (1.00f + F_ZOOMINC / 200.0f)))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int) ((float)FRACUNIT / (1.00f + F_ZOOMINC / 200.0f)))

#define PLAYERRADIUS    (16*(1<<MAPBITS)) // e6y

// translates between frame-buffer and map distances
#define FTOM(x) FixedMul(((x)<<16),scale_ftom)
// e6y: int64 version to avoid overflows
//#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
#define MTOF(x) (fixed_t)((((int64_t)(x) * scale_mtof) >> FRACBITS)>>FRACBITS)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))
// precise versions
#define MTOF_F(x) (((float)(x)*scale_mtof)/(float)FRACUNIT/(float)FRACUNIT)
#define CXMTOF_F(x)  ((float)f_x + MTOF_F((x)-m_x))
#define CYMTOF_F(y)  ((float)f_y + (f_h - MTOF_F((y)-m_y)))

extern const char* g_autopage;
extern int g_autopage_width;
extern int g_autopage_height;

static int maplump_width;
static int maplump_height;
static short mapystart = 0;     // y-value for the start of the map bitmap...used in parallax stuff.
static short mapxstart = 0;     //x-value for the bitmap.
static short prev_mapxstart, prev_mapystart;

//
//  The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle. (Raven player is a blade)
//
static int numplyrlines;
static mline_t *player_arrow;

// Raven Player Arrow
#define R ((8*PLAYERRADIUS)/7)
mline_t raven_player_arrow[] = {
  { { -R+R/4, 0 }, { 0, 0} }, // center line.
  { { -R+R/4, R/8 }, { R, 0} }, // blade
  { { -R+R/4, -R/8 }, { R, 0 } },
  { { -R+R/4, -R/4 }, { -R+R/4, R/4 } }, // crosspiece
  { { -R+R/8, -R/4 }, { -R+R/8, R/4 } },
  { { -R+R/8, -R/4 }, { -R+R/4, -R/4} }, //crosspiece connectors
  { { -R+R/8, R/4 }, { -R+R/4, R/4} },
  { { -R-R/4, R/8 }, { -R-R/4, -R/8 } }, //pommel
  { { -R-R/4, R/8 }, { -R+R/8, R/8 } },
  { { -R-R/4, -R/8}, { -R+R/8, -R/8 } }
};
#undef R
#define RAVEN_NUMPLYRLINES (sizeof(raven_player_arrow)/sizeof(mline_t))

// Doom Player Arrow
#define R ((8*PLAYERRADIUS)/7)
mline_t doom_player_arrow[] =
{
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(doom_player_arrow)/sizeof(mline_t))

#define R ((8*PLAYERRADIUS)/7)

// Kex Player Arrow (4-line variant)
mline_t kex_player_arrow[] =
{
  { { -7*R/8, -49*R/80 }, {  R, 0 } }, // Two long diagonals create the point "V"
  { { -7*R/8,  49*R/80 }, {  R, 0 } },
  { { -7*R/8, -49*R/80 }, { -7*R/20, 0 } }, // Two shorter diagonals form the rear notch "v"
  { { -7*R/8,  49*R/80 }, { -7*R/20, 0 } },
};

#undef R
#define KEX_NUMPLYRLINES (sizeof(kex_player_arrow)/sizeof(mline_t))

//
//  A line drawing of the player pointing right,
//   starting from the middle. (Raven player is longer)
//
static int numcheatplyrlines;
static mline_t *cheat_player_arrow;

// Raven Cheat Arrow
#define R ((8*PLAYERRADIUS)/7)
mline_t raven_cheat_player_arrow[] = {
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/6 } },  // ----->
  { { R, 0 }, { R-R/2, -R/6 } },
  { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
  { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
  { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
  { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
  { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
  { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
  { { -R/6, -R/6 }, { 0, -R/6 } },
  { { 0, -R/6 }, { 0, R/4 } },
  { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
  { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
  { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
  };
#undef R
#define RAVEN_NUMCHEATPLYRLINES (sizeof(raven_cheat_player_arrow)/sizeof(mline_t))

// Doom Cheat Arrow
#define R ((8*PLAYERRADIUS)/7)
mline_t doom_cheat_player_arrow[] =
{ // killough 3/22/98: He's alive, Jim :)
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } },
  { { -R/10-R/6, R/4}, {-R/10-R/6, -R/4} },  // J
  { { -R/10-R/6, -R/4}, {-R/10-R/6-R/8, -R/4} },
  { { -R/10-R/6-R/8, -R/4}, {-R/10-R/6-R/8, -R/8} },
  { { -R/10, R/4}, {-R/10, -R/4}},           // F
  { { -R/10, R/4}, {-R/10+R/8, R/4}},
  { { -R/10+R/4, R/4}, {-R/10+R/4, -R/4}},   // F
  { { -R/10+R/4, R/4}, {-R/10+R/4+R/8, R/4}},
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(doom_cheat_player_arrow)/sizeof(mline_t))

// Doom Key line drawing (X shape)
//jff 1/5/98 new symbol for keys on automap
#define R (FRACUNIT)
mline_t cross_mark[] =
{
  { { -R, 0 }, { R, 0} },
  { { 0, -R }, { 0, R } },
};
#undef R
#define NUMCROSSMARKLINES (sizeof(cross_mark)/sizeof(mline_t))
//jff 1/5/98 end of new symbol

// Raven Key line drawing
#define R ((8*PLAYERRADIUS)/7)
mline_t raven_keysquare[] = {
	{ { 0, 0 }, { R/4, -R/2 } },
	{ { R/4, -R/2 }, { R/2, -R/2 } },
	{ { R/2, -R/2 }, { R/2, R/2 } },
	{ { R/2, R/2 }, { R/4, R/2 } },
	{ { R/4, R/2 }, { 0, 0 } }, // handle part type thing
	{ { 0, 0 }, { -R, 0 } }, // stem
	{ { -R, 0 }, { -R, -R/2 } }, // end lockpick part
	{ { -3*R/4, 0 }, { -3*R/4, -R/4 } }
	};
#undef R
#define RAVEN_NUMKEYSQUARELINES (sizeof(raven_keysquare)/sizeof(mline_t))

// Default triangle shape
#define R (FRACUNIT)
mline_t thintriangle_guy[] =
{
{ { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(    R), (fixed_t)(    0) } },
{ { (fixed_t)(    R), (fixed_t)(    0) }, { (fixed_t)(-.5*R), (fixed_t)( .7*R) } },
{ { (fixed_t)(-.5*R), (fixed_t)( .7*R) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))

// Box radius shape
#define R (FRACUNIT)
mline_t thingbox_guy[] =
{
{ { (fixed_t)(-R), (fixed_t)(-R) }, { (fixed_t)( R), (fixed_t)(-R) } }, // Top
{ { (fixed_t)( R), (fixed_t)(-R) }, { (fixed_t)( R), (fixed_t)( R) } }, // Right
{ { (fixed_t)( R), (fixed_t)( R) }, { (fixed_t)(-R), (fixed_t)( R) } }, // Bottom
{ { (fixed_t)(-R), (fixed_t)( R) }, { (fixed_t)(-R), (fixed_t)(-R) } }, // Left
{ { (fixed_t)(-R), (fixed_t)(-R) }, { (fixed_t)( R), (fixed_t)( R) } }, // "\"
{ { (fixed_t)(-R), (fixed_t)( R) }, { (fixed_t)( R), (fixed_t)(-R) } }, // "/"
};
#undef R
#define NUMTHINGBOXGUYLINES (sizeof(thingbox_guy)/sizeof(mline_t))

// Digit Segments
#define R (FRACUNIT)
#define SEG_T  { { -R/2,  R }, {  R/2,  R } } // top
#define SEG_B  { { -R/2, -R }, {  R/2, -R } } // bottom
#define SEG_M  { { -R/2,  0 }, {  R/2,  0 } } // middle
#define SEG_TL { { -R/2,  R }, { -R/2,  0 } } // top-left
#define SEG_BL { { -R/2,  0 }, { -R/2, -R } } // bottom-left
#define SEG_TR { {  R/2,  R }, {  R/2,  0 } } // top-right
#define SEG_BR { {  R/2,  0 }, {  R/2, -R } } // bottom-right

#define SEG_1_Top    { { -R/8,  R/2 }, {  R/4,  R } } // 1 - top  "/"
#define SEG_1_Middle { {  R/4,  R   }, {  R/4, -R } } // 1 - stem "|"
#define SEG_7_Slant  { {  R/2,  R   }, { -R/8, -R } } // 7 - diagonal "/"

// 7-seg Digits 0-9
mline_t am_digit_0[] = { SEG_T, SEG_B, SEG_TL, SEG_BL, SEG_TR, SEG_BR };
mline_t am_digit_1[] = { SEG_1_Middle, SEG_1_Top };
mline_t am_digit_2[] = { SEG_T, SEG_M, SEG_B, SEG_TR, SEG_BL };
mline_t am_digit_3[] = { SEG_T, SEG_M, SEG_B, SEG_TR, SEG_BR };
mline_t am_digit_4[] = { SEG_M, SEG_TL, SEG_TR, SEG_BR };
mline_t am_digit_5[] = { SEG_T, SEG_M, SEG_B, SEG_TL, SEG_BR };
mline_t am_digit_6[] = { SEG_T, SEG_M, SEG_B, SEG_TL, SEG_BL, SEG_BR };
mline_t am_digit_7[] = { SEG_T, SEG_7_Slant };
mline_t am_digit_8[] = { SEG_T, SEG_M, SEG_B, SEG_TL, SEG_BL, SEG_TR, SEG_BR };
mline_t am_digit_9[] = { SEG_T, SEG_M, SEG_B, SEG_TL, SEG_TR, SEG_BR };

mline_t *am_digits[] =
{
  am_digit_0, am_digit_1, am_digit_2, am_digit_3, am_digit_4,
  am_digit_5, am_digit_6, am_digit_7, am_digit_8, am_digit_9
};

const int am_digit_lines[] =
{
  sizeof(am_digit_0)/sizeof(mline_t),
  sizeof(am_digit_1)/sizeof(mline_t),
  sizeof(am_digit_2)/sizeof(mline_t),
  sizeof(am_digit_3)/sizeof(mline_t),
  sizeof(am_digit_4)/sizeof(mline_t),
  sizeof(am_digit_5)/sizeof(mline_t),
  sizeof(am_digit_6)/sizeof(mline_t),
  sizeof(am_digit_7)/sizeof(mline_t),
  sizeof(am_digit_8)/sizeof(mline_t),
  sizeof(am_digit_9)/sizeof(mline_t),
};

#undef SEG_T
#undef SEG_B
#undef SEG_M
#undef SEG_TL
#undef SEG_BL
#undef SEG_TR
#undef SEG_BR
#undef SEG_1_Top
#undef SEG_1_Middle
#undef SEG_7_Slant
#undef R

int automap_active;
int automap_overlay;
int automap_rotate;
int automap_follow;
int automap_grid;
int autopage;
int autopage_fade;
int autopage_parallax;

// location of window on screen
static int  f_x;
static int  f_y;

// size of window on screen
static int  f_w;
static int  f_h;

static mpoint_t m_paninc;    // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul; // how far the window zooms each tic (map coords)
static fixed_t ftom_zoommul; // how far the window zooms each tic (fb coords)
static fixed_t curr_mtof_zoommul;

static fixed_t m_x, m_y;     // LL x,y window location on the map (map coords)
static fixed_t m_x2, m_y2;   // UR x,y window location on the map (map coords)

static fixed_t prev_m_x, prev_m_y;

//
// width/height of window on map (map coords)
//
static fixed_t  m_w;
static fixed_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y;
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w;          // max_x-min_x,
static fixed_t  max_h;          // max_y-min_y

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;
static fixed_t prev_scale_mtof = (fixed_t)INITSCALEMTOF;

static player_t *plr;           // the player represented by an arrow
static vertex_t oldplr;

// killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.

markpoint_t *markpoints = NULL;    // where the points are
int markpointnum = 0; // next point to be assigned (also number of points now)
int markpointnum_max = 0;       // killough 2/22/98

typedef struct
{
  fixed_t x;
  fixed_t y;
} trailpoint_t;

#define TRAIL_SIZE 350 // ten seconds
static trailpoint_t player_trail[TRAIL_SIZE];
static int trail_index;
static int trail_size;
static int trail_collisions;
static int trail_size_max;

map_trail_mode_t map_trail_mode;

am_frame_t am_frame;

array_t map_lines;

static void AM_rotate(fixed_t* x,  fixed_t* y, angle_t a);

static void AM_SetMPointFloatValue(mpoint_t *p)
{
  if (am_frame.precise)
  {
    p->fx = (float)p->x;
    p->fy = (float)p->y;
  }
}

static void AM_SetFPointFloatValue(fpoint_t *p)
{
  p->fx = (float)p->x;
  p->fy = (float)p->y;
}

static dboolean stop_zooming;
static int zoom_leveltime;

static void AM_StopZooming(void)
{
  mtof_zoommul = FRACUNIT;
  ftom_zoommul = FRACUNIT;
  stop_zooming = false;
}

//
// AM_activateNewScale()
//
// Changes the map scale after zooming or translating
//
// Passed nothing, returns nothing
//
static void AM_activateNewScale(void)
{
  m_x += m_w/2;
  m_y += m_h/2;
  m_w = FTOM(f_w);
  m_h = FTOM(f_h);
  m_x -= m_w/2;
  m_y -= m_h/2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
// AM_saveScaleAndLoc()
//
// Saves the current center and zoom
// Affects the variables that remember old scale and loc
//
// Passed nothing, returns nothing
//
static void AM_saveScaleAndLoc(void)
{
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;
}

//
// AM_restoreScaleAndLoc()
//
// restores the center and zoom from locally saved values
// Affects global variables for location and scale
//
// Passed nothing, returns nothing
//
static void AM_restoreScaleAndLoc(void)
{
  m_w = old_m_w;
  m_h = old_m_h;
  if (!automap_follow)
  {
    m_x = old_m_x;
    m_y = old_m_y;
  }
  else
  {
    m_x = (viewx >> FRACTOMAPBITS) - m_w/2;//e6y
    m_y = (viewy >> FRACTOMAPBITS) - m_h/2;//e6y
  }
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;

  // Change the scaling multipliers
  scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

void AM_setMarkParams(int num)
{
  int i, namelen;
  char namebuf[16];

  snprintf(namebuf, sizeof(namebuf), "%s", !raven ? "AMMNUM0" : "SMALLIN0");
  namelen = !raven ? 6 : 7;

  markpoints[num].w = 0;
  markpoints[num].h = 0;

  snprintf(markpoints[num].label, sizeof(markpoints[num].label), "%d", num);
  for (i = 0; i < (int)strlen(markpoints[num].label); i++)
  {
    namebuf[namelen] = markpoints[num].label[i];
    markpoints[num].widths[i] = V_NamePatchWidth(namebuf);
    markpoints[num].w += markpoints[num].widths[i] + 1;
    markpoints[num].h = MAX(markpoints[num].h, V_NamePatchHeight(namebuf));
  }
}

//
// AM_addMark()
//
// Adds a marker at the current location
// Affects global variables for marked points
//
// Passed nothing, returns nothing
//
static void AM_addMark(void)
{
  // killough 2/22/98:
  // remove limit on automap marks

  if (markpointnum >= markpointnum_max)
    markpoints = Z_Realloc(markpoints,
                        (markpointnum_max = markpointnum_max ?
                         markpointnum_max*2 : 16) * sizeof(*markpoints));

  markpoints[markpointnum].x = m_x + m_w/2;
  markpoints[markpointnum].y = m_y + m_h/2;
  AM_setMarkParams(markpointnum);
  markpointnum++;
}

//
// AM_findMinMaxBoundaries()
//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
// Passed nothing, returns nothing
//
static void AM_findMinMaxBoundaries(void)
{
  int i;

  min_x = min_y =  INT_MAX;
  max_x = max_y = -INT_MAX;

  for (i=0;i<numvertexes;i++)
  {
    if (vertexes[i].x < min_x)
      min_x = vertexes[i].x;
    else if (vertexes[i].x > max_x)
      max_x = vertexes[i].x;

    if (vertexes[i].y < min_y)
      min_y = vertexes[i].y;
    else if (vertexes[i].y > max_y)
      max_y = vertexes[i].y;
  }

  max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);//e6y
  max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);//e6y
}

void AM_SetMapCenter(fixed_t x, fixed_t y)
{
  m_x = (x >> FRACTOMAPBITS) - m_w / 2;
  m_y = (y >> FRACTOMAPBITS) - m_h / 2;
  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

static void AM_UpdateParallax(void)
{
    dboolean minimap = !automap_active;

    int dmapx;
    int dmapy;

    if (automap_follow && !dsda_Paused())
    {
        dmapx = (MTOF(plr->mo->x) >> FRACTOMAPBITS) - (MTOF(oldplr.x) >> FRACTOMAPBITS);    //fixed point
        dmapy = (MTOF(oldplr.y) >> FRACTOMAPBITS) - (MTOF(plr->mo->y) >> FRACTOMAPBITS);

        oldplr.x = plr->mo->x;
        oldplr.y = plr->mo->y;

        if (!automap_rotate)
        {
          if (autopage_parallax && !minimap) // disable parallax on minimap (same reason above)
          {
            mapxstart += dmapx >> 1;
            mapystart += dmapy >> 1;
          }
        }
    }

    // Clamp parallax values
    mapxstart = (mapxstart % maplump_width + maplump_width) % maplump_width;
    mapystart = (mapystart % maplump_height + maplump_height) % maplump_height;
}

static void AM_ParallaxPan(fixed_t incx, fixed_t incy)
{
  dboolean minimap = !automap_active;

  // [crispy] Disable map background scroll in non-follow + rotate mode.
  // The combination of the two effects is unappealing and slightly
  // nauseating.
  if (!automap_rotate)
  {
    if (autopage_parallax && !minimap) // disable parallax on minimap (same reason above)
    {
      mapxstart += MTOF(incx) >> 1;
      mapystart -= MTOF(incy) >> 1;
    }
  }
}

//
// AM_changeWindowLoc()
//
// Moves the map window by the global variables m_paninc.x, m_paninc.y
//
// Passed nothing, returns nothing
//
static void AM_changeWindowLoc(void)
{
  fixed_t incx, incy;

  if (m_paninc.x || m_paninc.y)
  {
    dsda_UpdateIntConfig(dsda_config_automap_follow, false, true);
  }

  if (movement_smooth)
  {
    incx = FixedMul(m_paninc.x, tic_vars.frac);
    incy = FixedMul(m_paninc.y, tic_vars.frac);
  }
  else
  {
    incx = m_paninc.x;
    incy = m_paninc.y;
  }

  if (automap_rotate)
  {
    AM_rotate(&incx, &incy, viewangle - ANG90);
  }

  m_x = prev_m_x + incx;
  m_y = prev_m_y + incy;

  if (!automap_rotate)
  {
    if (m_x + m_w/2 > max_x)
    {
      m_x = max_x - m_w/2;
      incx = 0;
    }
    else if (m_x + m_w/2 < min_x)
    {
      m_x = min_x - m_w/2;
      incx = 0;
    }

    if (m_y + m_h/2 > max_y)
    {
      m_y = max_y - m_h/2;
      incy = 0;
    }
    else if (m_y + m_h/2 < min_y)
    {
      m_y = min_y - m_h/2;
      incy = 0;
    }
  }

  AM_ParallaxPan(incx, incy);

  m_x2 = m_x + m_w;
  m_y2 = m_y + m_h;
}

//
// AM_SetScale
//
static void AM_SetScale(void)
{
  {
    fixed_t a, b;
    fixed_t scale_w, scale_h;

    scale_w = SCREENWIDTH << FRACBITS;
    scale_h = (SCREENHEIGHT - ST_SCALED_HEIGHT) << FRACBITS;

    a = FixedDiv(scale_w, max_w);
    b = FixedDiv(scale_h, max_h);
    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(scale_h, 2 * PLAYERRADIUS);
    mapxstart = mapystart = 0;
  }

  scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7*FRACUNIT));
  if (scale_mtof > max_scale_mtof)
    scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// AM_SetPosition
//
static void AM_SetPosition(void)
{
  if (automap_active)
  {
    if (!R_StatusBarVisible())
    {
      f_x = 0;
      f_y = 0;
      f_w = SCREENWIDTH;
      f_h = SCREENHEIGHT;
    }
    else
    {
      //default
      f_x = f_y = 0;
      f_w = SCREENWIDTH;           // killough 2/7/98: get rid of finit_ vars
      f_h = SCREENHEIGHT-ST_SCALED_HEIGHT;// to allow runtime setting of width/height
    }
  }
  else if (dsda_ShowMinimap())
  {
    void dsda_CopyMinimapCoordinates(int* f_x, int* f_y, int* f_w, int* f_h);

    dsda_CopyMinimapCoordinates(&f_x, &f_y, &f_w, &f_h);
  }
}

void AM_initPlayerTrail(void)
{
  trail_index = -1;
  trail_size = 0;
  trail_size_max = dsda_IntConfig(dsda_config_map_trail_size);
  trail_collisions = dsda_IntConfig(dsda_config_map_trail_collisions) ? map_trail_mode_include_collisions : map_trail_mode_ignore_collisions;
  map_trail_mode = dsda_IntConfig(dsda_config_map_trail) ? trail_collisions : map_trail_mode_off;
}

void AM_SetPlayerArrow(void)
{
  switch (dsda_IntConfig(dsda_config_map_player_arrow))
  {
    case map_player_arrow_kex:
      numplyrlines = KEX_NUMPLYRLINES;
      player_arrow = kex_player_arrow;
      numcheatplyrlines = raven ? RAVEN_NUMCHEATPLYRLINES : NUMCHEATPLYRLINES;
      cheat_player_arrow = raven ? raven_cheat_player_arrow : doom_cheat_player_arrow;
      break;
    case map_player_arrow_doom:
      numplyrlines = NUMPLYRLINES;
      player_arrow = doom_player_arrow;
      numcheatplyrlines = NUMCHEATPLYRLINES;
      cheat_player_arrow = doom_cheat_player_arrow;
      break;
    case map_player_arrow_raven:
      numplyrlines = RAVEN_NUMPLYRLINES;
      player_arrow = raven_player_arrow;
      numcheatplyrlines = RAVEN_NUMCHEATPLYRLINES;
      cheat_player_arrow = raven_cheat_player_arrow;
      break;
    case map_player_arrow_default:
    default:
      numplyrlines = raven ? RAVEN_NUMPLYRLINES : NUMPLYRLINES;
      player_arrow = raven ? raven_player_arrow : doom_player_arrow;
      numcheatplyrlines = raven ? RAVEN_NUMCHEATPLYRLINES : NUMCHEATPLYRLINES;
      cheat_player_arrow = raven ? raven_cheat_player_arrow : doom_cheat_player_arrow;
      break;
  }
}

//
// AM_initVariables()
//
// Initialize the variables for the automap
//
// Affects the automap global variables
// Status bar is notified that the automap has been entered
// Passed nothing, returns nothing
//
static void AM_initVariables(void)
{
  int pnum;
  stretch_param_t *params = dsda_StretchParams(VPT_STRETCH);

  AM_initPlayerTrail();
  AM_SetPlayerArrow();

  m_paninc.x = m_paninc.y = 0;
  ftom_zoommul = FRACUNIT;
  mtof_zoommul = FRACUNIT;

  if (!W_LumpNameExists(g_autopage)) // Raven AUTOPAGE RAW format (custom size)
    g_autopage_width = g_autopage_height = 64;

  maplump_width = (g_autopage_width * params->video->width) / 320;
  maplump_height = (g_autopage_height * params->video->height) / 200;

  m_w = FTOM(f_w);
  m_h = FTOM(f_h);

  // find player to center on initially
  if (!playeringame[pnum = consoleplayer])
    for (pnum = 0; pnum < g_maxplayers; pnum++)
      if (playeringame[pnum])
        break;

  plr = &players[pnum];
  oldplr.x = plr->mo->x;
  oldplr.y = plr->mo->y;
  m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;//e6y
  m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;//e6y
  AM_Ticker();
  AM_changeWindowLoc();

  AM_SetColors();

  // for saving & restoring
  old_m_x = m_x;
  old_m_y = m_y;
  old_m_w = m_w;
  old_m_h = m_h;
}

void AM_SetResolution(void)
{
  AM_SetPosition();
  AM_SetScale();
}

static void AM_ResetTagHighlight(void)
{
  Z_Free(highlight.connections);
  ZERO_DATA(highlight);
  highlight.x = INT_MIN;
  highlight.y = INT_MIN;
}

//
// AM_clearMarks()
//
// Sets the number of marks to 0, thereby clearing them from the display
//
// Affects the global variable markpointnum
// Passed nothing, returns nothing
//
void AM_clearMarks(void)
{
  AM_initPlayerTrail();
  AM_ResetTagHighlight();

  markpointnum = 0;
}

// [Alaux] Clear just the last mark
static void AM_clearLastMark(void)
{
  AM_initPlayerTrail();
  AM_ResetTagHighlight();

  if (markpointnum)
    markpointnum--;
}

void AM_InitParams(void)
{
  map_blinking_locks = dsda_IntConfig(dsda_config_map_blinking_locks);
  map_secret_after = dsda_IntConfig(dsda_config_map_secret_after);
  map_pan_speed = dsda_IntConfig(dsda_config_map_pan_speed);
  map_scroll_speed = dsda_IntConfig(dsda_config_map_scroll_speed);
  map_grid_size = dsda_IntConfig(dsda_config_map_grid_size);
  map_wheel_zoom = dsda_IntConfig(dsda_config_map_wheel_zoom);
  map_things_appearance = dsda_IntConfig(dsda_config_map_things_appearance);
  map_things_hitboxes = dsda_IntConfig(dsda_config_map_things_hitbox);
  map_opengl_nice_things = dsda_IntConfig(dsda_config_map_things_nice);
}

static void AM_ExchangeScales(int full_automap, int *last_full_automap)
{
  static int full_min_scale_mtof;
  static int full_max_scale_mtof;
  static int full_scale_mtof;
  static int full_scale_ftom;

  if (*last_full_automap && !full_automap)
  {
    int dsda_MinimapScale(void);

    full_min_scale_mtof = min_scale_mtof;
    full_max_scale_mtof = max_scale_mtof;
    full_scale_mtof = scale_mtof;
    full_scale_ftom = scale_ftom;

    min_scale_mtof =
    max_scale_mtof =
    scale_mtof = FixedDiv(f_w << FRACBITS, dsda_MinimapScale() << MAPBITS);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  }
  else if (!*last_full_automap && full_automap)
  {
    min_scale_mtof = full_min_scale_mtof;
    max_scale_mtof = full_max_scale_mtof;
    scale_mtof = full_scale_mtof;
    scale_ftom = full_scale_ftom;
  }

  *last_full_automap = full_automap;
}

//
// AM_Stop()
//
// Cease automap operations, unload patches, notify status bar
//
// Passed nothing, returns nothing
//
void AM_Stop (dboolean minimap)
{
  automap_active = false;

  if (minimap && dsda_ShowMinimap())
    AM_Start(false);
}

//
// AM_Start()
//
// Start up automap operations,
//  if a new level, or game start, (re)initialize level variables
//  init map variables
//  load mark patches
//
// Passed nothing, returns nothing
//
void AM_Start(dboolean full_automap)
{
  static int lastlevel = -1, lastepisode = -1;
  static int last_full_automap;

  AM_InitParams();

  automap_active = full_automap;

  AM_SetPosition();

  if (lastlevel != gamemap || lastepisode != gameepisode)
  {
    AM_findMinMaxBoundaries();
    AM_SetScale();
    lastlevel = gamemap;
    lastepisode = gameepisode;
    last_full_automap = true;
  }

  AM_ExchangeScales(full_automap, &last_full_automap);

  AM_initVariables();
}

//
// AM_minOutWindowScale()
//
// Set the window scale to the maximum size
//
// Passed nothing, returns nothing
//
static void AM_minOutWindowScale(void)
{
  scale_mtof = min_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

//
// AM_maxOutWindowScale(void)
//
// Set the window scale to the minimum size
//
// Passed nothing, returns nothing
//
static void AM_maxOutWindowScale(void)
{
  scale_mtof = max_scale_mtof;
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
  AM_activateNewScale();
}

static line_t *AM_ClosestLine(fixed_t x, fixed_t y, sector_t *sec)
{
  int i;
  line_t *closest_line = NULL;
  double closest_distance = DBL_MAX;

  for (i = 0; i < sec->linecount; ++i)
  {
    line_t *line;
    double dist;

    line = sec->lines[i];
    dist = dsda_DistancePointToLine(
      line->v1->x >> FRACTOMAPBITS, line->v1->y >> FRACTOMAPBITS,
      line->v2->x >> FRACTOMAPBITS, line->v2->y >> FRACTOMAPBITS,
      x, y
    );

    if (dist < closest_distance)
    {
      closest_line = line;
      closest_distance = dist;
    }
  }

  return closest_line;
}

static void AM_AddHighlightConnection(mpoint_t a, mpoint_t b)
{
  if (!highlight.connection_max)
  {
    highlight.connection_max = 4;
    highlight.connections =
      Z_Realloc(highlight.connections,
                highlight.connection_max * sizeof(*highlight.connections));
  }

  if (highlight.connection_count == highlight.connection_max)
  {
    highlight.connection_max *= 2;
    highlight.connections =
      Z_Realloc(highlight.connections,
                highlight.connection_max * sizeof(*highlight.connections));
  }

  highlight.connections[highlight.connection_count].a = a;
  highlight.connections[highlight.connection_count].b = b;
  ++highlight.connection_count;
}

static void AM_HighlightByTag(void)
{
  fixed_t x, y;
  sector_t *sec;
  line_t *line;
  dboolean repeat;

  x = m_x + m_w / 2;
  y = m_y + m_h / 2;

  repeat = (x == highlight.x && y == highlight.y);
  highlight.x = x;
  highlight.y = y;

  sec = R_PointInSector(x << FRACTOMAPBITS, y << FRACTOMAPBITS);
  line = AM_ClosestLine(x, y, sec);

  if (!repeat || (!highlight.sec && !highlight.line))
  {
    highlight.sec = sec;
    highlight.line = NULL;
    highlight.tag = sec->tag;

    doom_printf("Highlight sector %d, tag %d\n", highlight.sec->iSectorID, sec->tag);
  }
  else if (highlight.sec)
  {
    highlight.sec = NULL;
    highlight.line = line;
    highlight.tag = line->tag;

    doom_printf("Highlight line %d, tag %d\n", highlight.line->iLineID, line->tag);
  }
  else
  {
    highlight.line = NULL;
    highlight.tag = 0;

    doom_printf("Highlight nothing\n");
  }

  Z_Free(highlight.connections);
  highlight.connections = NULL;
  highlight.connection_count = 0;
  highlight.connection_max = 0;

  if (highlight.tag)
  {
    const int *id_p;
    mpoint_t origin;
    mpoint_t destination;

    if (highlight.line)
    {
      sector_t *sec;

      R_LineCenter(&origin.x, &origin.y, highlight.line);
      origin.x >>= FRACTOMAPBITS;
      origin.y >>= FRACTOMAPBITS;

      FIND_SECTORS(id_p, highlight.tag)
      {
        sec = &sectors[*id_p];

        R_SectorCenter(&destination.x, &destination.y, sec);
        destination.x >>= FRACTOMAPBITS;
        destination.y >>= FRACTOMAPBITS;

        AM_AddHighlightConnection(origin, destination);
      }
    }
    else
    {
      line_t *line;

      R_SectorCenter(&origin.x, &origin.y, highlight.sec);
      origin.x >>= FRACTOMAPBITS;
      origin.y >>= FRACTOMAPBITS;

      FIND_LINES(id_p, highlight.tag)
      {
        line = &lines[*id_p];

        R_LineCenter(&destination.x, &destination.y, line);
        destination.x >>= FRACTOMAPBITS;
        destination.y >>= FRACTOMAPBITS;

        AM_AddHighlightConnection(origin, destination);
      }
    }
  }
}

//
// AM_Responder()
//
// Handle events (user inputs) in automap mode
//
// Passed an input event, returns true if its handled
//
dboolean AM_Responder
( event_t*  ev )
{
  static int bigstate=0;

  // Extract overlay toggle to allow when minimap is visible
  if (dsda_InputActivated(dsda_input_map_overlay) && (automap_input || dsda_ShowMinimap()))
  {
    dsda_CycleConfig(dsda_config_automap_overlay, true);
    dsda_AddMessage(automap_overlay == 0 ? s_AMSTR_OVERLAYOFF :
                    automap_overlay == 1 ? s_AMSTR_OVERLAYON :
                    "Overlay Mode Dark");
    AM_SetPosition();
    AM_activateNewScale();

    return true;
  }

  if (!automap_input)
  {
    if (dsda_InputActivated(dsda_input_map))
    {
      AM_Start(true);
      return true;
    }
  }
  else if (dsda_InputActivated(dsda_input_map))
  {
    bigstate = 0;
    AM_Stop(true);

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_right))
  {
    if (!automap_follow)
    {
      m_paninc.x = FTOM(F_PANINC);
      return true;
    }
  }
  else if (dsda_InputActivated(dsda_input_map_left))
  {
    if (!automap_follow)
    {
      m_paninc.x = -FTOM(F_PANINC);
      return true;
    }
  }
  else if (dsda_InputDeactivated(dsda_input_map_right))
  {
    if (!automap_follow)
      m_paninc.x = 0;
  }
  else if (dsda_InputDeactivated(dsda_input_map_left))
  {
    if (!automap_follow)
      m_paninc.x = 0;
  }
  else if (dsda_InputActivated(dsda_input_map_up))
  {
    if (!automap_follow)
    {
      m_paninc.y = FTOM(F_PANINC);
      return true;
    }
  }
  else if (dsda_InputActivated(dsda_input_map_down))
  {
    if (!automap_follow)
    {
      m_paninc.y = -FTOM(F_PANINC);
      return true;
    }
  }
  else if (dsda_InputDeactivated(dsda_input_map_up))
  {
    if (!automap_follow)
      m_paninc.y = 0;
  }
  else if (dsda_InputDeactivated(dsda_input_map_down))
  {
    if (!automap_follow)
      m_paninc.y = 0;
  }
  else if (
    dsda_InputActivated(dsda_input_map_zoomout) ||
    (map_wheel_zoom && ev->type == ev_keydown && ev->data1.i == KEYD_MWHEELDOWN)
  )
  {
    mtof_zoommul = M_ZOOMOUT;
    ftom_zoommul = M_ZOOMIN;
    curr_mtof_zoommul = mtof_zoommul;
    zoom_leveltime = leveltime;

    return true;
  }
  else if (
    dsda_InputActivated(dsda_input_map_zoomin) ||
    (map_wheel_zoom && ev->type == ev_keydown && ev->data1.i == KEYD_MWHEELUP)
  )
  {
    mtof_zoommul = M_ZOOMIN;
    ftom_zoommul = M_ZOOMOUT;
    curr_mtof_zoommul = mtof_zoommul;
    zoom_leveltime = leveltime;

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_gobig))
  {
    bigstate = !bigstate;
    if (bigstate)
    {
      AM_saveScaleAndLoc();
      AM_minOutWindowScale();
    }
    else
      AM_restoreScaleAndLoc();

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_follow))
  {
    dsda_ToggleConfig(dsda_config_automap_follow, true);
    dsda_AddMessage(automap_follow ? g_am_followon : g_am_followoff);

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_grid))
  {
    dsda_ToggleConfig(dsda_config_automap_grid, true);
    dsda_AddMessage(automap_grid ? g_am_gridon : g_am_gridoff);

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_mark))
  {
    /* Ty 03/27/98 - *not* externalized
     * cph 2001/11/20 - use doom_printf so we don't have our own buffer */
    doom_printf("%s %d", s_AMSTR_MARKEDSPOT, markpointnum);
    AM_addMark();

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_clear))
  {
    // [Alaux] Clear just the last mark
    if (markpointnum)
      AM_clearLastMark();

    if (markpointnum)
      doom_printf("Cleared spot %d", markpointnum);
    else
      dsda_AddMessage(s_AMSTR_MARKSCLEARED);

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_rotate))
  {
    dsda_ToggleConfig(dsda_config_automap_rotate, true);
    dsda_AddMessage(automap_rotate ? s_AMSTR_ROTATEON : s_AMSTR_ROTATEOFF);

    return true;
  }
  else if (dsda_InputActivated(dsda_input_map_textured))
  {
    dsda_ToggleConfig(dsda_config_map_textured, true);
    dsda_AddMessage(map_textured ? s_AMSTR_TEXTUREDON : s_AMSTR_TEXTUREDOFF);

    return true;
  }
  else if (
    dsda_InputDeactivated(dsda_input_map_zoomout) ||
    dsda_InputDeactivated(dsda_input_map_zoomin) ||
    (
      map_wheel_zoom && ev->type == ev_keyup &&
      (ev->data1.i == KEYD_MWHEELDOWN || ev->data1.i == KEYD_MWHEELUP)
    )
  )
  {
    stop_zooming = true;

    if (leveltime != zoom_leveltime)
      AM_StopZooming();
  }
  else if (dsda_InputActivated(dsda_input_map_highlight_by_tag))
  {
    if (!dsda_RevealAutomap())
      doom_printf("Highlight requires iddt");
    else
      AM_HighlightByTag();

    return true;
  }

  return false;
}

//
// AM_rotate()
//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
// Passed the coordinates of a point, and an angle
// Returns the coordinates rotated by the angle
//
// CPhipps - made static & enhanced for automap rotation

static void AM_rotate(fixed_t* x,  fixed_t* y, angle_t a)
{
  fixed_t tmpx;

  tmpx = FixedMul(*x, finecosine[a>>ANGLETOFINESHIFT]) -
    FixedMul(*y, finesine[a>>ANGLETOFINESHIFT]);

  *y = FixedMul(*x, finesine[a>>ANGLETOFINESHIFT]) +
    FixedMul(*y, finecosine[a>>ANGLETOFINESHIFT]);

  *x = tmpx;
}

static void AM_rotatePoint(mpoint_t *p)
{
  fixed_t tmpx;

  if (am_frame.precise)
  {
    float f;

    p->fx = (float)p->x - am_frame.centerx_f;
    p->fy = (float)p->y - am_frame.centery_f;

    f     = (p->fx * am_frame.cos_f) - (p->fy * am_frame.sin_f) + am_frame.centerx_f;
    p->fy = (p->fx * am_frame.sin_f) + (p->fy * am_frame.cos_f) + am_frame.centery_f;
    p->fx = f;
  }

  p->x -= am_frame.centerx;
  p->y -= am_frame.centery;

  tmpx = FixedMul(p->x, am_frame.cos) - FixedMul(p->y, am_frame.sin) + am_frame.centerx;
  p->y = FixedMul(p->x, am_frame.sin) + FixedMul(p->y, am_frame.cos) + am_frame.centery;
  p->x = tmpx;
}

//
// AM_changeWindowScale()
//
// Automap zooming
//
// Passed nothing, returns nothing
//
static void AM_changeWindowScale(void)
{
  if (movement_smooth)
  {
    float f_zoominc = (float)F_ZOOMINC / (float)FRACUNIT * (float)tic_vars.frac;

    if (f_zoominc < 0.01f)
      f_zoominc = 0.01f;

    scale_mtof = prev_scale_mtof;
    if (curr_mtof_zoommul == M_ZOOMIN)
    {
      mtof_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_zoominc / 200.0f)));
      ftom_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_zoominc / 200.0f)));
    }
    if (curr_mtof_zoommul == M_ZOOMOUT)
    {
      mtof_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_zoominc / 200.0f)));
      ftom_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_zoominc / 200.0f)));
    }
  }

  scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
  scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

  if (scale_mtof < min_scale_mtof)
    AM_minOutWindowScale();
  else if (scale_mtof > max_scale_mtof)
    AM_maxOutWindowScale();
  else
    AM_activateNewScale();
}

//
// AM_doFollowPlayer()
//
// Turn on follow mode - the map scrolls opposite to player motion
//
// Passed nothing, returns nothing
//
static void AM_doFollowPlayer(void)
{
  AM_SetMapCenter(viewx, viewy);
}

//
// AM_Ticker()
//
// Updates on gametic - enter follow mode, zoom, or change map location
//
// Passed nothing, returns nothing
//
void AM_Ticker (void)
{
  prev_scale_mtof = scale_mtof;
  prev_m_x = m_x;
  prev_m_y = m_y;
  prev_mapxstart = mapxstart;
  prev_mapystart = mapystart;

  if (stop_zooming && leveltime - zoom_leveltime != 1)
    AM_StopZooming();
}

//
// AM_clipMline()
//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes. If the speed is needed,
// use a hash algorithm to handle the common cases.
//
// Passed the line's coordinates on map and in the frame buffer performs
// clipping on them in the lines frame coordinates.
// Returns true if any part of line was not clipped
//
static dboolean AM_clipMline
( mline_t*  ml,
  fline_t*  fl )
{
  enum
  {
    LEFT    =1,
    RIGHT   =2,
    BOTTOM  =4,
    TOP     =8
  };

  register int outcode1 = 0;
  register int outcode2 = 0;
  register int outside;

  fpoint_t  tmp;
  int   dx;
  int   dy;
  float dx_f, dy_f;


#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < f_y) (oc) |= TOP; \
  else if ((my) >= f_y + f_h) (oc) |= BOTTOM; \
  if ((mx) < f_x) (oc) |= LEFT; \
  else if ((mx) >= f_x + f_w) (oc) |= RIGHT;


  // do trivial rejects and outcodes
  if (ml->a.y > m_y2)
  outcode1 = TOP;
  else if (ml->a.y < m_y)
  outcode1 = BOTTOM;

  if (ml->b.y > m_y2)
  outcode2 = TOP;
  else if (ml->b.y < m_y)
  outcode2 = BOTTOM;

  if (outcode1 & outcode2)
  return false; // trivially outside

  if (ml->a.x < m_x)
  outcode1 |= LEFT;
  else if (ml->a.x > m_x2)
  outcode1 |= RIGHT;

  if (ml->b.x < m_x)
  outcode2 |= LEFT;
  else if (ml->b.x > m_x2)
  outcode2 |= RIGHT;

  if (outcode1 & outcode2)
  return false; // trivially outside

  // transform to frame-buffer coordinates.
  fl->a.x = CXMTOF(ml->a.x);
  fl->a.y = CYMTOF(ml->a.y);
  fl->b.x = CXMTOF(ml->b.x);
  fl->b.y = CYMTOF(ml->b.y);

  DOOUTCODE(outcode1, fl->a.x, fl->a.y);
  DOOUTCODE(outcode2, fl->b.x, fl->b.y);

  if (outcode1 & outcode2)
  return false;

  if (am_frame.precise)
  {
    fl->a.fx = CXMTOF_F(ml->a.fx);
    fl->a.fy = CYMTOF_F(ml->a.fy);
    fl->b.fx = CXMTOF_F(ml->b.fx);
    fl->b.fy = CYMTOF_F(ml->b.fy);
  }

  while (outcode1 | outcode2)
  {
    // may be partially inside box
    // find an outside point
    if (outcode1)
      outside = outcode1;
    else
      outside = outcode2;

    // clip to each side
    if (outside & TOP)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      // 'int64' math to avoid overflows on long lines
      tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y))/dy);
      tmp.y = f_y;
      if (am_frame.precise)
      {
        dy_f = fl->a.fy - fl->b.fy;
        dx_f = fl->b.fx - fl->a.fx;
        tmp.fx = fl->a.fx + (dx_f*(fl->a.fy-f_y))/dy_f;
        tmp.fy = (float)f_y;
      }
    }
    else if (outside & BOTTOM)
    {
      dy = fl->a.y - fl->b.y;
      dx = fl->b.x - fl->a.x;
      tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h)))/dy);
      tmp.y = f_y+f_h-1;
      if (am_frame.precise)
      {
        dy_f = fl->a.fy - fl->b.fy;
        dx_f = fl->b.fx - fl->a.fx;
        tmp.fx = fl->a.fx + (dx_f*(fl->a.fy-(f_y+f_h)))/dy_f;
        tmp.fy = (float)(f_y+f_h-1);
      }
    }
    else if (outside & RIGHT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x))/dx);
      tmp.x = f_x+f_w-1;
      if (am_frame.precise)
      {
        dy_f = fl->b.fy - fl->a.fy;
        dx_f = fl->b.fx - fl->a.fx;
        tmp.fy = fl->a.fy + (dy_f*(f_x+f_w-1 - fl->a.fx))/dx_f;
        tmp.fx = (float)(f_x+f_w-1);
      }
    }
    else if (outside & LEFT)
    {
      dy = fl->b.y - fl->a.y;
      dx = fl->b.x - fl->a.x;
      tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x))/dx);
      tmp.x = f_x;
      if (am_frame.precise)
      {
        dy_f = fl->b.fy - fl->a.fy;
        dx_f = fl->b.fx - fl->a.fx;
        tmp.fy = fl->a.fy + (dy_f*(f_x-fl->a.fx))/dx_f;
        tmp.fx = (float)f_x;
      }
    }

    if (outside == outcode1)
    {
      fl->a = tmp;
      DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    }
    else
    {
      fl->b = tmp;
      DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    }

    if (outcode1 & outcode2)
      return false; // trivially outside
  }

  return true;
}
#undef DOOUTCODE


//
// AM_GetLineWeight()
//
// Get weight for automap lines.
//

int AM_GetLineWeight(void) {
  int thickness = dsda_IntConfig(dsda_config_automap_linesize);

  // if auto setting
  if (!thickness)
  {
    thickness = round(SCREENHEIGHT / 640);
    if (thickness < 1) thickness = 1;
    if (thickness > 6) thickness = 6;
  }

  // Minimap should be just 1px
  if (!automap_active)
    thickness = 1;

  // else return the linesize
  return thickness;
}

//
// AM_drawMline()
//
// Clip lines, draw visible parts of lines.
//
// Passed the map coordinates of the line, and the color to draw it
// Color -1 is special and prevents drawing. Color 247 is special and
// is translated to black, allowing Color 0 to represent feature disable
// in the defaults file.
// Returns nothing.
//
static void AM_drawMline
( mline_t*  ml,
  int   color )
{
  static fline_t fl;

  if (color==-1)  // jff 4/3/98 allow not drawing any sort of line
    return;       // by setting its color to -1
  if (color==247) // jff 4/3/98 if color is 247 (xparent), use black
    color=0;

  if (AM_clipMline(ml, &fl))
  {
    // draws it on frame buffer using fb coords
    if (!raven && map_use_multisampling)
      V_DrawLineWu(&fl, color);
    else
      V_DrawLine(&fl, color);
  }
}

//
// AM_drawGrid()
//
// Draws blockmap aligned grid lines.
//
// Passed the color to draw the grid lines
// Returns nothing
//
static void AM_drawGrid(int color)
{
  fixed_t x, y;
  fixed_t start, end;
  mline_t ml;
  fixed_t minlen, extx, exty;
  fixed_t minx, miny;
  fixed_t gridsize = map_grid_size << MAPBITS;

  // [RH] Calculate a minimum for how long the grid lines should be so that
  // they cover the screen at any rotation.
  minlen = M_DoubleToInt(sqrt ((float)m_w*(float)m_w + (float)m_h*(float)m_h));
  extx = (minlen - m_w) / 2;
  exty = (minlen - m_h) / 2;

  minx = m_x;
  miny = m_y;

  // Fix vanilla automap grid bug: losing grid lines near the map boundary
  // due to unnecessary addition of MAPBLOCKUNITS to start
  // Proper math is to just subtract the remainder; AM_drawMLine will take care
  // of clipping if an extra line is offscreen.

  // Figure out start of vertical gridlines
  start = minx - extx;
  if ((start - (bmaporgx>>FRACTOMAPBITS)) % gridsize)
    start -= ((start - (bmaporgx>>FRACTOMAPBITS)) % gridsize);
  end = minx + minlen - extx;

  // draw vertical gridlines
  for (x = start; x < end; x += gridsize)
  {
    ml.a.x = x;
    ml.b.x = x;
    ml.a.y = miny - exty;
    ml.b.y = ml.a.y + minlen;
    if (automap_rotate)
    {
      AM_rotatePoint (&ml.a);
      AM_rotatePoint (&ml.b);
    }
    else
    {
      AM_SetMPointFloatValue(&ml.a);
      AM_SetMPointFloatValue(&ml.b);
    }
    AM_drawMline(&ml, color);
  }

  // Figure out start of horizontal gridlines
  start = miny - exty;
  if ((start - (bmaporgy>>FRACTOMAPBITS)) % gridsize)
    start -= ((start - (bmaporgy>>FRACTOMAPBITS)) % gridsize);
  end = miny + minlen - exty;

  // draw horizontal gridlines
  for (y = start; y < end; y += gridsize)
  {
    ml.a.x = minx - extx;
    ml.b.x = ml.a.x + minlen;
    ml.a.y = y;
    ml.b.y = y;
    if (automap_rotate)
    {
      AM_rotatePoint (&ml.a);
      AM_rotatePoint (&ml.b);
    }
    else
    {
      AM_SetMPointFloatValue(&ml.a);
      AM_SetMPointFloatValue(&ml.b);
    }
    AM_drawMline(&ml, color);
  }
}

static dboolean AM_DrawHiddenSecrets(void)
{
  return !!mapcolor_p->secr && !map_secret_after;
}

static dboolean AM_DrawRevealedSecrets(void)
{
  return !!mapcolor_p->revsecr;
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
// jff 1/5/98 many changes in this routine
// backward compatibility not needed, so just changes, no ifs
// addition of clauses for:
//    doors opening, keyed door id, secret sectors,
//    teleports, exit lines, key things
// ability to suppress any of added features or lines with no height changes
//
// support for gamma correction in automap abandoned
//
// jff 4/3/98 changed mapcolor_xxxx=0 as control to disable feature
// jff 4/3/98 changed mapcolor_xxxx=-1 to disable drawing line completely
//

static automap_style_t AM_wallStyle(int i)
{
  switch (lines[i].automap_style)
  {
    case ams_default:
      break;

    // These styles have no corresponding colors in nyan-doom
    case ams_extra_floor:
    case ams_portal:
    case ams_special:
      if (!lines[i].backsector)
        return ams_one_sided;
      return ams_two_sided;

    default:
      return lines[i].automap_style;
  }

  // if line has been seen or IDDT has been used
  if (dsda_RevealAutomap() || (lines[i].flags & ML_MAPPED))
  {
    if ((lines[i].flags & ML_DONTDRAW) && !dsda_RevealAutomap())
      return ams_invisible;

    if (
      (mapcolor_p->bdor || mapcolor_p->ydor || mapcolor_p->rdor) &&
      !(lines[i].flags & ML_SECRET) && dsda_DoorType(i) != -1
    )
      return ams_locked;

    if (mapcolor_p->exitsecr && (dsda_IsSecretExitLine(i) && !dsda_IsExitLine(i)))
      return ams_exit_secret;

    if (mapcolor_p->exit && (dsda_IsExitLine(i) || dsda_IsSecretExitLine(i)))
      return ams_exit;

    if (mapcolor_p->exitsecr && (dsda_IsDeathSecretExitLine(i) && !dsda_IsDeathExitLine(i)))
      return ams_exit_secret;

    if (mapcolor_p->exit && (dsda_IsDeathExitLine(i) || dsda_IsDeathSecretExitLine(i)))
      return ams_exit;

    if (!lines[i].backsector) // 1-sided
    {
      if (AM_DrawHiddenSecrets() && P_IsSecret(lines[i].frontsector))
        return ams_secret;
      else if (AM_DrawRevealedSecrets() && P_RevealedSecret(lines[i].frontsector))
        return ams_revealed_secret;
      else
        return ams_one_sided;
    }
    else // 2-sided
    {
      if (mapcolor_p->tele && !(lines[i].flags & ML_SECRET) && dsda_IsTeleportLine(i))
      {
        return ams_teleport;
      }
      else if (lines[i].flags & ML_SECRET)
      {
        return ams_one_sided;
      }
      else if (
        mapcolor_p->clsd &&
        !(lines[i].flags & ML_SECRET) &&
        ((lines[i].backsector->floorheight==lines[i].backsector->ceilingheight) ||
        (lines[i].frontsector->floorheight==lines[i].frontsector->ceilingheight))
      )
      {
        return ams_closed_door;
      }
      else if (
        AM_DrawHiddenSecrets() &&
        (P_IsSecret(lines[i].frontsector) || P_IsSecret(lines[i].backsector))
      )
      {
        return ams_secret;
      }
      else if (
        AM_DrawRevealedSecrets() &&
        (P_RevealedSecret(lines[i].frontsector) || P_RevealedSecret(lines[i].backsector))
      )
      {
        return ams_revealed_secret;
      }
      else if (
        (mapcolor_p->exitsecr && !mapcolor_p->exit) &&
        (P_IsDeathExit(lines[i].frontsector) || P_IsDeathExit(lines[i].backsector))
      )
      {
        return ams_exit_secret;
      }
      else if (
        (mapcolor_p->exit || mapcolor_p->exitsecr) &&
        (P_IsDeathExit(lines[i].frontsector) || P_IsDeathExit(lines[i].backsector))
      )
      {
        return ams_exit;
      }
      else if (lines[i].backsector->floorheight !=
                lines[i].frontsector->floorheight)
      {
        return ams_floor_diff;
      }
      else if (lines[i].backsector->ceilingheight !=
                lines[i].frontsector->ceilingheight)
      {
        return ams_ceiling_diff;
      }
      else if (mapcolor_p->flat && dsda_RevealAutomap())
      {
        return ams_two_sided;
      }
    }
  }
  else if (plr->powers[pw_allmap] || (lines[i].flags & ML_REVEALED))
  {
    if (!(lines[i].flags & ML_DONTDRAW))
    {
      if
      (
        mapcolor_p->flat ||
        !lines[i].backsector ||
        lines[i].backsector->floorheight != lines[i].frontsector->floorheight ||
        lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight
      )
        return ams_unseen;
    }
  }

  return ams_invisible;
}

static void AM_drawWalls(void)
{
  int i;
  automap_style_t automap_style;
  static mline_t l;
  int hide_locks;

  hide_locks = map_blinking_locks && (gametic & 16);

  // draw the unclipped visible portions of all lines
  for (i=0;i<numlines;i++)
  {
    if (lines[i].bbox[BOXLEFT] >> FRACTOMAPBITS > am_frame.bbox[BOXRIGHT] ||
      lines[i].bbox[BOXRIGHT] >> FRACTOMAPBITS < am_frame.bbox[BOXLEFT] ||
      lines[i].bbox[BOXBOTTOM] >> FRACTOMAPBITS > am_frame.bbox[BOXTOP] ||
      lines[i].bbox[BOXTOP] >> FRACTOMAPBITS < am_frame.bbox[BOXBOTTOM])
    {
      continue;
    }

    l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
    l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
    l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
    l.b.y = lines[i].v2->y >> FRACTOMAPBITS;

    if (automap_rotate)
    {
      AM_rotatePoint(&l.a);
      AM_rotatePoint(&l.b);
    }
    else
    {
      AM_SetMPointFloatValue(&l.a);
      AM_SetMPointFloatValue(&l.b);
    }

    automap_style = AM_wallStyle(i);

    switch (automap_style)
    {
      case ams_invisible:
        continue;

      case ams_locked:
        if (hide_locks)
        {
          AM_drawMline(&l, mapcolor_p->grid);
          continue;
        }

        switch (dsda_DoorType(i))
        {
          case 0: // red
            AM_drawMline(&l, mapcolor_p->rdor? mapcolor_p->rdor : mapcolor_p->cchg);
            continue;
          case 1: // blue
            AM_drawMline(&l, mapcolor_p->bdor? mapcolor_p->bdor : mapcolor_p->cchg);
            continue;
          case 2: // yellow
            AM_drawMline(&l, mapcolor_p->ydor? mapcolor_p->ydor : mapcolor_p->cchg);
            continue;
          default:
            AM_drawMline(&l, mapcolor_p->clsd? mapcolor_p->clsd : mapcolor_p->cchg);
            continue;
        }

      case ams_exit:
        AM_drawMline(&l, mapcolor_p->exit);
        continue;

      case ams_exit_secret:
        AM_drawMline(&l, mapcolor_p->exitsecr);
        continue;

      case ams_one_sided:
        AM_drawMline(&l, mapcolor_p->wall);
        continue;

      case ams_secret:
      case ams_unseen_secret:
        AM_drawMline(&l, mapcolor_p->secr);
        continue;

      case ams_revealed_secret:
        AM_drawMline(&l, mapcolor_p->revsecr);
        continue;

      case ams_teleport:
        AM_drawMline(&l, mapcolor_p->tele);
        continue;

      case ams_closed_door:
        AM_drawMline(&l, mapcolor_p->clsd);
        continue;

      case ams_floor_diff:
        AM_drawMline(&l, mapcolor_p->fchg);
        continue;

      case ams_ceiling_diff:
        AM_drawMline(&l, mapcolor_p->cchg);
        continue;

      case ams_two_sided:
        AM_drawMline(&l, mapcolor_p->flat);
        continue;

      case ams_unseen:
        AM_drawMline(&l, mapcolor_p->unsn);
        continue;

      default:
        continue;
    }
  }
}

static void AM_DrawConnections(void)
{
  int i;
  mline_t l;

  if (!dsda_RevealAutomap())
    return;

  for (i = 0; i < highlight.connection_count; ++i)
  {
    l = highlight.connections[i];

    if (automap_rotate)
    {
      AM_rotatePoint(&l.a);
      AM_rotatePoint(&l.b);
    }
    else
    {
      AM_SetMPointFloatValue(&l.a);
      AM_SetMPointFloatValue(&l.b);
    }

    AM_drawMline(&l, mapcolor_p->tagfinder);
  }
}

//
// AM_drawLineCharacter()
//
// Draws a vector graphic according to numerous parameters
//
// Passed the structure defining the vector graphic shape, the number
// of vectors in it, the scale to draw it at, the angle to draw it at,
// the color to draw it with, and the map coordinates to draw it at.
// Returns nothing
//
static void AM_drawLineCharacter
( mline_t*  lineguy,
  int   lineguylines,
  fixed_t scale,
  fixed_t box_scale,
  angle_t angle,
  int   color,
  fixed_t x,
  fixed_t y )
{
  int   i;
  mline_t l;

  if (box_scale)
    AM_drawLineCharacter(thingbox_guy, NUMTHINGBOXGUYLINES, box_scale, 0, 0x40000000, mapcolor_p->hitbox, x, y);

  if (automap_rotate) angle -= viewangle - ANG90; // cph

  for (i=0;i<lineguylines;i++)
  {
    l.a.x = lineguy[i].a.x;
    l.a.y = lineguy[i].a.y;

    if (scale)
    {
      l.a.x = FixedMul(scale, l.a.x);
      l.a.y = FixedMul(scale, l.a.y);
    }

    if (angle)
      AM_rotate(&l.a.x, &l.a.y, angle);

    l.a.x += x;
    l.a.y += y;

    l.b.x = lineguy[i].b.x;
    l.b.y = lineguy[i].b.y;

    if (scale)
    {
      l.b.x = FixedMul(scale, l.b.x);
      l.b.y = FixedMul(scale, l.b.y);
    }

    if (angle)
      AM_rotate(&l.b.x, &l.b.y, angle);

    l.b.x += x;
    l.b.y += y;

    l.a.fx = (float)l.a.x;
    l.a.fy = (float)l.a.y;
    l.b.fx = (float)l.b.x;
    l.b.fy = (float)l.b.y;

    AM_drawMline(&l, color);
  }
}

INLINE static void AM_GetMobjPosition(mobj_t *mo, mpoint_t *p, angle_t *angle)
{
  if (R_ViewInterpolation())
  {
    p->x = mo->PrevX + FixedMul(tic_vars.frac, mo->x - mo->PrevX);
    p->y = mo->PrevY + FixedMul(tic_vars.frac, mo->y - mo->PrevY);
    if (mo->player)
      *angle = mo->player->prev_viewangle + FixedMul(tic_vars.frac, R_SmoothPlaying_Get(mo->player) - mo->player->prev_viewangle);
    else
      *angle = mo->angle;
  }
  else
  {
    p->x = mo->x;
    p->y = mo->y;
    *angle = mo->angle;
  }

  p->x = p->x >> FRACTOMAPBITS;
  p->y = p->y >> FRACTOMAPBITS;
}

//
// AM_drawPlayers()
//
// Draws the player arrow in single player,
// or all the player arrows in a netgame.
//
// Passed nothing, returns nothing
//
static void AM_drawPlayers(void)
{
  int   i;
  angle_t angle;
  mpoint_t pt;
  fixed_t scale;
  fixed_t box_scale = 0;

#if defined(HAVE_LIBSDL2_IMAGE)
  if (V_IsOpenGLMode())
  {
    if (map_opengl_nice_things)
      return;
  }
#endif

  if (map_things_appearance == map_things_appearance_scaled)
    scale = (BETWEEN(4<<FRACBITS, 256<<FRACBITS, plr->mo->radius)>>FRACTOMAPBITS);
  else
    scale = 16<<MAPBITS;

  // Needed for hitboxes
  if (map_things_hitboxes && dsda_RevealAutomap() == 2)
    box_scale = (BETWEEN(4<<FRACBITS, 256<<FRACBITS, plr->mo->radius)>>FRACTOMAPBITS);

  if (!netgame)
  {
    pt.x = viewx >> FRACTOMAPBITS;
    pt.y = viewy >> FRACTOMAPBITS;
    if (automap_rotate)
      AM_rotatePoint(&pt);
    else
      AM_SetMPointFloatValue(&pt);

    if (dsda_RevealAutomap())
      AM_drawLineCharacter(cheat_player_arrow, numcheatplyrlines, scale, box_scale, viewangle, mapcolor_p->sngl, pt.x, pt.y);
    else
      AM_drawLineCharacter(player_arrow, numplyrlines, scale, box_scale, viewangle, mapcolor_p->sngl, pt.x, pt.y);
    return;
  }

  for (i = 0; i < g_maxplayers; i++) {
    player_t* p = &players[i];

    if ( (deathmatch && !demoplayback) && p != plr)
      continue;

    if (playeringame[i])
    {
      AM_GetMobjPosition(p->mo, &pt, &angle);

      if (automap_rotate)
        AM_rotatePoint(&pt);
      else
        AM_SetMPointFloatValue(&pt);

      AM_drawLineCharacter (player_arrow, numplyrlines, scale, box_scale, angle,
          p->powers[pw_invisibility] ? 246 /* *close* to black */
          : mapcolor_p->plyr[i], //jff 1/6/98 use default color
          pt.x, pt.y);
    }
  }
}

typedef struct map_nice_icon_param_s
{
  spritenum_t sprite;
  int icon;
  int radius;
  int rotate;
  unsigned char r, g, b;
} map_nice_icon_param_t;

static map_nice_icon_param_t *nice_icons;
static int nice_sprites_max;

static const map_nice_icon_param_t doom_icons[] =
{
  {SPR_STIM, am_icon_health, 12, 0, 100, 100, 200},
  {SPR_MEDI, am_icon_health, 16, 0, 100, 100, 200},
  {SPR_BON1, am_icon_health, 10, 0,   0,   0, 200},

  {SPR_BON2, am_icon_armor,  10, 0,   0, 200,   0},
  {SPR_ARM1, am_icon_armor,  16, 0, 100, 200, 100},
  {SPR_ARM2, am_icon_armor,  16, 0, 100, 100, 200},

  {SPR_CLIP, am_icon_ammo,   10, 0, 180, 150,  50},
  {SPR_AMMO, am_icon_ammo,   16, 0, 180, 150,  50},
  {SPR_ROCK, am_icon_ammo,   10, 0, 180, 150,  50},
  {SPR_BROK, am_icon_ammo,   16, 0, 180, 150,  50},

  {SPR_CELL, am_icon_ammo,   10, 0, 180, 150,  50},
  {SPR_CELP, am_icon_ammo,   16, 0, 180, 150,  50},
  {SPR_SHEL, am_icon_ammo,   10, 0, 180, 150,  50},
  {SPR_SBOX, am_icon_ammo,   16, 0, 180, 150,  50},
  {SPR_BPAK, am_icon_ammo,   16, 0, 180, 150,  50},

  {SPR_BKEY, am_icon_key,    10, 0,   0,   0, 255},
  {SPR_BSKU, am_icon_key,    10, 0,   0,   0, 255},
  {SPR_YKEY, am_icon_key,    10, 0, 255, 255,   0},
  {SPR_YSKU, am_icon_key,    10, 0, 255, 255,   0},
  {SPR_RKEY, am_icon_key,    10, 0, 255,   0,   0},
  {SPR_RSKU, am_icon_key,    10, 0, 255,   0,   0},

  {SPR_PINV, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_PSTR, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_PINS, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_SUIT, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_PMAP, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_PVIS, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_SOUL, am_icon_power,  16, 0, 220, 100, 220},
  {SPR_MEGA, am_icon_power,  16, 0, 220, 100, 220},

  {SPR_BFUG, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_MGUN, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_CSAW, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_LAUN, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_PLAS, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_SHOT, am_icon_weap,   20, 0, 220, 180, 100},
  {SPR_SGN2, am_icon_weap,   20, 0, 220, 180, 100},

  {SPR_BLUD, am_icon_bullet,  8, 0, 255,   0,   0},
  {SPR_PUFF, am_icon_bullet,  8, 0, 255, 255, 115},
  {SPR_MISL, am_icon_bullet,  8, 0,  91,  71,  43},
  {SPR_PLSS, am_icon_bullet,  8, 0, 115, 115, 255},
  {SPR_PLSE, am_icon_bullet,  8, 0, 115, 115, 255},
  {SPR_BFS1, am_icon_bullet, 12, 0, 119, 255, 111},
  {SPR_BFE1, am_icon_bullet, 12, 0, 119, 255, 111},

  {DOOM_NUMSPRITES}
};

static const map_nice_icon_param_t heretic_icons[] =
{
  {HERETIC_SPR_PTN1, am_icon_health, 10, 0,   0, 255, 255},
  {HERETIC_SPR_PTN2, am_icon_health, 16, 0,   0, 255, 255},

  {HERETIC_SPR_SHLD, am_icon_armor,  10, 0, 180, 180, 180},
  {HERETIC_SPR_SHD2, am_icon_armor,  16, 0, 255, 255,   0},

  {HERETIC_SPR_AMG1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMG2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_AMC1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMC2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_AMB1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMB2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_AMP1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMP2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_AMS1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMS2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_AMM1, am_icon_heretic_ammo,   10, 0, 180, 150,  50},
  {HERETIC_SPR_AMM2, am_icon_heretic_ammo,   16, 0, 180, 150,  50},
  {HERETIC_SPR_BAGH, am_icon_heretic_ammo,   16, 0, 180, 150,  50}, // bag of holding

  {HERETIC_SPR_BKYY, am_icon_key,    16, 0,   0,   0, 255}, // blue key
  {HERETIC_SPR_CKYY, am_icon_key,    16, 0, 255, 255,   0}, // yellow key
  {HERETIC_SPR_AKYY, am_icon_key,    16, 0,   0, 255,   0}, // green key

  {HERETIC_SPR_SPHL, am_icon_power,  16, 0, 220, 100, 220}, // mystic urn
  {HERETIC_SPR_FBMB, am_icon_power,  16, 0, 220, 100, 220}, // time bomb
  {HERETIC_SPR_EGGC, am_icon_power,  16, 0, 220, 100, 220}, // egg
  {HERETIC_SPR_SPMP, am_icon_power,  16, 0, 220, 100, 220}, // map scroll
  {HERETIC_SPR_TRCH, am_icon_power,  16, 0, 220, 100, 220}, // torch
  {HERETIC_SPR_ATLP, am_icon_power,  16, 0, 220, 100, 220}, // chaos device
  {HERETIC_SPR_INVS, am_icon_power,  16, 0, 220, 100, 220}, // shadowsphere
  {HERETIC_SPR_INVU, am_icon_power,  16, 0, 220, 100, 220}, // invuln
  {HERETIC_SPR_SOAR, am_icon_power,  16, 0, 220, 100, 220}, // wings
  {HERETIC_SPR_PWBK, am_icon_power,  16, 0, 220, 100, 220}, // tome of power

  {HERETIC_SPR_WBOW, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // crossbow
  {HERETIC_SPR_WBLS, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // dragon claw
  {HERETIC_SPR_WSKL, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // hellstaff
  {HERETIC_SPR_WPHX, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // phoenix rod
  {HERETIC_SPR_WMCE, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // mace
  {HERETIC_SPR_WGNT, am_icon_heretic_weap,   20, 0, 220, 180, 100}, // gauntlets

  {HERETIC_SPR_PPOD, am_icon_bullet, 10, 0, 0,   255,   0}, // pod
  {HERETIC_SPR_TGLT, am_icon_bullet,  4, 0, 255,   0,   0}, // teleport particles
  {HERETIC_SPR_BLOD, am_icon_bullet,  8, 0, 255,   0,   0}, // blood

  {HERETIC_SPR_PUF1, am_icon_bullet,  4, 0, 255, 255, 115}, // gaunlet puff
  {HERETIC_SPR_PUF3, am_icon_bullet,  8, 0, 255, 255, 115}, // staff puff
  {HERETIC_SPR_PUF4, am_icon_bullet,  8, 0,   0,   0, 255}, // staff puff [tome]
  {HERETIC_SPR_FX01, am_icon_bullet,  8, 0, 255, 255,   0}, // wand puff
  {HERETIC_SPR_PUF2, am_icon_bullet,  8, 0, 255, 255,   0}, // wand puff [tome]
  {HERETIC_SPR_FX03, am_icon_bullet,  8, 0,   0, 255,   0}, // crossbow bolt
  {HERETIC_SPR_FX17, am_icon_bullet,  8, 0,   0,   0, 255}, // dragon claw bolt
  {HERETIC_SPR_FX18, am_icon_bullet,  8, 0,   0,   0, 255}, // dragon claw bolt [tome]
  {HERETIC_SPR_FX02, am_icon_bullet,  8, 0, 175, 175, 175}, // mace balls
  {HERETIC_SPR_FX00, am_icon_bullet,  8, 0, 255,   0,   0}, // hellstaff fireball
  {HERETIC_SPR_FX20, am_icon_bullet,  4, 0,   0, 255,   0}, // hellstaff rain [green]
  {HERETIC_SPR_FX21, am_icon_bullet,  4, 0, 255, 255,   0}, // hellstaff rain [yellow]
  {HERETIC_SPR_FX22, am_icon_bullet,  4, 0, 255,   0,   0}, // hellstaff rain [red]
  {HERETIC_SPR_FX23, am_icon_bullet,  4, 0,   0,   0, 255}, // hellstaff rain [blue]
  {HERETIC_SPR_FX04, am_icon_bullet,  8, 0, 255, 255,   0}, // phoenix rod fireball
  {HERETIC_SPR_FX08, am_icon_bullet, 12, 0, 255, 255,   0}, // phoenix rod hit
  {HERETIC_SPR_FX09, am_icon_bullet,  4, 0, 255, 255,   0}, // phoneix rod [idk]

  {HERETIC_NUMSPRITES}
};

static const map_nice_icon_param_t hexen_icons[] =
{
  // Health
  {HEXEN_SPR_PTN1, am_icon_health, 10, 0,   0, 255, 255},
  {HEXEN_SPR_PTN2, am_icon_health, 16, 0,   0, 255, 255},

  // Mana
  {HEXEN_SPR_MAN1, am_icon_hexen_mana,  10, 0, 100, 100, 200},
  {HEXEN_SPR_MAN2, am_icon_hexen_mana,  10, 0, 100, 200, 100},
  {HEXEN_SPR_MAN3, am_icon_hexen_mana,  16, 0, 200, 100, 100},

  // Armor
  {HEXEN_SPR_ARM1, am_icon_armor,  10, 0, 180, 180, 180},
  {HEXEN_SPR_ARM2, am_icon_armor,  10, 0, 180, 180, 180},
  {HEXEN_SPR_ARM3, am_icon_armor,  10, 0, 180, 180, 180},
  {HEXEN_SPR_ARM4, am_icon_armor,  10, 0, 180, 180, 180},

  // Keys
  {HEXEN_SPR_KEY1, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY2, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY3, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY4, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY5, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY6, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY7, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY8, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEY9, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEYA, am_icon_key,    16, 0,   0, 255,   0},
  {HEXEN_SPR_KEYB, am_icon_key,    16, 0,   0, 255,   0},

  // Puzzle Items
  {HEXEN_SPR_ASKU, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_ABGM, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGMR, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGMG, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGG2, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGMB, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGB2, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_ABK1, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_ABK2, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_ASK2, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AFWP, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_ACWP, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AMWP, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGER, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGR2, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGR3, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},
  {HEXEN_SPR_AGR4, am_icon_hexen_puzzle,    16, 0, 255, 255,   0},

  // Artifacts
  {HEXEN_SPR_SPHL, am_icon_power,  16, 0, 220, 100, 220}, // mystic urn
  {HEXEN_SPR_SUMN, am_icon_power,  16, 0, 220, 100, 220}, // dark servant
  {HEXEN_SPR_PSBG, am_icon_power,  16, 0, 220, 100, 220}, // flechette
  {HEXEN_SPR_TRCH, am_icon_power,  16, 0, 220, 100, 220}, // torch
  {HEXEN_SPR_ATLP, am_icon_power,  16, 0, 220, 100, 220}, // chaos device
  {HEXEN_SPR_SPED, am_icon_power,  16, 0, 220, 100, 220}, // boots of speed
  {HEXEN_SPR_INVU, am_icon_power,  16, 0, 220, 100, 220}, // invuln
  {HEXEN_SPR_SOAR, am_icon_power,  16, 0, 220, 100, 220}, // wings
  {HEXEN_SPR_PORK, am_icon_power,  16, 0, 220, 100, 220}, // pork / egg
  {HEXEN_SPR_BMAN, am_icon_power,  16, 0, 100, 100, 100}, // krater of might
  {HEXEN_SPR_BMAN, am_icon_power,  16, 0, 100, 100, 100}, // krater of might
  {HEXEN_SPR_BLST, am_icon_power,  16, 0, 100, 100, 100}, // disc of repulsion
  {HEXEN_SPR_TELO, am_icon_power,  16, 0, 100, 100, 100}, // banishment device
  {HEXEN_SPR_BRAC, am_icon_power,  16, 0, 100, 100, 100}, // dragonskin bracers
  {HEXEN_SPR_HRAD, am_icon_power,  16, 0, 100, 100, 100}, // mystic ambit incant

  // Weapons
  {HEXEN_SPR_WFAX, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Fighter Axe
  {HEXEN_SPR_WFHM, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Fighter Hammer
  {HEXEN_SPR_WFR1, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Fighter Sword Piece 1
  {HEXEN_SPR_WFR2, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Fighter Sword Piece 2
  {HEXEN_SPR_WFR3, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Fighter Sword Piece 3
  {HEXEN_SPR_WCSS, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Cleric Serpent Staff
  {HEXEN_SPR_WCFM, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Cleric Firestorm
  {HEXEN_SPR_WCH1, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Cleric Holy Piece 1
  {HEXEN_SPR_WCH2, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Cleric Holy Piece 2
  {HEXEN_SPR_WCH3, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Cleric Holy Piece 3
  {HEXEN_SPR_WMCS, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Mage Ice Shards
  {HEXEN_SPR_WMLG, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Mage Lightning
  {HEXEN_SPR_WMS1, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Mage Bloodscourge Piece 1
  {HEXEN_SPR_WMS2, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Mage Bloodscourge Piece 2
  {HEXEN_SPR_WMS3, am_icon_hexen_weap,   20, 0, 220, 180, 100}, // Mage Bloodscourge Piece 3

  // Misc Effects
  {HEXEN_SPR_TSMK, am_icon_bullet, 10, 0, 255,   0,   0}, // red teleport cloud
  {HEXEN_SPR_TLGL, am_icon_bullet,  4, 0,   0,   0, 255}, // blue teleport particles
  {HEXEN_SPR_BLOD, am_icon_bullet,  8, 0, 255,   0,   0}, // blood splat

  // Player weapon projectiles / puffs
  {HEXEN_SPR_FHFX, am_icon_bullet,  4, 0, 255, 255, 115}, // Fighter punch puff
  {HEXEN_SPR_FAXE, am_icon_bullet,  4, 0,   0,   0, 255}, // Fighter Axe hit
  {HEXEN_SPR_FHFX, am_icon_bullet,  8, 0, 255,   0,   0}, // Fighter Hammer missile
  {HEXEN_SPR_FSFX, am_icon_bullet,  8, 0,   0, 255,   0}, // Fighter sword projectiles
  {HEXEN_SPR_CSSF, am_icon_bullet,  8, 0,   0, 255,   0}, // Cleric Serpent staff missile
  {HEXEN_SPR_FHFX, am_icon_bullet,  4, 0,   0, 255,   0}, // Cleric Serpent staff puff
  {HEXEN_SPR_CFCF, am_icon_bullet,  4, 0, 255, 255,   0}, // Cleric flame
  {HEXEN_SPR_CFFX, am_icon_bullet,  8, 0, 255, 255,   0}, // Cleric flame missile
  {HEXEN_SPR_SPIR, am_icon_bullet, 10, 0, 235, 235, 235}, // Cleric holy spirit missile
  {HEXEN_SPR_MWND, am_icon_bullet,  8, 0,   0, 235, 255}, // Mage Wand puff / missile
  {HEXEN_SPR_SHRD, am_icon_bullet,  8, 0,   0, 235, 255}, // Mage ice shards
  {HEXEN_SPR_MLFX, am_icon_bullet,  8, 0,   0, 235, 255}, // Mage Lightning 1
  {HEXEN_SPR_MLF2, am_icon_bullet,  8, 0,   0, 235, 255}, // Mage Lightning 2
  {HEXEN_SPR_MSP1, am_icon_bullet, 10, 0, 255,   0,   0}, // Mage Bloodscourge missile 1
  {HEXEN_SPR_MSP2, am_icon_bullet, 10, 0, 255,   0,   0}, // Mage Bloodscourge missile 1

  {HEXEN_NUMSPRITES}
};

static void AM_InitNiceThings(void)
{
  int i;
  const map_nice_icon_param_t* og_icons;

  nice_sprites_max = raven ? heretic ? HERETIC_NUMSPRITES : HEXEN_NUMSPRITES : DOOM_NUMSPRITES;
  og_icons         = raven ? heretic ? heretic_icons      : hexen_icons      : doom_icons;

  nice_icons = Z_Calloc(nice_sprites_max, sizeof(*nice_icons));

  for (i = 0; i < nice_sprites_max; ++i)
    nice_icons[i] = og_icons[i];
}

static void AM_ProcessNiceThing(mobj_t* mobj, angle_t angle, fixed_t x, fixed_t y)
{
  const float shadow_scale_factor = 1.3f;
  angle_t ang;
  int i, type, radius, rotate, need_shadow;
  float fx, fy, fradius, rot, shadow_radius;
  unsigned char r, g, b, a;

  if (!nice_sprites_max)
    AM_InitNiceThings();

  need_shadow = true;

  type = am_icon_normal;
  r = 220;
  g = 180;
  b = 100;
  a = 255;
  radius = mobj->radius;
  rotate = true;

  if (mobj->player)
  {
    player_t *p = mobj->player;
    int color = mapcolor_p->plyr[p - players];
    const unsigned char *playpal = V_GetPlaypal();

    if ((deathmatch && !demoplayback) && p != plr)
      return;

    type = am_icon_player;

    r = playpal[3 * color + 0];
    g = playpal[3 * color + 1];
    b = playpal[3 * color + 2];
    a = p->powers[pw_invisibility] ? 128 : 255;

    radius = mobj->radius;
    rotate = true;
  }
  else if (mobj->flags & MF_COUNTKILL)
  {
    if (mobj->flags & MF_CORPSE)
    {
      need_shadow = false;
      type = am_icon_corpse;
      r = 120;
      a = 128;
    }
    else
    {
      type = am_icon_monster;
      r = 200;
    }
    g = 0;
    b = 0;
    radius = BETWEEN(4<<FRACBITS, 256<<FRACBITS, mobj->radius);
    rotate = true;
  }
  else
  {
    i = 0;
    while (nice_icons[i].sprite < nice_sprites_max)
    {
      if (mobj->sprite == nice_icons[i].sprite)
      {
        type = nice_icons[i].icon;
        r = nice_icons[i].r;
        g = nice_icons[i].g;
        b = nice_icons[i].b;
        radius = nice_icons[i].radius << 16;
        rotate = nice_icons[i].rotate;

        break;
      }
      i++;
    }
  }

  fradius = MTOF_F(radius >> FRACTOMAPBITS);
  if (fradius < 1.0f)
    return;
  if (fradius < 4.0f)
    need_shadow = false;

  fx = CXMTOF_F(x);
  fy = CYMTOF_F(y);

  shadow_radius = fradius * shadow_scale_factor;
  if (fx + shadow_radius < 0 ||
      fx - shadow_radius > (float)SCREENWIDTH ||
      fy + shadow_radius < 0 ||
      fy - shadow_radius > (float)SCREENHEIGHT)
  {
    return;
  }

  ang = (rotate ? angle : 0) + (automap_rotate ? ANG90 - viewangle : 0);
  rot = -(float)ang / (float)(1u << 31) * (float)M_PI;

  gld_AddNiceThing(type, fx, fy, fradius, rot, r, g, b, a);
  if (need_shadow)
  {
    gld_AddNiceThing(am_icon_shadow, fx, fy, shadow_radius, rot, 0, 0, 0, 128);
  }
}

static void AM_DrawNiceEasyKeys(void)
{
  int i;
  mobj_t* t;
  mpoint_t p;
  angle_t angle;

  // return if IDDT
  if (dsda_RevealAutomap() == 2)
    return;

  // for all sectors
  for (i = 0; i < numsectors; i++)
  {
    if (!(players[displayplayer].cheats & CF_NOCLIP) &&
      (sectors[i].bbox[BOXLEFT] > am_frame.bbox[BOXRIGHT] ||
      sectors[i].bbox[BOXRIGHT] < am_frame.bbox[BOXLEFT] ||
      sectors[i].bbox[BOXBOTTOM] > am_frame.bbox[BOXTOP] ||
      sectors[i].bbox[BOXTOP] < am_frame.bbox[BOXBOTTOM]))
    {
      continue;
    }

    t = sectors[i].thinglist;
    while (t) // for all things in that sector
    {
      if (mapcolor_p->rkey || mapcolor_p->ykey || mapcolor_p->bkey)
      {
        int nice_key = false;
        if (hexen)
        {
          switch(t->info->doomednum)
          {
            // hexen keys
            case 8030: case 8031: case 8032: case 8033: case 8034: case 8035:
            case 8036: case 8037: case 8038: case 8039: case 8200:
            // hexen puzzle parts - use key color
            case 9002: case 9003: case 9004: case 9005: case 9006: case 9007: case 9008:
            case 9009: case 9010: case 9011: case 9012: case 9014: case 9015: case 9016:
            case 9017: case 9018: case 9019: case 9020: case 9021:
              nice_key++; break;
          }
        }
        else if (heretic) // Heretic Keys
        {
          switch(t->info->doomednum)
          {
            case 73: // green key
            case 80: // yellow key
            case 79: // blue key
              nice_key++; break;
          }
        }
        else   // Doom Keys
        {
          switch(t->info->doomednum)
          {
            //jff 1/5/98 treat keys special
            case 38: case 13: // red keys
            case 39: case 6:  // yellow keys
            case 40: case 5:  // blue keys
              nice_key++; break;
          }
        }

        if (nice_key)
        {
          AM_GetMobjPosition(t, &p, &angle);
          if (automap_rotate)
            AM_rotatePoint(&p);
          AM_ProcessNiceThing(t, angle, p.x, p.y);
        }
        t = t->snext;
      }
    }
  }
}

static void AM_DrawNiceThings(void)
{
  int i;
  mobj_t* t;
  mpoint_t p;
  angle_t angle;
  int showkeys = skill_info.flags & SI_EASY_KEY || dsda_ShowAutomapKeys();

  gld_ClearNiceThings();

  // draw players
  for (i = 0; i < g_maxplayers; i++)
  {
    if ((deathmatch && !demoplayback) && &players[i] != plr)
      continue;

    if (playeringame[i])
    {
      t = players[i].mo;
      AM_GetMobjPosition(t, &p, &angle);
      if (automap_rotate)
        AM_rotatePoint(&p);
      else
        AM_SetMPointFloatValue(&p);
      AM_ProcessNiceThing(t, angle, p.x, p.y);
    }
  }

  // walls
  if (dsda_RevealAutomap() == 2)
  {
    // for all sectors
    for (i = 0; i < numsectors; i++)
    {
      if (!(players[displayplayer].cheats & CF_NOCLIP) &&
        (sectors[i].bbox[BOXLEFT] > am_frame.bbox[BOXRIGHT] ||
        sectors[i].bbox[BOXRIGHT] < am_frame.bbox[BOXLEFT] ||
        sectors[i].bbox[BOXBOTTOM] > am_frame.bbox[BOXTOP] ||
        sectors[i].bbox[BOXTOP] < am_frame.bbox[BOXBOTTOM]))
      {
        continue;
      }

      t = sectors[i].thinglist;
      while (t) // for all things in that sector
      {
        // Avoid drawing the main player again
        // But draw voodoo dolls!
        if (!(t->player && t == t->player->mo))
        {
          AM_GetMobjPosition(t, &p, &angle);
          if (automap_rotate)
            AM_rotatePoint(&p);
          AM_ProcessNiceThing(t, angle, p.x, p.y);
        }
        t = t->snext;
      }
    }
  }

  // draw nice easy keys
  if (showkeys)
    AM_DrawNiceEasyKeys();

  // marked locations on the automap
  {
    float radius;
    int anim_flash_level = (gametic % 32);

    // Flashing animation for hilight
    // Pulsates between 0.5-1.0f (multiplied with hilight alpha)
    if (anim_flash_level >= 16)
    {
      anim_flash_level = 32 - anim_flash_level;
    }
    anim_flash_level = 127 + anim_flash_level * 8;

    // do not want to have too small marks
    radius = MTOF_F(16 << MAPBITS);
    radius = BETWEEN(8.0f, 128.0f, radius);

    for (i = 0; i < markpointnum; i++) // killough 2/22/98: remove automap mark limit
    {
      if (markpoints[i].x != -1)
      {
        mpoint_t p = { 0 };

        p.x = markpoints[i].x;
        p.y = markpoints[i].y;

        if (automap_rotate)
          AM_rotatePoint(&p);
        else
          AM_SetMPointFloatValue(&p);

        p.fx = CXMTOF_F(p.fx);
        p.fy = CYMTOF_F(p.fy);

        gld_AddNiceThing(am_icon_mark, p.fx, p.fy, radius, 0, 255, 255, 0, (unsigned char)anim_flash_level);
      }
    }
  }
}

//
// AM_drawThings()
//
// Draws the things on the automap in double IDDT cheat mode
//
// Passed colors and colorrange, no longer used
// Returns nothing
//
static void AM_drawThings(void)
{
  int   i;
  mobj_t* t;
  mline_t* lineguy = thintriangle_guy;
  int lineguylines = NUMTHINTRIANGLEGUYLINES;
  int showkeys = skill_info.flags & SI_EASY_KEY || dsda_ShowAutomapKeys();

#if defined(HAVE_LIBSDL2_IMAGE)
  if (V_IsOpenGLMode())
  {
    if (map_opengl_nice_things)
    {
      AM_DrawNiceThings();
      return;
    }
  }
#endif

  if (!showkeys && dsda_RevealAutomap() != 2)
    return;

  // for all sectors
  for (i=0;i<numsectors;i++)
  {
   // e6y
   // Two-pass method for better usability of automap:
   // The first one will draw all things except enemies
   // The second one is for enemies only
   // Stop after first pass if the current sector has no enemies
   int pass;
   int enemies = 0;

   if (!(players[displayplayer].cheats & CF_NOCLIP) &&
     (sectors[i].bbox[BOXLEFT] > am_frame.bbox[BOXRIGHT] ||
     sectors[i].bbox[BOXRIGHT] < am_frame.bbox[BOXLEFT] ||
     sectors[i].bbox[BOXBOTTOM] > am_frame.bbox[BOXTOP] ||
     sectors[i].bbox[BOXTOP] < am_frame.bbox[BOXBOTTOM]))
   {
     continue;
   }

   for (pass = 0; pass < 2; pass += (enemies ? 1 : 2))
   {
    t = sectors[i].thinglist;
    while (t) // for all things in that sector
    {
      mpoint_t p;
      angle_t angle;
      fixed_t scale;
      fixed_t box_scale = 0;
      int color = -1;

      //e6y: stop if all enemies from current sector already has been drawn
      if (pass == 1 && enemies == 0)
        break;
      if (pass == ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL ?
        (pass == 0 ? enemies++: enemies--), 0 : 1))
      {
        t = t->snext;
        continue;
      }

      if (map_things_appearance == map_things_appearance_scaled)
        scale = (BETWEEN(4<<FRACBITS, 256<<FRACBITS, t->radius)>>FRACTOMAPBITS);// * 16 / 20;
      else
        scale = 16<<MAPBITS;

      // Needed for hitboxes
      if (map_things_hitboxes)
        box_scale = (BETWEEN(4<<FRACBITS, 256<<FRACBITS, t->radius)>>FRACTOMAPBITS);// * 16 / 20;

      AM_GetMobjPosition(t, &p, &angle);

      if (automap_rotate)
        AM_rotatePoint(&p);
      else
        AM_SetMPointFloatValue(&p);

      //jff 1/5/98 case over doomednum of thing being drawn
      if (mapcolor_p->rkey || mapcolor_p->ykey || mapcolor_p->bkey)
      {
        if (hexen) // Hexen keys and puzzle pieces
        {
          // hexen keys
          if (mapcolor_p->ykey)
          {
            int hexkey_scale = scale*2; // Hexen keys are very small on automap
            switch(t->info->doomednum)
            {
              // hexen keys
              case 8030: case 8031: case 8032: case 8033: case 8034: case 8035:
              case 8036: case 8037: case 8038: case 8039: case 8200:
                color = mapcolor_p->ykey != -1 ? mapcolor_p->ykey : mapcolor_p->sprt; break;
            }

            if (color != -1)
            {
              AM_drawLineCharacter(raven_keysquare, RAVEN_NUMKEYSQUARELINES, hexkey_scale, box_scale, 0, color, p.x, p.y);
              t = t->snext;
              continue;
            }
          }

          // hexen puzzle parts - use key color
          if (mapcolor_p->bkey)
          {
            switch(t->info->doomednum)
            {
              case 9002: case 9003: case 9004: case 9005: case 9006: case 9007: case 9008:
              case 9009: case 9010: case 9011: case 9012: case 9014: case 9015: case 9016:
              case 9017: case 9018: case 9019: case 9020: case 9021:
                color = mapcolor_p->bkey != -1 ? mapcolor_p->bkey : mapcolor_p->sprt; break;
            }

            if (color != -1)
            {
              AM_drawLineCharacter(cross_mark, NUMCROSSMARKLINES, scale, box_scale, t->angle, color, p.x, p.y);
              t = t->snext;
              continue;
            }
          }
        }
        else if (heretic) // Heretic keys
        {
          switch(t->info->doomednum)
          {
            case 73: // green key
              color = mapcolor_p->rkey != -1? mapcolor_p->rkey : mapcolor_p->sprt; break;
            case 80: // yellow key
              color = mapcolor_p->ykey != -1? mapcolor_p->ykey : mapcolor_p->sprt; break;
            case 79: // blue key
              color = mapcolor_p->bkey != -1? mapcolor_p->bkey : mapcolor_p->sprt; break;
          }

          if (color != -1)
          {
            AM_drawLineCharacter(raven_keysquare, RAVEN_NUMKEYSQUARELINES, scale, box_scale, 0, color, p.x, p.y);
            t = t->snext;
            continue;
          }
        }
        else // Doom Keys
        {
          switch(t->info->doomednum)
          {
            //jff 1/5/98 treat keys special
            case 38: case 13: //jff  red key
              color = mapcolor_p->rkey != -1? mapcolor_p->rkey : mapcolor_p->sprt; break;
            case 39: case 6: //jff yellow key
              color = mapcolor_p->ykey != -1? mapcolor_p->ykey : mapcolor_p->sprt; break;
            case 40: case 5: //jff blue key
              color = mapcolor_p->bkey != -1? mapcolor_p->bkey : mapcolor_p->sprt; break;
          }

          if (color != -1)
          {
            AM_drawLineCharacter(cross_mark, NUMCROSSMARKLINES, scale, box_scale, t->angle, color, p.x, p.y);
            t = t->snext;
            continue;
          }
        }
      }

      // return if not IDDT
      if (dsda_RevealAutomap() != 2)
      {
        t = t->snext;
        continue;
      }

      // hexen artifacts use item color
      if (hexen && mapcolor_p->item)
      {
        switch(t->info->doomednum)
        {
          case 30: case 32: case 33: case 36: case 82:
          case 83: case 84: case 86: case 8000: case 8002:
          case 8003: case 8041: case 10040: case 10110: case 10120:
            color = mapcolor_p->item != -1 ? mapcolor_p->item : mapcolor_p->sprt; break;
        }

        if (color != -1)
        {
          AM_drawLineCharacter(lineguy, lineguylines, scale, box_scale, angle, color, p.x, p.y);
          t = t->snext;
          continue;
        }
      }

      // friends color
      if (t->flags & MF_FRIEND && !t->player)
        color = mapcolor_p->frnd;
      // countable kills color - red (cph 2006/07/30)
      else if ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL)
        color = mapcolor_p->enemy;
      // countable items color - yellow (bbm 2/28/03)
      else if (t->flags & MF_COUNTITEM)
        color = mapcolor_p->item;
      else if (t->flags & MF_SPECIAL)
        color = mapcolor_p->pickup;
      // generic sprite color
      else 
        color = mapcolor_p->sprt;

      //jff 1/5/98 end added code for keys
      //jff previously entire code
      AM_drawLineCharacter(lineguy, lineguylines, scale, box_scale, angle, color, p.x, p.y);
      t = t->snext;
    }
   }
  }
}

void AM_updatePlayerTrail(fixed_t x, fixed_t y)
{
  trailpoint_t pt;

  pt.x = x >> FRACTOMAPBITS;
  pt.y = y >> FRACTOMAPBITS;

  if (trail_index == -1 ||
      player_trail[trail_index].x != pt.x ||
      player_trail[trail_index].y != pt.y)
  {
    trail_index = (trail_index + 1) % trail_size_max;
    player_trail[trail_index] = pt;

    if (trail_size < trail_index + 1)
      trail_size = trail_index + 1;
  }
}

static void AM_drawPlayerTrail(void)
{
  int i;

  for (i = 0; i < trail_size; ++i)
  {
    mpoint_t p;

    p.x = player_trail[i].x;
    p.y = player_trail[i].y;

    if (automap_rotate)
      AM_rotatePoint(&p);
    else
      AM_SetMPointFloatValue(&p);

    p.x = CXMTOF(p.x);
    p.y = CYMTOF(p.y);
    if (am_frame.precise)
    {
      p.fx = CXMTOF_F(p.fx);
      p.fy = CYMTOF_F(p.fy);
    }

    if (p.x >= f_x && p.y >= f_y && p.x < f_x + f_w && p.y < f_y + f_h)
    {
      mpoint_t a, b, c, d;
      mpoint_t e, f, g, h;
      mline_t line;

      a.x = b.x = player_trail[i].x - FRACUNIT;
      c.x = d.x = player_trail[i].x + FRACUNIT;
      a.y = d.y = player_trail[i].y - FRACUNIT;
      b.y = c.y = player_trail[i].y + FRACUNIT;

      e.x = player_trail[i].x - FRACUNIT / 8;
      e.y = player_trail[i].y - FRACUNIT / 8;
      f.x = player_trail[i].x + FRACUNIT / 8;
      f.y = player_trail[i].y + FRACUNIT / 8;
      g.x = player_trail[i].x - FRACUNIT / 8;
      g.y = player_trail[i].y + FRACUNIT / 8;
      h.x = player_trail[i].x + FRACUNIT / 8;
      h.y = player_trail[i].y - FRACUNIT / 8;

      if (automap_rotate)
      {
        AM_rotatePoint(&a);
        AM_rotatePoint(&b);
        AM_rotatePoint(&c);
        AM_rotatePoint(&d);
        AM_rotatePoint(&e);
        AM_rotatePoint(&f);
        AM_rotatePoint(&g);
        AM_rotatePoint(&h);
      }
      else
      {
        AM_SetMPointFloatValue(&a);
        AM_SetMPointFloatValue(&b);
        AM_SetMPointFloatValue(&c);
        AM_SetMPointFloatValue(&d);
        AM_SetMPointFloatValue(&e);
        AM_SetMPointFloatValue(&f);
        AM_SetMPointFloatValue(&g);
        AM_SetMPointFloatValue(&h);
      }

      {
        int color;

        color = (i % 2) ? mapcolor_p->trail_1 : mapcolor_p->trail_2;

        // Cross marking center
        line.a = e; line.b = f;
        AM_drawMline(&line, color);
        line.a = g; line.b = h;
        AM_drawMline(&line, color);

        // Bounding box
        line.a = a; line.b = b;
        AM_drawMline(&line, color);
        line.a = b; line.b = c;
        AM_drawMline(&line, color);
        line.a = c; line.b = d;
        AM_drawMline(&line, color);
        line.a = d; line.b = a;
        AM_drawMline(&line, color);
      }
    }
  }
}

//
// AM_drawMarks()
//
// Draw the marked locations on the automap
//
// Passed nothing, returns nothing
//
// killough 2/22/98:
// Rewrote AM_drawMarks(). Removed limit on marks.
//
static void AM_drawMarks(void)
{
  int i, namelen;
  char namebuf[16];

  snprintf(namebuf, sizeof(namebuf), "%s", !raven ? "AMMNUM0" : "SMALLIN0");
  namelen = !raven ? 6 : 7;

  if (map_trail_mode && dsda_RevealAutomap())
    AM_drawPlayerTrail();

#if defined(HAVE_LIBSDL2_IMAGE)
  if (V_IsOpenGLMode())
  {
    if (map_opengl_nice_things)
      return;
  }
#endif

  for (i = 0; i < markpointnum; i++) // killough 2/22/98: remove automap mark limit
  {
    if (markpoints[i].x != -1)
    {
      int k, w;
      mpoint_t p;
      fixed_t mx, my;

      p.x = markpoints[i].x;// - m_x + prev_m_x;
      p.y = markpoints[i].y;// - m_y + prev_m_y;

      if (automap_rotate)
        AM_rotatePoint(&p);
      else
        AM_SetMPointFloatValue(&p);

      // Save coordinates for vector
      mx = p.x;
      my = p.y;

      p.x = CXMTOF(p.x) - markpoints[i].w * SCREENWIDTH / 320 / 2;
      p.y = CYMTOF(p.y) - markpoints[i].h * SCREENHEIGHT / 200 / 2;
      if (am_frame.precise)
      {
        p.fx = CXMTOF_F(p.fx) - (float)markpoints[i].w * SCREENWIDTH / 320.0f / 2.0f;
        p.fy = CYMTOF_F(p.fy) - (float)markpoints[i].h * SCREENHEIGHT / 200.0f / 2.0f;
      }

      if (V_IsOpenGLMode() ?
          p.y < f_y + f_h && p.y + markpoints[i].h * SCREENHEIGHT / 200 >= f_y :
          p.y < f_y + f_h && p.y >= f_y)
      {
        // Vector Markers
        if (dsda_IntConfig(dsda_config_map_marker_style))
        {
          fixed_t digit_size    = FRACUNIT * 2 * SCREENWIDTH / 320;
          fixed_t digit_scale   = FixedDiv(digit_size, scale_mtof);
          fixed_t digit_spacing = FixedMul(digit_scale, FRACUNIT*2);
          angle_t digit_angle   = automap_rotate ? (viewangle - ANG90) : 0;

          // Center digit on marker
          fixed_t bx = mx;
          fixed_t by = my;

          int len = (int)strlen(markpoints[i].label);
          if (len > 1)
            bx -= (digit_spacing * (len - 1)) / 2;

          for (k = 0; k < len; k++)
          {
            int d = markpoints[i].label[k] - '0';
            if ((unsigned)d <= 9)
            {
              AM_drawLineCharacter(am_digits[d], am_digit_lines[d],
                                  digit_scale, 0, digit_angle, mapcolor_p->marker,
                                  bx, by);
            }
            bx += digit_spacing;
          }

          continue;
        }
        // Patch Markers
        else
        {
          w = 0;
          for (k = 0; k < (int)strlen(markpoints[i].label); k++)
          {
            namebuf[namelen] = markpoints[i].label[k];

            if (p.x < f_x + f_w &&
                p.x + markpoints[i].widths[k] * SCREENWIDTH / 320 >= f_x)
            {
              float fx, fy;
              int x, y, flags;

              switch (render_stretch_hud)
              {
                default:
                case patch_stretch_not_adjusted:
                  fx = (float)p.fx / patches_scalex;
                  fy = (float)p.fy * 200.0f / SCREENHEIGHT;

                  x = p.x / patches_scalex;
                  y = p.y * 200 / SCREENHEIGHT;

                  flags = VPT_ALIGN_LEFT | VPT_STRETCH;
                  break;
                case patch_stretch_doom_format:
                  fx = (float)p.fx * 320.0f / WIDE_SCREENWIDTH;
                  fy = (float)p.fy * 200.0f / WIDE_SCREENHEIGHT;

                  x = p.x * 320 / WIDE_SCREENWIDTH;
                  y = p.y * 200 / WIDE_SCREENHEIGHT;

                  flags = VPT_ALIGN_LEFT | VPT_STRETCH;
                  break;
                case patch_stretch_fit_to_width:
                  fx = (float)p.fx * 320.0f / SCREENWIDTH;
                  fy = (float)p.fy * 200.0f / SCREENHEIGHT;

                  x = p.x * 320 / SCREENWIDTH;
                  y = p.y * 200 / SCREENHEIGHT;

                  flags = VPT_ALIGN_WIDE | VPT_STRETCH;
                  break;
              }

              if (am_frame.precise)
              {
                V_DrawNamePatchPrecise(fx, fy, namebuf, CR_DEFAULT, flags);
              }
              else
              {
                V_DrawNamePatch(x, y, namebuf, CR_DEFAULT, flags);
              }
            }

            w += markpoints[i].widths[k] + 1;
            p.x += w * SCREENWIDTH / 320;
            if (am_frame.precise)
            {
              p.fx += (float)w * SCREENWIDTH / 320.0f;
            }
          }
        }
      }
    }
  }
}

//
// AM_drawCrosshair()
//
// Draw the single square crosshair representing map center
//
// Passed the color to draw the square with
// Returns nothing
//
// CPhipps - made static inline, and use the general pixel plotter function

static void AM_drawCrosshair(int color)
{
  // [crispy] do not draw the useless dot on the player arrow
  if (!automap_follow)
  {
    fline_t line;

    int thickness = AM_GetLineWeight();

    // Instead of two lines, draw a square instead.
    // With thick lines, the square will appear to be a circle.
    int x0 = f_x+(f_w/2);
    int x1 = f_x+(f_w/2)+thickness;
    int y0 = f_y+(f_h/2);
    int y1 = f_y+(f_h/2)+thickness;

    // top
    line.a.x = x0; line.a.y = y0;
    line.b.x = x1; line.b.y = y0;
    AM_SetFPointFloatValue(&line.a);
    AM_SetFPointFloatValue(&line.b);
    V_DrawLine(&line, color);

    // bottom
    line.a.x = x0; line.a.y = y1;
    line.b.x = x1; line.b.y = y1;
    AM_SetFPointFloatValue(&line.a);
    AM_SetFPointFloatValue(&line.b);
    V_DrawLine(&line, color);

    // left
    line.a.x = x0; line.a.y = y0;
    line.b.x = x0; line.b.y = y1;
    AM_SetFPointFloatValue(&line.a);
    AM_SetFPointFloatValue(&line.b);
    V_DrawLine(&line, color);

    // right
    line.a.x = x1; line.a.y = y0;
    line.b.x = x1; line.b.y = y1;
    AM_SetFPointFloatValue(&line.a);
    AM_SetFPointFloatValue(&line.b);
    V_DrawLine(&line, color);
  }
}

void M_ChangeMapTextured(void)
{
  map_textured = dsda_IntConfig(dsda_config_map_textured);

  if (in_game && gamestate == GS_LEVEL && V_IsOpenGLMode())
  {
    gld_ProcessTexturedMap();
  }
}

void M_ChangeMapMultisamling(void)
{
  map_use_multisampling = dsda_IntConfig(dsda_config_map_use_multisamling);

  if (!raven && map_use_multisampling && V_IsSoftwareMode())
  {
    V_InitFlexTranTable();
  }
}

//=============================================================================
//
// AM_drawSubsectors
//
//=============================================================================

static void AM_drawSubsectors(void)
{
  if (V_IsOpenGLMode())
  {
    gld_MapDrawSubsectors(plr, f_x, f_y, m_x, m_y, f_w, f_h, scale_mtof);
  }
}

static void AM_setFrameVariables(void)
{
  float angle;

  angle = (float)(ANG90 - viewangle) / (float)(1u << 31) * (float)M_PI;
  am_frame.sin_f = (float)sin(angle);
  am_frame.cos_f = (float)cos(angle);
  am_frame.sin = finesine[(ANG90 - viewangle)>>ANGLETOFINESHIFT];
  am_frame.cos = finecosine[(ANG90 - viewangle)>>ANGLETOFINESHIFT];

  am_frame.centerx = m_x + m_w / 2;
  am_frame.centery = m_y + m_h / 2;
  am_frame.centerx_f = (float)m_x + (float)m_w / 2.0f;
  am_frame.centery_f = (float)m_y + (float)m_h / 2.0f;

  if (automap_rotate)
  {
    float dx = (float)(m_x2 - am_frame.centerx);
    float dy = (float)(m_y2 - am_frame.centery);
    fixed_t r = M_DoubleToInt(sqrt(dx * dx + dy * dy));

    am_frame.bbox[BOXLEFT] = am_frame.centerx - r;
    am_frame.bbox[BOXRIGHT] = am_frame.centerx + r;
    am_frame.bbox[BOXBOTTOM] = am_frame.centery - r;
    am_frame.bbox[BOXTOP] = am_frame.centery + r;
  }
  else
  {
    am_frame.bbox[BOXLEFT] = m_x;
    am_frame.bbox[BOXRIGHT] = m_x2;
    am_frame.bbox[BOXBOTTOM] = m_y;
    am_frame.bbox[BOXTOP] = m_y2;
  }

  am_frame.precise = (V_IsOpenGLMode());
}

//=============================================================================
//
// AM_DrawBackground
//
//=============================================================================

static void AM_DrawBackground (void)
{
  if (automap_bg) { // Automap Parallax Background
    V_BeginUIDraw(); // OpenGL doesn't like flats in AutomapDraw()

    if (W_LumpNameExists(g_autopage)) // Raven AUTOPAGE RAW format (custom size)
      V_FillNameRawAdv(g_autopage, f_x, f_y, g_autopage_width, g_autopage_height, f_w, f_h, mapxstart, mapystart, VPT_STRETCH);

    else if (W_FlatNameExists(g_autopage)) // AUTOPAGE flat format (64x64)
      V_FillNameFlatAdv(g_autopage, f_x, f_y, f_w, f_h, mapxstart, mapystart, VPT_STRETCH);

    else // AUTOPAGE flat format (64x64) - fallback
      V_FillNameFlatAdv("FLOOR4_6", f_x, f_y, f_w, f_h, mapxstart, mapystart, VPT_STRETCH);

    V_EndUIDraw();

    if (autopage_fade > 0) // Darken flat
      V_DrawShaded(f_x, f_y, f_w, f_h, autopage_fade * 31 / 100);

    return;
  }

  V_FillRect(f_x, f_y, f_w, f_h, (byte)mapcolor_p->back); //jff 1/5/98 background default color
}

//
// AM_Drawer()
//
// Draws the entire automap
//
// Passed nothing, returns nothing
//

void AM_Drawer (dboolean minimap)
{
  if (!automap_active && !minimap)
    return;

  if (automap_active && automap_overlay == 2 && minimap)
    return;

  V_BeginAutomapDraw();

  if (automap_follow || minimap)
    AM_doFollowPlayer();

  // Change the zoom if necessary
  if (ftom_zoommul != FRACUNIT)
    AM_changeWindowScale();

  // Change x,y location
  if (m_paninc.x || m_paninc.y)
    AM_changeWindowLoc();

  AM_setFrameVariables();

  if (V_IsOpenGLMode())
  {
    // do not use multisampling in automap mode if map_use_multisampling 0
    gld_MultisamplingSet();
  }

  if (!automap_overlay) // cph - If not overlay mode, clear background for the automap
    AM_DrawBackground();
  if (automap_overlay == 2 && !M_MenuIsShaded())   // If fade overlay mode, add shaded background
    V_DrawShaded(f_x, f_y, f_w, f_h, screenshade);

  if (map_textured)
  {
    V_BeginUIDraw();
    AM_drawSubsectors();
    V_EndUIDraw();
  }

  if (automap_grid)
    AM_drawGrid(mapcolor_p->grid);      //jff 1/7/98 grid default color
  AM_drawWalls();
  AM_drawPlayers();
  AM_drawThings(); //jff 1/5/98 default double IDDT sprite
  AM_DrawConnections();
  AM_drawCrosshair(mapcolor_p->hair);   //jff 1/7/98 default crosshair color
  AM_UpdateParallax();

  if (V_IsOpenGLMode())
  {
    gld_DrawMapLines();
    M_ArrayClear(&map_lines);

#if defined(HAVE_LIBSDL2_IMAGE)
    if (map_opengl_nice_things)
    {
      gld_DrawNiceThings(f_x, f_y, f_w, f_h);
    }
#endif
  }

  AM_drawMarks();

  V_EndAutomapDraw();
}
