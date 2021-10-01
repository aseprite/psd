// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"

#include <cassert>
#include <cstdio>

class Delegate : public psd::DecoderDelegate {
public:
  psd::LayersInformation layers;

  void onFileHeader(const psd::PSDHeader& fileHeader) override {
    std::printf(
      "File Header\n"
      "  version=%d\n"
      "  nchannels=%d\n"
      "  width=%d\n"
      "  height=%d\n"
      "  depth=%d (bits per channel)\n"
      "  colorMode=%d (%s)\n",
      fileHeader.version,
      fileHeader.nchannels,
      fileHeader.width,
      fileHeader.height,
      fileHeader.depth,
      fileHeader.colorMode,
      psd::color_mode_string(fileHeader.colorMode));
  }

  void onLayersInfo(const psd::LayersInformation& layers) override {
    this->layers = layers;
  }

  void onImageResource(const psd::ImageResource& res) override {
    bool limit = false;

    std::printf(
      "Image Resource ID=%04x (%s) Name='%s' Length=%d\n",
      res.resourceID, psd::ImageResource::resIDString(res.resourceID),
      res.name.c_str(), int(res.data.size()));

    size_t i=0;
    for (; i<int(res.data.size())
           && (!limit || i<16*8)
           ; ) {
      std::printf("  ");
      size_t j = i;
      size_t k = 0;
      for (k=0; k<16 && j<int(res.data.size()); ++j, ++k) {
        std::printf("%c", std::isgraph(res.data[j]) ? res.data[j]: '.');
      }
      for (; k<16; ++k)
        std::printf(" ");
      std::printf(" ");
      j = i;
      for (k=0; k<16 && j<int(res.data.size()); ++j, ++k) {
        std::printf("%02x ", res.data[j]);
      }
      i = j;
      std::printf("\n");
    }
    if (i<int(res.data.size()))
      std::printf("  ...\n");
  }

  void onBeginImage(const psd::ImageData& img) override {
    std::printf("  \n");
    std::printf(
      "  Begin Image\n"
      "    nchannels=%zu\n"
      "    width=%d\n"
      "    height=%d\n"
      "    depth=%d (bits per channel)\n"
      "    compression=%d\n",
      img.channels.size(),
      img.width,
      img.height,
      img.depth,
      (int)img.compressionMethod);
  }

  void onImageScanline(const psd::ImageData& img,
                       const int y,
                       const psd::ChannelID chanID,
                       const uint8_t* data,
                       const int bytes) override {
    if (y == 0)
      std::printf("  -- Channel %d --\n", chanID);

    std::printf("    ");
    for (int x=0; x<bytes; ++x) {
      std::printf("%02x ", data[x]);
    }
    std::printf("\n");
  }

  void onEndImage(const psd::ImageData& img) override {
    std::printf("  End Image\n");
  }

};

int main(int argc, char** argv){
  if (argc < 2) {
    std::printf("Usage: %s file.psd\n", argv[0]);
    return 1;
  }

  FILE* f = std::fopen(argv[1], "rb");
  if (!f) {
    std::printf("File not found '%s'\n", argv[1]);
    return 1;
  }

  psd::StdioFileInterface fileInterface(f);
  Delegate delegate;
  if (!psd::decode_psd(&fileInterface, &delegate))
    return 1;

  for (auto& layer : delegate.layers.layers)
    std::printf("Layer name='%s' opacity=%d blendmode=%c%c%c%c\n",
      layer.name.c_str(),
      layer.opacity,
      (int(layer.blendMode) >> 24) & 255,
      (int(layer.blendMode) >> 16) & 255,
      (int(layer.blendMode) >> 8) & 255,
      int(layer.blendMode) & 255);

  std::fclose(f);
}
