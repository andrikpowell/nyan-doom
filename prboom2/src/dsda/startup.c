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
//  DSDA STARTUP
//

#include <string.h>

#include "SDL.h"

#include "doomdef.h"
#include "doomstat.h"
#include "gl_intern.h"
#include "gl_opengl.h"
#include "i_system.h"
#include "i_video.h"
#include "lprintf.h"
#include "s_sound.h"
#include "sounds.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/configuration.h"
#include "dsda/gameinfo.h"
#include "dsda/gl/render_scale.h"
#include "dsda/startup.h"

#include "textscreen/fonts/normal.h"

#define PROGRESS_STEP_DURATION_MS 25

static void dsda_WaitForProgressStep(unsigned int start_time)
{
  unsigned int elapsed;

  while ((elapsed = SDL_GetTicks() - start_time) < PROGRESS_STEP_DURATION_MS)
    I_uSleep((PROGRESS_STEP_DURATION_MS - elapsed) * 1000);
}

//
// HEXEN STARTUP
//
// Hexen is pretty complex cuz it uses actual graphics, unlike ENDOOM
//

#define STARTUP_PAL_SIZE (16 * 3)
#define STARTUP_W 640
#define STARTUP_H 480
#define NOTCH_W 16
#define NOTCH_H 23
#define MAX_NOTCHES 32
#define PROGRESS_X 64
#define PROGRESS_Y 441
#define FADE_STEPS 16
#define NO_FADE FADE_STEPS
#define FADE_TIME_US 1000000
#define HEADER_H 32
#define BORDER_COLOR 16

// Hexen's original hardcoded notch, converted to packed pixels
static const byte hexen_notch_table[] = {
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x70, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x68, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x87, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xd8, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x8d, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xd8, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x8d, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xd8, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd7, 0x7d, 0x60, 0x00, 0x00,
  0x00, 0x66, 0x99, 0x99, 0x96, 0x69, 0x66, 0x00, 0x00, 0x69, 0x96, 0x99, 0x69, 0x96, 0x96, 0x00,
  0x06, 0x9d, 0x99, 0x69, 0x96, 0xd9, 0x79, 0x60, 0x06, 0x7d, 0xdd, 0xdd, 0xdd, 0xdd, 0x77, 0x60,
  0x06, 0x78, 0x88, 0x88, 0x88, 0x88, 0xd6, 0x60, 0x06, 0x7a, 0xaa, 0xaa, 0xaa, 0xaa, 0xd6, 0x60,
  0x06, 0x7a, 0x77, 0x77, 0x77, 0xa7, 0x96, 0x60, 0x06, 0x77, 0xa7, 0x77, 0x77, 0xa7, 0x96, 0x60,
  0x06, 0x97, 0xa7, 0x79, 0x77, 0x77, 0x96, 0x60, 0x00, 0x67, 0x79, 0x99, 0x99, 0xd7, 0x96, 0x60,
  0x00, 0x69, 0x99, 0x66, 0x69, 0x69, 0x66, 0x00,
};

static dboolean startup_shown;
static dboolean startup_active;
static dboolean startup_skipped;
static byte palette_map[16];
static byte startup_dynamic_palette[256 * 3];
static const byte *startup_bitmap;
static int notch_position;
static byte custom_notch[NOTCH_W * NOTCH_H / 2];
static const byte *notch_bitmap = hexen_notch_table;
static byte *gl_startup_size;
static byte *gl_texture_data;
static GLuint gl_texture;

static void GL_InitStartup(void);
static void GL_DrawStartup(void);
static void GL_CopyBlock(const byte *data, int x, int y, int width, int height, dboolean notch);
static void GL_RestoreStartup(void);
static void GL_FinishStartup(void);

// Skip STARTUP with key press
static dboolean StartupSkipped(void)
{
  SDL_Event event;

  SDL_PumpEvents();

  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYUP) > 0)
    if (event.type == SDL_KEYDOWN)
      startup_skipped = true;

  return startup_skipped;
}

// While keys are being pressed during STARTUP, they are added to a cue
// Clear them once STARTUP finishes
static void ClearKeyPresses(void)
{
  SDL_PumpEvents();
  SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
}

static int ExpandVGAColor(int color)
{
  return (color << 2) | (color >> 4);
}

static int HeaderHeight(void)
{
  const char *title = dsda_GameInfoStartupTitle();

  return title && *title ? HEADER_H : 0;
}

static int CanvasHeight(void)
{
  return STARTUP_H + HeaderHeight();
}

static int PlanarPixel(const byte *data, int width, int height, int x, int y)
{
  int plane;
  int bit = y * width + x;
  int plane_size = width * height / 8;
  int colour = 0;

  for (plane = 0; plane < 4; ++plane)
    colour |= ((data[plane * plane_size + bit / 8] >> (7 - bit % 8)) & 1) << plane;

  return colour;
}

static int NotchPixel(const byte *data, int width, int height, int x, int y)
{
  int pixel;
  byte packed;

  if (x < 0 || x >= width || y < 0 || y >= height)
    return 0;

  pixel = y * width + x;
  packed = data[pixel / 2];

  return pixel & 1 ? packed & 15 : packed >> 4;
}

static int StartupPixel(const byte *data, int width, int height, int x, int y, dboolean notch)
{
  if (notch)
    return NotchPixel(data, width, height, x, y);

  return PlanarPixel(data, width, height, x, y);
}

static void SW_GetCanvasRect(int *x, int *y, int *width, int *height)
{
  int inset;

  *x = 0;
  *y = 0;
  *width = 320;
  *height = 200;
  V_GetWideRect(x, y, width, height, VPT_STRETCH);

  // STARTUP by itself is 4:3 (640x480)
  // If added title banner, fit in 5:4 to keep aspect ratio
  if (HeaderHeight())
  {
    inset = *width / 32; // Header
    *x += inset;
    *width -= inset * 2;
  }
}

static void SW_FillCanvasRect(int x, int y, int width, int height, byte colour)
{
  int viewport_x, viewport_y, viewport_w, viewport_h;
  int left, right, top, bottom;

  SW_GetCanvasRect(&viewport_x, &viewport_y, &viewport_w, &viewport_h);
  left = viewport_x + x * viewport_w / STARTUP_W;
  right = viewport_x + (x + width) * viewport_w / STARTUP_W;
  top = viewport_y + y * viewport_h / CanvasHeight();
  bottom = viewport_y + (y + height) * viewport_h / CanvasHeight();
  V_FillRect(left, top, MAX(1, right - left), MAX(1, bottom - top), colour);
}

static void SW_DrawBlock(const byte *data, int x, int y, int width, int height, dboolean notch)
{
  int header_height = HeaderHeight();
  int canvas_height = STARTUP_H + header_height;
  int viewport_x = 0;
  int viewport_y = 0;
  int viewport_w = 320;
  int viewport_h = 200;
  int sy;

  SW_GetCanvasRect(&viewport_x, &viewport_y, &viewport_w, &viewport_h);

  for (sy = 0; sy < height; ++sy)
  {
    int sx = 0;
    while (sx < width)
    {
      int run = sx + 1;
      int colour = StartupPixel(data, width, height, sx, sy, notch);
      int left, right, top, bottom;

      while (run < width && StartupPixel(data, width, height, run, sy, notch) == colour)
        ++run;

      left = viewport_x + (x + sx) * viewport_w / STARTUP_W;
      right = viewport_x + (x + run) * viewport_w / STARTUP_W;
      top = viewport_y + (header_height + y + sy) * viewport_h / canvas_height;
      bottom = viewport_y + (header_height + y + sy + 1) * viewport_h / canvas_height;
      V_FillRect(left, top, MAX(1, right - left), MAX(1, bottom - top), palette_map[colour]);
      sx = run;
    }
  }
}

static void DrawBlock(const byte *data, int x, int y, int width, int height, dboolean notch)
{
  if (V_IsOpenGLMode())
  {
    GL_CopyBlock(data, x, y, width, height, notch);
    GL_DrawStartup();
  }
  else // Software
  {
    SW_DrawBlock(data, x, y, width, height, notch);
    I_FinishUpdate();
  }
}

static void Hexen_DrawBanner(void)
{
  const char *title = dsda_GameInfoStartupTitle();
  dsda_gameinfo_color_t foreground_rgb;
  dsda_gameinfo_color_t background_rgb;
  byte foreground;
  byte background;
  int length;
  int font_width = (int)normal_font.w;
  int font_height = (int)normal_font.h;
  int x;
  int i;

  if (!title || !*title)
    return;

  if (!dsda_GameInfoStartupColors(&foreground_rgb, &background_rgb))
  {
    foreground_rgb.r = foreground_rgb.g = foreground_rgb.b = 255;
    background_rgb.r = background_rgb.g = background_rgb.b = 0;
  }

  foreground = V_BestColor(startup_dynamic_palette, foreground_rgb.r, foreground_rgb.g, foreground_rgb.b);
  background = V_BestColor(startup_dynamic_palette, background_rgb.r, background_rgb.g, background_rgb.b);
  length = MIN((int)strlen(title), 40);
  x = (STARTUP_W - length * font_width) / 2;

  SW_FillCanvasRect(0, 0, STARTUP_W, HEADER_H, background);

  for (i = 0; i < length; ++i)
  {
    const byte *glyph = normal_font.data + (unsigned char)title[i] * font_height;
    int row;

    for (row = 0; row < font_height; ++row)
    {
      int column;
      for (column = 0; column < font_width; ++column)
      {
        if (glyph[row] & (1 << column))
        {
          int xx = x + i * font_width + column;
          int yy = row + 7;
          SW_FillCanvasRect(xx, yy, 1, 1, foreground);
        }
      }
    }
  }
}

static void Hexen_DrawStartup(void)
{
  startup_shown = true;

  if (V_IsOpenGLMode())
    RETURN(GL_DrawStartup());

  V_SetDynamicPalette(startup_dynamic_palette);

  V_FillRect(0, 0, SCREENWIDTH, SCREENHEIGHT, BORDER_COLOR);

  DrawBlock(startup_bitmap, 0, 0, STARTUP_W, STARTUP_H, false);
  Hexen_DrawBanner();
  I_FinishUpdate();
}

static void Hexen_UpdateStartupPalette(const byte *startup, int fade_step)
{
  for (int i = 0; i < 16; ++i)
  {
    palette_map[i] = i;
    startup_dynamic_palette[i * 3]      = ExpandVGAColor(startup[i * 3])      * fade_step / FADE_STEPS;
    startup_dynamic_palette[i * 3 + 1]  = ExpandVGAColor(startup[i * 3 + 1])  * fade_step / FADE_STEPS;
    startup_dynamic_palette[i * 3 + 2]  = ExpandVGAColor(startup[i * 3 + 2])  * fade_step / FADE_STEPS;
  }

  startup_dynamic_palette[BORDER_COLOR * 3] = 0;
  startup_dynamic_palette[BORDER_COLOR * 3 + 1] = 0;
  startup_dynamic_palette[BORDER_COLOR * 3 + 2] = 0;
}

static void Hexen_FadeStartup(const byte *startup)
{
  int step;

  for (step = 0; step <= FADE_STEPS; ++step)
  {
    if (StartupSkipped())
      return;

    Hexen_UpdateStartupPalette(startup, step);
    Hexen_DrawStartup();

    if (step < FADE_STEPS)
      I_uSleep(FADE_TIME_US / FADE_STEPS);
  }
}

static void Hexen_InitStartup(void)
{
  const byte *startup;
  const char *startup_song;
  int lump;
  int notch_lump;
  dboolean hexen_startup_fade;

  startup_active = false;
  notch_position = 0;
  startup_bitmap = NULL;
  notch_bitmap = hexen_notch_table;

  lump = W_CheckNumForName("STARTUP");

  if (lump == LUMP_NOT_FOUND)
  {
    lprintf(LO_WARN, "Hexen startup: STARTUP lump not found.\n");
    return;
  }

  if (W_LumpLength(lump) < STARTUP_PAL_SIZE + STARTUP_W * STARTUP_H / 2)
  {
    lprintf(LO_WARN, "Hexen startup: STARTUP lump has unexpected size %d.\n", W_LumpLength(lump));
    return;
  }

  startup = W_LumpByNum(lump);
  startup_bitmap = startup + STARTUP_PAL_SIZE;

  notch_lump = W_CheckNumForName("NOTCH");

  if (notch_lump != LUMP_NOT_FOUND)
  {
    if (W_PWADLumpNumExists2(notch_lump) && W_LumpLength(notch_lump) == NOTCH_W * NOTCH_H / 2)
    {
      memcpy(custom_notch, W_LumpByNum(notch_lump), sizeof(custom_notch));
      notch_bitmap = custom_notch;
      lprintf(LO_DEBUG, "Hexen startup: using PWAD NOTCH lump.\n");
    }
  }

  if (V_IsOpenGLMode())
    GL_InitStartup();

  startup_song = dsda_GameInfoStartupSong();

  if (startup_song)
    S_StartSongName(startup_song, true);
  else if (hexen)
    S_StartSongName("orb", true);

  // Vanilla Hexen has a palette fade for STARTUP. ZDoom ports do not.
  // Only do the fade, if GAMEINFO doesn't exist.
  hexen_startup_fade = hexen && dsda_GameInfoStartupType() == dsda_startup_default;

  if (hexen_startup_fade)
    Hexen_FadeStartup(startup);
  else
  {
    Hexen_UpdateStartupPalette(startup, NO_FADE);
    Hexen_DrawStartup();
  }

  startup_active = true;
  lprintf(LO_DEBUG, "Hexen startup: displaying STARTUP lump.\n");
}

static void Hexen_DrawProgressNotch(void)
{
  DrawBlock(notch_bitmap, PROGRESS_X + notch_position * NOTCH_W, PROGRESS_Y, NOTCH_W, NOTCH_H, true);

  if (hexen)
    S_StartVoidSound(hexen_sfx_startup_tick);

  ++notch_position;
}

static void Hexen_FinishStartup(void)
{
  if (gl_startup_size)
    GL_FinishStartup();

  if (startup_active)
  {
    V_ClearDynamicPalette();

    if (V_IsSoftwareMode())
      V_FillRect(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
  }

  startup_active = false;
}

void dsda_HexenStartup(void)
{
  Hexen_InitStartup();

  // Draw progress bar
  for (int i = 0; i < MAX_NOTCHES; ++i)
  {
    unsigned int start_time;

    if (!startup_active || StartupSkipped())
      break;

    start_time = SDL_GetTicks();
    Hexen_DrawProgressNotch();
    dsda_WaitForProgressStep(start_time);
  }

  Hexen_FinishStartup();
}

//
// Startup OpenGL stuff
//

static void GL_SetupOrtho(int w, int h)
{
  const int logical_width = STARTUP_W;
  const int logical_height = CanvasHeight();
  float scale, scale_x, scale_y;
  float scaled_width, scaled_height;
  float offset_x, offset_y;

  scale_x = (float)w / logical_width;
  scale_y = (float)h / logical_height;
  scale = MIN(scale_x, scale_y);

  scaled_width = logical_width * scale;
  scaled_height = logical_height * scale;

  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  offset_x = (w - scaled_width) / 2.0f;
  offset_y = (h - scaled_height) / 2.0f;

  glTranslatef(offset_x, offset_y, 0.0f);
  glScalef(scale, scale, 1.0f);
}

static void GL_CopyBlock(const byte *data, int x, int y, int width, int height, dboolean notch)
{
  int sx, sy;

  for (sy = 0; sy < height; ++sy)
    for (sx = 0; sx < width; ++sx)
      gl_startup_size[(y + sy) * STARTUP_W + x + sx] = StartupPixel(data, width, height, sx, sy, notch);
}

static void GL_InitStartup(void)
{
  int drawable_w, drawable_h;
  int tex_w2 = STARTUP_W * 2;
  int tex_h2 = CanvasHeight() * 2;

  gl_startup_size = Z_Malloc(STARTUP_W * STARTUP_H);
  GL_CopyBlock(startup_bitmap, 0, 0, STARTUP_W, STARTUP_H, false);

  gl_texture_data = Z_Malloc(tex_w2 * tex_h2 * 4);

  SDL_GL_GetDrawableSize(I_GetSDLWindow(), &drawable_w, &drawable_h);
  GL_SetupOrtho(drawable_w, drawable_h);

}

static void GL_SetPixel(int tex_w, int x, int y, int r, int g, int b)
{
  int dx, dy;
  int tex_w2 = tex_w * 2;

  for (dy = 0; dy < 2; ++dy)
    for (dx = 0; dx < 2; ++dx)
    {
      int idx = ((y * 2 + dy) * tex_w2 + x * 2 + dx) * 4;
      gl_texture_data[idx + 0] = r;
      gl_texture_data[idx + 1] = g;
      gl_texture_data[idx + 2] = b;
      gl_texture_data[idx + 3] = 255;
    }
}

static void GL_DrawBanner(int width, int header_height)
{
  int length;
  int x;
  int i;

  const char *title = dsda_GameInfoStartupTitle();
  dsda_gameinfo_color_t foreground = { 255, 255, 255 };
  dsda_gameinfo_color_t background = { 0, 0, 0 };

  dsda_GameInfoStartupColors(&foreground, &background);

  if (!header_height || !title || !*title)
    return;

  for (i = 0; i < width * header_height; ++i)
    GL_SetPixel(width, i % width, i / width, background.r, background.g, background.b);

  length = MIN((int)strlen(title), 40);
  x = (width - length * (int)normal_font.w) / 2;

  for (i = 0; i < length; ++i)
  {
    const byte *glyph = normal_font.data + (unsigned char)title[i] * normal_font.h;
    int row, column;

    for (row = 0; row < (int)normal_font.h; ++row)
      for (column = 0; column < (int)normal_font.w; ++column)
        if (glyph[row] & (1 << column))
          GL_SetPixel(width, x + i * (int)normal_font.w + column, 7 + row, foreground.r, foreground.g, foreground.b);
  }
}

static void GL_DrawStartup(void)
{
  const byte *palette = startup_dynamic_palette;
  int width = STARTUP_W;
  int height = STARTUP_H;
  int header_height = HeaderHeight();
  int canvas_height = height + header_height;
  int tex_w2 = width * 2;
  int tex_h2 = canvas_height * 2;
  int i;

  for (i = 0; i < width * height; ++i)
  {
    int colour = gl_startup_size[i];
    int destination = i + width * header_height;
    GL_SetPixel(width, destination % width, destination / width, palette[colour * 3], palette[colour * 3 + 1], palette[colour * 3 + 2]);
  }

  GL_DrawBanner(width, header_height);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw 2x texture
  if (gl_texture == 0)
  {
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w2, tex_h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, gl_texture_data);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w2, tex_h2, GL_RGBA, GL_UNSIGNED_BYTE, gl_texture_data);
  }

  // Draw 2x texture (image) normal size in window
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glBegin(GL_QUADS);
      glTexCoord2f(0, 0); glVertex2f(0, 0);
      glTexCoord2f(1, 0); glVertex2f((GLfloat)width, 0);
      glTexCoord2f(1, 1); glVertex2f((GLfloat)width, (GLfloat)canvas_height);
      glTexCoord2f(0, 1); glVertex2f(0, (GLfloat)canvas_height);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  SDL_GL_SwapWindow(I_GetSDLWindow());
}

static void GL_FinishStartup(void)
{
  glDeleteTextures(1, &gl_texture);
  gl_texture = 0;

  Z_Free(gl_texture_data);
  Z_Free(gl_startup_size);
  gl_texture_data = NULL;
  gl_startup_size = NULL;

  GL_RestoreStartup();
}

static void GL_RestoreStartup(void)
{
  gld_EnableTexture2D(GL_TEXTURE0_ARB, false);
  dsda_GLSetRenderViewport();
  dsda_GLSetRenderViewportScissor();
  gld_Set2DMode();
  gld_ResetLastTexture();
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(I_GetSDLWindow());
  glClear(GL_COLOR_BUFFER_BIT);
}

//
// HERETIC STARTUP
//
// Heretic is pretty simple since it reuses ENDOOM code
//

#define TEXT_W 80
#define TEXT_H 25
#define THERM_X 14
#define THERM_Y 14
#define THERM_LENGTH 52

void dsda_HereticStartup(void)
{
  unsigned char *screen;
  const byte *loading;
  const char *title;
  dsda_gameinfo_color_t foreground = { 255, 255, 255 };
  dsda_gameinfo_color_t background = { 0, 0, 0 };
  int lump;
  int i;

  lump = W_CheckNumForName("LOADING");

  if (lump == LUMP_NOT_FOUND || W_LumpLength(lump) != TEXT_W * TEXT_H * 2)
  {
    lprintf(LO_WARN, "Heretic startup: valid LOADING lump not found.\n");
    return;
  }

  title = dsda_GameInfoStartupTitle();

  if (title && *title)
  {
    dsda_GameInfoStartupColors(&foreground, &background);
    TXT_SetHeader(title);
  }

  TXT_PreInit(I_GetSDLWindow(), I_GetSDLRenderer(), V_IsOpenGLMode());

  if (!TXT_Init())
  {
    lprintf(LO_ERROR, "Heretic startup: failed to initialize textscreen.\n");
    return;
  }

  if (title && *title)
  {
    TXT_SetColor(TXT_COLOR_HEADER_FOREGROUND, foreground.r, foreground.g, foreground.b);
    TXT_SetColor(TXT_COLOR_HEADER_BACKGROUND, background.r, background.g, background.b);
  }

  loading = W_LumpByNum(lump);
  screen = TXT_GetScreenData();
  memcpy(screen, loading, TEXT_W * TEXT_H * 2);
  TXT_UpdateScreen();
  startup_shown = true;

  for (i = 0; i < THERM_LENGTH; ++i)
  {
    int offset;
    unsigned int start_time;

    if (StartupSkipped())
      break;

    start_time = SDL_GetTicks();
    offset = ((THERM_Y * TEXT_W) + THERM_X + i) * 2;
    screen[offset] = 0xdb;
    screen[offset + 1] = 0x2a;
    TXT_UpdateScreen();
    dsda_WaitForProgressStep(start_time);
  }

  TXT_Shutdown();

  if (V_IsSoftwareMode())
  {
    SDL_RenderSetLogicalSize(I_GetSDLRenderer(), SCREENWIDTH, ACTUALHEIGHT);
    V_FillRect(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
  }
  else
    GL_RestoreStartup();

}

//
// Main Function
//

void dsda_Startup(const char* game_name)
{
  dsda_startup_type_t type;

  startup_shown = false;
  startup_skipped = false;

  dsda_LoadGameInfo();                          // Get STARTUP info from GAMEINFO
  dsda_SetGameInfoStartupDefaults(game_name);   // Set STARTUP defaults

  type = dsda_GameInfoStartupType();

  // We don't support doom startups cuz they allow for formats like PNG
  if (type == dsda_startup_default)
    type = hexen ? dsda_startup_hexen : heretic ? dsda_startup_heretic : dsda_startup_none;

  if (type == dsda_startup_hexen)
    dsda_HexenStartup();
  else if (type == dsda_startup_heretic)
    dsda_HereticStartup();

  if (startup_shown)
    ClearKeyPresses();
}
