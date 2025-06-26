## GAMEVERS Lump

The GAMEVERS lump can be used only when the [COMPLVL](complvl.md) lump has the key of `vanilla`, to specify an exact vanilla complevel and [limit-removing](limit_removing.md).

GAMEVERS will NOT be read if COMPLVL is absent or if COMPLVL has any other key than `vanilla`.

The order of precedence is highest to lowest: parameter > lump > config. This means that if a another complevel is set and/or if limit-removing isn't in the parameters (example: `-complevel 2`), the GAMEVERS `value` or `key` will be ignored and the game will run ignoring the contents of COMPLVL and GAMEVERS.

### Specification

The GAMEVERS lump has two values:

`complvl nolimits`

The `complvl` value defines the specific vanilla complevel, while the `nolimits` value defines limit-removing or not. The `nolimits` value can be ommited to run in normal mode.

### complvl

- `1.2`
  - Selects complevel 0, for Doom 1.2 compatibility.
- `1.666`
  - Selects complevel 1, for Doom 1.666 compatibility.
- `1.9`
  - Selects complevel 2, for Doom (pre-ultimate) / Doom 2 compatibility.
- `ultimate`
  - Selects complevel 3, for Ultimate Doom compatibility.
- `final`
  - Selects complevel 4, for Final Doom (Plutonia, TNT) compatibility.
- `nolimits`
  - Allows the port to automatically detect the vanilla complevel, while activating limit-removing.
  - When using this, the second `value` is not needed.

### limit

- `nolimits`
  - Runs complevel under [limit-removing](limit_removing.md) mode. Removes overflow errors and emulation. Omit to run in normal mode. (Note this can be overriden by parameters).

### Examples:

- `1.666` - Selects complevel 1
- `1.666 nolimits` - Selects complevel 1, while activating limit-removing
- `nolimits` - Auto-selects vanilla complevel, while activating limit-removing 