// Aseprite PSD Library
// Copyright (C) 2019-2021 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef PSD_PSD_H_INCLUDED
#define PSD_PSD_H_INCLUDED
#pragma once

#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <stdexcept>
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

  struct FileHeader {
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

  struct OSTypeDescriptor;
  struct ImageResource {
    uint16_t resourceID;
    std::string name;
    std::vector<uint8_t> data;
    std::unique_ptr<OSTypeDescriptor> descriptor;

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
    cust = PSD_DEFINE_DWORD('c', 'u', 's', 't'),
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
    mlst = PSD_DEFINE_DWORD('m', 'l', 's', 't'),
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
    tmln = PSD_DEFINE_DWORD('t', 'm', 'l', 'n'),
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

  enum class OSTypeKey : uint32_t {
    Reference = PSD_DEFINE_DWORD('o', 'b', 'j', ' '),
    Descriptor = PSD_DEFINE_DWORD('O', 'b', 'j', 'c'),
    List = PSD_DEFINE_DWORD('V', 'l', 'L', 's'),
    Double = PSD_DEFINE_DWORD('d', 'o', 'u', 'b'),
    UnitFloat = PSD_DEFINE_DWORD('U', 'n', 't', 'F'),
    String = PSD_DEFINE_DWORD('T', 'E', 'X', 'T'),
    Enumerated = PSD_DEFINE_DWORD('e', 'n', 'u', 'm'),
    Long = PSD_DEFINE_DWORD('l', 'o', 'n', 'g'),
    LargeInteger = PSD_DEFINE_DWORD('c', 'o', 'm', 'p'),
    Boolean = PSD_DEFINE_DWORD('b', 'o', 'o', 'l'),
    GlobalObject = PSD_DEFINE_DWORD('G', 'l', 'b', 'O'),
    ClassType = PSD_DEFINE_DWORD('t', 'y', 'p', 'e'),
    GlobalClass = PSD_DEFINE_DWORD('G', 'l', 'b', 'C'),
    Alias = PSD_DEFINE_DWORD('a', 'l', 'i', 's'),
    RawData = PSD_DEFINE_DWORD('t', 'd', 't', 'a'),

    RefProperty = PSD_DEFINE_DWORD('p', 'r', 'o', 'p'),
    RefClass = PSD_DEFINE_DWORD('C', 'l', 's', 's'),
    RefEnum = PSD_DEFINE_DWORD('E', 'n', 'm', 'r'),
    RefOffset = PSD_DEFINE_DWORD('r', 'e', 'l', 'e'),
    RefIdentifier = PSD_DEFINE_DWORD('I', 'd', 'n', 't'),
    RefIndex = PSD_DEFINE_DWORD('i', 'd', 'n', 'x'),
    RefName = PSD_DEFINE_DWORD('n', 'a', 'm', 'e'),
  };

  enum class ImageResourceSection : uint32_t {
    ANDS = PSD_DEFINE_DWORD('A', 'n', 'D', 's'),
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

  enum class SectionType: uint32_t {
    Others,
    OpenFolder,
    CloseFolder,
    BoundingSection,
  };

  struct Channel {
    ChannelID channelID;
    uint64_t length;
  };

  struct OSType {
    virtual ~OSType() { }
    virtual OSTypeKey type() const = 0;
    virtual double numberValue() const { return 0.0; }

    template<typename T>
    const T* as() const {
      if (type() == T::kType)
        return static_cast<const T*>(this);
      else
        throw std::runtime_error("Invalid cast");
    }
  };

  struct OSTypeClassMetaType {
    std::string name;
  };

  struct OSTypeUnitFloat : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::UnitFloat;
    enum class Unit {
      Angle = PSD_DEFINE_DWORD('#', 'A', 'n', 'g'), // base degrees
      Density = PSD_DEFINE_DWORD('#', 'R', 's', 'l'), // base per inch
      Distance = PSD_DEFINE_DWORD('#', 'R', 'l', 't'), // base 72ppi
      None = PSD_DEFINE_DWORD('#', 'N', 'n', 'e'), // coerced
      Percent = PSD_DEFINE_DWORD('#', 'P', 'r', 'c'), // unit value
      Pixel = PSD_DEFINE_DWORD('#', 'P', 'x', 'l'), // tagged unit value
    };

    Unit unit;
    double value = 0.0;

    OSTypeUnitFloat(Unit u, double v)
      : unit(u)
      , value(v)
    { }
    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeDouble : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Double;
    double value = 0.0;

    OSTypeDouble(double v): value(v) { }
    OSTypeKey type() const override { return kType; }
    double numberValue() const override { return value; }
  };

  struct OSTypeClass : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::ClassType;
    std::wstring className;
    OSTypeClassMetaType meta;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeString : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::String;
    std::wstring value;

    OSTypeString(const std::wstring& v): value(v) { }
    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeEnumeratedRef : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::RefEnum;
    std::wstring refClassID;
    OSTypeClassMetaType classID;
    OSTypeClassMetaType typeID;
    OSTypeClassMetaType enumValue;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeOffset : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::RefOffset;
    std::wstring offsetName;
    OSTypeClassMetaType classID;
    uint32_t value = 0;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeBoolean : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Boolean;
    bool value = false;

    OSTypeBoolean(bool v): value(v) { }
    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeAlias : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Alias;
    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeList : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::List;
    std::vector<std::unique_ptr<OSType>> values;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeLargeInt : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::LargeInteger;
    std::uint64_t value = 0;

    OSTypeLargeInt(uint64_t v): value(v) { }
    OSTypeKey type() const override { return kType; }
    double numberValue() const override { return value; }
  };

  struct OSTypeInt : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Long;
    std::uint32_t value = 0;

    OSTypeInt(uint32_t v): value(v) { }
    OSTypeKey type() const override { return kType; }
    double numberValue() const override { return value; }
  };

  struct OSTypeRawData : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::RawData;
    std::vector<uint8_t> value;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeProperty : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::RefProperty;
    std::wstring propName;
    OSTypeClassMetaType classID;
    OSTypeClassMetaType keyID;

    OSTypeKey type() const override { return kType; }
  };

  class DescriptorMap {
  public:
    using Map = std::map<std::string, std::unique_ptr<OSType>>;

    template<typename T>
    const T* getValue(const std::string& key) const {
      const auto ptr = find(key);
      if (!ptr || ptr->type() != T::kType)
        return nullptr;
      return static_cast<const T*>(ptr);
    }

    const OSType* find(const std::string& key) const {
      const auto iter = m_items.find(key);
      if (iter == m_items.cend())
        return nullptr;
      return iter->second.get();
    }

    Map::size_type size() const { return m_items.size(); }
    Map& items() { return m_items; }
    const Map& items() const { return m_items; }

  private:
    Map m_items;
  };

  struct OSTypeDescriptor : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Descriptor;
    std::wstring descriptorName;
    OSTypeClassMetaType classId;
    DescriptorMap descriptor;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeEnum : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Enumerated;
    OSTypeClassMetaType typeID;
    OSTypeClassMetaType enumValue;

    OSTypeKey type() const override { return kType; }
  };

  struct OSTypeReference : public OSType {
    static constexpr OSTypeKey kType = OSTypeKey::Reference;
    std::vector<std::unique_ptr<OSType>> refs;

    OSTypeKey type() const override { return kType; }
  };

  struct Bound {
    uint32_t top = 0;
    uint32_t left = 0;
    uint32_t bottom = 0;
    uint32_t right = 0;
  };

  struct Slice {
    uint32_t sliceID = 0;
    uint32_t groupID = 0;
    uint32_t origin = 0;
    uint32_t assocLayerID = 0; // only available if origin is 1
    uint32_t type = 0;
    uint32_t horizontalAlignment = 0;
    uint32_t verticalAlignment = 0;
    Bound    bound;
    uint8_t  alpha = 0;
    uint8_t  red = 0;
    uint8_t  green = 0;
    uint8_t  blue = 0;
    bool     celTextIsHTML = false;
    std::wstring name;
    std::wstring url;
    std::wstring target;
    std::wstring message;
    std::wstring altTag;
    std::wstring celText;
  };

  struct Slices {
    Bound bound;
    std::wstring groupName;
    std::vector<Slice> slices;
    // PS 7.0 added a descriptor at the end of the block.
    // If available, user may use `Decoder::getSlices()` to
    // decode this data and get a `SliceData` object from it
    std::shared_ptr<OSTypeDescriptor> desc;
  };

  struct LayerRecord {
    // structure to hold the visibility of layer in each
    // frame in an animation, if any.
    struct FrameVisibility {
      uint32_t frameID;
      bool     isVisibleInFrame;
    };

    int32_t top, left, bottom, right;
    uint32_t layerID;
    std::vector<Channel> channels;
    std::vector<FrameVisibility> inFrames;
    LayerBlendMode blendMode;
    SectionType sectionType;
    uint8_t opacity;
    uint8_t clipping;
    uint8_t flags;
    std::string name;

    bool isTransparencyProtected() const { return flags & 1; }
    bool isVisible() const { return (flags & 2) == 0; }
    bool isOpenGroup() const { return sectionType == SectionType::BoundingSection; }
    bool isCloseGroup() const { return sectionType == SectionType::OpenFolder; }
    int width() const { return right - left; }
    int height() const { return bottom - top; }
  };

  struct GlobalMaskInfo {
    enum class MaskKind: uint8_t {
      Inverted = 0,
      ColorProtected = 1,
      ExactPixelValue = 128,
    };

    uint16_t opacity;
    MaskKind kind;
  };

  struct LayersInformation {
    std::vector<LayerRecord> layers;
    GlobalMaskInfo maskInfo;
  };

  struct FrameInformation {
    uint32_t id = 0;
    uint32_t duration = 0;
    double ga = 0.0;
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
    virtual void onFileHeader(const FileHeader& fileHeader) { }
    virtual void onColorModeData(const ColorModeData& data) { }
    virtual void onImageResources(const ImageResources& res) { }
    virtual void onImageResource(const ImageResource& res) { }
    virtual void onLayersAndMask(const LayersInformation& layers) { }
    virtual void onLayersInfo(const LayersInformation& layers) { }
    virtual void onImageData(const ImageData& imageData) { }
    virtual void onBeginLayer(const LayerRecord& layer) { }
    virtual void onEndLayer(const LayerRecord& layer) { }
    virtual void onSlicesData(const Slices& slices) { }
    virtual void onFramesData(const std::vector<FrameInformation>& framesInfo,
                              const uint32_t activeFrameIndex) { }
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

    const FileHeader& fileHeader() const { return m_header; }

    bool readFileHeader();
    bool readColorModeData();
    bool readImageResources();
    bool readLayersAndMask();
    bool readImageData();
    bool getSlices(const OSTypeDescriptor* desc, Slices& slices);

  private:
    bool readLayersInfo(LayersInformation& layers);
    bool readLayersInfo(const uint64_t length, LayersInformation& layers);
    bool readLayerRecord(LayersInformation& layers,
                         LayerRecord& layerRecord);
    bool readGlobalMaskInfo(LayersInformation& layers);
    bool readImage(const ImageData& img);
    bool readSectionDivider(LayerRecord& layerRecord, const uint64_t length);
    bool readLayerMLSTSection(LayerRecord& layerRecord);
    bool readLayerTMLNSection(LayerRecord& layerRecord);
    bool readLayerCUSTSection(LayerRecord& layerRecord);
    bool readResourceSlicesV6();
    bool readResourceSlices();
    uint64_t readAdditionalLayerInfo(LayerRecord& layerRecord);
    std::unique_ptr<OSTypeDescriptor> readAnimatedDataSection();
    std::unique_ptr<OSType> parseOsTypeVariable();
    std::unique_ptr<OSTypeReference> parseReferenceType();
    std::unique_ptr<OSTypeDescriptor> parseDescriptor();
    std::unique_ptr<OSTypeList> parseListType();
    std::unique_ptr<OSTypeClass> parseClassType();
    std::unique_ptr<OSTypeEnum> parseEnumeratedType();
    std::unique_ptr<OSTypeAlias> parseAliasType();
    OSTypeClassMetaType parseDescrVariable();
    std::wstring getUnicodeString();

    uint8_t read8() { return m_file->read8(); }
    uint16_t read16();
    uint32_t read32();
    uint64_t read64();
    uint32_t read16or32Length();
    uint64_t read32or64Length();
    double readDouble();
    std::string readPascalString(const int alignment);

    DecoderDelegate* m_delegate;
    FileInterface* m_file;
    FileHeader m_header;
  };

  bool decode_psd(FileInterface* file, DecoderDelegate* delegate);

} // namespace psd

#endif
