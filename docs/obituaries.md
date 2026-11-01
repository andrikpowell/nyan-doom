# Obituaries

Nyan Doom has support for obituary messages for when a player dies. Obituaries are supported in:
- Doom DeHackEd
- Heretic [HeHackEd](hehacked.md)
- [DSDHacked](dsdhacked.md) for Doom/Heretic

Obituaries can be customized in two ways:

- Assign directly to an actor in a `Thing` block.
- `OB_` messages in a `[STRINGS]` block.

## Placeholders

The following placeholders can be used in obituary messages:

| Placeholder | Replacement |
| --- | --- |
| `%o` | Victim's name |
| `%k` | Killer's name |
| `%g` | `he` / `she` / `they` / `it` |
| `%h` | `him` / `her` / `them` / `it` |
| `%p` | `his` / `her` / `their` / `its` |
| `%s` | `his` / `hers` / `theirs` / `its` |
| `%r` | `he's` / `she's` / `they're` / `it's` |

Pronouns will use the player's set gender when the victim is the local player. Other players use neutral pronouns.

## Classic Obituary Strings

Classic boom obituary messages are strings beginning with `OB_`. Replace them in a BEX/REX `[STRINGS]` block:

```text
[STRINGS]
OB_IMP = %o was toasted by an imp.
OB_IMPHIT = %o got too close to an imp.
OB_MPROCKET = %o caught %k's rocket.
```

These are useful for replacing environmental deaths, or multiplayer weapon messages.

## General Strings

| String | Default message |
| --- | --- |
| `OB_CRUSH` | `%o was squished.` |
| `OB_FALLING` | `%o fell too far.` |
| `OB_SLIME` | `%o mutated.` |
| `OB_LAVA` | `%o melted.` |
| `OB_KILLEDSELF` | `%o killed %hself.` |
| `OB_VOODOO` | `%o was killed by the power of voodoo.` |
| `OB_MONTELEFRAG` | `%o was telefragged.` |
| `OB_DEFAULT` | `%o died.` |
| `OB_MPDEFAULT` | `%o was killed by %k.` |
| `OB_MPTELEFRAG` | `%o was telefragged by %k.` (Doom only) |

## Doom Strings

### Monster generic messages:

| String | Default message |
| --- | --- |
| `OB_ZOMBIE` | `%o was killed by a zombieman.` |
| `OB_SHOTGUY` | `%o was shot by a sergeant.` |
| `OB_VILE` | `%o was incinerated by an archvile.` |
| `OB_UNDEAD` | `%o couldn't evade a revenant's fireball.` |
| `OB_FATSO` | `%o was squashed by a mancubus.` |
| `OB_CHAINGUY` | `%o was perforated by a chaingunner.` |
| `OB_SKULL` | `%o was spooked by a lost soul.` |
| `OB_IMP` | `%o was burned by an imp.` |
| `OB_CACO` | `%o was smitten by a cacodemon.` |
| `OB_BARON` | `%o was bruised by a Baron of Hell.` |
| `OB_KNIGHT` | `%o was splayed by a Hell Knight.` |
| `OB_SPIDER` | `%o stood in awe of the spider demon.` |
| `OB_BABY` | `%o let an arachnotron get %h.` |
| `OB_CYBORG` | `%o was splattered by a cyberdemon.` |
| `OB_WOLFSS` | `%o met a Nazi.` |

### Monster melee messages:

| String | Default message |
| --- | --- |
| `OB_UNDEADHIT` | `%o was punched by a revenant.` |
| `OB_IMPHIT` | `%o was slashed by an imp.` |
| `OB_CACOHIT` | `%o got too close to a cacodemon.` |
| `OB_DEMONHIT` | `%o was bit by a demon.` |
| `OB_SPECTREHIT` | `%o was eaten by a spectre.` |
| `OB_BARONHIT` | `%o was ripped open by a Baron of Hell.` |
| `OB_KNIGHTHIT` | `%o was gutted by a Hell Knight.` |

### Multiplayer messages:

| String | Default message |
| --- | --- |
| `OB_MPFIST` | `%o chewed on %k's fist.` |
| `OB_MPCHAINSAW` | `%o was mowed over by %k's chainsaw.` |
| `OB_MPPISTOL` | `%o was tickled by %k's pea shooter.` |
| `OB_MPSHOTGUN` | `%o chewed on %k's boomstick.` |
| `OB_MPSSHOTGUN` | `%o was splattered by %k's super shotgun.` |
| `OB_MPCHAINGUN` | `%o was mowed down by %k's chaingun.` |
| `OB_MPROCKET` | `%o rode %k's rocket.` |
| `OB_MPPLASMARIFLE` | `%o was melted by %k's plasma gun.` |
| `OB_MPBFG_BOOM` | `%o was splintered by %k's BFG.` |

## Heretic Strings

### Monster generic messages:

| String | Default message |
| --- | --- |
| `OB_CHICKEN` | `%o was pecked to death.` |
| `OB_BEAST` | `%o was charred by a weredragon.` |
| `OB_CLINK` | `%o was slashed by a sabreclaw.` |
| `OB_DSPARIL1` | `%o was scalded by D'Sparil's serpent.` |
| `OB_DSPARIL2` | `%o was no match for D'Sparil.` |
| `OB_HERETICIMP` | `%o was scarred by a gargoyle.` |
| `OB_IRONLICH` | `%o was devastated by an ironlich.` |
| `OB_BONEKNIGHT` | `%o was axed by an undead warrior.` |
| `OB_MINOTAUR` | `%o was blasted into cinders by a Maulotaur.` |
| `OB_MUMMY` | `%o was smashed by a golem.` |
| `OB_MUMMYLEADER` | `%o was shrieked to death by a nitrogolem.` |
| `OB_SNAKE` | `%o was rattled by an ophidian.` |
| `OB_WIZARD` | `%o was cursed by a wizard.` |

### Monster melee messages:

| String | Default message |
| --- | --- |
| `OB_DSPARIL1HIT` | `%o was chewed up by D'Sparil's serpent.` |
| `OB_DSPARIL2HIT` | `%o was smacked down by D'Sparil.` |
| `OB_HERETICIMPHIT` | `%o was hacked by a gargoyle.` |
| `OB_IRONLICHHIT` | `%o got up-close and personal with an ironlich.` |
| `OB_BONEKNIGHTHIT` | `%o was slain by an undead warrior.` |
| `OB_MINOTAURHIT` | `%o was pulped by a Maulotaur.` |
| `OB_WIZARDHIT` | `%o was palpated by a wizard.` |

### Normal multiplayer weapon messages:

| String | Default message |
| --- | --- |
| `OB_MPSTAFF` | `%o got staffed by %k.` |
| `OB_MPGAUNTLETS` | `%o got a shock from %k's gauntlets.` |
| `OB_MPGOLDWAND` | `%o waved goodbye to %k's elven wand.` |
| `OB_MPCROSSBOW` | `%o was pegged by %k's ethereal crossbow.` |
| `OB_MPBLASTER` | `%o was blasted a new one by %k's dragon claw.` |
| `OB_MPSKULLROD` | `%o got sent down under by %k's hellstaff.` |
| `OB_MPPHOENIXROD` | `%o was scorched to cinders by %k's phoenix rod.` |
| `OB_MPMACE` | `%o was bounced by %k's firemace.` |

### Tome-powered multiplayer weapon messages:

| String | Default message |
| --- | --- |
| `OB_MPPSTAFF` | `%o got clapped by %k's charged staff.` |
| `OB_MPPGAUNTLETS` | `%o was bled dry by %k's gauntlets.` |
| `OB_MPPGOLDWAND` | `%o was assaulted by %k's elven wand.` |
| `OB_MPPCROSSBOW` | `%o was shafted by %k's ethereal crossbow.` |
| `OB_MPPBLASTER` | `%o was ripped apart by %k's dragon claw.` |
| `OB_MPPSKULLROD` | `%k poured the hellstaff on %o.` |
| `OB_MPPPHOENIXROD` | `%o was burned down by %k's phoenix staff.` |
| `OB_MPPMACE` | `%o was squished by %k's giant mace sphere.` |

## Custom Thing Obituaries

Messages can also be assigned directly inside a Doom/Heretic `Thing` block. This allows obituaries for non-standard or DSDHacked things.

```text
Thing 12 (Imp)
Obituary = %o was scorched by an imp.
Melee obituary = %o was clawed by an imp.
Self obituary = %o was caught in %p own explosion.
```

| Field | Description |
| --- | --- |
| `Obituary` | Generic message for deaths caused by this actor. |
| `Melee obituary` | Message used when this actor causes melee damage. Falls back to `Obituary` when omitted. |
| `Self obituary` | Message used when this actor is the inflictor in a self-kill, such as an explosive projectile killing its owner. Falls back to `OB_KILLEDSELF` when omitted. |

A message assigned directly to a thing takes priority over that thing's classic default `OB_` message.

Thing obituaries also work with new DSDHacked thing indices:

```text
Thing 300
Obituary = %o was erased by something new.
Melee obituary = %o was torn apart by something new.
```
