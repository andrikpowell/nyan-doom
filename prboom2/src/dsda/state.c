//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA State
//

#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "state.h"

state_t* states;
int num_states;
byte* defined_codeptr_args;
statenum_t* seenstate_tab;

static actionf_t* deh_codeptr;
static int states_capacity;

static void dsda_ResetStates(int from, int to) {
  int i;

  // Raven doesn't have a null sprite
  int null_sprite = raven ? states[0].sprite : SPR_TNT1;

  for (i = from; i < to; ++i) {
    states[i].sprite = null_sprite;
    states[i].tics = -1;
    states[i].nextstate = i;
  }
}

static void dsda_PrepAllocation(void) {
  static int first_allocation = true;

  if (first_allocation) {
    state_t* source = states;

    first_allocation = false;
    states = malloc(num_states * sizeof(*states));
    memcpy(states, source, num_states * sizeof(*states));
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= states_capacity) {
    int old_capacity = states_capacity;

    dsda_PrepAllocation();

    states_capacity *= 2;

    states = realloc(states, states_capacity * sizeof(*states));
    memset(states + old_capacity, 0, (states_capacity - old_capacity) * sizeof(*states));

    if (deh_codeptr)
    {
      deh_codeptr = realloc(deh_codeptr, states_capacity * sizeof(*deh_codeptr));
      memset(deh_codeptr + old_capacity, 0, (states_capacity - old_capacity) * sizeof(*deh_codeptr));
    }

    if (defined_codeptr_args)
    {
      defined_codeptr_args = realloc(defined_codeptr_args, states_capacity * sizeof(*defined_codeptr_args));
      memset(defined_codeptr_args + old_capacity, 0, (states_capacity - old_capacity) * sizeof(*defined_codeptr_args));
    }

    if (seenstate_tab)
    {
      seenstate_tab = realloc(seenstate_tab, states_capacity * sizeof(*seenstate_tab));
      memset(seenstate_tab + old_capacity, 0, (states_capacity - old_capacity) * sizeof(*seenstate_tab));
    }

    dsda_ResetStates(old_capacity, states_capacity);
  }
}

static void dsda_EnsureIndex(int index) {
  dsda_EnsureCapacity(index);

  if (index >= num_states)
    num_states = index + 1;
}

dsda_deh_state_t dsda_GetDehState(int index) {
  dsda_deh_state_t deh_state;

  dsda_EnsureIndex(index);

  deh_state.state = &states[index];
  deh_state.codeptr = &deh_codeptr[index];
  deh_state.defined_codeptr_args = &defined_codeptr_args[index];

  return deh_state;
}

void dsda_InitializeStates(state_t* source, int count) {
  int i;
  extern int hexen;

  num_states = count;
  states_capacity = count;

  states = source;

  if (hexen) return;

  seenstate_tab = calloc(num_states, sizeof(*seenstate_tab));

  deh_codeptr = malloc(num_states * sizeof(*deh_codeptr));
  for (i = 0; i < num_states; i++)
    deh_codeptr[i] = states[i].action;

  defined_codeptr_args = calloc(num_states, sizeof(*defined_codeptr_args));
}

void dsda_FreeDehStates(void) {
  free(defined_codeptr_args);
  free(deh_codeptr);
}

int dsda_AppendStates(int count) {
  int first = num_states;

  if (count <= 0)
    return first;

  dsda_PrepAllocation();
  dsda_EnsureIndex(num_states + count - 1);
  memset(states + first, 0, count * sizeof(*states));
  dsda_ResetStates(first, first + count);

  return first;
}
