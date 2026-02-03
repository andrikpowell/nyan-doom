//
// Copyright(C) 2024 by Ryan Krafnick
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
//	DSDA Aim
//

#include "hu_stuff.h"
#include "lprintf.h"
#include "p_map.h"
#include "r_main.h"
#include "tables.h"
#include "v_video.h"

#include "dsda/settings.h"
#include "dsda/excmd.h"

#include "aim.h"

angle_t dsda_PlayerPitch(player_t* player)
{
  return dsda_FreeAim() ? player->mo->pitch : -(angle_t)(player->lookdir * ANG1 / M_PI);
}

fixed_t dsda_PlayerSlope(player_t* player)
{
  return dsda_FreeAim() ? finetangent[(ANG90 - player->mo->pitch) >> ANGLETOFINESHIFT] :
         raven ? ((player->lookdir) << FRACBITS) / 173 :
         0;
}

int dsda_PitchToLookDir(angle_t pitch)
{
  return -(int) ((((long long)(int) pitch * FIXED_PI) >> FRACBITS) / ANG1);
}

angle_t dsda_LookDirToPitch(int lookdir)
{
  return (angle_t) -FixedDiv(lookdir * ANG1, FIXED_PI);
}

int dsda_PlayerLookDir(player_t* player)
{
  return dsda_FreeAim() ? dsda_PitchToLookDir(player->mo->pitch) : player->lookdir;
}

dboolean dsda_TargetNearScreenCenter(mobj_t* target, float max_distance_fraction)
{
  float winx, winy, winz;
  float x, y, z;

  if (!target)
    return false;

  // Note: Software has no way to check near the center of the screen
  if (V_IsSoftwareMode())
    return true;

  x = -(float)target->x / MAP_SCALE;
  z =  (float)target->y / MAP_SCALE;
  y =  (float)target->z / MAP_SCALE;

  if (R_Project(x, y, z, &winx, &winy, &winz))
  {
    float screen_center_x, screen_center_y;
    float reference;
    float dx, dy;
    float dist_sq, max_dist, max_dist_sq;

    reference = SCREENWIDTH < SCREENHEIGHT ? SCREENWIDTH : SCREENHEIGHT;

    screen_center_x = SCREENWIDTH  / 2.0f;
    screen_center_y = SCREENHEIGHT / 2.0f;

    // I don't know why we have this here
    // (like in the Crosshair function)
    // Because Software can't use R_Project()
    if (V_IsSoftwareMode())
    {
      int top = SCREENHEIGHT;
      int bottom = top - viewheight;

      winy += (float)(viewheight / 2 - centery);
      winy = BETWEEN(bottom, top, winy);
    }

    dx = winx - screen_center_x;
    dy = winy - screen_center_y;

    dist_sq = dx * dx + dy * dy;
    max_dist = max_distance_fraction * reference;  // example: 0.05f for 5%
    max_dist_sq = max_dist * max_dist;

    return (dist_sq <= max_dist_sq);
  }

  return false;
}

dboolean dsda_PlayerFreelookAutoAim(void)
{
  mobj_t* target = HU_Target();

  return target && dsda_TargetNearScreenCenter(target, dsda_MouselookAutoAimPercent() * 0.01f);
}

void dsda_PlayerAim(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask)
{
  dboolean freelook_autoaim = false;
  aim->angle = angle;

  if (dsda_MouselookAutoAim())
    freelook_autoaim = dsda_PlayerFreelookAutoAim();

  if (dsda_FreeAim() && !freelook_autoaim)
  {
    aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
    aim->z_offset = 0;
  }
  else
  {
    do
    {
      aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

      if (!dsda_DisableHorizAutoaim())
      {
        if (!linetarget)
        {
          aim->angle += 1 << 26;
          aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
        }

        if (!linetarget)
        {
          aim->angle -= 2 << 26;
          aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
        }
      }

      if (!linetarget) {
        aim->angle = angle;
        aim->slope = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
      }
    }
    while (target_mask && (target_mask = 0, !linetarget));  // killough 8/2/98

    aim->z_offset = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
  }
}

void dsda_PlayerAimBad(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask)
{
  dboolean freelook_autoaim = false;
  aim->angle = angle;
  aim->z_offset = 0;

  if (dsda_MouselookAutoAim())
    freelook_autoaim = dsda_PlayerFreelookAutoAim();

  if (dsda_FreeAim() && !freelook_autoaim)
  {
    aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
    aim->z_offset = 0;
  }
  else
  {
    do
    {
      aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

      if (!linetarget)
      {
        aim->angle += 1 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (!linetarget)
      {
        aim->angle -= 2 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (heretic && !linetarget)
        aim->slope = ((source->player->lookdir) << FRACBITS) / 173;
    }
    while (target_mask && (target_mask = 0, !linetarget));  // killough 8/2/98
  }
}
