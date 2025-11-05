//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Message
//

#include "d_player.h"
#include "doomstat.h"
#include "z_zone.h"

#include "dsda/settings.h"

#include "messenger.h"

#define MESSAGE_LIFETIME 140

typedef enum {
  message_alert,
  message_yellow,
  message_normal,
} message_priority_t;

typedef struct message_s {
  char* str;
  message_priority_t priority;
  int tics;
  struct message_s* next_message;
} message_t;

static message_t* current_message;
static message_t* last_message;

static void dsda_FreeMessage(message_t* message) {
  Z_Free(message->str);
  Z_Free(message);
}

static void dsda_ClearMessages(void) {
  message_t* message_p;

  while (current_message) {
    message_p = current_message->next_message;
    dsda_FreeMessage(current_message);
    current_message = message_p;
  }
}

static void dsda_AppendMessage(message_t* message) {
  if (!current_message)
    current_message = message;
  else {
    message_t* message_p;

    message_p = current_message;
    while (message_p->next_message)
      message_p = message_p->next_message;

    message_p->next_message = message;
  }
}

static void dsda_QueueMessage(const char* str, message_priority_t priority, dboolean first_time) {
  message_t* new_message;
  int message_lifetime;

  // Set default message duration
  message_lifetime = MESSAGE_LIFETIME;
  
  // Hexen's Yellow Messages last 5 times longer than normal messages
  if (first_time)
    if (priority == message_yellow)
      message_lifetime = 5*MESSAGE_LIFETIME;  // hexen_note - yellow message

  if (current_message) {
    if (current_message->priority < priority)
      return;
    else if (current_message->priority > priority)
      dsda_ClearMessages();
    else if (priority == message_normal) {
      Z_Free(current_message->str);

      current_message->str = Z_Strdup(str);
      current_message->tics = message_lifetime;

      return;
    }
  }

  new_message = Z_Calloc(1, sizeof(*new_message));
  new_message->str = Z_Strdup(str);
  new_message->priority = priority;
  new_message->tics = message_lifetime;

  dsda_AppendMessage(new_message);
}

void dsda_AddPlayerAlert(const char* str, player_t* player) {
  if (player == &players[displayplayer])
    dsda_QueueMessage(str, message_alert, true);
}

void dsda_AddAlert(const char* str) {
  dsda_QueueMessage(str, message_alert, true);
}

void dsda_AddPlayerYellowMessage(const char* str, player_t* player, dboolean ultmsg) {
  dboolean show_message = ultmsg ? true : dsda_ShowMessages();

  if (show_message && player == &players[displayplayer])
    dsda_QueueMessage(str, message_yellow, true);
}

void dsda_AddYellowMessage(const char* str, dboolean ultmsg) {
  dboolean show_message = ultmsg ? true : dsda_ShowMessages();

  if (show_message)
    dsda_QueueMessage(str, message_yellow, true);
}

void dsda_AddPlayerMessage(const char* str, player_t* player) {
  if (dsda_ShowMessages() && player == &players[displayplayer])
    dsda_QueueMessage(str, message_normal, true);
}

void dsda_AddMessage(const char* str) {
  if (dsda_ShowMessages())
    dsda_QueueMessage(str, message_normal, true);
}

void dsda_AddUnblockableMessage(const char* str) {
  dsda_QueueMessage(str, message_normal, true);
}

void dsda_UpdateMessenger(void) {
  if (!current_message)
    return;

  --current_message->tics;
  if (!current_message->tics) {
    if (last_message)
      dsda_FreeMessage(last_message);
    last_message = current_message;
    current_message = current_message->next_message;
  }
}

void dsda_InitMessenger(void) {
  dsda_ClearMessages();
  if (last_message)
    dsda_FreeMessage(last_message);
  last_message = NULL;
}

void dsda_ReplayMessage(void) {
  if (last_message) {
    dsda_ClearMessages();
    dsda_QueueMessage(last_message->str, last_message->priority, false); // all replay messages should use the default duration (ex: hexen yellow message)
  }
}

char* dsda_PlayerMessage(void) {
  if (!current_message)
    return NULL;

  return current_message->str;
}

int dsda_PlayerMessageIsYellow(void) {
  if (!current_message || current_message->priority != message_yellow)
    return false;

  return true;
}
