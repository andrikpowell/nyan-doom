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
//	NYAN Vanilla Patch / Texture Logic
//  - Tutti-Frutti Emulation
//  - Medusa Emulation
//

#include "w_wad.h"
#include "r_main.h"
#include "lprintf.h"

#include "dsda/settings.h"

//
//
// Choco Tutti-Frutti Header
//
//

// [AR] Match Chocolate Doom's heap layout for multi-patch textures
static int R_FakeChocolateHeapHeader(byte *header, int lumpSize)
{
  const int is64Bit = sizeof(void *) == 8; // Choco does tutti-frutti differently on 32 vs 64-bit systems
  const int heapAlign = is64Bit ? 8 : 4;
  const int headerSize = is64Bit ? 40 : 24;
  const int tag = 7;
  const int zoneId = 0x1d4a11; // Choco ZONEID
  const int tagOffset = is64Bit ? 16 : 8;
  const int idOffset = is64Bit ? 20 : 12;
  int blockSize = ((lumpSize + heapAlign - 1) & ~(heapAlign - 1)) + headerSize;

  memset(header, 0, 40); // clear 64-bit (max) Choco Heap Header size - 40

  header[0] = blockSize & 0xff;
  header[1] = (blockSize >> 8) & 0xff;
  header[2] = (blockSize >> 16) & 0xff;
  header[3] = (blockSize >> 24) & 0xff;

  header[tagOffset]     = tag & 0xff;
  header[tagOffset + 1] = (tag >> 8) & 0xff;
  header[tagOffset + 2] = (tag >> 16) & 0xff;
  header[tagOffset + 3] = (tag >> 24) & 0xff;

  header[idOffset]     = zoneId & 0xff;
  header[idOffset + 1] = (zoneId >> 8) & 0xff;
  header[idOffset + 2] = (zoneId >> 16) & 0xff;
  header[idOffset + 3] = (zoneId >> 24) & 0xff;

  return headerSize;
}

//
//
// Tutti-Frutti
//
//

typedef struct
{
  int pixelDataSize;
  int pixelPadding;
  int pixelOffset;
  unsigned char *pixels;
  unsigned char *padding;
} artifact_pixel_data_t;

typedef struct
{
  int pixelCount;
  int pixelPadding;
  int pixelDataSizeWithArtifacts;
  artifact_pixel_data_t vanilla_composite; // texture with patches
  artifact_pixel_data_t tutti_patch;     // raw patch
} vanilla_data_t;

// Reserve extra texture data for vanilla artifacts.
static void R_CalculateVanillaDataSize(const texture_t *texture, const rpatch_t *texturePatch, const count_t *countsInColumn, int normalPixelDataSize, vanilla_data_t *data)
{
  int i, x;
  dboolean tutti_frutti = dsda_VanillaTextureEmulation();

  *data = (vanilla_data_t){0};

  // get normal patch size
  data->pixelCount = texturePatch->width * texturePatch->height;
  data->pixelDataSizeWithArtifacts = normalPixelDataSize;

  // get size for short textures that vanilla draws as 128 tall
  if (tutti_frutti)
  {
    data->pixelPadding = texturePatch->height * 128;
    if (data->pixelPadding)
      data->pixelDataSizeWithArtifacts = (data->pixelCount + data->pixelPadding + 4) & ~3;
  }

  // get size for composite tutti-frutti textures
  if (tutti_frutti)
  {
    for (x=0; x<texture->width; x++)
      if (countsInColumn[x].patches != 1)
        data->vanilla_composite.pixelDataSize += texturePatch->height;
  }

  // get size for overflowing patch columns
  if (tutti_frutti)
  {
    for (i=0; i<texture->patchcount; i++)
    {
      const texpatch_t *texpatch = &texture->patches[i];
      int patchNum = texpatch->patch;
      const patch_t *oldPatch = (const patch_t*)W_LumpByNum(patchNum);
      const byte *raw_patch = (const byte *)oldPatch;
      int patchSize = W_LumpLength(patchNum);

      for (x=0; x<LittleShort(oldPatch->width); x++)
      {
        const column_t *oldColumn;
        const byte *source;
        int sourceOffset;
        int tx = texpatch->originx + x;

        if (tx < 0)
          continue;
        if (tx >= texturePatch->width)
          break;
        if (countsInColumn[tx].patches != 1)
          continue;

        oldColumn = (const column_t *)(raw_patch + LittleLong(oldPatch->columnofs[x]));
        source = (const byte *)oldColumn + 3;
        sourceOffset = (int)(source - raw_patch);

        if (patchSize - sourceOffset < 128)
          data->tutti_patch.pixelDataSize += 128;
      }
    }
  }

  // get padding for multipatch column overflows
  data->vanilla_composite.pixelPadding = data->vanilla_composite.pixelDataSize ? texturePatch->height * 128 : 0;

  // get final size with vanilla artifacts
  if (data->vanilla_composite.pixelDataSize || data->tutti_patch.pixelDataSize || data->vanilla_composite.pixelPadding)
  {
    data->pixelDataSizeWithArtifacts =
      (data->pixelCount + data->pixelPadding +
       data->vanilla_composite.pixelDataSize + data->vanilla_composite.pixelPadding +
       data->tutti_patch.pixelDataSize + 4) & ~3;
  }
}

//
//
// Tutti-Frutti
//
//

static void R_InitVanillaColumns(rpatch_t *composite_patch, const texture_t *texture, const count_t *countsInColumn, vanilla_data_t *data)
{
  int x;
  dboolean tutti_frutti = dsda_VanillaTextureEmulation();

  data->vanilla_composite.pixelOffset = 0;
  data->tutti_patch.pixelOffset = 0;

  if (tutti_frutti)
  {
    for (x=0; x<texture->width; x++)
    {
      if (countsInColumn[x].patches != 1)
      {
        composite_patch->columns[x].vanilla_pixels = data->vanilla_composite.pixels + data->vanilla_composite.pixelOffset;
        data->vanilla_composite.pixelOffset += composite_patch->height;
      }
    }
  }
}

// add artifacts for texture data read directly from one patch
static void R_AddTuttiFruttiPatchArtifacts(rpatch_t *composite_patch, const texture_t *texture, const count_t *countsInColumn, artifact_pixel_data_t *tuttiFruttiPatch, int patchNum, const patch_t *oldPatch, const column_t *oldColumn, int tx)
{
  const byte *raw_patch;
  const byte *source;
  int patchSize;
  int sourceOffset;
  int sourceAvailable;

  // Vanilla reads data directly from single-patch areas
  if (countsInColumn[tx].patches != 1)
    return;

  raw_patch = (const byte *)oldPatch;
  source = (const byte *)oldColumn + 3;
  patchSize = W_LumpLength(patchNum);
  sourceOffset = (int)(source - raw_patch);

  // count bytes from the first post's pixels to the end of the patch lump
  sourceAvailable = patchSize - sourceOffset;

  // rebuild vanilla's 128-byte read if it crosses the lump boundary
  if (sourceAvailable < 128)
  {
    byte *vanilla = tuttiFruttiPatch->pixels + tuttiFruttiPatch->pixelOffset;
    int copyCount = CLAMP(sourceAvailable, 0, 128);
    int lumpNum;
    int y;
    dboolean wroteZoneHeader = false;

    tuttiFruttiPatch->pixelOffset += 128;
    memcpy(vanilla, source, copyCount);

    y = copyCount;

    // continue the overflow into following lumps
    for (lumpNum = patchNum + 1; y<128 && lumpNum<numlumps; lumpNum++)
    {
      int lumpSize = W_LumpLength(lumpNum);
      int lumpCopy = 128 - y;

      if (!lumpSize)
        continue;

      // include Chocolate Doom's heap header between lumps
      // this shifts artifact offsets for multi-patch textures
      if (texture->patchcount > 1 && !wroteZoneHeader)
      {
        byte vanillaHeapHeader[40]; // 64-bit (max) Choco Heap Header size - 40
        int vanillaHeapHeaderSize = R_FakeChocolateHeapHeader(vanillaHeapHeader, lumpSize);
        int headerCopy = 128 - y;

        if (headerCopy > vanillaHeapHeaderSize)
          headerCopy = vanillaHeapHeaderSize;

        memcpy(vanilla + y, vanillaHeapHeader, headerCopy);
        y += headerCopy;
        wroteZoneHeader = true;

        if (y >= 128)
          break;
      }

      lumpCopy = 128 - y;
      if (lumpCopy > lumpSize)
        lumpCopy = lumpSize;

      memcpy(vanilla + y, W_LumpByNum(lumpNum), lumpCopy);
      y += lumpCopy;
    }

    // fill in the rest of the overflow
    for (; y<128; y++)
      vanilla[y] = 0;

    composite_patch->columns[tx].vanilla_pixels = vanilla;
  }

  // use original data when the overflow stays within the patch lump
  else
  {
    composite_patch->columns[tx].vanilla_pixels = source;
  }
}

// build vanilla's multipatch texture buffer and overflow data
static void R_AddVanillaCompositeArtifacts(rpatch_t *composite_patch, const texture_t *texture, const count_t *countsInColumn, artifact_pixel_data_t *vanilla_composite)
{
  int i, x, y;

  // fill in the overflow with patch metadata
  if (vanilla_composite->pixelPadding)
  {
    int pad_offset = 0;
    int pad_size = vanilla_composite->pixelPadding;
    byte *pad = vanilla_composite->padding;

    // repeat the patch data until the overflow is full
    while (pad_offset < pad_size)
    {
      int copied = 0;

      for (i=0; i<texture->patchcount && pad_offset < pad_size; i++)
      {
        const texpatch_t *texpatch = &texture->patches[i];
        int patchNum = texpatch->patch;
        const patch_t *oldPatch = (const patch_t*)W_LumpByNum(patchNum);
        const byte *raw_patch = (const byte *)oldPatch;
        const int patch_header_size = 8;
        int j;

        for (j=0; j<patch_header_size && pad_offset < pad_size; j++)
        {
          pad[pad_offset] = raw_patch[j];
          pad_offset++;
          copied = 1;
        }

        for (x=0; x<LittleShort(oldPatch->width) && pad_offset < pad_size; x++)
        {
          const byte *columnofs = raw_patch + 8 + x * 4;
          const column_t *oldColumn;

          // copy column offset
          for (j=0; j<4 && pad_offset < pad_size; j++)
          {
            pad[pad_offset] = columnofs[j];
            pad_offset++;
            copied = 1;
          }

          oldColumn = (const column_t *)((const byte *)oldPatch + LittleLong(oldPatch->columnofs[x]));

          for (;;)
          {
            const byte *raw_column = (const byte *)oldColumn;

            if (oldColumn->topdelta == 0xff)
            {
              if (pad_offset < pad_size)
                pad[pad_offset++] = raw_column[0];
              copied = 1;
              break;
            }

            if (pad_offset < pad_size)
              pad[pad_offset++] = raw_column[0];
            if (pad_offset < pad_size)
              pad[pad_offset++] = raw_column[1];
            if (pad_offset < pad_size)
              pad[pad_offset++] = raw_column[2];
            if (pad_offset < pad_size)
              pad[pad_offset++] = raw_column[oldColumn->length + 3];
            copied = 1;

            oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
          }
        }
      }

      if (!copied)
        break;
    }
  }

  // fill in the multipatch pixels
  vanilla_composite->pixelOffset = 0;

  for (x=0; x<texture->width; x++)
  {
    byte *vanilla_pixels;

    if (countsInColumn[x].patches == 1)
      continue;

    vanilla_pixels = vanilla_composite->pixels + vanilla_composite->pixelOffset;
    vanilla_composite->pixelOffset += composite_patch->height;

    // replace transparent gaps with color 0
    for (y=0; y<composite_patch->height; y++)
    {
      byte color = composite_patch->columns[x].pixels[y];
      vanilla_pixels[y] = color == playpal_transparent ? 0 : color;
    }
  }
}

// Fill extra texture data used by tutti-frutti
static void R_FillTuttiFruttiOverflow(rpatch_t *composite_patch, const vanilla_data_t *data)
{
  // Repeat texture pixels for tutti-frutti overflows
  for (int i = 0; i < data->pixelPadding; ++i)
    composite_patch->pixels[data->pixelCount + i] = data->pixelCount ? composite_patch->pixels[i % data->pixelCount] : 0;
}

//
//
// Medusa
//
//

#define MEDUSA_MAX_PIXELS     512
#define MEDUSA_MAX_POSTS      32
#define MEDUSA_TEXTURE_BYTES  8
#define MEDUSA_SAMPLE_SKIP    17

typedef struct
{
  rcolumn_t column;
  rpost_t post;
  byte pixels[MEDUSA_MAX_PIXELS];
} medusa_column_t;

static void R_AddMedusaPatchPixels(byte *pixels, int *offset, int patch_num)
{
  const rpatch_t *source_patch = R_PatchByNum(patch_num);
  int source_count = source_patch->width * source_patch->height;
  int source_offset = source_count ? patch_num * MEDUSA_SAMPLE_SKIP % source_count : 0;
  int y;

  for (y=0; (y < MEDUSA_TEXTURE_BYTES) && (*offset < MEDUSA_MAX_PIXELS) && (source_count); y++)
  {
    int sample = (source_offset + y * MEDUSA_SAMPLE_SKIP) % source_count;
    int searched = 0;
    byte color = source_patch->pixels[sample];

    while (color == playpal_transparent && ++searched < source_count)
    {
      sample = (sample + 1) % source_count;
      color = source_patch->pixels[sample];
    }

    if (color != playpal_transparent)
      pixels[(*offset)++] = color;
  }
}

static int R_MedusaStartTexture(void)
{
  int start = numsides ? (numsides * 37 + 17) % numsides : 0;
  int i;

  for (i=0; i<numsides; i++)
  {
    const side_t *side = &sides[(start + i) % numsides];
    int texture_nums[3] = { side->toptexture, side->midtexture, side->bottomtexture };
    int j;

    for (j=0; j<3; j++)
    {
      int texture_num = texture_nums[(start + i + j) % 3];

      if (texture_num > 0 && texture_num < numtextures)
        return texture_num;
    }
  }

  return 1;
}

static void R_FillMedusaPixels(byte *pixels, const rpatch_t *composite_patch)
{
  int sampled_patches[MEDUSA_MAX_PIXELS / MEDUSA_TEXTURE_BYTES];
  int sampled_count = 0;
  int start_texture = R_MedusaStartTexture();
  int composite_count = composite_patch->width * composite_patch->height;
  int offset = 0;
  int texture_offset;
  int y;

  // Start with pixels from the affected composite texture
  for (y=0; y<MEDUSA_MAX_POSTS && composite_count; y++)
  {
    int sample = y * MEDUSA_SAMPLE_SKIP % composite_count;
    byte color = composite_patch->pixels[sample];

    if (color != playpal_transparent)
      pixels[offset++] = color;
  }

  // Continue from a texture used by the map into later WAD patches
  for (texture_offset = 0; (texture_offset < numtextures - 1) && (offset < MEDUSA_MAX_PIXELS); texture_offset++)
  {
    int texture_num = 1 + (start_texture - 1 + texture_offset) % (numtextures - 1);
    const texture_t *texture;
    int texture_patch;

    texture = textures[texture_num];

    for (texture_patch = 0; (texture_patch < texture->patchcount) && (offset < MEDUSA_MAX_PIXELS); texture_patch++)
    {
      int patch_num = texture->patches[texture_patch].patch;
      int i;

      for (i=0; i<sampled_count && sampled_patches[i] != patch_num; i++)
        ;
      if (i<sampled_count || sampled_count == arrlen(sampled_patches))
        continue;

      sampled_patches[sampled_count++] = patch_num;
      R_AddMedusaPatchPixels(pixels, &offset, patch_num);
    }
  }

  while (offset < MEDUSA_MAX_PIXELS)
  {
    pixels[offset] = offset ? pixels[offset % MEDUSA_TEXTURE_BYTES] : 0;
    offset++;
  }
}

static dboolean R_BuildMedusaColumn(const rpatch_t *patch, const rcolumn_t *source_column, medusa_column_t *medusa)
{
  if (source_column->patch_count <= 1 || patch->width <= 0 || patch->height <= 0)
    return false;

  memset(medusa, 0, sizeof(*medusa));

  medusa->column.posts = &medusa->post;
  medusa->column.pixels = medusa->pixels;
  medusa->column.vanilla_pixels = NULL;
  medusa->column.patch_count = source_column->patch_count;

  R_FillMedusaPixels(medusa->pixels, patch);

  medusa->post.topdelta = 0;
  medusa->post.length = MEDUSA_MAX_PIXELS;
  medusa->post.slope = 0;
  medusa->column.numPosts = 1;

  return true;
}

dboolean R_SetMedusaColumn(const rpatch_t *patch, const rcolumn_t **column, const rcolumn_t **prevcolumn, const rcolumn_t **nextcolumn)
{
  static medusa_column_t medusa;
  static const byte *medusa_source;

  if (!dsda_VanillaTextureEmulation())
    return false;

  if ((*column)->patch_count <= 1)
    return false;

  if (medusa_source != patch->pixels)
  {
    if (!R_BuildMedusaColumn(patch, *column, &medusa))
      return false;

    medusa_source = patch->pixels;
  }

  *column = &medusa.column;
  *prevcolumn = &medusa.column;
  *nextcolumn = &medusa.column;

  return true;
}
