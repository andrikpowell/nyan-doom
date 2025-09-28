//
// Copyright(C) 2025 Andrik Powell
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
//
// OpenGL Text mode emulation in SDL
//

#include <stdlib.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_gl.h"
#include "txt_main.h"
#include "txt_utf8.h"
#include "fonts/normal.h"
#include "lprintf.h"

#define FONT_CHAR_W 8
#define FONT_CHAR_H 16
#define FONT_COLS 16
#define FONT_ROWS 16
#define FONT_WIDTH 128
#define FONT_HEIGHT 256
#define SCREEN_COLS 80
#define SCREEN_ROWS 25

unsigned char font_texture[FONT_WIDTH * FONT_HEIGHT];
unsigned char font_texture_rgba[FONT_WIDTH * FONT_HEIGHT * 4];

// Time between character blinks in ms

#define BLINK_PERIOD 250

static unsigned char *glscreendata;
static SDL_Window *glwindow = NULL;
static GLuint font_tex = 0;

static TxtSDLEventCallbackFunc event_callback;
static void *event_callback_data;

// Mapping from SDL keyboard scancode to internal key code.
static const int scancode_translate_table[] = SCANCODE_TO_KEYS_ARRAY;

// Current input mode.
static txt_input_mode_t input_mode = TXT_INPUT_NORMAL;

static int glwindow_w = 0;
static int glwindow_h = 0;

static void GL_TXT_SetupOrtho(int w, int h)
{
    const int logical_width = SCREEN_COLS * FONT_CHAR_W;
    const int logical_height = SCREEN_ROWS * FONT_CHAR_H;

    // Apply aspect ratio correction.
    const int logical_h = logical_height * 6 / 5;

    float scale_x = (float)w / logical_width;
    float scale_y = (float)h / logical_h;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    // Calculate the scaled width and height
    float scaled_width = logical_width * scale;
    float scaled_height = logical_height * scale;
    float scaled_h = logical_h * scale;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, scaled_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // offsets to center the image
    float offset_x = (w - scaled_width) / 2.0f;
    float offset_y = (h - scaled_h) / 2.0f;

    // Apply translation and scaling
    glTranslatef(offset_x, offset_y, 0.0f);
    glScalef(scale, scale, 1.0f);
}

int GL_TXT_CreateFontTexture(void)
{
    int i;
    int ch;
    int tx, ty;
    int row, col;

    for (ch = 0; ch < 256; ++ch) {
        tx = (ch % FONT_COLS) * FONT_CHAR_W;
        ty = (ch / FONT_COLS) * FONT_CHAR_H;

        for (row = 0; row < FONT_CHAR_H; ++row) {
                unsigned char bits = normal_font_data[ch * FONT_CHAR_H + row];
                for (col = 0; col < FONT_CHAR_W; ++col) {
                    int bit, x, y;
                    bit = (bits >> (7 - col)) & 1;
                    x = tx + col;
                    y = ty + row;
                    font_texture[y * FONT_WIDTH + x] = bit ? 255 : 0;
            }
        }
    }

    for (i = 0; i < FONT_WIDTH * FONT_HEIGHT; i++) {
        font_texture_rgba[i * 4 + 0] = 255;                 // R
        font_texture_rgba[i * 4 + 1] = 255;                 // G
        font_texture_rgba[i * 4 + 2] = 255;                 // B
        font_texture_rgba[i * 4 + 3] = font_texture[i];     // Alpha
    }

    glGenTextures(1, &font_tex);
    glBindTexture(GL_TEXTURE_2D, font_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FONT_WIDTH, FONT_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, font_texture_rgba);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return 1;
}

void GL_TXT_UpdateCharacter(int x, int y, unsigned char ch)
{
    const int char_w = FONT_CHAR_W;
    const int char_h = FONT_CHAR_H;
    const int tex_w = FONT_WIDTH;
    const int tex_h = FONT_HEIGHT;

    int col = ch % FONT_COLS;
    int row = ch / FONT_COLS;

    const float padding = 0.5f; // half a pixel padding
    float pw = 1.0f / (float)tex_w;
    float ph = 1.0f / (float)tex_h;

    float tx = col * char_w * pw + padding * pw;
    float ty = row * char_h * ph + padding * ph;
    float tw = (char_w - 2 * padding) * pw;
    float th = (char_h - 2 * padding) * ph;

    float ty_top = ty;
    float ty_bottom = ty + th;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    glBegin(GL_QUADS);

    // Bottom-left
    glTexCoord2f(tx + tw, ty_bottom);
    glVertex2i(x, y + char_h);

    // Bottom-right
    glTexCoord2f(tx, ty_bottom);
    glVertex2i(x + char_w, y + char_h);

    // Top-right
    glTexCoord2f(tx, ty_top);
    glVertex2i(x + char_w, y);

    // Top-left
    glTexCoord2f(tx + tw, ty_top);
    glVertex2i(x, y);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void GL_TXT_PreInit(SDL_Window *preset_window)
{
    if (preset_window != NULL)
    {
        glwindow = preset_window;
    }
}

int GL_TXT_Init(void)
{
    if (!GL_TXT_CreateFontTexture())
    {
        lprintf(LO_ERROR, "Failed to create OpenGL font texture!\n");
        return 0;
    }

    if (!glwindow)
        return 0;

    SDL_GetWindowSize(glwindow, &glwindow_w, &glwindow_h);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GL_TXT_SetupOrtho(glwindow_w, glwindow_h);

    glscreendata = malloc(TXT_SCREEN_W * TXT_SCREEN_H * 2);
    memset(glscreendata, 0, TXT_SCREEN_W * TXT_SCREEN_H * 2);

    return 1;
}

void GL_TXT_Shutdown(void)
{
    if (font_tex != 0)
    {
        glDeleteTextures(1, &font_tex);
        font_tex = 0;
    }

    free(glscreendata);
    glscreendata = NULL;
}

unsigned char *GL_TXT_SetScreenData(void)
{
    return glscreendata;
}

static const float glpalette[16][3] = {
    {0, 0, 0},        // 0: Black
    {0, 0, 170},      // 1: Blue
    {0, 170, 0},      // 2: Green
    {0, 170, 170},    // 3: Cyan
    {170, 0, 0},      // 4: Red
    {170, 0, 170},    // 5: Magenta
    {170, 85, 0},     // 6: Brown
    {170, 170, 170},  // 7: Gray
    {85, 85, 85},     // 8: Dark gray
    {85, 85, 255},    // 9: Bright blue
    {85, 255, 85},    // 10: Bright green
    {85, 255, 255},   // 11: Bright cyan
    {255, 85, 85},    // 12: Bright red
    {255, 85, 255},   // 13: Bright magenta
    {255, 255, 85},   // 14: Yellow
    {255, 255, 255},  // 15: Bright white
};

static void GL_TXT_SetColorFromPalette(int index)
{
    glColor4f(glpalette[index][0] / 255.0f,
             glpalette[index][1] / 255.0f,
             glpalette[index][2] / 255.0f,
             1.0f);
}

int GL_TXT_ScreenHasBlinkingChars(void)
{
    int x, y;
    unsigned char *p;

    // Check all characters in screen buffer

    for (y=0; y<TXT_SCREEN_H; ++y)
    {
        for (x=0; x<TXT_SCREEN_W; ++x) 
        {
            p = &glscreendata[(y * TXT_SCREEN_W + x) * 2];

            if (p[1] & 0x80)
            {
                // This character is blinking

                return 1;
            }
        }
    }

    // None found

    return 0;
}

void GL_TXT_Sleep(int timeout)
{
    unsigned int start_time;

    if (GL_TXT_ScreenHasBlinkingChars())
    {
        int time_to_next_blink;

        time_to_next_blink = BLINK_PERIOD - (SDL_GetTicks() % BLINK_PERIOD);

        // There are blinking characters on the screen, so we 
        // must time out after a while
       
        if (timeout == 0 || timeout > time_to_next_blink)
        {
            // Add one so it is always positive

            timeout = time_to_next_blink + 1;
        }
    }

    if (timeout == 0)
    {
        // We can just wait forever until an event occurs
        SDL_WaitEvent(NULL);
    }
    else
    {
        // Sit in a busy loop until the timeout expires or we have to
        // redraw the blinking screen

        start_time = SDL_GetTicks();

        while (SDL_GetTicks() < start_time + timeout)
        {
            if (SDL_PollEvent(NULL) != 0)
            {
                // Received an event, so stop waiting
                break;
            }

            // Don't hog the CPU
            SDL_Delay(1);
        }
    }
}

void GL_TXT_UpdateScreen(void)
{
    if (!glscreendata)
        return;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);

    for (int y = 0; y < SCREEN_ROWS; y++) {
        for (int x = 0; x < SCREEN_COLS; x++) {
            int idx = (y * SCREEN_COLS + x) * 2;
            unsigned char chr = glscreendata[idx];
            unsigned char attr = glscreendata[idx + 1];

            int fg = attr & 0x0F;            // 16-color fg
            int bg = (attr >> 4) & 0x0F;     // 16-color bg fixed here

            if (bg & 0x8)
            {
                // blinking

                bg &= ~0x8;

                if (((SDL_GetTicks() / BLINK_PERIOD) % 2) == 0)
                {
                    fg = bg;
                }
            }

            float px = (float)(x * FONT_CHAR_W);
            float py = (float)(y * FONT_CHAR_H);

            // Draw background
            GL_TXT_SetColorFromPalette(bg);
            glBegin(GL_QUADS);
            glVertex2f(px, py);
            glVertex2f(px + FONT_CHAR_W, py);
            glVertex2f(px + FONT_CHAR_W, py + FONT_CHAR_H);
            glVertex2f(px, py + FONT_CHAR_H);
            glEnd();

            // Draw foreground
            GL_TXT_SetColorFromPalette(fg);
            GL_TXT_UpdateCharacter((int)px, (int)py, chr);
        }
    }

    glDisable(GL_BLEND);
    SDL_GL_SwapWindow(glwindow);
}

//
// Translates the SDL key
//

// XXX: duplicate from doomtype.h
#define arrlen(array) (sizeof(array) / sizeof(*array))

static int GL_TXT_TranslateScancode(SDL_Scancode scancode)
{
    switch (scancode)
    {
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
            return KEY_RCTRL;

        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
            return KEY_RSHIFT;

        case SDL_SCANCODE_LALT:
            return KEY_LALT;

        case SDL_SCANCODE_RALT:
            return KEY_RALT;

        default:
            if (scancode < arrlen(scancode_translate_table))
            {
                return scancode_translate_table[scancode];
            }
            else
            {
                return 0;
            }
    }
}

static int GL_TXT_TranslateKeysym(const SDL_Keysym *sym)
{
    int translated;

    // We cheat here and make use of TranslateScancode. The range of keys
    // associated with printable characters is pretty contiguous, so if it's
    // inside that range we want the localized version of the key instead.
    translated = GL_TXT_TranslateScancode(sym->scancode);

    if (translated >= 0x20 && translated < 0x7f)
    {
        return sym->sym;
    }
    else
    {
        return translated;
    }
}

// Convert an SDL button index to textscreen button index.
//
// Note special cases because 2 == mid in SDL, 3 == mid in textscreen/setup

static int GL_TXT_SDLButtonToTXTButton(int button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            return TXT_MOUSE_LEFT;
        case SDL_BUTTON_RIGHT:
            return TXT_MOUSE_RIGHT;
        case SDL_BUTTON_MIDDLE:
            return TXT_MOUSE_MIDDLE;
        default:
            return TXT_MOUSE_BASE + button - 1;
    }
}

// Convert an SDL wheel motion to a textscreen button index.

static int GL_TXT_SDLWheelToTXTButton(const SDL_MouseWheelEvent *wheel)
{
    if (wheel->y <= 0)
    {
        return TXT_MOUSE_SCROLLDOWN;
    }
    else
    {
        return TXT_MOUSE_SCROLLUP;
    }
}

void GL_TXT_GetMousePosition(int *x, int *y)
{
    SDL_GetMouseState(x, y);

    // Translate mouse position from 'pixel' position into character position.
    // We are working here in screen coordinates and not pixels, since this is
    // what SDL_GetWindowSize() returns.
    SDL_GetWindowSize(glwindow, &glwindow_w, &glwindow_h);
    *x = ((*x) * TXT_SCREEN_W) / glwindow_w;
    *y = ((*y) * TXT_SCREEN_H) / glwindow_h;

    if (*x < 0)
    {
        *x = 0;
    }
    else if (*x >= TXT_SCREEN_W)
    {
        *x = TXT_SCREEN_W - 1;
    }
    if (*y < 0)
    {
        *y = 0;
    }
    else if (*y >= TXT_SCREEN_H)
    {
        *y = TXT_SCREEN_H - 1;
    }
}

static int GL_TXT_MouseHasMoved(void)
{
    static int last_x = 0, last_y = 0;
    int x, y;

    GL_TXT_GetMousePosition(&x, &y);

    if (x != last_x || y != last_y)
    {
        last_x = x; last_y = y;
        return 1;
    }
    else
    {
        return 0;
    }
}

signed int GL_TXT_GetChar(void)
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        // If there is an event callback, allow it to intercept this
        // event.

        if (event_callback != NULL)
        {
            if (event_callback(&ev, event_callback_data))
            {
                continue;
            }
        }

        // Process the event.

        switch (ev.type)
        {
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button < TXT_MAX_MOUSE_BUTTONS)
                {
                    return GL_TXT_SDLButtonToTXTButton(ev.button.button);
                }
                break;

            case SDL_MOUSEWHEEL:
                return GL_TXT_SDLWheelToTXTButton(&ev.wheel);

            case SDL_KEYDOWN:
                switch (input_mode)
                {
                    case TXT_INPUT_RAW:
                        return GL_TXT_TranslateScancode(ev.key.keysym.scancode);
                    case TXT_INPUT_NORMAL:
                        return GL_TXT_TranslateKeysym(&ev.key.keysym);
                    case TXT_INPUT_TEXT:
                        // We ignore key inputs in this mode, except for a
                        // few special cases needed during text input:
                        if (ev.key.keysym.sym == SDLK_ESCAPE
                         || ev.key.keysym.sym == SDLK_BACKSPACE
                         || ev.key.keysym.sym == SDLK_RETURN)
                        {
                            return GL_TXT_TranslateKeysym(&ev.key.keysym);
                        }
                        break;
                }
                break;

            case SDL_TEXTINPUT:
                if (input_mode == TXT_INPUT_TEXT)
                {
                    // TODO: Support input of more than just the first char?
                    const char *p = ev.text.text;
                    int result = TXT_DecodeUTF8(&p);
                    // 0-127 is ASCII, but we map non-ASCII Unicode chars into
                    // a higher range to avoid conflicts with special keys.
                    return TXT_UNICODE_TO_KEY(result);
                }
                break;

            case SDL_QUIT:
                // Quit = escape
                return 27;

            case SDL_MOUSEMOTION:
                if (GL_TXT_MouseHasMoved())
                {
                    return 0;
                }

            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    GL_TXT_SetupOrtho(ev.window.data1, ev.window.data2);
                }
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                SDL_PushEvent(&ev);
                break;

            default:
                break;
        }
    }

    return -1;
}
