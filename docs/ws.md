# Widescreen Native Lump Support

Nyan Doom supports new widescreen background and statusbar lumps with the prefix `W_`.

For custom widescreen lump names and control over automatic replacement, see [ANIMINFO](animinfo.md).

When any of the `widescreen` lumps are loaded within a WAD, they will take priority over the following `original` lumps.

Note there are no widescreen versions for [animated lumps](animbg.md) as animate lumps take priority over widescreen lumps. The lump priority is: `start/end animations` > `widescreen` > `original`.

### Doom Patches

| Original       | Widescreen              |
|----------------|-------------------------|
| STBAR          | W_STBAR                 |
| TITLEPIC       | W_TITLEP                |
| INTERPIC       | W_INTERP                |
| CREDIT         | W_CREDIT                |
| HELP           | W_HELP                  |
| HELP1          | W_HELP1                 |
| HELP2          | W_HELP2                 |
| BOSSBACK       | W_BOSSBA                |
| WIMAP0         | W_WIMAP0                |
| WIMAP1         | W_WIMAP1                |
| WIMAP2         | W_WIMAP2                |
| VICTORY2       | W_VICTOR                |
| PFUB1          | W_PFUB1                 |
| PFUB2          | W_PFUB2                 |
| ENDPIC         | W_ENDPIC                |

### Boom Patches

| Original       | Widescreen              |
|----------------|-------------------------|
| HELP01         | W_HELP01                |
| HELP02         | W_HELP02                |
| ...            | ...                     |
| HELP99         | W_HELP99                |

### UMAPINFO Patches
Nyan Doom automatically truncates strings larger than 6 characters to fit the prefix `W_` in the 8 character limit. Strings under than 6 characters won't be truncated.

| UMAPINFO KEY   | Example        | WS Example              |
|----------------|----------------|-------------------------|
| ENTERPIC       | INTER1EN       | W_INTER1                |
| EXITPIC        | EXIT1          | W_EXIT1                 |
| ENDPIC         | EXAMP1PC       | W_EXAMP1                |
| INTERBACKDROP  | MEOW1          | W_MEOW1                 |

### Heretic/Hexen Patches

Lumps not yet implimented.