// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"

namespace psd {

const char* color_mode_string(const ColorMode colorMode)
{
  switch (colorMode) {
    case ColorMode::Bitmap: return "Bitmap";
    case ColorMode::Grayscale: return "Grayscale";
    case ColorMode::Indexed: return "Indexed";
    case ColorMode::RGB: return "RGB";
    case ColorMode::CMYK: return "CMYK";
    case ColorMode::Multichannel: return "Multichannel";
    case ColorMode::Duotone: return "Duotone";
    case ColorMode::Lab: return "Lab";
  }
  return "Unknown";
}

bool decode_psd(FileInterface* file,
                DecoderDelegate* delegate)
{
  Decoder decoder(file, delegate);

  try {
    decoder.readFileHeader();
    decoder.readColorModeData();
    decoder.readImageResources();
    decoder.readLayersAndMask();
    decoder.readImageData();
  } catch (const std::exception& ) {
    return false;
  }
  return true;
}

} // namespace psd
