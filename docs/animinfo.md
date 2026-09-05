## ANIMINFO

ANIMINFO is a lump that allows authors to define animated and widescreen replacements for graphic lumps. It acts as a substitution system: the original lump remains available to ports that do not support ANIMINFO, while supporting ports like Nyan Doom can display an animation or widescreen graphic in its place.

Nyan Doom recognizes a set of default [animation ranges](animbg.md) and [widescreen names](ws.md). ANIMINFO provides more control over lump replacements when it comes to lump names and animation timing.

Note that ANIMINFO is not currently support for Raven games.

## Usage

ANIMINFO is a plain-text lump. All ANIMINFO lumps are parsed in wad load order. Entries for the same graphic stack by property: a later `animate` proptery replaces the earlier animation, while a later `widepic` property replaces the earlier widescreen graphic. Omitting one property leaves its previous value unchanged.

Every ANIMINFO lump must begin with exactly one metadata block. ANIMINFO lumps are stackable based on wad load order.

## Metadata

```c
metadata "ANIMINFO"
{
  version = "1.0.0";
}
```

The metadata block must be first, at the top in the lump, and may occur only once.

| Property | Description |
| --- | --- |
| **version = "\<Version\>";** | Selects the ANIMINFO version used for parsing. Future updates may come with more features. |

The latest ANIMINFO verison is `1.0.0`.

## Lump Entry

```c
lump "TITLEPIC"
{
  animate = clear;
  widepic = "W_TITLEP";
}
```

Defines replacements for a graphic lump.

| Property | Description |
| --- | --- |
| **animate = { _properties_ }** | Defines a range or sequence animation. A later `animate` property replaces the earlier animation definition. |
| **widepic = "\<LumpName\>";** | Specifies what lump to use for the widescreen replacement. |
| **animate = clear;** | Disables animation for this lump, including automatic `S_`/`E_` detection. |
| **widepic = clear;** | Disables widescreen for this lump, including automatic `W_` detection. |

## Animation

ANIMINFO supports range and sequence animations. Every animation block requires a single `type`. 

### Range

```c
animate =
{
  type = range;
  oscillate = true;
  tics = 4;
  startpic = "S_HELP";
  endpic = "E_HELP";
}
```

A range animation displays every lump from `startpic` through `endpic`, in wad-directory order. This format is not compatible with directory-type formats like `pk3` and is only supported by `wad` files. The start lump must come before the end lump.

| Property | Description |
| --- | --- |
| **type = range;** | Selects a range animation. |
| **oscillate = \<Boolean\>;** | Plays animation forward and then backward in a loop. Defaults to `false`. |
| **tics = \<Tics\>;** | Sets one fixed duration for every frame. The value must be positive. |
| **tics = rand(\<Min\>, \<Max\>);** | Randomizes each frame duration inclusively between two numbers. `Min` must not exceed `Max`. Whitespace is allowed between `()`. |
| **startpic = "\<LumpName\>";** | Sets the first lump in the animation range. Required. |
| **endpic = "\<LumpName\>";** | Sets the last lump in the animation range. Required. |

When a range uses random timing, it converts the range into a sequence so each frame can retain its selected duration.

The limit for tic values is 65,535 tics (~30 minutes).

### Sequence

```c
animate =
{
  type = sequence;
  oscillate = true;
  pic = "S_TITLEP"; tics = 4;
  pic = "TITLEPIC"; tics = rand(8, 30);
  pic = "E_TITLEP"; tics = 10;
}
```

A sequence animation lists its animation frame-by-frame. Every `pic` must be followed by its own `tics` property. Timing is not inherited from the previous frame.

| Property | Description |
| --- | --- |
| **type = sequence;** | Selects a sequence animation. |
| **oscillate = \<Boolean\>;** | Plays animation forward and then backward in a loop. Defaults to `false`. |
| **pic = "\<LumpName\>";** | Adds the lump as the next frame. At least one frame is required. |
| **tics = \<Tics\>;** | Sets the frame's fixed duration. |
| **tics = rand(\<Min\>, \<Max\>);** | Randomizes the frame's duration between two positive values. The duration is selected once when that frame begins. Whitespace is allowed between `()`. |

## Full ANIMINFO Example

```c
metadata "ANIMINFO"
{
  version = "1.0.0";
}

lump "TITLEPIC"
{
  animate =
  {
    type = sequence;
    pic = "S_TITLEP"; tics = 4;
    pic = "TITLEPIC"; tics = rand(8, 30);
    pic = "TITLEP2"; tics = 4;
    pic = "TITLEP3"; tics = 7;
    pic = "TITLEP4"; tics = 22;
    pic = "TITLEP5"; tics = 45;
    pic = "TITLEP6"; tics = 5;
    pic = "TITLEP10"; tics = 8;
    pic = "E_TITLEP"; tics = 10;
  }
  widepic = "W_TITLEP";
}

lump "HELP"
{
  animate =
  {
    type = range;
    tics = 4;
    startpic = "S_HELP";
    endpic = "E_HELP";
  }
  widepic = "W_HELP";
}

lump "CREDIT"
{
  animate =
  {
    type = range;
    tics = 4;
    startpic = "S_CREDIT";
    endpic = "E_CREDIT";
  }
  widepic = "W_CREDIT";
}
```

## Nyan Doom's Default Animation System

Without a lump being defined by ANIMINFO, Nyan Doom checks for its default animation ranges (`S_`/`E_`) and widescreen (`W_`) names. Nyan Doom's automatic detection system will silently not enable animations / widescreen assets when said lumps are not present. By default, the `tics` duration for these animations are `8` tics (the default duration from the `ANIMATED` lump).

ANIMINFO is intentionally stricter. Missing the required properties or missing lumps referenced by `pic`, `startpic`, `endpic`, and `widepic` will produce an error at startup. Currently `range` animations only check if the start and end pics are valid.

You can use `animate = clear;` and `widepic = clear;` to disable Nyan Doom's automatic animations and widescreen asset system.
