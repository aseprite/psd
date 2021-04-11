// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef PSD_DETAILS_H_INCLUDED
#define PSD_DETAILS_H_INCLUDED
#pragma once

#define PSD_FILE_MAGIC_NUMBER        (('8' << 24) | ('B' << 16) | ('P' << 8) | 'S')
#define PSD_IMAGE_BLOCK_MAGIC_NUMBER (('8' << 24) | ('B' << 16) | ('I' << 8) | 'M')
#define PSD_LAYER_INFO_MAGIC_NUMBER  (('8' << 24) | ('B' << 16) | ('I' << 8) | 'M')
#define PSD_LAYER_INFO_MAGIC_NUMBER2 (('8' << 24) | ('B' << 16) | ('6' << 8) | '4')
#define PSD_BLEND_MODE_MAGIC_NUMBER  (('8' << 24) | ('B' << 16) | ('I' << 8) | 'M')

#endif
