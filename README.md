<a href="#"><img src="https://andrikarkane.com/doom/nyan-doom/nyan-doom-logo.png" width="500"></a>

# Nyan Doom
This is the most furry, fuzzy, and most cuddly Doom port you will ever come across! Based on the DSDA-Doom source port, Nyan Doom focuses on new quality-of-life and never-before-seen features that you won't find anywhere else! Check out Nyan Doom on [Doomworld!](https://www.doomworld.com/forum/topic/145913/)

You don't have to worry about Nyan Doom falling behind, as it's kept up to date with the latest DSDA-Doom. 

### Nyan Features For Mappers
- New [animated background / menu element](./docs/animbg.md) lump support (for animated M_DOOM, TITLEPIC, INTERPIC, and even UMAPINFO backgrounds!)
- Native [widescreen](./docs/ws.md) lump support for widescreen assets (avoiding the need for separate asset WADs)
- Support for [GAMEVERS](./docs/gamevers.md) lump in combination with [COMPLVL](./docs/complvl.md) to further specify Vanilla compatibility.
- [Limit-Removing Support](./docs/limit_removing.md) for Vanilla style maps to ignore overflow errors (such as all-ghosts, etc)

### Nyan Features For Players
- New innovative features you won't find anywhere else such as "flashing item bonuses", in-depth "Boom Translucency" customization, a smarter "Colored Blood" option, and a new experimental "Light Amplification Visor" screen effect
- Quality-of-life features such as "Skip IWAD Story text For PWADs" or "Extended Intermission Pause" so you can actually read the next map name
- Native "berserk" and "armour" elements on the statusbar *(Options > Status Bar / HUD)*
- "Status Widget" for the extended HUD, that shows currently active powerups *(Options > Status Bar / HUD > Status Widget)*
- Savegame support for per-game modifiers (`pistol start`, `respawn`, `fast monsters`, etc)
- Enhanced ENDOOM support
- and more!

### Patch Notes
- [v1.2](./patch_notes/v1.2.md)
- [v1.1](./patch_notes/v1.1.md)

### Nyan-Launcher
- [Windows Version](https://github.com/andrikpowell/nyan-launcher/releases/latest)
- MacOS Version [(in Releases)](https://github.com/andrikpowell/nyan-doom/releases/latest)

### Credits
- Nyan Doom and its logo design are by Andrik "Arsinikk" Powell.

___

# From the original dsda-doom project
This is a successor of prboom+ with many new features, including:
- Heretic, Hexen, MBF21, Doom-in-Hexen, UDMF, and MAPINFO support
- In-game console and scripting
- Full controller support
- Palette-based opengl renderer
- Debugging features for testing
- Strict mode for speedrunning
- Various quality of life improvements
- Advanced tools for TASing
- Rewind

### Patch Notes
- [v0.28](./patch_notes/v0.28.md)
- [v0.27](./patch_notes/v0.27.md)
- [v0.26](./patch_notes/v0.26.md)

### Launcher
There is a dedicated launcher for this port available [here](https://github.com/Pedro-Beirao/dsda-launcher) by PBeGood4.

### Doom-in-Hexen Support
- [Full details](./docs/doom_in_hexen.md)

### UDMF Support
- [Full details](./docs/udmf.md)

### MAPINFO Support
- [Full details](./docs/mapinfo.md)

### Hexen Support
- DSDA-Doom includes demo-compatible support for hexen.
  - Use -iwad HEXEN.WAD (-file HEXDD.WAD for the expansion)
    - Or drag wads onto the exe
  - You can force hexen engine behaviour with `-hexen` (shouldn't be necessary)
- Don't need to supply complevel (hexen is complevel 0 by necessity)
- Known issues
  - Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for hexen (it will default instead to "Doom format")
  - The "Apply multisampling" automap option is disabled for hexen
  - Automap colors are not configurable for hexen
  - Some of the more advanced features are not implemented for hexen yet, and using them may cause crashes or other odd behaviour.
  - Some menus extend over the hud.
  - Monster counter doesn't work as expected, due to cluster format (ex hud / levelstat)
  - Hexen-style skies aren't implemented yet (layering, etc)
  - The ALTSHADOW thing flag isn't affecting the rendering
  - Dynamic fade palettes aren't being used
  - The yellow message variant isn't implemented

### Heretic Support
- DSDA-Doom includes demo-compatible support for heretic (all the demos stored on dsda are in sync).
- Heretic game logic should be set automatically if you use `HERETIC.WAD` as the iwad. If it doesn't work, please use the `-heretic` commandline option. This flips a switch in the engine that determines all the core game data.
- Don't need to supply complevel (heretic is complevel 0 by necessity)
- Known issues
  - Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for heretic (it will default instead to "Doom format").
  - The "Apply multisampling" automap option is disabled for heretic.
  - Automap colors are not configurable for heretic.
  - Some of the more advanced features are not implemented for heretic yet, and using them may cause crashes or other odd behaviour.
  - Dehacked support for heretic isn't implemented yet.
  - Some menus extend over the hud.

### Other Standards
- [MBF21 v1.4](https://github.com/kraflab/mbf21)
- [UMAPINFO v2.2](https://github.com/kraflab/umapinfo)

### Maintainers
- @fabiangreffrath, @rfomin, and @Pedro-Beirao 2024-
- @kraflab 2020-2024

### Credits
- The DSDA-Doom icon was designed by Mal (129thVisplane). Thanks!
