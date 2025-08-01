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

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "sounds.h"
#include "s_sound.h"
#include "r_main.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_inter.h"
#include "p_enemy.h"
#include "p_tick.h"

#include "dsda/skill_info.h"
#include "hexen/p_things.h"

#include "a_action.h"

int orbitTableX[256] = {
    983025, 982725, 981825, 980340, 978255, 975600, 972330, 968490,
    964065, 959070, 953475, 947325, 940590, 933300, 925440, 917025,
    908055, 898545, 888495, 877905, 866775, 855135, 842985, 830310,
    817155, 803490, 789360, 774735, 759660, 744120, 728130, 711690,
    694845, 677565, 659880, 641805, 623340, 604500, 585285, 565725,
    545820, 525600, 505050, 484200, 463065, 441645, 419955, 398010,
    375840, 353430, 330810, 307995, 285000, 261825, 238485, 215010,
    191400, 167685, 143865, 119955, 95970, 71940, 47850, 23745,
    -375, -24495, -48600, -72690, -96720, -120705, -144600, -168420,
    -192150, -215745, -239220, -262545, -285720, -308715, -331530, -354135,
    -376530, -398700, -420630, -442320, -463725, -484860, -505695, -526230,
    -546450, -566340, -585885, -605085, -623925, -642375, -660435, -678105,
    -695370, -712215, -728625, -744600, -760125, -775200, -789795, -803925,
    -817575, -830715, -843375, -855510, -867135, -878235, -888810, -898845,
    -908340, -917295, -925695, -933540, -940815, -947520, -953670, -959235,
    -964215, -968625, -972450, -975690, -978330, -980400, -981870, -982740,
    -983025, -982725, -981825, -980340, -978255, -975600, -972330, -968490,
    -964065, -959070, -953475, -947325, -940590, -933300, -925440, -917025,
    -908055, -898545, -888495, -877905, -866775, -855135, -842985, -830310,
    -817155, -803490, -789360, -774735, -759660, -744120, -728130, -711690,
    -694845, -677565, -659880, -641805, -623340, -604485, -585285, -565725,
    -545820, -525600, -505050, -484200, -463065, -441645, -419955, -398010,
    -375840, -353430, -330810, -307995, -285000, -261825, -238485, -215010,
    -191400, -167685, -143865, -119955, -95970, -71940, -47850, -23745,
    375, 24495, 48600, 72690, 96720, 120705, 144600, 168420,
    192150, 215745, 239220, 262545, 285720, 308715, 331530, 354135,
    376530, 398700, 420630, 442320, 463725, 484860, 505695, 526230,
    546450, 566340, 585885, 605085, 623925, 642375, 660435, 678105,
    695370, 712215, 728625, 744600, 760125, 775200, 789795, 803925,
    817575, 830715, 843375, 855510, 867135, 878235, 888810, 898845,
    908340, 917295, 925695, 933540, 940815, 947520, 953670, 959235,
    964215, 968625, 972450, 975690, 978330, 980400, 981870, 982740
};

int orbitTableY[256] = {
    375, 24495, 48600, 72690, 96720, 120705, 144600, 168420,
    192150, 215745, 239220, 262545, 285720, 308715, 331530, 354135,
    376530, 398700, 420630, 442320, 463725, 484860, 505695, 526230,
    546450, 566340, 585885, 605085, 623925, 642375, 660435, 678105,
    695370, 712215, 728625, 744600, 760125, 775200, 789795, 803925,
    817575, 830715, 843375, 855510, 867135, 878235, 888810, 898845,
    908340, 917295, 925695, 933540, 940815, 947520, 953670, 959235,
    964215, 968625, 972450, 975690, 978330, 980400, 981870, 982740,
    983025, 982725, 981825, 980340, 978255, 975600, 972330, 968490,
    964065, 959070, 953475, 947325, 940590, 933300, 925440, 917025,
    908055, 898545, 888495, 877905, 866775, 855135, 842985, 830310,
    817155, 803490, 789360, 774735, 759660, 744120, 728130, 711690,
    694845, 677565, 659880, 641805, 623340, 604500, 585285, 565725,
    545820, 525600, 505050, 484200, 463065, 441645, 419955, 398010,
    375840, 353430, 330810, 307995, 285000, 261825, 238485, 215010,
    191400, 167685, 143865, 119955, 95970, 71940, 47850, 23745,
    -375, -24495, -48600, -72690, -96720, -120705, -144600, -168420,
    -192150, -215745, -239220, -262545, -285720, -308715, -331530, -354135,
    -376530, -398700, -420630, -442320, -463725, -484860, -505695, -526230,
    -546450, -566340, -585885, -605085, -623925, -642375, -660435, -678105,
    -695370, -712215, -728625, -744600, -760125, -775200, -789795, -803925,
    -817575, -830715, -843375, -855510, -867135, -878235, -888810, -898845,
    -908340, -917295, -925695, -933540, -940815, -947520, -953670, -959235,
    -964215, -968625, -972450, -975690, -978330, -980400, -981870, -982740,
    -983025, -982725, -981825, -980340, -978255, -975600, -972330, -968490,
    -964065, -959070, -953475, -947325, -940590, -933300, -925440, -917025,
    -908055, -898545, -888495, -877905, -866775, -855135, -842985, -830310,
    -817155, -803490, -789360, -774735, -759660, -744120, -728130, -711690,
    -694845, -677565, -659880, -641805, -623340, -604485, -585285, -565725,
    -545820, -525600, -505050, -484200, -463065, -441645, -419955, -398010,
    -375840, -353430, -330810, -307995, -285000, -261825, -238485, -215010,
    -191400, -167685, -143865, -119955, -95970, -71940, -47850, -23745
};

extern fixed_t FloatBobOffsets[64];

void A_PotteryExplode(mobj_t * actor)
{
    mobj_t *mo = NULL;
    int i;

    for (i = (P_Random(pr_hexen) & 3) + 3; i; i--)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_POTTERYBIT1);
        P_SetMobjState(mo, mo->info->spawnstate + (P_Random(pr_hexen) % 5));
        mo->momz = ((P_Random(pr_hexen) & 7) + 5) * (3 * FRACUNIT / 4);
        mo->momx = P_SubRandom() << (FRACBITS - 6);
        mo->momy = P_SubRandom() << (FRACBITS - 6);
    }
    S_StartMobjSound(mo, hexen_sfx_pottery_explode);
    if (actor->special_args[0])
    {                           // Spawn an item
        if (!(skill_info.flags & SI_NO_MONSTERS)
            || !(mobjinfo[TranslateThingType[actor->special_args[0]]].
                 flags & MF_COUNTKILL))
        {                       // Only spawn monsters if not -nomonsters
            P_SpawnMobj(actor->x, actor->y, actor->z,
                        TranslateThingType[actor->special_args[0]]);
        }
    }
    P_RemoveMobj(actor);
}

void A_PotteryChooseBit(mobj_t * actor)
{
    P_SetMobjState(actor, actor->info->deathstate + (P_Random(pr_hexen) % 5) + 1);
    actor->tics = 256 + (P_Random(pr_hexen) << 1);
}

void A_PotteryCheck(mobj_t * actor)
{
    int i;
    mobj_t *pmo;

    if (!netgame)
    {
        pmo = players[consoleplayer].mo;
        if (P_CheckSight(actor, pmo)
	  && (abs((int)R_PointToAngle2(pmo->x, pmo->y, actor->x, actor->y)
           - (int)pmo->angle) <= ANG45))
        {                       // Previous state (pottery bit waiting state)
            P_SetMobjState(actor, actor->state - &states[0] - 1);
        }
        else
        {
            return;
        }
    }
    else
    {
        for (i = 0; i < g_maxplayers; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }
            pmo = players[i].mo;
            if (P_CheckSight(actor, pmo)
              && (abs((int)R_PointToAngle2(pmo->x, pmo->y, actor->x, actor->y)
               - (int)pmo->angle) <= ANG45))
            {                   // Previous state (pottery bit waiting state)
                P_SetMobjState(actor, actor->state - &states[0] - 1);
                return;
            }
        }
    }
}

void P_SpawnDirt(mobj_t * actor, fixed_t radius)
{
    fixed_t x, y, z;
    int dtype = 0;
    mobj_t *mo;
    angle_t angle;

    angle = P_Random(pr_hexen) << 5;    // <<24 >>19
    x = actor->x + FixedMul(radius, finecosine[angle]);
    y = actor->y + FixedMul(radius, finesine[angle]);
    z = actor->z + (P_Random(pr_hexen) << 9) + FRACUNIT;
    switch (P_Random(pr_hexen) % 6)
    {
        case 0:
            dtype = HEXEN_MT_DIRT1;
            break;
        case 1:
            dtype = HEXEN_MT_DIRT2;
            break;
        case 2:
            dtype = HEXEN_MT_DIRT3;
            break;
        case 3:
            dtype = HEXEN_MT_DIRT4;
            break;
        case 4:
            dtype = HEXEN_MT_DIRT5;
            break;
        case 5:
            dtype = HEXEN_MT_DIRT6;
            break;
    }
    mo = P_SpawnMobj(x, y, z, dtype);
    if (mo)
    {
        mo->momz = P_Random(pr_hexen) << 10;
    }
}

void A_CorpseBloodDrip(mobj_t * actor)
{
    if (P_Random(pr_hexen) > 128)
    {
        return;
    }
    P_SpawnMobj(actor->x, actor->y, actor->z + actor->height / 2,
                HEXEN_MT_CORPSEBLOODDRIP);
}

void A_CorpseExplode(mobj_t * actor)
{
    mobj_t *mo;
    int i;

    for (i = (P_Random(pr_hexen) & 3) + 3; i; i--)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_CORPSEBIT);
        P_SetMobjState(mo, mo->info->spawnstate + (P_Random(pr_hexen) % 3));
        mo->momz = ((P_Random(pr_hexen) & 7) + 5) * (3 * FRACUNIT / 4);
        mo->momx = P_SubRandom() << (FRACBITS - 6);
        mo->momy = P_SubRandom() << (FRACBITS - 6);
    }
    // Spawn a skull
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_CORPSEBIT);
    P_SetMobjState(mo, HEXEN_S_CORPSEBIT_4);
    if (mo)
    {
        mo->momz = ((P_Random(pr_hexen) & 7) + 5) * (3 * FRACUNIT / 4);
        mo->momx = P_SubRandom() << (FRACBITS - 6);
        mo->momy = P_SubRandom() << (FRACBITS - 6);
        S_StartMobjSound(mo, hexen_sfx_fired_death);
    }
    P_RemoveMobj(actor);
}

void A_LeafSpawn(mobj_t * actor)
{
    mobj_t *mo;
    int i;

    for (i = (P_Random(pr_hexen) & 3) + 1; i; i--)
    {
        // Official release of Hexen's source code relies on unspecified behavior
        // the in order of function's argument evaluation,
        // see ISO-IEC 9899-1999, [6.5.2.2.10]
        mobjtype_t type = HEXEN_MT_LEAF1 + (P_Random(pr_hexen) & 1);
        fixed_t z = actor->z + (P_Random(pr_hexen) << 14);
        fixed_t y = actor->y + (P_SubRandom() << 14);
        fixed_t x = actor->x + (P_SubRandom() << 14);

        mo = P_SpawnMobj(x, y, z, type);
        if (mo)
        {
            P_ThrustMobj(mo, actor->angle, (P_Random(pr_hexen) << 9) + 3 * FRACUNIT);
            P_SetTarget(&mo->target, actor);
            mo->special1.i = 0;
        }
    }
}

void A_LeafThrust(mobj_t * actor)
{
    if (P_Random(pr_hexen) > 96)
    {
        return;
    }
    actor->momz += (P_Random(pr_hexen) << 9) + FRACUNIT;
}

void A_LeafCheck(mobj_t * actor)
{
    actor->special1.i++;
    if (actor->special1.i >= 20)
    {
        P_SetMobjState(actor, HEXEN_S_NULL);
        return;
    }
    if (P_Random(pr_hexen) > 64)
    {
        if (!actor->momx && !actor->momy)
        {
            P_ThrustMobj(actor, actor->target->angle,
                         (P_Random(pr_hexen) << 9) + FRACUNIT);
        }
        return;
    }
    P_SetMobjState(actor, HEXEN_S_LEAF1_8);
    actor->momz = (P_Random(pr_hexen) << 9) + FRACUNIT;
    P_ThrustMobj(actor, actor->target->angle,
                 (P_Random(pr_hexen) << 9) + 2 * FRACUNIT);
    actor->flags |= MF_MISSILE;
}

void A_BridgeOrbit(mobj_t * actor)
{
    if (actor->target->special1.i)
    {
        P_SetMobjState(actor, HEXEN_S_NULL);
    }
    actor->special_args[0] += 3;
    actor->special_args[0] &= 0xff;
    actor->x = actor->target->x + orbitTableX[actor->special_args[0]];
    actor->y = actor->target->y + orbitTableY[actor->special_args[0]];
    actor->z = actor->target->z;
}

void A_BridgeInit(mobj_t * actor)
{
    byte startangle;
    mobj_t *ball1, *ball2, *ball3;
    fixed_t cx, cy, cz;

    cx = actor->x;
    cy = actor->y;
    cz = actor->z;
    startangle = P_Random(pr_hexen);
    actor->special1.i = 0;

    // Spawn triad into world
    ball1 = P_SpawnMobj(cx, cy, cz, HEXEN_MT_BRIDGEBALL);
    ball1->special_args[0] = startangle;
    P_SetTarget(&ball1->target, actor);

    ball2 = P_SpawnMobj(cx, cy, cz, HEXEN_MT_BRIDGEBALL);
    ball2->special_args[0] = (startangle + 85) & 255;
    P_SetTarget(&ball2->target, actor);

    ball3 = P_SpawnMobj(cx, cy, cz, HEXEN_MT_BRIDGEBALL);
    ball3->special_args[0] = (startangle + 170) & 255;
    P_SetTarget(&ball3->target, actor);

    A_BridgeOrbit(ball1);
    A_BridgeOrbit(ball2);
    A_BridgeOrbit(ball3);
}

void A_BridgeRemove(mobj_t * actor)
{
    actor->special1.i = true;     // Removing the bridge
    actor->flags &= ~MF_SOLID;
    P_SetMobjState(actor, HEXEN_S_FREE_BRIDGE1);
}

void A_SetShootable(mobj_t * actor)
{
    actor->flags2 &= ~MF2_NONSHOOTABLE;
    actor->flags |= MF_SHOOTABLE;
}

void A_UnSetShootable(mobj_t * actor)
{
    actor->flags2 |= MF2_NONSHOOTABLE;
    actor->flags &= ~MF_SHOOTABLE;
}

void A_SetAltShadow(mobj_t * actor)
{
    actor->flags &= ~MF_SHADOW;
    actor->flags |= MF_ALTSHADOW;
}

void A_Summon(mobj_t * actor)
{
    mobj_t *mo;
    mobj_t *master;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_MINOTAUR);
    if (mo)
    {
        if (P_TestMobjLocation(mo) == false || !actor->special1.m)
        {                       // Didn't fit - change back to artifact
            P_SetMobjState(mo, HEXEN_S_NULL);
            mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_SUMMONMAULATOR);
            if (mo)
                mo->flags |= MF_DROPPED;
            return;
        }

        // An attempt to just use the int directly failed - it's not clear why
        mo->special_args[0] = leveltime & 0xff;
        mo->special_args[1] = (leveltime >> 8) & 0xff;
        mo->special_args[2] = (leveltime >> 16) & 0xff;
        mo->special_args[3] = (leveltime >> 24) & 0xff;
        master = actor->special1.m;
        if (master->flags & MF_CORPSE)
        {                       // Master dead
            P_SetTarget(&mo->special1.m, NULL);   // No master
        }
        else
        {
            P_SetTarget(&mo->special1.m, actor->special1.m);     // Pointer to master (mobj_t *)
            P_GivePower(master->player, pw_minotaur);
        }

        // Make smoke puff
        P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_MNTRSMOKE);
        S_StartMobjSound(actor, hexen_sfx_maulator_active);
    }
}

//==========================================================================
// Fog Variables:
//
//              args[0]         Speed (0..10) of fog
//              args[1]         Angle of spread (0..128)
//              args[2]         Frequency of spawn (1..10)
//              args[3]         Lifetime countdown
//              args[4]         Boolean: fog moving?
//              special1                Internal:  Counter for spawn frequency
//              special2                Internal:  Index into floatbob table
//
//==========================================================================

void A_FogSpawn(mobj_t * actor)
{
    mobj_t *mo = NULL;
    angle_t delta;

    if (actor->special1.i-- > 0)
        return;

    actor->special1.i = actor->special_args[2];   // Reset frequency count

    switch (P_Random(pr_hexen) % 3)
    {
        case 0:
            mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_FOGPATCHS);
            break;
        case 1:
            mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_FOGPATCHM);
            break;
        case 2:
            mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_FOGPATCHL);
            break;
    }

    if (mo)
    {
        delta = actor->special_args[1];
        if (delta == 0)
            delta = 1;
        mo->angle =
            actor->angle + (((P_Random(pr_hexen) % delta) - (delta >> 1)) << 24);
        P_SetTarget(&mo->target, actor);
        if (actor->special_args[0] < 1)
            actor->special_args[0] = 1;
        mo->special_args[0] = (P_Random(pr_hexen) % (actor->special_args[0])) + 1;      // Random speed
        mo->special_args[3] = actor->special_args[3];   // Set lifetime
        mo->special_args[4] = 1;        // Set to moving
        mo->special2.i = P_Random(pr_hexen) & 63;
    }
}

void A_FogMove(mobj_t * actor)
{
    int speed = actor->special_args[0] << FRACBITS;
    angle_t angle;
    int weaveindex;

    if (!(actor->special_args[4]))
        return;

    if (actor->special_args[3]-- <= 0)
    {
        P_SetMobjStateNF(actor, actor->info->deathstate);
        return;
    }

    if ((actor->special_args[3] % 4) == 0)
    {
        weaveindex = actor->special2.i;
        actor->z += FloatBobOffsets[weaveindex] >> 1;
        actor->special2.i = (weaveindex + 1) & 63;
    }

    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(speed, finecosine[angle]);
    actor->momy = FixedMul(speed, finesine[angle]);
}

void A_PoisonBagInit(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 28 * FRACUNIT,
                     HEXEN_MT_POISONCLOUD);
    if (!mo)
    {
        return;
    }
    mo->momx = 1;               // missile objects must move to impact other objects
    mo->special1.i = 24 + (P_Random(pr_hexen) & 7);
    mo->special2.i = 0;
    P_SetTarget(&mo->target, actor->target);
    mo->radius = 20 * FRACUNIT;
    mo->height = 30 * FRACUNIT;
    mo->flags &= ~MF_NOCLIP;
}

void A_PoisonBagCheck(mobj_t * actor)
{
    if (!--actor->special1.i)
    {
        P_SetMobjState(actor, HEXEN_S_POISONCLOUD_X1);
    }
    else
    {
        return;
    }
}

void A_PoisonBagDamage(mobj_t * actor)
{
    int bobIndex;

    extern void A_Explode(mobj_t * actor);

    A_Explode(actor);

    bobIndex = actor->special2.i;
    actor->z += FloatBobOffsets[bobIndex] >> 4;
    actor->special2.i = (bobIndex + 1) & 63;
}

void A_PoisonShroom(mobj_t * actor)
{
    actor->tics = 128 + (P_Random(pr_hexen) << 1);
}

void A_CheckThrowBomb(mobj_t * actor)
{
    if (abs(actor->momx) < 1.5 * FRACUNIT && abs(actor->momy) < 1.5 * FRACUNIT
        && actor->momz < 2 * FRACUNIT
        && actor->state == &states[HEXEN_S_THROWINGBOMB6])
    {
        P_SetMobjState(actor, HEXEN_S_THROWINGBOMB7);
        actor->z = actor->floorz;
        actor->momz = 0;
        actor->flags2 &= ~MF2_FLOORBOUNCE;
        actor->flags &= ~MF_MISSILE;
    }
    if (!--actor->health)
    {
        P_SetMobjState(actor, actor->info->deathstate);
    }
}

//===========================================================================
// Quake variables
//
//              args[0]         Intensity on richter scale (2..9)
//              args[1]         Duration in tics
//              args[2]         Radius for damage
//              args[3]         Radius for tremor
//              args[4]         TID of map thing for focus of quake
//
//===========================================================================

dboolean A_LocalQuake(byte * args, mobj_t * actor)
{
    mobj_t *focus, *target;
    int lastfound = 0;
    int success = false;

    // Find all quake foci
    do
    {
        target = P_FindMobjFromTID(args[4], &lastfound);
        if (target)
        {
            focus = P_SpawnMobj(target->x,
                                target->y, target->z, HEXEN_MT_QUAKE_FOCUS);
            if (focus)
            {
                focus->special_args[0] = args[0];
                focus->special_args[1] = args[1] >> 1;  // decremented every 2 tics
                focus->special_args[2] = args[2];
                focus->special_args[3] = args[3];
                focus->special_args[4] = args[4];
                success = true;
            }
        }
    }
    while (target != NULL);

    return (success);
}

int localQuakeHappening[MAX_MAXPLAYERS];

void A_Quake(mobj_t * actor)
{
    angle_t an;
    player_t *player;
    mobj_t *victim;
    int richters = actor->special_args[0];
    int playnum;
    fixed_t dist;

    if (actor->special_args[1]-- > 0)
    {
        for (playnum = 0; playnum < g_maxplayers; playnum++)
        {
            player = &players[playnum];
            if (!playeringame[playnum])
                continue;

            victim = player->mo;
            dist = P_AproxDistance(actor->x - victim->x,
                                   actor->y - victim->y) >> (FRACBITS + 6);
            // Tested in tile units (64 pixels)
            if (dist < actor->special_args[3])  // In tremor radius
            {
                localQuakeHappening[playnum] = richters;
            }
            // Check if in damage radius
            if ((dist < actor->special_args[2]) && (victim->z <= victim->floorz))
            {
                if (P_Random(pr_hexen) < 50)
                {
                    P_DamageMobj(victim, NULL, NULL, HITDICE(1));
                }
                // Thrust player around
                an = victim->angle + ANG1 * P_Random(pr_hexen);
                P_ThrustMobj(victim, an, richters << (FRACBITS - 1));
            }
        }
    }
    else
    {
        for (playnum = 0; playnum < g_maxplayers; playnum++)
        {
            localQuakeHappening[playnum] = false;
        }
        P_SetMobjState(actor, HEXEN_S_NULL);
    }
}

#define TELEPORT_LIFE 1

void A_TeloSpawnA(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_TELOTHER_FX2);
    if (mo)
    {
        mo->special1.i = TELEPORT_LIFE;   // Lifetime countdown
        mo->angle = actor->angle;
        P_SetTarget(&mo->target, actor->target);
        mo->momx = actor->momx >> 1;
        mo->momy = actor->momy >> 1;
        mo->momz = actor->momz >> 1;
    }
}

void A_TeloSpawnB(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_TELOTHER_FX3);
    if (mo)
    {
        mo->special1.i = TELEPORT_LIFE;   // Lifetime countdown
        mo->angle = actor->angle;
        P_SetTarget(&mo->target, actor->target);
        mo->momx = actor->momx >> 1;
        mo->momy = actor->momy >> 1;
        mo->momz = actor->momz >> 1;
    }
}

void A_TeloSpawnC(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_TELOTHER_FX4);
    if (mo)
    {
        mo->special1.i = TELEPORT_LIFE;   // Lifetime countdown
        mo->angle = actor->angle;
        P_SetTarget(&mo->target, actor->target);
        mo->momx = actor->momx >> 1;
        mo->momy = actor->momy >> 1;
        mo->momz = actor->momz >> 1;
    }
}

void A_TeloSpawnD(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_TELOTHER_FX5);
    if (mo)
    {
        mo->special1.i = TELEPORT_LIFE;   // Lifetime countdown
        mo->angle = actor->angle;
        P_SetTarget(&mo->target, actor->target);
        mo->momx = actor->momx >> 1;
        mo->momy = actor->momy >> 1;
        mo->momz = actor->momz >> 1;
    }
}

void A_CheckTeleRing(mobj_t * actor)
{
    if (actor->special1.i-- <= 0)
    {
        P_SetMobjState(actor, actor->info->deathstate);
    }
}

//===========================================================================
//
// Thrust floor stuff
//
// Thrust Spike Variables
//              special1                pointer to dirt clump mobj
//              special2                speed of raise
//              args[0]         0 = lowered,  1 = raised
//              args[1]         0 = normal,   1 = bloody
//===========================================================================

void A_ThrustInitUp(mobj_t * actor)
{
    actor->special2.i = 5;        // Raise speed
    actor->special_args[0] = 1;         // Mark as up
    actor->floorclip = 0;
    actor->flags = MF_SOLID;
    actor->flags2 = MF2_NOTELEPORT | MF2_FOOTCLIP;
    P_SetTarget(&actor->special1.m, NULL);
}

void A_ThrustInitDn(mobj_t * actor)
{
    mobj_t *mo;
    actor->special2.i = 5;        // Raise speed
    actor->special_args[0] = 0;         // Mark as down
    actor->floorclip = actor->info->height;
    actor->flags = 0;
    actor->flags2 = MF2_NOTELEPORT | MF2_FOOTCLIP | MF2_DONTDRAW;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_DIRTCLUMP);
    P_SetTarget(&actor->special1.m, mo);
}


void A_ThrustRaise(mobj_t * actor)
{
    if (A_RaiseMobj(actor))
    {                           // Reached it's target height
        actor->special_args[0] = 1;
        if (actor->special_args[1])
            P_SetMobjStateNF(actor, HEXEN_S_BTHRUSTINIT2_1);
        else
            P_SetMobjStateNF(actor, HEXEN_S_THRUSTINIT2_1);
    }

    // Lose the dirt clump
    if ((actor->floorclip < actor->height) && actor->special1.m)
    {
        P_RemoveMobj(actor->special1.m);
        P_SetTarget(&actor->special1.m, NULL);
    }

    // Spawn some dirt
    if (P_Random(pr_hexen) < 40)
        P_SpawnDirt(actor, actor->radius);
    actor->special2.i++;          // Increase raise speed
}

void A_ThrustLower(mobj_t * actor)
{
    if (A_SinkMobj(actor))
    {
        actor->special_args[0] = 0;
        if (actor->special_args[1])
            P_SetMobjStateNF(actor, HEXEN_S_BTHRUSTINIT1_1);
        else
            P_SetMobjStateNF(actor, HEXEN_S_THRUSTINIT1_1);
    }
}

void A_ThrustBlock(mobj_t * actor)
{
    actor->flags |= MF_SOLID;
}

void A_ThrustImpale(mobj_t * actor)
{
    // Impale all shootables in radius
    PIT_ThrustSpike(actor);
}

void A_SoAExplode(mobj_t * actor)
{
    mobj_t *mo;
    int i;
    int r1,r2,r3;

    for (i = 0; i < 10; i++)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        mo = P_SpawnMobj(actor->x + ((r3 - 128) << 12),
                         actor->y + ((r2 - 128) << 12),
                         actor->z + (r1 * actor->height / 256),
                         HEXEN_MT_ZARMORCHUNK);
        P_SetMobjState(mo, mo->info->spawnstate + i);
        mo->momz = ((P_Random(pr_hexen) & 7) + 5) * FRACUNIT;
        mo->momx = P_SubRandom() << (FRACBITS - 6);
        mo->momy = P_SubRandom() << (FRACBITS - 6);
    }
    if (actor->special_args[0])
    {                           // Spawn an item
        // TODO: should this be on or off?
#if 0 // Checks are not present in version 1.1
        if (!(skill_info.flags & SI_NO_MONSTERS)
            || !(mobjinfo[TranslateThingType[actor->special_args[0]]].
                 flags & MF_COUNTKILL))
#endif
        {                       // Only spawn monsters if not -nomonsters
            P_SpawnMobj(actor->x, actor->y, actor->z,
                        TranslateThingType[actor->special_args[0]]);
        }
    }
    S_StartMobjSound(mo, hexen_sfx_suitofarmor_break);
    P_RemoveMobj(actor);
}

void A_BellReset1(mobj_t * actor)
{
    actor->flags |= MF_NOGRAVITY;
    actor->height <<= 2;
}

void A_BellReset2(mobj_t * actor)
{
    actor->flags |= MF_SHOOTABLE;
    actor->flags &= ~MF_CORPSE;
    actor->health = 5;
}

void A_FlameCheck(mobj_t * actor)
{
    if (!actor->special_args[0]--)      // Called every 8 tics
    {
        P_SetMobjState(actor, HEXEN_S_NULL);
    }
}

//===========================================================================
// Bat Spawner Variables
//      special1        frequency counter
//      special2
//      args[0]         frequency of spawn (1=fastest, 10=slowest)
//      args[1]         spread angle (0..255)
//      args[2]
//      args[3]         duration of bats (in octics)
//      args[4]         turn amount per move (in degrees)
//
// Bat Variables
//      special2        lifetime counter
//      args[4]         turn amount per move (in degrees)
//===========================================================================

void A_BatSpawnInit(mobj_t * actor)
{
    actor->special1.i = 0;        // Frequency count
}

void A_BatSpawn(mobj_t * actor)
{
    mobj_t *mo;
    int delta;
    angle_t angle;

    // Countdown until next spawn
    if (actor->special1.i-- > 0)
        return;
    actor->special1.i = actor->special_args[0];   // Reset frequency count

    delta = actor->special_args[1];
    if (delta == 0)
        delta = 1;
    angle = actor->angle + (((P_Random(pr_hexen) % delta) - (delta >> 1)) << 24);
    mo = P_SpawnMissileAngle(actor, HEXEN_MT_BAT, angle, 0);
    if (mo)
    {
        mo->special_args[0] = P_Random(pr_hexen) & 63;  // floatbob index
        mo->special_args[4] = actor->special_args[4];   // turn degrees
        mo->special2.i = actor->special_args[3] << 3;     // Set lifetime
        P_SetTarget(&mo->target, actor);
    }
}

void A_BatMove(mobj_t * actor)
{
    angle_t newangle;
    fixed_t speed;

    if (actor->special2.i < 0)
    {
        P_SetMobjState(actor, actor->info->deathstate);
    }
    actor->special2.i -= 2;       // Called every 2 tics

    if (P_Random(pr_hexen) < 128)
    {
        newangle = actor->angle + ANG1 * actor->special_args[4];
    }
    else
    {
        newangle = actor->angle - ANG1 * actor->special_args[4];
    }

    // Adjust momentum vector to new direction
    newangle >>= ANGLETOFINESHIFT;
    speed = FixedMul(actor->info->speed, P_Random(pr_hexen) << 10);
    actor->momx = FixedMul(speed, finecosine[newangle]);
    actor->momy = FixedMul(speed, finesine[newangle]);

    if (P_Random(pr_hexen) < 15)
        S_StartMobjSound(actor, hexen_sfx_bat_scream);

    // Handle Z movement
    actor->z = actor->target->z + 2 * FloatBobOffsets[actor->special_args[0]];
    actor->special_args[0] = (actor->special_args[0] + 3) & 63;
}

void A_TreeDeath(mobj_t * actor)
{
    if (!(actor->flags2 & MF2_FIREDAMAGE))
    {
        actor->height <<= 2;
        actor->flags |= MF_SHOOTABLE;
        actor->flags &= ~(MF_CORPSE + MF_DROPOFF);
        actor->health = 35;
        return;
    }
    else
    {
        P_SetMobjState(actor, actor->info->meleestate);
    }
}

void A_NoGravity(mobj_t * actor)
{
    actor->flags |= MF_NOGRAVITY;
}
