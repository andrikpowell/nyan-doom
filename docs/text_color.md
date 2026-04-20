## Text Color Configuration

Text Color configurations are *no longer* stored in the NYANTC lump.

You can access them in-game under `Options > Display > Colors` which are then saved in `nyan-doom.cfg`.

### Config Specification

Colors can also be manually edited in the config file under the `# Text colors` header with each option holding the prefix `dsda_tc_`.

Various elements of the extended hud, the automap, and the menus can be changed. The color indices correspond to entries in the [DSDACR lump](./color_range.md).

### Color Range to Index

The number values are tied to the following color ranges:

| Color Range | Color Index |
| ----------- | ------|
| default (untranslated)     | 0     |
| brick       | 1     |
| tan         | 2     |
| gray        | 3     |
| green       | 4     |
| brown       | 5     |
| gold        | 6     |
| red         | 7     |
| blue        | 8     |
| orange      | 9     |
| yellow      | 10    |
| light blue  | 11    |
| black       | 12    |
| purple      | 13    |
| white       | 14    |
