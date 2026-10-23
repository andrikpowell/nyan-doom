//
// Copyright(C) 2024 Roman Fomin
// Copyright(C) 2026 Fabian Greffrath
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
//	DSDA JSON Parser
//

#include "m_json.h"

#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "lprintf.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "yyjson.h"

dboolean JS_GetVersion(json_t *json, version_t *version)
{
    json_t *js_version = JS_GetObject(json, "version");
    if (!JS_IsString(js_version))
    {
        return false;
    }

    const char *string = JS_GetString(js_version);
    if (M_ParseVersion(string, version))
    {
        return true;
    }

    return false;
}

typedef struct
{
    yyjson_doc *json_doc;
    int lumpnum;
} doc_t;

static array_t docs;

json_t *JS_OpenOptions(int lumpnum, dboolean comments)
{
    const char *string = W_LumpByNum(lumpnum);
    int length = W_LumpLength(lumpnum);

    yyjson_read_flag flag = comments ? YYJSON_READ_ALLOW_COMMENTS : 0;

    yyjson_read_err err;
    // yyjson only modifies the input when YYJSON_READ_INSITU is requested.
    yyjson_doc *json_doc = yyjson_read_opts((char *)(void *)string, length,
                                            flag, NULL, &err);

    if (!json_doc)
    {
        size_t line, col, chr;
        yyjson_locate_pos(string, length, err.pos, &line, &col, &chr);
        const char *name = W_LumpName(lumpnum);
        lprintf(LO_ERROR, "%.8s(%d:%d): read error: %s\n", name, (int)line,
                (int)col, err.msg);
        return NULL;
    }

    doc_t *doc = M_ArrayGetNewItem(&docs, sizeof(*doc));
    *doc = (doc_t){json_doc, lumpnum};

    json_t *json = yyjson_doc_get_root(json_doc);

    return json;
}

json_t *JS_Open(const char *lump, const char *type, version_t maxversion)
{
    int lumpnum = W_CheckNumForName(lump);
    if (lumpnum < 0)
    {
        return NULL;
    }

    json_t *json = JS_OpenOptions(lumpnum, false);
    if (!json)
    {
        return NULL;
    }

    json_t *js_type = JS_GetObject(json, "type");
    if (!JS_IsString(js_type))
    {
        lprintf(LO_ERROR, "%s: no type string", lump);
        JS_CloseOptions(lumpnum);
        return NULL;
    }

    const char *s = JS_GetString(js_type);
    if (strcmp(s, type))
    {
        lprintf(LO_ERROR, "%s: wrong type %s", lump, s);
        JS_CloseOptions(lumpnum);
        return NULL;
    }

    version_t v = {0};
    if (!JS_GetVersion(json, &v))
    {
        lprintf(LO_ERROR, "%s: no version string", lump);
        JS_CloseOptions(lumpnum);
        return NULL;
    }

    if (M_CompareVersions(&v, &maxversion) > 0)
    {
        lprintf(LO_ERROR, "%s: max supported version %d.%d.%d", lump,
                maxversion.major, maxversion.minor, maxversion.revision);
        JS_CloseOptions(lumpnum);
        return NULL;
    }

    return json;
}

void JS_CloseOptions(int lumpnum)
{
    doc_t *doc = docs.data;

    for (int i = 0; i < docs.count; ++i)
    {
        if (doc[i].lumpnum == lumpnum)
        {
            yyjson_doc_free(doc[i].json_doc);
            doc[i] = doc[--docs.count];
            break;
        }
    }
}

void JS_Close(const char *lump)
{
    int lumpnum = W_CheckNumForName(lump);
    if (lumpnum < 0)
    {
        return;
    }

    JS_CloseOptions(lumpnum);
}

dboolean JS_IsObject(json_t *json)
{
    return yyjson_is_obj(json);
}

dboolean JS_IsNull(json_t *json)
{
    return yyjson_is_null(json);
}

dboolean JS_IsBoolean(json_t *json)
{
    return yyjson_is_bool(json);
}

dboolean JS_IsNumber(json_t *json)
{
    return yyjson_is_num(json);
}

dboolean JS_IsString(json_t *json)
{
    return yyjson_is_str(json);
}

dboolean JS_IsArray(json_t *json)
{
    return yyjson_is_arr(json);
}

json_t *JS_GetObject(json_t *json, const char *string)
{
    return yyjson_obj_get(json, string);
}

int JS_GetArraySize(json_t *json)
{
    return (int)yyjson_arr_size(json);
}

json_t *JS_GetArrayItem(json_t *json, int index)
{
    return yyjson_arr_get(json, index);
}

json_arr_iter_t *JS_ArrayIterator(json_t *json)
{
    json_arr_iter_t *iter = Z_Malloc(sizeof(*iter));
    yyjson_arr_iter_init(json, iter);
    return iter;
}

json_t *JS_ArrayNext(json_arr_iter_t *iter)
{
    json_t *value = yyjson_arr_iter_next(iter);
    if (!value)
    {
        Z_Free(iter);
    }
    return value;
}

dboolean JS_GetBoolean(json_t *json)
{
    return yyjson_get_bool(json);
}

dboolean JS_GetBooleanValue(json_t *json, const char *string)
{
    json_t *obj = JS_GetObject(json, string);
    if (JS_IsBoolean(obj))
    {
        return JS_GetBoolean(obj);
    }
    return false;
}

double JS_GetNumber(json_t *json)
{
    return yyjson_get_num(json);
}

double JS_GetNumberValue(json_t *json, const char *string)
{
    json_t *obj = JS_GetObject(json, string);
    if (JS_IsNumber(obj))
    {
        return JS_GetNumber(obj);
    }
    return 0;
}

int JS_GetInteger(json_t *json)
{
    return yyjson_get_int(json);
}

int JS_GetIntegerValueDefault(json_t *json, const char *string, const int def)
{
    json_t *obj = JS_GetObject(json, string);
    if (JS_IsNumber(obj))
    {
        return JS_GetInteger(obj);
    }
    return def;
}

int JS_GetIntegerValue(json_t *json, const char *string)
{
    return JS_GetIntegerValueDefault(json, string, 0);
}

const char *JS_GetString(json_t *json)
{
    return yyjson_get_str(json);
}

const char *JS_GetStringValue(json_t *json, const char *string)
{
    json_t *obj = JS_GetObject(json, string);
    if (JS_IsString(obj))
    {
        return JS_GetString(obj);
    }
    return NULL;
}

json_obj_iter_t *JS_ObjectIterator(json_t *json)
{
    yyjson_obj_iter *iter = Z_Malloc(sizeof(*iter));
    if (yyjson_obj_iter_init(json, iter))
    {
        return iter;
    }
    Z_Free(iter);
    return NULL;
}

dboolean JS_ObjectNext(json_obj_iter_t *iter, json_t **key, json_t **value)
{
    *key = yyjson_obj_iter_next(iter);
    if (*key == NULL)
    {
        Z_Free(iter);
        return false;
    }
    *value = yyjson_obj_iter_get_val(*key);
    return true;
}
