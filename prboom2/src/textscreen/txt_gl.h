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

#ifndef TXT_GL_H
#define TXT_GL_H

#include "SDL.h"

void GL_TXT_PreInit(SDL_Window *sdl_window);
int GL_TXT_Init(void);
void GL_TXT_Shutdown(void);
unsigned char *GL_TXT_GetScreenData(void);
signed int GL_TXT_GetChar(void);
void GL_TXT_UpdateScreen(void);
void GL_TXT_Sleep(int timeout);

#endif

