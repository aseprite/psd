// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef PSD_PSD_H_INCLUDED
#define PSD_PSD_H_INCLUDED
#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace psd {

#define PSD_DEFINE_DWORD(a, b, c, d) \
  (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

  enum class Version {
    Psd = 1,
    Psb = 2,
  };

  enum class ColorMode {
    Bitmap = 0,
    Grayscale = 1,
    Indexed = 2,
    RGB = 3,
    CMYK = 4,
    Multichannel = 7,
    Duotone = 8,
    Lab = 9,
  };

  const char* color_mode_string(const ColorMode colorMode);

  struct PSDHeader {
    Version version;
    int nchannels;
    int width;
    int height;
    int depth;
    ColorMode colorMode;
  };

  struct IndexColor {
    uint8_t r, g, b;
    IndexColor() : r(0), g(0), b(0) { }
    IndexColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
  };

  struct ColorModeData {
    uint32_t length;

    // Only valid for IndexedColorMode
    std::vector<IndexColor> colors;

    // Raw data to restore in DuotoneColorMode
    std::vector<uint8_t> data;
  };

  struct ImageResource {
    uint16_t resourceID;
    std::string name;
    std::vector<uint8_t> data;

    static const char* resIDString(uint16_t resID);
    static bool resIDHasDescriptor(uint16_t resID);
  };

  struct ImageResources {
    std::vector<ImageResource> resources;
  };

  enum class LayerBlendMode : uint32_t {
    PassThrough = PSD_DEFINE_DWORD('p', 'a', 's', 's'),
    Normal = PSD_DEFINE_DWORD('n', 'o', 'r', 'm'),
    Dissolve = PSD_DEFINE_DWORD('d', 'i', 's', 's'),
    Darken = PSD_DEFINE_DWORD('d', 'a', 'r', 'k'),
    Multiply = PSD_DEFINE_DWORD('m', 'u', 'l', ' '),
    ColorBurn = PSD_DEFINE_DWORD('i', 'd', 'i', 'v'),
    LinearBurn = PSD_DEFINE_DWORD('l', 'b', 'r', 'n'),
    DarkerColor = PSD_DEFINE_DWORD('d', 'k', 'C', 'l'),
    Lighten = PSD_DEFINE_DWORD('l', 'i', 't', 'e'),
    Screen = PSD_DEFINE_DWORD('s', 'c', 'r', 'n'),
    ColorDodge = PSD_DEFINE_DWORD('d', 'i', 'v', ' '),
    LinearDodge = PSD_DEFINE_DWORD('l', 'd', 'd', 'g'),
    LighterColor = PSD_DEFINE_DWORD('l', 'g', 'C', 'l'),
    Overlay = PSD_DEFINE_DWORD('o', 'v', 'e', 'r'),
    SoftLight = PSD_DEFINE_DWORD('s', 'L', 'i', 't'),
    HardLight = PSD_DEFINE_DWORD('h', 'L', 'i', 't'),
    VividLight = PSD_DEFINE_DWORD('v', 'L', 'i', 't'),
    LinearLight = PSD_DEFINE_DWORD('l', 'L', 'i', 't'),
    PinLight = PSD_DEFINE_DWORD('p', 'L', 'i', 't'),
    HardMix = PSD_DEFINE_DWORD('h', 'M', 'i', 'x'),
    Difference = PSD_DEFINE_DWORD('d', 'i', 'f', 'f'),
    Exclusion = PSD_DEFINE_DWORD('s', 'm', 'u', 'd'),
    Subtract = PSD_DEFINE_DWORD('f', 's', 'u', 'b'),
    Divide = PSD_DEFINE_DWORD('f', 'd', 'i', 'v'),
    Hue = PSD_DEFINE_DWORD('h', 'u', 'e', ' '),
    Saturation = PSD_DEFINE_DWORD('s', 'a', 't', ' '),
    Color = PSD_DEFINE_DWORD('c', 'o', 'l', 'r'),
    Luminosity = PSD_DEFINE_DWORD('l', 'u', 'm', ' '),
  };

  enum class LayerInfoKey : uint32_t {
    Alph = PSD_DEFINE_DWORD('A', 'l', 'p', 'h'),
    Anno = PSD_DEFINE_DWORD('A', 'n', 'n', 'o'),
    CgEd = PSD_DEFINE_DWORD('C', 'g', 'E', 'd'),
    FEid = PSD_DEFINE_DWORD('F', 'E', 'i', 'd'),
    FMsk = PSD_DEFINE_DWORD('F', 'M', 's', 'k'),
    FXid = PSD_DEFINE_DWORD('F', 'X', 'i', 'd'),
    GdFl = PSD_DEFINE_DWORD('G', 'd', 'F', 'l'),
    LMsk = PSD_DEFINE_DWORD('L', 'M', 's', 'k'),
    Layr = PSD_DEFINE_DWORD('L', 'a', 'y', 'r'),
    Lr16 = PSD_DEFINE_DWORD('L', 'r', '1', '6'),
    Lr32 = PSD_DEFINE_DWORD('L', 'r', '3', '2'),
    Mt16 = PSD_DEFINE_DWORD('M', 't', '1', '6'),
    Mt32 = PSD_DEFINE_DWORD('M', 't', '3', '2'),
    Mtrn = PSD_DEFINE_DWORD('M', 't', 'r', 'n'),
    Pat2 = PSD_DEFINE_DWORD('P', 'a', 't', '2'),
    Pat3 = PSD_DEFINE_DWORD('P', 'a', 't', '3'),
    Patt = PSD_DEFINE_DWORD('P', 'a', 't', 't'),
    PtFl = PSD_DEFINE_DWORD('P', 't', 'F', 'l'),
    PxSD = PSD_DEFINE_DWORD('P', 'x', 'S', 'D'),
    PxSc = PSD_DEFINE_DWORD('P', 'x', 'S', 'c'),
    SoCo = PSD_DEFINE_DWORD('S', 'o', 'C', 'o'),
    SoLE = PSD_DEFINE_DWORD('S', 'o', 'L', 'E'),
    SoLd = PSD_DEFINE_DWORD('S', 'o', 'L', 'd'),
    Txt2 = PSD_DEFINE_DWORD('T', 'x', 't', '2'),
    TySh = PSD_DEFINE_DWORD('T', 'y', 'S', 'h'),
    abdd = PSD_DEFINE_DWORD('a', 'b', 'd', 'd'),
    anFX = PSD_DEFINE_DWORD('a', 'n', 'F', 'X'),
    artb = PSD_DEFINE_DWORD('a', 'r', 't', 'b'),
    artd = PSD_DEFINE_DWORD('a', 'r', 't', 'd'),
    blnc = PSD_DEFINE_DWORD('b', 'l', 'n', 'c'),
    blwh = PSD_DEFINE_DWORD('b', 'l', 'w', 'h'),
    brit = PSD_DEFINE_DWORD('b', 'r', 'i', 't'),
    brst = PSD_DEFINE_DWORD('b', 'r', 's', 't'),
    cinf = PSD_DEFINE_DWORD('c', 'i', 'n', 'f'),
    clbl = PSD_DEFINE_DWORD('c', 'l', 'b', 'l'),
    clrL = PSD_DEFINE_DWORD('c', 'l', 'r', 'L'),
    curv = PSD_DEFINE_DWORD('c', 'u', 'r', 'v'),
    expA = PSD_DEFINE_DWORD('e', 'x', 'p', 'A'),
    ffxi = PSD_DEFINE_DWORD('f', 'f', 'x', 'i'),
    fxrp = PSD_DEFINE_DWORD('f', 'x', 'r', 'p'),
    grdm = PSD_DEFINE_DWORD('g', 'r', 'd', 'm'),
    hue  = PSD_DEFINE_DWORD('h', 'u', 'e', ' '),
    hue2 = PSD_DEFINE_DWORD('h', 'u', 'e', '2'),
    infx = PSD_DEFINE_DWORD('i', 'n', 'f', 'x'),
    knko = PSD_DEFINE_DWORD('k', 'n', 'k', 'o'),
    lclr = PSD_DEFINE_DWORD('l', 'c', 'l', 'r'),
    levl = PSD_DEFINE_DWORD('l', 'e', 'v', 'l'),
    lfx2 = PSD_DEFINE_DWORD('l', 'f', 'x', '2'),
    lmgm = PSD_DEFINE_DWORD('l', 'm', 'g', 'm'),
    lnk2 = PSD_DEFINE_DWORD('l', 'n', 'k', '2'),
    lnk3 = PSD_DEFINE_DWORD('l', 'n', 'k', '3'),
    lnkD = PSD_DEFINE_DWORD('l', 'n', 'k', 'D'),
    lnsr = PSD_DEFINE_DWORD('l', 'n', 's', 'r'),
    lrFX = PSD_DEFINE_DWORD('l', 'r', 'F', 'X'),
    lsct = PSD_DEFINE_DWORD('l', 's', 'c', 't'),
    lspf = PSD_DEFINE_DWORD('l', 's', 'p', 'f'),
    luni = PSD_DEFINE_DWORD('l', 'u', 'n', 'i'),
    lyid = PSD_DEFINE_DWORD('l', 'y', 'i', 'd'),
    lyvr = PSD_DEFINE_DWORD('l', 'y', 'v', 'r'),
    mixr = PSD_DEFINE_DWORD('m', 'i', 'x', 'r'),
    nvrt = PSD_DEFINE_DWORD('n', 'v', 'r', 't'),
    phfl = PSD_DEFINE_DWORD('p', 'h', 'f', 'l'),
    plLd = PSD_DEFINE_DWORD('p', 'l', 'L', 'd'),
    post = PSD_DEFINE_DWORD('p', 'o', 's', 't'),
    pths = PSD_DEFINE_DWORD('p', 't', 'h', 's'),
    selc = PSD_DEFINE_DWORD('s', 'e', 'l', 'c'),
    shmd = PSD_DEFINE_DWORD('s', 'h', 'm', 'd'),
    shpa = PSD_DEFINE_DWORD('s', 'h', 'p', 'a'),
    sn2P = PSD_DEFINE_DWORD('s', 'n', '2', 'P'),
    thrs = PSD_DEFINE_DWORD('t', 'h', 'r', 's'),
    tsly = PSD_DEFINE_DWORD('t', 's', 'l', 'y'),
    tySh = PSD_DEFINE_DWORD('t', 'y', 'S', 'h'),
    vibA = PSD_DEFINE_DWORD('v', 'i', 'b', 'A'),
    vmgm = PSD_DEFINE_DWORD('v', 'm', 'g', 'm'),
    vmsk = PSD_DEFINE_DWORD('v', 'm', 's', 'k'),
    vogk = PSD_DEFINE_DWORD('v', 'o', 'g', 'k'),
    vscg = PSD_DEFINE_DWORD('v', 's', 'c', 'g'),
    vsms = PSD_DEFINE_DWORD('v', 's', 'm', 's'),
    vstk = PSD_DEFINE_DWORD('v', 's', 't', 'k'),
  };

  enum class ChannelID : int {
    Red = 0,
    Green = 1,
    Blue = 2,
    Alpha = 3,
    TransparencyMask = -1,
    UserSuppliedMask = -2,
    RealUserSuppliedMask = -3,
  };

  struct Channel {
    ChannelID channelID;
    uint64_t length;
  };

  struct LayerRecord {
    int32_t top, left, bottom, right;
    std::vector<Channel> channels;
    LayerBlendMode blendMode;
    uint8_t opacity;
    uint8_t clipping;
    uint8_t flags;
    std::string name;

    bool isTransparencyProtected() const { return flags & 1; }
    bool isVisible() const { return flags & 2; }
    int width() const { return bottom - top; }
    int height() const { return right - left; }
  };

  struct LayersInformation {
    std::vector<LayerRecord> layers;
    std::vector<uint8_t> globalMaskData;
  };

  enum class CompressionMethod {
    RawImageData = 0,
    RLE = 1,
    ZIPWithoutPrediction = 2,
    ZIPWithPrediction = 3,
  };

  struct ImageData {
    CompressionMethod compressionMethod;
    int width;
    int height;
    int depth;
    std::vector<ChannelID> channels;
  };

  class FileInterface {
  public:
    virtual ~FileInterface() { }

    // Returns true if we can read/write bytes from/into the file
    virtual bool ok() const = 0;

    // Current position in the file
    virtual size_t tell() = 0;

    // Jump to the given position in the file
    virtual void seek(size_t absPos) = 0;

    // Returns the next byte in the file or 0 if ok() = false
    virtual uint8_t read8() = 0;
    virtual bool read(uint8_t* buf, uint32_t size) = 0;

    // Writes one byte in the file (or do nothing if ok() = false)
    virtual void write8(uint8_t value) = 0;
    virtual bool write(const uint8_t* buf, uint32_t size) = 0;
  };

  class StdioFileInterface : public psd::FileInterface {
  public:
    StdioFileInterface(FILE* file);
    bool ok() const override;
    size_t tell() override;
    void seek(size_t absPos) override;
    uint8_t read8() override;
    bool read(uint8_t* buf, uint32_t size) override;
    void write8(uint8_t value) override;
    bool write(const uint8_t* buf, uint32_t size) override;

  private:
    FILE* m_file;
    bool m_ok;
  };

  class DecoderDelegate {
  public:
    virtual ~DecoderDelegate() { }
    virtual void onFileHeader(const PSDHeader& fileHeader) { }
    virtual void onColorModeData(const ColorModeData& data) { }
    virtual void onImageResources(const ImageResources& res) { }
    virtual void onImageResource(const ImageResource& res) { }
    virtual void onLayersAndMask(const LayersInformation& layers) { }
    virtual void onLayersInfo(const LayersInformation& layers) { }
    virtual void onImageData(const ImageData& imageData) { }

    // Function to read image data (from layers or from the whole
    // document).
    virtual void onBeginImage(const ImageData& img) { }
    virtual void onImageScanline(const ImageData& img,
                                 const int y,
                                 const ChannelID chanID,
                                 const uint8_t* data,
                                 const int bytes) { }
    virtual void onEndImage(const ImageData& img) { }
  };

  class Decoder {
  public:
    Decoder(FileInterface* file,
            DecoderDelegate* delegate);

    const PSDHeader& fileHeader() const { return m_header; }

    bool readFileHeader();
    bool readColorModeData();
    bool readImageResources();
    bool readLayersAndMask();
    bool readImageData();

  private:
    bool readLayersInfo(LayersInformation& layers);
    bool readLayerRecord(LayersInformation& layers,
                         LayerRecord& layerRecord);
    bool readGlobalMaskInfo(LayersInformation& layers);
    bool readImage(const ImageData& img);

    uint8_t read8() { return m_file->read8(); }
    uint16_t read16();
    uint32_t read32();
    uint64_t read64();
    uint32_t read16or32Length();
    uint64_t read32or64Length();
    std::string readPascalString(const int alignment);

    DecoderDelegate* m_delegate;
    FileInterface* m_file;
    PSDHeader m_header;
  };

  class EncoderDelegate {
  public:
    virtual void onHeaderWritten(const PSDHeader& header){}
    virtual void onColorModeWritten(const ColorModeData& colorModeData){}
    virtual void onImageResourcesWritten(const ImageResources& imageResources){}
  };

  class Encoder {
  public:
    Encoder(FileInterface* file,
            EncoderDelegate* delegate = nullptr);
    bool writePSDHeader(const PSDHeader& header);
    bool writeColorModeData(const ColorModeData& colorModeData);
    bool writeImageResources(const ImageResources& imageResources);

  private:
    void write8(const uint8_t value);
    void write16(const uint16_t value);
    void write32(const uint32_t value);
    void write64(const uint64_t value );
    void write32or64Length(const uint64_t value);
    void writeRawData(const uint8_t* const data, 
                        std::size_t const length);
    void writePascalString(const std::string& str, const int alignment);

    std::size_t imageResourceBlockSize(const ImageResource&, 
        const int stringAlignment);
    std::size_t pascalStringSize(const std::string& str, 
        const int stringAlignment);

    FileInterface* m_file;
    EncoderDelegate* m_delegate;
    PSDHeader m_header;
  };

  bool decode_psd(FileInterface* file, DecoderDelegate* delegate);

} // namespace psd

#endif
