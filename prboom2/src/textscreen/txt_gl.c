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

#include "doomkeys.h"

#include "txt_gl.h"
#include "txt_main.h"
#include "txt_utf8.h"
#include "fonts/normal.h"
#include "lprintf.h"

#define FONT_CHAR_W normal_font.w
#define FONT_CHAR_H normal_font.h
#define FONT_WIDTH 128
#define FONT_HEIGHT 288
#define SCREEN_COLS 80
#define SCREEN_ROWS 25

// Time between character blinks in ms

#define BLINK_PERIOD 250

static unsigned char* glscreendata;
static SDL_Window *glwindow = NULL;
static SDL_Surface *screenbuffer;
static GLuint texture_upscaled = 0;

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
    float scale, scale_x, scale_y;
    float scaled_width, scaled_height;
    float offset_x, offset_y;

    scale_x = (float)w / logical_width;
    scale_y = (float)h / logical_height;
    scale = (scale_x < scale_y) ? scale_x : scale_y;

    // Calculate the scaled width and height
    scaled_width = logical_width * scale;
    scaled_height = logical_height * scale;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // offsets to center the image
    offset_x = (w - scaled_width) / 2.0f;
    offset_y = (h - scaled_height) / 2.0f;

    // Apply translation and scaling
    glTranslatef(offset_x, offset_y, 0.0f);
    glScalef(scale, scale, 1.0f);
}

void GL_TXT_PreInit(SDL_Window *preset_window)
{
    if (preset_window != NULL)
    {
        glwindow = preset_window;
    }
}

static int screen_image_w, screen_image_h;

int GL_TXT_Init(void)
{
    font = &normal_font;

    screen_image_w = TXT_SCREEN_W * font->w;
    screen_image_h = TXT_SCREEN_H * font->h;

    screenbuffer = SDL_CreateRGBSurface(0, screen_image_w, screen_image_h, 8, 0, 0, 0, 0);
    if (!screenbuffer)
    {
        lprintf(LO_ERROR, "Failed to create software screenbuffer!\n");
        return 0;
    }

    SDL_LockSurface(screenbuffer);
    SDL_SetPaletteColors(screenbuffer->format->palette, ega_colors, 0, 16);
    SDL_UnlockSurface(screenbuffer);

    glscreendata = malloc(TXT_SCREEN_W * TXT_SCREEN_H * 2);
    memset(glscreendata, 0, TXT_SCREEN_W * TXT_SCREEN_H * 2);

    if (!glwindow)
        return 0;

    SDL_GL_GetDrawableSize(glwindow, &glwindow_w, &glwindow_h);

    GL_TXT_SetupOrtho(glwindow_w, glwindow_h);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_WIDTH, FONT_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, font->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return 1;
}

void GL_TXT_Shutdown(void)
{
    if (texture_upscaled != 0)
    {
        glDeleteTextures(1, &texture_upscaled);
        texture_upscaled = 0;
    }

    free(glscreendata);
    glscreendata = NULL;
    SDL_FreeSurface(screenbuffer);
    screenbuffer = NULL;
}

unsigned char *GL_TXT_GetScreenData(void)
{
    return glscreendata;
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
    static GLubyte *texture_data;
    int tex_w, tex_h, tex_w2, tex_h2;

    static GLuint texture_upscaled = 0;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_LockSurface(screenbuffer);

    texture_data = NULL;
    tex_w = TXT_SCREEN_W * FONT_CHAR_W;
    tex_h = TXT_SCREEN_H * FONT_CHAR_H;

    // We will scale software image by 2x
    // then scale back down for sharpness
    tex_w2 = tex_w * 2;
    tex_h2 = tex_h * 2;

    if (!texture_data)
        texture_data = malloc(tex_w2 * tex_h2 * 4);

    for (int y = 0; y < TXT_SCREEN_H; y++) {
        for (int x = 0; x < TXT_SCREEN_W; x++) {
            unsigned char *p = &glscreendata[(y * TXT_SCREEN_W + x) * 2];
            unsigned char chr = p[0];
            unsigned char attr = p[1];
            int fg, bg;
            SDL_Color fg_col, bg_col;

            fg = attr & 0x0F;            // 16-color fg
            bg = (attr >> 4) & 0x0F;     // 16-color bg fixed here

            if (bg & 0x8)
            {
                // blinking

                bg &= ~0x8;

                if (((SDL_GetTicks() / BLINK_PERIOD) % 2) == 0)
                {
                    fg = bg;
                }
            }

            fg_col = ega_colors[fg];
            bg_col = ega_colors[bg];

            // Draw character pixels into screen_rgba
            for (int cy = 0; cy < (int)FONT_CHAR_H; cy++)
            {
                unsigned char row = font->data[chr * FONT_CHAR_H + cy];
                for (int cx = 0; cx < (int)FONT_CHAR_W; cx++)
                {
                    int bit = (row >> cx) & 1;
                    GLubyte r = bit ? fg_col.r : bg_col.r;
                    GLubyte g = bit ? fg_col.g : bg_col.g;
                    GLubyte b = bit ? fg_col.b : bg_col.b;

                    int dst_x = x * FONT_CHAR_W * 2 + cx * 2;
                    int dst_y = y * FONT_CHAR_H * 2 + cy * 2;
                    for (int dy = 0; dy < 2; dy++)
                        for (int dx = 0; dx < 2; dx++)
                        {
                            int idx = ((dst_y + dy) * tex_w2 + (dst_x + dx)) * 4;
                            texture_data[idx + 0] = r;
                            texture_data[idx + 1] = g;
                            texture_data[idx + 2] = b;
                            texture_data[idx + 3] = 255;
                        }
                }
            }
        }
    }

    SDL_UnlockSurface(screenbuffer);

    // Draw 2x texture
    if (texture_upscaled == 0)
    {
        glGenTextures(1, &texture_upscaled);
        glBindTexture(GL_TEXTURE_2D, texture_upscaled);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w2, tex_h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture_upscaled);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w2, tex_h2, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    }

    // Draw 2x texture (image) normal size in window
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_upscaled);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f((GLfloat)tex_w, 0);
        glTexCoord2f(1, 1); glVertex2f((GLfloat)tex_w, (GLfloat)tex_h);
        glTexCoord2f(0, 1); glVertex2f(0, (GLfloat)tex_h);
    glEnd();

    glDisable(GL_TEXTURE_2D);

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
    // what SDL_GL_GetDrawableSize() returns.
    SDL_GL_GetDrawableSize(glwindow, &glwindow_w, &glwindow_h);
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
