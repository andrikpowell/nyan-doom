## NYANSKLL

NYANSKLL is a lump that allows authors to configure a custom 6th skill level. Most features listed here form a subset of gzdoom's MAPINFO SKILL specification. Only the "new" format of mapinfo skill defintion is valid in Nyan Doom (the curly braces).

It is recommended to only use NYANSKLL in non-UDMF maps, unlike Nyan Doom's MAPINFO support which is meant for UDMF maps.

## Usage

By default, Nyan Doom will only load NYANSKLL under Doom under Casual settings *(aka not recording or playing back demos).* This is done for compatibility purposes. See the full specification below.

### Skill

**skill \<ID\> { _properties_ }**

Defines a new skill level in the 6th slot after "Nightmare!". The **ID** isn't used anywhere in-game.

#### Skill Properties

| Property | Description |
| --- | --- |
| **AmmoFactor = \<Float\>** | Sets the ammo pickup multiplier (2.0 means double ammo). |
| **DamageFactor = \<Float\>** | Sets the player damage received multiplier (2.0 means double damage). |
| **ArmorFactor = \<Float\>** | Sets the armor pickup multiplier for armor bonuses and the green armor (2.0 means double armor). |
| **HealthFactor = \<Float\>** | Sets the player healing received multiplier (2.0 means double healing). |
| **MonsterHealth = \<Float\>** | Sets the enemy spawn health multiplier (2.0 means double spawn health). |
| **FriendlyHealth = \<Float\>** | Sets the friend spawn health multiplier (2.0 means double spawn health). |
| **RespawnTime = \<Int\>** | Sets the minimum time in seconds before a monster can respawn (the default is 0, which disables respawning). |
| **SpawnFilter = \<Int\>** | Sets which skill value to filter by when spawning things. For instance, if you want to create a new skill level that spawns the same things as UV, use 4. |
| **Key = "\<Char\>"** | Sets the menu hotkey. |
| **MustConfirm [= "\<Message\>"]** | Prompts the player to confirm when selecting this skill level. Optionally set the message given to the player. |
| **Name = "\<String\>"** | The name of the skill in the menu. |
| **PicName = "\<Lump\>"** | The graphic used for the skill in the menu (replaces the **Name** if every skill has a valid graphic). |
| **TextColor = "\<Color\>"** | Sets the color of the skill in the menu. |
| **SpawnMulti** | Spawns multiplayer objects even in single-player. |
| **FastMonsters** | Turns on fast monsters and projectiles. |
| **InstantReaction** | Allows instant attack reactions by monsters. |
| **NoPain** | Disables the pain state. |
| **DefaultSkill** | Defaults the selector to this skill when entering the skill select. |
| **PlayerRespawn** | Allow the player to respawn without resetting the map in single-player. |
| **EasyBossBrain** | Reduces the rate of boss brain spawns. |
