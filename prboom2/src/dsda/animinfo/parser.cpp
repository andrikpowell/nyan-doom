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
//	NYAN ANIMINFO Parser
//

#include <string.h>
#include <vector>

#include "scanner.h"
#include "parser.h"

extern "C"
{
#include "m_misc.h"
#include "w_wad.h"
}

// ~30 minutes max
#define ANIMINFO_MAX_TICS 65535

typedef struct
{
    anim_frame_mode_t mode;
    int min;
    int max;
} animinfo_tics_t;

typedef enum
{
    ANIMINFO_VERSION_1_0_0 = 100,
} animinfo_version_t;

static animinfo_version_t version;

static int ParsePositiveInteger(Scanner &scanner, const char* lump_name, const char* property)
{
    scanner.MustGetInteger();

    if (scanner.number <= 0)
        scanner.ErrorF("ANIMINFO: lump '%s': %s must be positive", lump_name, property);

    if (scanner.number > ANIMINFO_MAX_TICS)
        scanner.ErrorF("ANIMINFO: lump '%s': %s exceeds maximum of %d tics", lump_name, property, ANIMINFO_MAX_TICS);

    return scanner.number;
}

static animinfo_tics_t ParseRandomTics(Scanner &scanner, const char* lump_name)
{
    animinfo_tics_t tics;

    scanner.MustGetToken('(');
    tics.min = ParsePositiveInteger(scanner, lump_name, "rand minimum");
    scanner.MustGetToken(',');
    tics.max = ParsePositiveInteger(scanner, lump_name, "rand maximum");
    scanner.MustGetToken(')');

    if (tics.max < tics.min)
        scanner.ErrorF("ANIMINFO: lump '%s': 'rand(min,max)' requires min <= max", lump_name);

    tics.mode = ANIM_TICS_RANDOM;
    return tics;
}

static animinfo_tics_t ParseTics(Scanner &scanner, const char* lump_name)
{
    animinfo_tics_t tics;

    // Parse single tic value
    if (scanner.CheckToken(TK_IntConst))
    {
        int value = scanner.number;

        if (value <= 0)
            scanner.ErrorF("ANIMINFO: lump '%s': fixed 'tics' must be positive", lump_name);
        if (value > ANIMINFO_MAX_TICS)
            scanner.ErrorF("ANIMINFO: lump '%s': fixed 'tics' exceeds maximum of %d tics", lump_name, ANIMINFO_MAX_TICS);

        tics.mode = ANIM_TICS_FIXED;
        tics.min = tics.max = value;
        return tics;
    }

    // Parse random tic range
    if (scanner.CheckToken(TK_Identifier) && scanner.StringMatch("rand"))
        return ParseRandomTics(scanner, lump_name);

    scanner.ErrorF("ANIMINFO: lump '%s': 'tics' must be a positive integer or rand(min, max)", lump_name);

    return tics;
}

static void SetRandomRangeTics(animinfo_entry_t* entry, int start, int end, const animinfo_tics_t &tics, std::vector<anim_frame_t> &frames)
{
    frames.clear();
    frames.reserve((size_t)(end - start + 1));

    for (int lump = start; lump <= end; ++lump)
    {
        anim_frame_t frame;
        frame.pic_lumpnum = lump;
        frame.mode = tics.mode;
        frame.tics_min = tics.min;
        frame.tics_max = tics.max;
        frames.push_back(frame);
    }

    entry->type = ANIM_SEQUENCE;
}

static int ParsePicLump(Scanner &scanner, const char* property, const char* lump_name, char* parsed_name)
{
    int lump;

    scanner.MustGetToken(TK_StringConst);
    if (strlen(scanner.string) > 8)
    {
        if (lump_name)
            scanner.ErrorF("ANIMINFO: lump '%s': %s lump name '%s' exceeds 8 characters",
                           lump_name, property, scanner.string);
        else
            scanner.ErrorF("ANIMINFO: %s lump name '%s' exceeds 8 characters", property, scanner.string);
    }

    if (parsed_name)
    {
        strncpy(parsed_name, scanner.string, 8);
        parsed_name[8] = '\0';
    }

    lump = W_CheckNumForName(scanner.string);
    if (lump == LUMP_NOT_FOUND)
    {
        if (lump_name)
            scanner.ErrorF("ANIMINFO: lump '%s': %s lump '%s' does not exist",
                           lump_name, property, scanner.string);
        else
            scanner.ErrorF("ANIMINFO: lump '%s' does not exist", scanner.string);
    }

    return lump;
}

static void ParseAnimateBlock(Scanner &scanner, animinfo_entry_t* entry, const char* lump_name, std::vector<anim_frame_t> &frames)
{
    anim_type_t type = ANIM_NONE;
    int startpic = LUMP_NOT_FOUND;
    int endpic = LUMP_NOT_FOUND;
    animinfo_tics_t range_tics = { ANIM_TICS_FIXED, 0, 0 };
    int pending_frame = false;

    frames.clear();
    scanner.MustGetToken('{');
    while (!scanner.CheckToken('}'))
    {
        scanner.MustGetToken(TK_Identifier);

        if (scanner.StringMatch("type"))
        {
            scanner.MustGetToken('=');
            scanner.MustGetToken(TK_StringConst);
            if (!stricmp(scanner.string, "range"))
                type = ANIM_RANGE;
            else if (!stricmp(scanner.string, "sequence"))
                type = ANIM_SEQUENCE;
            else
                scanner.ErrorF("ANIMINFO: lump '%s': unknown animation type '%s'", lump_name, scanner.string);
        }
        else if (scanner.StringMatch("pic"))
        {
            anim_frame_t frame = { 0 };

            if (pending_frame)
                scanner.ErrorF("ANIMINFO: lump '%s': every sequence 'pic' requires its own 'tics' property", lump_name);

            scanner.MustGetToken('=');
            frame.pic_lumpnum = ParsePicLump(scanner, "pic", lump_name, NULL);
            frame.mode = ANIM_TICS_FIXED;
            frames.push_back(frame);
            pending_frame = true;
        }
        else if (scanner.StringMatch("tics"))
        {
            animinfo_tics_t tics;

            scanner.MustGetToken('=');
            tics = ParseTics(scanner, lump_name);
            if (pending_frame)
            {
                anim_frame_t &frame = frames.back();
                frame.mode = tics.mode;
                frame.tics_min = tics.min;
                frame.tics_max = tics.max;
                pending_frame = false;
            }
            else
            {
                if (range_tics.min)
                    scanner.ErrorF("ANIMINFO: lump '%s': 'range' animation has more than one 'tics' property", lump_name);
                range_tics = tics;
            }
        }
        else if (scanner.StringMatch("startpic"))
        {
            scanner.MustGetToken('=');
            startpic = ParsePicLump(scanner, "startpic", lump_name, NULL);
        }
        else if (scanner.StringMatch("endpic"))
        {
            scanner.MustGetToken('=');
            endpic = ParsePicLump(scanner, "endpic", lump_name, NULL);
        }
        else
        {
            scanner.ErrorF("ANIMINFO: lump '%s': unknown 'animate' property '%s'", lump_name, scanner.string);
        }

        scanner.MustGetToken(';');
    }

    if (type == ANIM_RANGE)
    {
        if (!frames.empty())
            scanner.ErrorF("ANIMINFO: lump '%s': 'pic' is only valid for 'sequence' animations", lump_name);
        if (startpic == LUMP_NOT_FOUND || endpic == LUMP_NOT_FOUND || !range_tics.min)
            scanner.ErrorF("ANIMINFO: lump '%s': 'range' animations require 'startpic', 'endpic', and 'tics'", lump_name);
        if (startpic >= endpic)
            scanner.ErrorF("ANIMINFO: lump '%s': 'range' startpic must precede endpic", lump_name);

        if (range_tics.mode == ANIM_TICS_RANDOM)
            SetRandomRangeTics(entry, startpic, endpic, range_tics, frames);
        else
        {
            entry->type = ANIM_RANGE;
            entry->startpic = startpic;
            entry->endpic = endpic;
            entry->tics = range_tics.min;
        }
    }
    else if (type == ANIM_SEQUENCE)
    {
        if (startpic != LUMP_NOT_FOUND || endpic != LUMP_NOT_FOUND || range_tics.min)
            scanner.ErrorF("ANIMINFO: lump '%s': 'startpic', 'endpic', and unpaired 'tics' are only valid for 'range' animations", lump_name);
        if (pending_frame)
            scanner.ErrorF("ANIMINFO: lump '%s': every sequence 'pic' requires its own 'tics' property", lump_name);
        if (frames.empty())
            scanner.ErrorF("ANIMINFO: lump '%s': sequence animations require at least one 'pic'", lump_name);
        entry->type = ANIM_SEQUENCE;
    }
    else
    {
        scanner.ErrorF("ANIMINFO: lump '%s': 'animate' block requires a 'type' property", lump_name);
    }

    entry->animation_override = true;
    scanner.MustGetToken(';');
}

static void ParseMetadataBlock(Scanner &scanner)
{
    int version_defined = false;

    scanner.MustGetToken(TK_StringConst);
    if (stricmp(scanner.string, "ANIMINFO"))
        scanner.ErrorF("ANIMINFO: metadata identifier must be 'ANIMINFO'");

    scanner.MustGetToken('{');
    while (!scanner.CheckToken('}'))
    {
        scanner.MustGetToken(TK_Identifier);
        if (!scanner.StringMatch("version"))
            scanner.ErrorF("ANIMINFO: unknown 'metadata' property '%s'", scanner.string);
        if (version_defined)
            scanner.ErrorF("ANIMINFO: 'metadata' version is defined more than once");

        scanner.MustGetToken('=');
        scanner.MustGetToken(TK_StringConst);
        if (!strcmp(scanner.string, "1.0.0"))
            version = ANIMINFO_VERSION_1_0_0;
        else
            scanner.ErrorF("ANIMINFO: unsupported specification version '%s'", scanner.string);
        version_defined = true;
        scanner.MustGetToken(';');
    }

    if (!version_defined)
        scanner.ErrorF("ANIMINFO: 'metadata' requires a 'version' property");

    scanner.MustGetToken(';');
}

static void ParseLumpBlock(Scanner &scanner)
{
    animinfo_entry_t entry = { 0 };
    std::vector<anim_frame_t> frames;
    char lump_name[9];

    entry.lump = ParsePicLump(scanner, "lump", NULL, lump_name);
    scanner.MustGetToken('{');

    while (!scanner.CheckToken('}'))
    {
        scanner.MustGetToken(TK_Identifier);

        if (scanner.StringMatch("animate"))
        {
            scanner.MustGetToken('=');
            if (scanner.CheckToken(TK_Identifier))
            {
                if (!scanner.StringMatch("clear"))
                    scanner.ErrorF("ANIMINFO: lump '%s': animate expects 'clear' or a block", lump_name);
                entry.animation_override = true;
                entry.type = ANIM_NONE;
                frames.clear();
                scanner.MustGetToken(';');
            }
            else
            {
                ParseAnimateBlock(scanner, &entry, lump_name, frames);
            }
        }
        else if (scanner.StringMatch("widepic"))
        {
            scanner.MustGetToken('=');
            entry.widepic_override = true;
            if (scanner.CheckToken(TK_Identifier))
            {
                if (!scanner.StringMatch("clear"))
                    scanner.ErrorF("ANIMINFO: lump '%s': widepic expects 'clear' or a lump name", lump_name);
                entry.widepic = LUMP_NOT_FOUND;
            }
            else
            {
                entry.widepic = ParsePicLump(scanner, "widepic", lump_name, NULL);
            }
            scanner.MustGetToken(';');
        }
        else
        {
            scanner.ErrorF("ANIMINFO: lump '%s': unknown property '%s'", lump_name, scanner.string);
        }
    }

    scanner.MustGetToken(';');
    entry.frames = frames.empty() ? NULL : frames.data();
    entry.num_frames = (int)frames.size();
    N_ApplyAnimInfo(&entry);
}

int ParseAnimInfo(const unsigned char *buffer, size_t length, animinfo_errorfunc err)
{
	Scanner scanner((const char*)buffer, (int)length);

    scanner.SetErrorCallback(err);

    if (!scanner.TokensLeft())
        scanner.ErrorF("ANIMINFO: lump is empty; metadata block required");

    scanner.MustGetToken(TK_Identifier);
    if (!scanner.StringMatch("metadata"))
        scanner.ErrorF("ANIMINFO: 'metadata' block must be first");
    ParseMetadataBlock(scanner);

    while (scanner.TokensLeft())
    {
        scanner.MustGetToken(TK_Identifier);
        if (scanner.StringMatch("lump"))
            ParseLumpBlock(scanner);
        else if (scanner.StringMatch("metadata"))
            scanner.ErrorF("ANIMINFO: 'metadata' block is defined more than once");
        else
            scanner.ErrorF("ANIMINFO: expected lump block, got '%s'", scanner.string);
    }

    return 1;
}
