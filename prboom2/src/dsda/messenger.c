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
#include "hu_lib.h"
#include "lprintf.h"
#include "m_misc.h"
#include "z_zone.h"

#include "dsda/settings.h"

#include "deh/strings.h"
#include "heretic/hhe/strings.h"

#include "messenger.h"

#define MESSAGE_LIFETIME        (TICRATE*4)
#define YELLOW_MESSAGE_LIFETIME (5*MESSAGE_LIFETIME)

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
      message_lifetime = YELLOW_MESSAGE_LIFETIME;  // hexen_note - yellow message

  if (current_message) {
    if (current_message->priority < priority)
      return;
    else if (current_message->priority > priority)
      dsda_ClearMessages();
    else if (priority == message_normal || priority == message_yellow) {
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

void dsda_AddPlayerObituary(const char* str, player_t* player) {
  if (dsda_ShowMessages() && player == &players[displayplayer])
    dsda_QueueMessage(str, message_normal, true);
}

void dsda_AddObituary(const char* str) {
  if (dsda_ShowMessages())
  dsda_QueueMessage(str, message_normal, true);
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

// Multi-Colored Messages

typedef struct
{
  const char * const *deh_string;
  const char *word;
  int cm;
} dsda_msg_color_rule_t;

static const dsda_msg_color_rule_t msg_color_rules[] =
{
    { &s_GOTBLUECARD, "blue",   CR_BLUE },
    { &s_GOTBLUESKUL, "blue",   CR_BLUE },
    { &s_GOTREDCARD,  "red",    CR_RED  },
    { &s_GOTREDSKULL, "red",    CR_RED  },
    { &s_GOTYELWCARD, "yellow", CR_GOLD },
    { &s_GOTYELWSKUL, "yellow", CR_GOLD },
    { &s_PD_BLUEC,    "blue",   CR_BLUE },
    { &s_PD_BLUEK,    "blue",   CR_BLUE },
    { &s_PD_BLUEO,    "blue",   CR_BLUE },
    { &s_PD_BLUES,    "blue",   CR_BLUE },
    { &s_PD_REDC,     "red",    CR_RED  },
    { &s_PD_REDK,     "red",    CR_RED  },
    { &s_PD_REDO,     "red",    CR_RED  },
    { &s_PD_REDS,     "red",    CR_RED  },
    { &s_PD_YELLOWC,  "yellow", CR_GOLD },
    { &s_PD_YELLOWK,  "yellow", CR_GOLD },
    { &s_PD_YELLOWO,  "yellow", CR_GOLD },
    { &s_PD_YELLOWS,  "yellow", CR_GOLD },

    // Heretic
    { &s_HERETIC_TXT_GOTBLUEKEY,    "blue",   CR_BLUE   },
    { &s_HERETIC_TXT_GOTYELLOWKEY,  "yellow", CR_GOLD   },
    { &s_HERETIC_TXT_GOTGREENKEY,   "green",  CR_GREEN  },
    { &s_HERETIC_TXT_NEEDBLUEKEY,   "blue",   CR_BLUE   },
    { &s_HERETIC_TXT_NEEDYELLOWKEY, "yellow", CR_GOLD   },
    { &s_HERETIC_TXT_NEEDGREENKEY,  "green",  CR_GREEN  },

    // Hexen keys
    { &s_TXT_KEY_STEEL,         "steel",    CR_WHITE  },
    { &s_TXT_KEY_CAVE,          "cave",     CR_TAN    },
    { &s_TXT_KEY_AXE,           "axe",      CR_WHITE  },
    { &s_TXT_KEY_FIRE,          "fire",     CR_RED    },
    { &s_TXT_KEY_EMERALD,       "emerald",  CR_GREEN  },
    { &s_TXT_KEY_DUNGEON,       "dungeon",  CR_WHITE  },
    { &s_TXT_KEY_SILVER,        "silver",   CR_WHITE  },
    { &s_TXT_KEY_RUSTED,        "rusted",   CR_WHITE  },
    { &s_TXT_KEY_HORN,          "horn",     CR_TAN    },
    { &s_TXT_KEY_SWAMP,         "swamp",    CR_GREEN  },
    { &s_TXT_KEY_CASTLE,        "castle",   CR_GOLD   },

    // Hexen doors
    { &s_TXT_NEED_KEY_STEEL,    "steel",    CR_WHITE  },
    { &s_TXT_NEED_KEY_CAVE,     "cave",     CR_BROWN  },
    { &s_TXT_NEED_KEY_AXE,      "axe",      CR_WHITE  },
    { &s_TXT_NEED_KEY_FIRE,     "fire",     CR_RED    },
    { &s_TXT_NEED_KEY_EMERALD,  "emerald",  CR_GREEN  },
    { &s_TXT_NEED_KEY_DUNGEON,  "dungeon",  CR_WHITE  },
    { &s_TXT_NEED_KEY_SILVER,   "silver",   CR_WHITE  },
    { &s_TXT_NEED_KEY_RUSTED,   "rusted",   CR_WHITE  },
    { &s_TXT_NEED_KEY_HORN,     "horn",     CR_BROWN  },
    { &s_TXT_NEED_KEY_SWAMP,    "swamp",    CR_GREEN  },
    { &s_TXT_NEED_KEY_CASTLE,   "castle",   CR_GOLD   },

    // Hexen Puzzles
    { &s_TXT_ARTIPUZZSKULL,     "YORICK'S SKULL",      CR_GOLD   },
    { &s_TXT_ARTIPUZZGEMBIG,    "HEART OF D'SPARIL",   CR_RED    },
    { &s_TXT_ARTIPUZZGEMRED,    "RUBY",                CR_RED    },
    { &s_TXT_ARTIPUZZGEMGREEN1, "EMERALD",             CR_GREEN  },
    { &s_TXT_ARTIPUZZGEMGREEN2, "EMERALD",             CR_GREEN  },
    { &s_TXT_ARTIPUZZGEMBLUE1,  "SAPPHIRE",            CR_BLUE   },
    { &s_TXT_ARTIPUZZGEMBLUE2,  "SAPPHIRE",            CR_BLUE   },
    { &s_TXT_ARTIPUZZBOOK1,     "DAEMON CODEX",        CR_BROWN  },
    { &s_TXT_ARTIPUZZBOOK2,     "LIBER OSCURA",        CR_BROWN  },
    { &s_TXT_ARTIPUZZSKULL2,    "FLAME MASK",          CR_RED    },
    { &s_TXT_ARTIPUZZFWEAPON,   "GLAIVE SEAL",         CR_GREEN  },
    { &s_TXT_ARTIPUZZCWEAPON,   "HOLY RELIC",          CR_GOLD   },
    { &s_TXT_ARTIPUZZMWEAPON,   "SIGIL OF THE MAGUS",  CR_RED    },
    { &s_TXT_ARTIPUZZGEAR,      "CLOCK GEAR",          CR_GOLD   },

    // Hexen mana
    { &s_TXT_MANA_1,    "Blue",     CR_BLUE   },
    { &s_TXT_MANA_2,    "Green",    CR_GREEN  },
    { &s_TXT_MANA_BOTH, "Combined", CR_RED    },
};

static const dsda_msg_color_rule_t *dsda_GetColorRuleForMessage(const char *str)
{
  int i;

  for (i = 0; i < arrlen(msg_color_rules); ++i)
    if (str == *msg_color_rules[i].deh_string)
      return &msg_color_rules[i];

  return NULL;
}

const char *dsda_ColorizeMessage(const char *str)
{
  static char buf[1024];

  const dsda_msg_color_rule_t *rule;
  char repl[128];
  char *out;

  if (!dsda_ColorizeMessages() || !str)
    return str;

  rule = dsda_GetColorRuleForMessage(str);
  if (!rule)
    return str;

  snprintf(repl, sizeof(repl), "%s%s%s",
           HU_ColorFromValue(rule->cm),
           rule->word,
           HU_ColorReset());

  out = M_StringReplaceWord(str, rule->word, repl);
  if (!out)
    I_Error("alloc failed");

  if (strcmp(out, str) == 0)
  {
    Z_Free(out);
    return str;
  }

  snprintf(buf, sizeof(buf), "%s", out);
  Z_Free(out);

  return buf;
}

void dsda_AddPlayerColoredMessage(const char* str, player_t* player) {
  if (dsda_ShowMessages() && player == &players[displayplayer])
    dsda_QueueMessage(dsda_ColorizeMessage(str), message_normal, true);
}
