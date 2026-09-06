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
#include "r_png.h"
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
static byte startup_dynamic_palette[256 * 3];
static const byte *startup_bitmap;
static int notch_position;
static byte custom_notch[NOTCH_W * NOTCH_H / 2];
static const byte *notch_bitmap = hexen_notch_table;
static byte *startup_rgba;
static GLuint gl_texture;
static png_t startup_png;
static png_t notch_png;

static void InitStartupCanvas(void);
static void StartupSetPixel(int tex_w, int x, int y, int r, int g, int b);
static void GL_UpdateStartup(void);
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

static int HeaderHeight(void)
{
  const char *title = dsda_GameInfoStartupTitle();

  return title && *title ? HEADER_H : 0;
}

static int CanvasHeight(void)
{
  return STARTUP_H + HeaderHeight();
}

//
// STARTUP OPENGL STUFF
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

static void GL_InitStartupCanvas(void)
{
  int drawable_w, drawable_h;
  SDL_GL_GetDrawableSize(I_GetSDLWindow(), &drawable_w, &drawable_h);
  GL_SetupOrtho(drawable_w, drawable_h);
}


static void GL_UpdateStartup(void)
{
  int width = STARTUP_W;
  int canvas_height = CanvasHeight();
  int tex_w2 = width * 2;
  int tex_h2 = canvas_height * 2;

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw 2x texture
  if (gl_texture == 0)
  {
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w2, tex_h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, startup_rgba);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w2, tex_h2, GL_RGBA, GL_UNSIGNED_BYTE, startup_rgba);
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
// STARTUP CANVAS
//

static void UpdateStartup(void)
{
  int width;
  int canvas_height;
  int tex_w2;
  int tex_h2;
  SDL_Renderer *renderer;
  SDL_Texture *texture;

  if (V_IsOpenGLMode())
    RETURN(GL_UpdateStartup());

  width = STARTUP_W;
  canvas_height = CanvasHeight();
  tex_w2 = width * 2;
  tex_h2 = canvas_height * 2;
  renderer = I_GetSDLRenderer();
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, tex_w2, tex_h2);

  if (!texture)
    return;

  // Software startup rendering uses the RGBA canvas (for PNG support)
  // Instead of previously using the indexed game framebuffer
  SDL_UpdateTexture(texture, NULL, startup_rgba, tex_w2 * 4);
  SDL_RenderSetLogicalSize(renderer, width, canvas_height);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture);
  SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, ACTUALHEIGHT);
}

static void DrawStartupBanner(void)
{
  const char *title = dsda_GameInfoStartupTitle();
  dsda_gameinfo_color_t foreground = { 255, 255, 255 };
  dsda_gameinfo_color_t background = { 0, 0, 0 };
  int font_width = (int)normal_font.w;
  int font_height = (int)normal_font.h;
  int header_height = HeaderHeight();
  int width = STARTUP_W;
  int length;
  int x;

  if (!header_height || !title || !*title)
    return;

  dsda_GameInfoStartupColors(&foreground, &background);

  for (int i = 0; i < width * header_height; ++i)
    StartupSetPixel(width, i % width, i / width, background.r, background.g, background.b);

  length = MIN((int)strlen(title), 40);
  x = (width - length * font_width) / 2;

  for (int i = 0; i < length; ++i)
  {
    const byte *glyph = normal_font.data + (unsigned char)title[i] * font_height;

    for (int row = 0; row < font_height; ++row)
    {
      for (int column = 0; column < font_width; ++column)
      {
        if (glyph[row] & (1 << column))
        {
          int xx = x + i * font_width + column;
          int yy = row + 7;
          StartupSetPixel(width, xx, yy, foreground.r, foreground.g, foreground.b);
        }
      }
    }
  }
}

static void StartupSetPixel(int tex_w, int x, int y, int r, int g, int b)
{
  int dx, dy;
  int tex_w2 = tex_w * 2;

  for (dy = 0; dy < 2; ++dy)
    for (dx = 0; dx < 2; ++dx)
    {
      int idx = ((y * 2 + dy) * tex_w2 + x * 2 + dx) * 4;
      startup_rgba[idx + 0] = r;
      startup_rgba[idx + 1] = g;
      startup_rgba[idx + 2] = b;
      startup_rgba[idx + 3] = 255;
    }
}

static void BlitPNG(const png_t *png, int x, int y)
{
  int sx, sy;

  for (sy = 0; sy < png->height; ++sy)
    for (sx = 0; sx < png->width; ++sx)
    {
      const byte *pixel = png->image + (sy * png->width + sx) * 4;

      if (pixel[3] >= 128)
        StartupSetPixel(STARTUP_W, x + sx, y + sy,
                        pixel[0],
                        pixel[1],
                        pixel[2]);
    }
}

static void InitStartupCanvas(void)
{
  int tex_w2 = STARTUP_W * 2;
  int tex_h2 = CanvasHeight() * 2;

  startup_rgba = Z_Malloc(tex_w2 * tex_h2 * 4);
  memset(startup_rgba, 0, tex_w2 * tex_h2 * 4);

  if (startup_png.image)
    BlitPNG(&startup_png, 0, HeaderHeight());

  DrawStartupBanner();

  if (V_IsOpenGLMode())
    GL_InitStartupCanvas();
}

static void FinishStartup(void)
{
  if (V_IsOpenGLMode())
    GL_FinishStartup();

  if (startup_rgba)
  {
    Z_Free(startup_rgba);
    startup_rgba = NULL;
  }

  if (startup_png.image)
    FreePNG(&startup_png);
  if (notch_png.image)
    FreePNG(&notch_png);

  startup_active = false;
}

//
// HEXEN STARTUP
//

static int Hexen_StartupPixel(const byte *data, int width, int height, int x, int y, dboolean notch)
{
  // Hexen Notch
  if (notch)
  {
    int pixel;
    byte packed;

    if (x < 0 || x >= width || y < 0 || y >= height)
      return 0;

    pixel = y * width + x;
    packed = data[pixel / 2];

    return pixel & 1 ? packed & 15 : packed >> 4;
  }

  // Hexen Planar
  else
  {
    int plane;
    int bit = y * width + x;
    int plane_size = width * height / 8;
    int colour = 0;

    for (plane = 0; plane < 4; ++plane)
      colour |= ((data[plane * plane_size + bit / 8] >> (7 - bit % 8)) & 1) << plane;

    return colour;
  }

  // Should never reach here
  return false;
}

static void Hexen_DrawBlock(const byte *data, int x, int y, int width, int height, dboolean notch)
{
  for (int sy = 0; sy < height; ++sy)
  {
    for (int sx = 0; sx < width; ++sx)
    {
      int color = Hexen_StartupPixel(data, width, height, sx, sy, notch);

      StartupSetPixel(STARTUP_W, x + sx, HeaderHeight() + y + sy,
                      startup_dynamic_palette[color * 3],
                      startup_dynamic_palette[color * 3 + 1],
                      startup_dynamic_palette[color * 3 + 2]);
    }
  }

  UpdateStartup();
}

static void Hexen_DrawStartup(void)
{
  startup_shown = true;

  if (startup_png.image)
    UpdateStartup();
  else
    Hexen_DrawBlock(startup_bitmap, 0, 0, STARTUP_W, STARTUP_H, false);
}

static int ExpandVGAColor(int color)
{
  return (color << 2) | (color >> 4);
}

static void Hexen_UpdateStartupPalette(const byte *startup, int fade_step)
{
  // Skip fade for PNG
  if (startup_png.image)
    return;

  for (int i = 0; i < 16; ++i)
  {
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

  // Skip fade for PNG
  if (startup_png.image)
  {
    Hexen_DrawStartup();
    return;
  }

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

  memset(&startup_png, 0, sizeof(startup_png));
  memset(&notch_png, 0, sizeof(notch_png));

  lump = W_CheckNumForName("STARTUP");

  if (lump == LUMP_NOT_FOUND)
  {
    lprintf(LO_WARN, "Hexen startup: STARTUP lump not found.\n");
    return;
  }

  // PNG Startup
  if (R_IsPNGLump(lump))
  {
    // Invalid PNG
    if (!InitPNG(&startup_png, W_LumpByNum(lump), W_LumpLength(lump)) || !DecodePNG_RGBA(&startup_png) ||
        startup_png.width != STARTUP_W || startup_png.height != STARTUP_H)
    {
      FreePNG(&startup_png);
      lprintf(LO_WARN, "Hexen startup: invalid PNG STARTUP lump.\n");
      return;
    }

    startup = NULL;
  }
  // Planar Startup
  else
  {
    if (W_LumpLength(lump) < STARTUP_PAL_SIZE + STARTUP_W * STARTUP_H / 2)
    {
      lprintf(LO_WARN, "Hexen startup: STARTUP lump has unexpected size %d.\n", W_LumpLength(lump));
      return;
    }

    startup = W_LumpByNum(lump);
    startup_bitmap = startup + STARTUP_PAL_SIZE;
  }

  notch_lump = W_CheckNumForName("NOTCH");

  if (notch_lump != LUMP_NOT_FOUND)
  {
    // PNG Notch
    if (R_IsPNGLump(notch_lump))
    {
      // Invalid PNG
      if (!(InitPNG(&notch_png, W_LumpByNum(notch_lump), W_LumpLength(notch_lump)) && DecodePNG_RGBA(&notch_png) &&
          notch_png.width == NOTCH_W && notch_png.height == NOTCH_H))
      {
        FreePNG(&notch_png);
        memset(&notch_png, 0, sizeof(notch_png));
      }
      else
        lprintf(LO_DEBUG, "Hexen startup: using PNG NOTCH lump.\n");
    }
    // Default Notch
    else if (W_PWADLumpNumExists2(notch_lump) && W_LumpLength(notch_lump) == NOTCH_W * NOTCH_H / 2)
    {
      memcpy(custom_notch, W_LumpByNum(notch_lump), sizeof(custom_notch));
      notch_bitmap = custom_notch;
      lprintf(LO_DEBUG, "Hexen startup: using PWAD NOTCH lump.\n");
    }
  }

  InitStartupCanvas();

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
  if (notch_png.image)
  {
    BlitPNG(&notch_png, PROGRESS_X + notch_position * NOTCH_W, HeaderHeight() + PROGRESS_Y);
    UpdateStartup();
  }
  else
    Hexen_DrawBlock(notch_bitmap, PROGRESS_X + notch_position * NOTCH_W, PROGRESS_Y, NOTCH_W, NOTCH_H, true);

  if (hexen)
    S_StartVoidSound(hexen_sfx_startup_tick);

  ++notch_position;
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

  FinishStartup();
}

//
// DOOM STARTUP
//
//

static void dsda_DoomStartup(void)
{
  const char *startup_song;
  int lump = W_CheckNumForName("STARTUP");
  int i;

  startup_active = false;
  memset(&startup_png, 0, sizeof(startup_png));
  memset(&notch_png, 0, sizeof(notch_png));

  if (lump == LUMP_NOT_FOUND || !R_IsPNGLump(lump))
  {
    lprintf(LO_WARN, "Doom startup: valid 640x480 PNG STARTUP lump not found.\n");
    return;
  }

  if (!InitPNG(&startup_png, W_LumpByNum(lump), W_LumpLength(lump)) ||
      !DecodePNG_RGBA(&startup_png) || startup_png.width != STARTUP_W ||
      startup_png.height != STARTUP_H)
  {
    FreePNG(&startup_png);
    lprintf(LO_WARN, "Doom startup: valid 640x480 PNG STARTUP lump not found.\n");
    return;
  }

  InitStartupCanvas();
  UpdateStartup();
  startup_shown = true;
  startup_active = true;

  startup_song = dsda_GameInfoStartupSong();
  if (startup_song)
    S_StartSongName(startup_song, true);

  for (i = 0; i < MAX_NOTCHES; ++i)
  {
    unsigned int start_time;

    if (StartupSkipped())
      break;

    start_time = SDL_GetTicks();
    dsda_WaitForProgressStep(start_time);
  }

  FinishStartup();
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

  if (type == dsda_startup_default)
    type = hexen ? dsda_startup_hexen : heretic ? dsda_startup_heretic : dsda_startup_none;

  if (type == dsda_startup_doom)
    dsda_DoomStartup();
  else if (type == dsda_startup_hexen)
    dsda_HexenStartup();
  else if (type == dsda_startup_heretic)
    dsda_HereticStartup();

  if (startup_shown)
    ClearKeyPresses();
}
