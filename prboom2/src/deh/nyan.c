//
// Copyright(C) 2026 by Andrik Powell
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
//	DEH Nyan Extensions
//

#include "deh/func.h"
#include "deh/nyan.h"
#include "doomdef.h"
#include "p_enemy.h"
#include "v_video.h"

#include "dsda/configuration.h"

void deh_InitNyanTweaks(void)
{
  deh_changeColoredBlood();
  deh_changeBonusFlash();
}

int init_blood_check;
int do_baron_blood;
int do_knight_blood;
int do_caco_blood;
int do_spectre_blood;

static void deh_InitColoredBlood(void)
{
  extern byte* edited_mobjinfo_bits;
  #define baron mobjinfo[MT_BRUISER]
  #define knight mobjinfo[MT_KNIGHT]
  #define caco mobjinfo[MT_HEAD]
  #define spectre mobjinfo[MT_SHADOWS]

  int edited_baron = !(
      (baron.doomednum == 3003) &&
      (baron.spawnhealth == 1000) &&
      (baron.radius == 24*FRACUNIT) &&
      (baron.height == 64*FRACUNIT) &&
      //(baron.missilestate == S_BOSS_ATK1) &&
      (!edited_mobjinfo_bits[MT_BRUISER])
      );

  int edited_knight = !(
      (knight.doomednum == 69) &&
      (knight.spawnhealth == 500) &&
      (knight.radius == 24*FRACUNIT) &&
      (knight.height == 64*FRACUNIT) &&
      //(knight.missilestate == S_BOS2_ATK1) &&
      (!edited_mobjinfo_bits[MT_KNIGHT])
      );

  int edited_caco = !(
      (caco.doomednum == 3005) &&
      (caco.spawnhealth == 400) &&
      (caco.radius == 31*FRACUNIT) &&
      (caco.height == 56*FRACUNIT) &&
      //(caco.missilestate == S_HEAD_ATK1) &&
      (!edited_mobjinfo_bits[MT_HEAD])
      );

  int edited_spectre = !(
      (spectre.doomednum == 58) &&
      (spectre.radius == 30*FRACUNIT) &&
      (spectre.height == 56*FRACUNIT) &&
      (!edited_mobjinfo_bits[MT_SHADOWS])
      );

  do_baron_blood = (baron.bloodcolor == V_BloodColor(0)) ? (edited_baron ? 2 : 1) : 0;
  do_knight_blood = (knight.bloodcolor == V_BloodColor(0)) ? (edited_knight ? 2 : 1) : 0;
  do_caco_blood = (caco.bloodcolor == V_BloodColor(0)) ? (edited_caco ? 2 : 1) : 0;
  do_spectre_blood = (spectre.bloodcolor == V_BloodColor(0)) ? (edited_spectre ? 2 : 1) : 0;

  init_blood_check = true;
}

void deh_changeColoredBlood(void)
{
  extern byte* edited_mobjinfo_bits;
  int nyan_blood_color;
  int baron_blood_color;
  int knight_blood_color;
  int caco_blood_color;
  int spectre_blood_color;

  if (raven) return;

  nyan_blood_color = dsda_IntConfig(nyan_config_colored_blood);
  baron_blood_color = dsda_IntConfig(nyan_config_colored_blood_baron);
  knight_blood_color = dsda_IntConfig(nyan_config_colored_blood_knight);
  caco_blood_color = dsda_IntConfig(nyan_config_colored_blood_caco);
  spectre_blood_color = dsda_IntConfig(nyan_config_colored_blood_spectre);

  if (!init_blood_check)
    deh_InitColoredBlood();

  if (do_baron_blood > 0)
  {
    if ((nyan_blood_color==1 && do_baron_blood==1) || nyan_blood_color==2)
      mobjinfo[MT_BRUISER].bloodcolor = (nyan_blood_color > 0) ? V_BloodColor(baron_blood_color) : 0;
    else
      mobjinfo[MT_BRUISER].bloodcolor = 0;
  }

  if (do_knight_blood > 0)
  {
    if ((nyan_blood_color==1 && do_knight_blood==1) || nyan_blood_color==2)
      mobjinfo[MT_KNIGHT].bloodcolor = (nyan_blood_color > 0) ? V_BloodColor(knight_blood_color) : 0;
    else
      mobjinfo[MT_KNIGHT].bloodcolor = 0;
  }

  if (do_caco_blood > 0)
  {
    if ((nyan_blood_color==1 && do_caco_blood==1) || nyan_blood_color==2)
      mobjinfo[MT_HEAD].bloodcolor = (nyan_blood_color > 0) ? V_BloodColor(caco_blood_color) : 0;
    else
      mobjinfo[MT_HEAD].bloodcolor = 0;
  }

  if (do_spectre_blood > 0)
  {
    if ((nyan_blood_color==1 && do_spectre_blood==1) || nyan_blood_color==2)
      mobjinfo[MT_SHADOWS].bloodcolor = (nyan_blood_color > 0) ? V_BloodColor(spectre_blood_color) : 0;
    else
      mobjinfo[MT_SHADOWS].bloodcolor = 0;
  }
}

int vanilla_health_bonus = -1;
int vanilla_armor_bonus = -1;
#define health_flash states[S_BON1C]
#define armor_flash states[S_BON2C]

static void deh_InitBonusFlash(void)
{
  extern byte* edited_mobjinfo_bits;

  vanilla_health_bonus = (
      (health_flash.sprite == SPR_BON1) &&
      (health_flash.frame == 3) &&
      (health_flash.tics == 6) &&
      (health_flash.action == NULL) &&
      (health_flash.nextstate == S_BON1D) &&
      (!edited_mobjinfo_bits[MT_MISC2] ||
       (mobjinfo[MT_MISC2].flags == MF_SPECIAL))
      );

  vanilla_armor_bonus = (
      (armor_flash.sprite == SPR_BON2) &&
      (armor_flash.frame == 3) &&
      (armor_flash.tics == 6) &&
      (armor_flash.action == NULL) &&
      (armor_flash.nextstate == S_BON2D) &&
      (!edited_mobjinfo_bits[MT_MISC3] ||
       (mobjinfo[MT_MISC3].flags == MF_SPECIAL))
      );
}

void deh_changeBonusFlash(void)
{
  int i;
  int bonus_flash;
  int predefined_bonuses[] = {
    MT_MISC2, MT_MISC3
  };

  if (raven) return;

  bonus_flash = dsda_IntConfig(nyan_config_item_bonus_flash);

  if (vanilla_health_bonus == -1 || vanilla_armor_bonus == -1)
    deh_InitBonusFlash();

  for (i = 0; (size_t)i < sizeof(predefined_bonuses) / sizeof(predefined_bonuses[0]); i++)
  {
    if (vanilla_health_bonus)
        health_flash.frame = bonus_flash ? 32771 : 3;
    if (vanilla_armor_bonus)
        armor_flash.frame = bonus_flash ? 32771 : 3;
  }
}
