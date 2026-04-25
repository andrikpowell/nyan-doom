## HUD Configuration

HUD configurations are stored in the NYANHUD lump. These defaults can be changed by replacing the lump or specifying a hud config file with `-hud filename`. There are separate configurations for each game (doom, heretic, and hexen), with or without the status bar.

NYANHUD is very similar to DSDAHUD, but does include extra widgets, thus is incompatible with DSDA-Doom.

Multiple NYANHUD lumps can be stacked similar to UMAPINFO or DEHACKED. Note that the entire hud blocks will be replaced by the most recent version. This also applies to the `full` variants.

Options marked with 😸 are Nyan exclusive.

## Specification

A HUD configuration starts with the environment: `game variant`
- The `game` is `doom`, `heretic`, or `hexen`
- The `variant` options are:
  - `ex` (with status bar and extended hud on)
  - `off` (with status bar and extended hud off)
  - `full` (without status bar)*
  - `map` (on top of the automap)

> *😸 `full` variant supports 16 huds. [Learn more](#full-variant-specification)

The configuration then consists of a series of components to display: `name x y alignment [args]`
- The `name` specifies which component to display
- The `x` and `y` fields set the location of the component
  - `y` values are automatically adjusted for different font heights
- The `alignment` controls how the position is translated in different screen sizes
  - `top`
  - `top_left`
    - This alignment accounts for the size of the message area
  - `top_right`
  - 😸 `top_center`
    - component is not centered, it's anchored center
  - `bottom`
  - `bottom_left`
  - `bottom_right`
  - 😸 `bottom_center`
    - component is not centered, it's anchored center
  - `left`
  - `right`
  - 😸 `center`
    - component is not centered, it's anchored center
  - `none`
- The `args` are a series of optional parameters (see specific components for more info)
- For convenience, bottom-aligned `y` values are the distance from the bottom of the screen (`full`) or top of the status bar (`ex` and `off`)

This example configures the extended hud for doom with just the stat totals and time in the bottom left (above the status bar):
```
doom ex
stat_totals 2 8 bottom_left
composite_time 2 16 bottom_left
```

Finally, there is a positioning helper: `add_offset y alignment`
- This compensates for mixed stretching between the message font and the extended hud font
- Set `y` as the number of message font component lines at the edge of the given alignment

You can find the current default configuration [here](../prboom2/data/lumps/nyanhud.lmp).

## `Full` Variant Specification

😸 The `full` variant (without status bar) has been expanded to **16 slots**: `game full # "name"`
- The `game` is `doom`, `heretic`, or `hexen`
- The `#` is the hud number (0-16)
- The `name` (in quotes) is an optional name for the hud that shows in-game when cycling through huds
> Note that the legacy `game full` structure is still supported, it's just not possible to name the hud.

Here's an example of a `full` customisable hud:
```
doom full 5 "boom classic"
health_text 2 8 bottom_left 1
```

By default Nyan Doom comes with a NYANHUD with multiple huds *(~5 or 6 depending on game type)*. If you would like to limit the amount of default huds, you can use the command `clearhud` in the full hud defintion.

Here's an example of a `full` cleared hud:
```
doom full 2 "modern"
clearhud
```

## NYANHUD Components

Unless otherwise specified, argument values are integers. For toggles, a 1 means on and a 0 means off. For example, `stat_totals 2 8 bottom_left 1 0 1` would turn off items but keep kills and secrets enabled.

### Basic Ex-Hud Stuff

- `composite_time`: shows the current level time and the total time
  - Supports 1 argument: `show_label`
    - `show_label`: shows the "time" label
- `local_time`: shows the local time
  - Supports 2 arguments: `hide_seconds am_pm`
    - 😸 `hide_seconds`: hides the seconds on the time
    - 😸 `am_pm`: converts the time to AM / PM format
- `free_text`: shows arbitrary text
  - Update the text via the "exhud free text" option
  - ...or from the console `free_text.update <text>` / `free_text.clear`
  - Use `\n` to create a new line
  - Use `\cXY` to change to color `XY`

### Big Widgets

- 😸 `doomguy_face`: shows Doomguy's handsome face ;)
- 😸 `loading_disk`: shows either the `STDISK` or `STCDROM` loading icons
- `keys`: shows the acquired keys
  - Supports 3 arguments: `horizontal spacing boom_classic`
    - `horizontal`: displays the component horizontally rather than vertically
    - 😸 `spacing`: allows customisation for the spacing between keys (default is 3)
    - 😸 `boom_classic`: displays the `KEY` label in front of keys (requires `horizontal`) while also separating the skull keys from keycards
- 😸 `big_ammo_text`: shows the ammo for the current weapon in the status bar font
  - Supports 2 arguments: `right_align x_anchor`
    - `right_align`: aligns the text to the right (x is still anchored left)
    - `x_anchor`: draws element to the left (0), center (1), or right (2)
- 😸 `big_ammo`: shows the ammo for the current weapon in the status bar font with the ammo sprite
  - Supports 2 arguments: `right_align x_anchor`
    - `right_align`: moves the sprite to the right of the text and aligns the text to the right (x is still anchored left)
    - `x_anchor`: draws element to the left (0), center (1), or right (2)
- 😸 `big_mana`: shows the mana (blue, green, or both) for the current weapon with mana icons and medium font
  - Only works in Hexen
  - Default draws blue above green, each with icons (vertical)
  - Supports 4 arguments: `right_align x_anchor horizontal show_active`
    - `right_align`: moves the sprite to the right of the text and aligns the text to the right
    - `x_anchor`: draws element to the left (0), center (1), or right (2)
    - `horizontal`: displays the component horizontally rather than vertically
    - `show_active`: only draws the current mana type for the selected weapon
- `big_armor`: shows the player armor (color-coded) in the status bar font with the armor sprite
  - Supports 3 arguments: `right_align x_anchor percent`
    - 😸 `right_align`: moves the sprite to the right of the text and aligns the text to the right  (x is still anchored left)
    - 😸 `x_anchor`: draws element to the left (0), center (1), right (2), or percent (3)
    - 😸 `percent`: draws a percent at the end
- `big_armor_text`: shows the player armor (color-coded) in the status bar font
  - Supports 3 arguments: `right_align x_anchor percent`
    - 😸 `right_align`: aligns the text to the right  (x is still anchored left)
    - 😸 `x_anchor`: draws element to the left (0), center (1), right (2), or percent (3)
    - 😸 `percent`: draws a percent at the end
- `big_health`: shows the player health (color-coded) in the status bar font with the health sprite
  - Supports 3 arguments: `right_align x_anchor percent`
    - 😸 `right_align`: moves the sprite to the right of the text and aligns the text to the right  (x is still anchored left)
    - 😸 `x_anchor`: draws element to the left (0), center (1), right (2), or percent (3)
    - 😸 `percent`: draws a percent at the end
- `big_health_text`: shows the player health (color-coded) in the status bar font
  - Supports 3 arguments: `right_align x_anchor percent`
    - 😸 `right_align`: aligns the text to the right  (x is still anchored left)
    - 😸 `x_anchor`: draws element to the left (0), center (1), right (2), or percent (3)
    - 😸 `percent`: draws a percent at the end

### Small Widgets
- `ammo_text`: shows the weapons and ammo as the status bar does
  - Supports 1 argument: `show_names`
    - `show_names`: shows ammo names in the component
- `weapon_text`: shows the acquired weapons (color-coded for berserk).
  - Supports 3 arguments: `grid show_label color_ammo`
    - `grid`: displays the weapons in a 3x3 grid rather than horizontally
    - `show_label`: shows the "wpn" label
    - `color_ammo`: colors the weapons based on thir ammo percentage
- `armor_text`: shows the player armor (color-coded)
  - Supports 1 argument: `draw_boom_bar`
    - 😸 `draw_boom_bar`: draws a boom style bar with text
- `health_text`: shows the player health (color-coded)
  - Supports 1 argument: `draw_boom_bar`
    - 😸 `draw_boom_bar`: draws a boom style bar with text
- `ready_ammo_text`: shows the ammo for the current weapon (color-coded)
  - Supports 1 argument: `draw_boom_bar`
    - 😸 `draw_boom_bar`: draws a boom style bar with text (not supported in Hexen)

### Demo / Test Stuff
- `fps`: shows the current fps
- `attempts`: shows the current and total demo attempts
- `render_stats`: shows various render stats (`idrate`)
- `speed_text`: shows the game clock rate
  - Supports 1 argument: `show_label`
    - `show_label`: shows the "speed" label
- `command_display`: shows the history of player commands (demo or otherwise)
- `coordinate_display`: shows various coordinate and velocity data
- `event_split`: shows the time of an event tracked by the `-time_*` arguments
- `level_splits`: shows the splits for the level time and the total time (intermission screen)
- `line_display`: shows the last lines the player activated
  - requires menu option "Show loading disk" to be set
- `tracker`: shows the active trackers (they stack *vertically*)
- `color_test`: shows the hud fonts in different color modes

### Raven Artifacts
- `big_artifact`: shows the current artifact as seen on the status bar
  - Supports 3 arguments: `simple always_show center`
    - 😸 `simple`: doesn't show the artifact box around artifacts
    - 😸 `always_show`: doesn't hide the artifact box when `big_artifact_bar` is visible (vanilla hides the artifact box)
    - 😸 `center`: centers the component horizontally
- 😸 `big_artifact_bar`: shows a transparent artifact inventory bar as seen on the status bar
  - disappears when `big_artifact` is visible
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally
- 😸 `artifact_desc`: shows heretic/hexen inventory names and/or descriptions above the inventory (based on KEX)
  - Uses the message font
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally

### Automap
- `map_time`: shows the level / total time
  - Uses the message font
- `map_coordinates`: shows the player's position
  - Uses the message font
- `map_title`: shows the current map's title
  - Uses the message font
  - Supports 1 argument: `no_author`
    - 😸 `no_author`: Forces the map title to never cycle to the author
- `map_totals`: shows the kills / secrets / items on the current map
  - Uses the message font with word labels
  - Supports 4 arguments: `show_kills show_items show_secrets stat_format`
    - `show_kills`: shows kills in the component
    - `show_items`: shows items in the component
    - `show_secrets`: shows secrets in the component
- `minimap`: shows the minimap
  - Supports 3 arguments: `width height scale`
    - `width`: width of the component
    - `height`: height of the component
    - `scale`: width of the component in map units

### Messages
- `message`: shows the current player message
  - Uses the message font
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally
- `secret_message`: shows the secret revealed message
  - Uses the message font
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally
- 😸 `announce_message`: shows the announce map message with author on next line
  - Uses the message font
  - Requires menu option "announce message" to be set to "on"
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally

### Stats
- `stat_totals`: shows the kills / secrets / items on the current map
  - Supports 7 arguments: `show_kills show_items show_secrets vertical show_labels stat_format`
    - `show_kills`: shows kills in the component
    - `show_items`: shows items in the component
    - `show_secrets`: shows secrets in the component
    - `vertical`: displays the stats vertically rather than horizontally
    - `show_labels`: shows the "K" "I" "S" labels
    - 😸 `stat_label`: shows `STS` label in front of stats

### Status
- 😸 `status_widget`: shows icons for armor type, backpack, and other powerups
  - Only available for Doom (not Heretic / Hexen)
  - Supports 2 arguments: `vertical reverse`
    - `vertical`: draws icons vertically top to bottom
    - `reverse`: draws icons from right to left... or if `vertical`, draws bottom to top
- 😸 `status_timers`: shows timers and status for powerups and other statuses (armor type, backpack, etc)
  - Works with all games: Doom / Heretic / Hexen.
  - Supports 1 argument: `right_align`
    - `right_align`: aligns text to the right
- 😸 `target_health`: displays the current health of enemies (based on CRL)
  - Uses the message font (coloured)
  - Supports 1 argument: `center`
    - `center`: centers the component horizontally
