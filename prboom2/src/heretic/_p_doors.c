//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// P_doors.c

#include "doomdef.h"
#include "deh_str.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

//==================================================================
//==================================================================
//
//                                                      VERTICAL DOORS
//
//==================================================================
//==================================================================

//==================================================================
//
//      EV_VerticalDoor : open a door manually, no tag value
//
//==================================================================
void EV_VerticalDoor(line_t * line, mobj_t * thing)
{
    player_t *player;
    sector_t *sec;
    vldoor_t *door;
    int side;

    side = 0;                   // only front sides can be used
//
//      Check for locks
//
    player = thing->player;
    switch (line->special)
    {
        case 26:               // Blue Lock
        case 32:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_blue])
            {
                P_SetMessage(player, DEH_String(TXT_NEEDBLUEKEY), false);
                S_StartSound(NULL, sfx_plroof);
                return;
            }
            break;
        case 27:               // Yellow Lock
        case 34:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_yellow])
            {
                P_SetMessage(player, DEH_String(TXT_NEEDYELLOWKEY), false);
                S_StartSound(NULL, sfx_plroof);
                return;
            }
            break;
        case 28:               // Green Lock
        case 33:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_green])
            {
                P_SetMessage(player, DEH_String(TXT_NEEDGREENKEY), false);
                S_StartSound(NULL, sfx_plroof);
                return;
            }
            break;
    }

    // if the sector has an active thinker, use it
    sec = sides[line->sidenum[side ^ 1]].sector;
    if (sec->specialdata)
    {
        door = sec->specialdata;
        switch (line->special)
        {
            case 1:            // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
            case 26:
            case 27:
            case 28:
                if (door->direction == -1)
                {
                    door->direction = 1;        // go back up
                }
                else
                {
                    if (!thing->player)
                    {           // Monsters don't close doors
                        return;
                    }
                    door->direction = -1;       // start going down immediately
                }
                return;
        }
    }

    // for proper sound
    switch (line->special)
    {
        case 1:                // NORMAL DOOR SOUND
        case 31:
            S_StartSound(&sec->soundorg, sfx_doropn);
            //S_StartSound(&sec->soundorg, sfx_dormov);
            break;
        default:               // LOCKED DOOR SOUND
            S_StartSound(&sec->soundorg, sfx_doropn);
            //S_StartSound(&sec->soundorg, sfx_dormov);
            break;
    }

    //
    // new door thinker
    //
    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    switch (line->special)
    {
        case 1:
        case 26:
        case 27:
        case 28:
            door->type = vld_normal;
            break;
        case 31:
        case 32:
        case 33:
        case 34:
            door->type = vld_open;
            line->special = 0;
            break;
    }
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

    //
    // find the top and bottom of the movement range
    //
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
}

//==================================================================
//
//      Spawn a door that closes after 30 seconds
//
//==================================================================
void P_SpawnDoorCloseIn30(sector_t * sec)
{
    vldoor_t *door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = vld_normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * 35;
}

//==================================================================
//
//      Spawn a door that opens after 5 minutes
//
//==================================================================
void P_SpawnDoorRaiseIn5Mins(sector_t * sec, int secnum)
{
    vldoor_t *door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = vld_raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * 35;
}
