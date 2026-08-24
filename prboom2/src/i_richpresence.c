//
// Copyright (C) 2025 Roman Fomin
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
//	NYAN Discord Rich Presence
//


#include "config.h"
#include "doomtype.h"
#include "i_system.h"
#include "i_richpresence.h"
#include "lprintf.h"

#include "dsda/gameinfo.h"

#ifdef HAVE_DISCORD_RPC

#include <ctype.h>
#include <time.h>

#include "discord_rpc.h"

// https://discord.com/oauth2/authorize?client_id=1541344518778134558
#define DEFAULT_DISCORD_APP_ID "1541344518778134558"

static void DiscordReady(const DiscordUser *connectedUser)
{
  //lprintf(LO_INFO, "Discord: connected as %s\n", connectedUser->username);
  lprintf(LO_INFO, "Discord: connected to user %s#%s - %s", connectedUser->username,
          connectedUser->discriminator, connectedUser->userId);
}

static void DiscordDisconnected(int errcode, const char *message)
{
  lprintf(LO_WARN, "Discord: disconnected (%d: %s)\n", errcode, message);
}

static void DiscordError(int errcode, const char *message)
{
  lprintf(LO_WARN, "Discord: error (%d: %s)\n", errcode, message);
}

/*
static void DiscordSpectate(const char *secret)
{
    lprintf(LO_INFO, "Discord: spectate (%s)", secret);
}

static void DiscordJoin(const char *secret)
{
    lprintf(LO_INFO, "Discord: join (%s)", secret);
}

static void DiscordJoinRequest(const DiscordUser *request)
{
    // we can't join in-game
    int response = DISCORD_REPLY_NO;
    Discord_Respond(request->userId, response);
    lprintf(LO_INFO, "Discord: join request from %s#%s - %s", request->username,
           request->discriminator, request->userId);
}
*/

static const char* I_GetDiscordGameTitle(const char* fallback)
{
  const unsigned char* p;
  const char* startup_title;

  // Grab startup title if found
  dsda_LoadGameInfo();
  startup_title = dsda_GameInfoStartupTitle();

  if (!startup_title)
    return fallback;

  // Ignore just spaces or blank startup title
  for (p = (const unsigned char*)startup_title; *p; ++p)
  {
    if (!isspace(*p))
      return startup_title;
  }

  return fallback;
}

static void I_ShutdownDiscordPresence(void)
{
  Discord_ClearPresence();
  Discord_Shutdown();
}

void I_UpdateDiscordPresence(const char *curstate, const char *curstatus)
{
  static dboolean initialized;
  static int64_t starttime;

  const char *curappid = DEFAULT_DISCORD_APP_ID;
  DiscordRichPresence presence = {0};

  if (!initialized)
  {
    DiscordEventHandlers handlers = {
      .ready = DiscordReady,
      .disconnected = DiscordDisconnected,
      .errored = DiscordError,
      //.joinGame = 0,
      //.spectateGame = 0,
      //.joinRequest = 0
    };

    initialized = true;

    Discord_Initialize(curappid, &handlers, 1, NULL);

    I_AtExit(I_ShutdownDiscordPresence, true, "I_ShutdownDiscordPresence", exit_priority_normal);
  }

  presence.state = curstate;
  if (!starttime)
  {
    starttime = (int64_t)time(NULL);
  }
  presence.startTimestamp = starttime;
  presence.details = I_GetDiscordGameTitle(curstatus);
  presence.largeImageKey = "game-image";
  presence.instance = 0;
  Discord_UpdatePresence(&presence);
}

#else

void I_UpdateDiscordPresence(const char *curstate, const char *curstatus)
{
    ;
}

#endif
