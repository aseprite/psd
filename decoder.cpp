// Aseprite PSD Library
// Copyright (C) 2019-2021 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"
#include "psd_debug.h"
#include "psd_details.h"

#include <cinttypes>
#include <stdexcept>

namespace psd {

Decoder::Decoder(FileInterface* file,
                 DecoderDelegate* delegate)
  : m_delegate(delegate)
  , m_file(file)
{
}

bool Decoder::readFileHeader()
{
  const uint32_t magic = read32(); // Magic ("8BPS")
  uint16_t version = read16();     // Version
  for (int i=0; i<6; ++i)          // 6 reserved bytes (must be 0)
    read8();
  uint16_t nchannels = read16(); // Number of channels in the image
  uint32_t height = read32();    // Height of the image
  uint32_t width = read32();     // Width of the image
  uint16_t depth = read16();     // Bits per channel
  uint16_t colorMode = read16();

  TRACE("File Header magic=%c%c%c%c version=%d nchannels=%d width=%d height=%d depth=%d colorMode=%d\n",
        (magic >> 24),
        (magic >> 16) & 0xff,
        (magic >> 8) & 0xff,
        magic & 0xff,
        version, nchannels,
        width, height, depth, colorMode);

  if (magic != PSD_FILE_MAGIC_NUMBER)
    throw std::runtime_error(
      "The magic number in the header do not match");

  if (depth != 1 && depth != 8 && depth != 16 && depth != 32)
    throw std::runtime_error("Unsupported image depth");

  if (colorMode != uint16_t(ColorMode::Bitmap) &&
      colorMode != uint16_t(ColorMode::Grayscale) &&
      colorMode != uint16_t(ColorMode::Indexed) &&
      colorMode != uint16_t(ColorMode::RGB) &&
      colorMode != uint16_t(ColorMode::CMYK) &&
      colorMode != uint16_t(ColorMode::Multichannel) &&
      colorMode != uint16_t(ColorMode::Duotone) &&
      colorMode != uint16_t(ColorMode::Lab))
    throw std::runtime_error("Invalid color mode found in the header");

  // Check valid supported size depending on file version
  switch (Version(version)) {
    case Version::Psd:
      if (width > 30000 || height > 30000)
        throw std::runtime_error(
          "Unexpected width/height for a PSD file");
      break;
    case Version::Psb:
      if (width > 300000 || height > 300000)
        throw std::runtime_error(
          "Unexpected width/height for a PSB file");
      break;
    default:
      throw std::runtime_error("Invalid version number");
  }

  m_header.version = Version(version);
  m_header.nchannels = nchannels;
  m_header.width = width;
  m_header.height = height;
  m_header.depth = depth;
  m_header.colorMode = ColorMode(colorMode);

  if (m_delegate)
    m_delegate->onFileHeader(m_header);
  return true;
}

bool Decoder::readColorModeData()
{
  ColorModeData data;
  data.length = read32();

  TRACE("Color Mode Data length=%d\n", data.length);
  // Only indexed and duotone have color mode, all other modes
  // have their length set to 0

  if (data.length == 0) {
    if (m_header.colorMode == ColorMode::Indexed
      || m_header.colorMode == ColorMode::Duotone)
      throw std::runtime_error(
        "The color mode cannot be indexed/duotone and have size zero,"
        "this must be a corrupt file");
    else
      return m_file->ok();
  }

  if (m_header.colorMode == ColorMode::Indexed) {
    if (data.length != 768)
      throw std::runtime_error("Unexpected palette length for indexed image");

    data.colors.resize(256);
    for (int i=0; i<256; ++i) data.colors[i].r = read8();
    for (int i=0; i<256; ++i) data.colors[i].g = read8();
    for (int i=0; i<256; ++i) data.colors[i].b = read8();
  }
  // For Duotone we should keep this (undocumented) data as it is, and
  // use pixel information as a grayscale image. Then this data should
  // be preserve when saving the file.
  else {
    // Read raw data
    data.data.resize(data.length);
    m_file->read(&data.data[0], data.length);
  }

  if (m_delegate)
    m_delegate->onColorModeData(data);
  return m_file->ok();
}

bool Decoder::readImageResources()
{
  ImageResources res;
  uint32_t length = read32();
  const uint32_t begin = m_file->tell();
  const uint32_t end = begin + length;

  TRACE("All Image Resources Length=%d End=%d\n", length, end);
  while (length > 0) {
    const uint32_t resBegin = m_file->tell();

    const uint32_t magic = read32();
    if (magic != PSD_IMAGE_BLOCK_MAGIC_NUMBER)
      break;

    const uint16_t resID = read16();
#ifdef _DEBUG
    const char* resourceName = ImageResource::resIDString(resID);
    (void)resourceName;
    TRACE("%s\n", resourceName);
#endif // _DEBUG

    const std::string name = readPascalString(2);
    const uint32_t resLength = read32();
    const size_t filePos = m_file->tell();

    ImageResource res;
    res.resourceID = resID;
    res.name = name;
    if (resLength) {
      if (ImageResource::resIDHasDescriptor(resID)) {
        const uint32_t descVersion = read32();
        if (descVersion == 16)
          res.descriptor = parseDescriptor();
      }
      else if (resID == 4003) {
        // This section is usually where the animation info is kept.
        // Details of how it is parsed is in readAnimatedDataSection()
        read32(); // An unknown resource, name is written backward
        read32(); // Unknown block
        read32(); // Another unknown block (a size?)
        const uint32_t signature = read32();
        if (signature == PSD_LAYER_INFO_MAGIC_NUMBER) {
          const uint32_t key = read32();
          if (key == (uint32_t)ImageResourceSection::ANDS)
            res.descriptor = readAnimatedDataSection();
        }
      }
      else {
        res.data.resize(resLength);
        m_file->read(&res.data[0], resLength);
      }
    }

    m_file->seek(filePos+resLength);
    // Padded to make it even
    if (resLength & 1)
      read8();

    const uint32_t resEnd = m_file->tell();

    if (m_delegate)
      m_delegate->onImageResource(res);

    length -= (resEnd - resBegin);
  }
  m_file->seek(end);

  return (length == 0);
}

double Decoder::getTypeNumber(const std::unique_ptr<OSType>& v)
{
  if (v->type() == OSTypeKey::LargeInteger) {
    return static_cast<const OSTypeLargeInt*>(v.get())->value;
  }
  else if (v->type() == OSTypeKey::Long) {
    return static_cast<const OSTypeInt*>(v.get())->value;
  }
  else if (v->type() == OSTypeKey::Double) {
    return static_cast<const OSTypeDouble*>(v.get())->value;
  }
  throw std::runtime_error("wrong descriptor type");
};

// the parsing used here is written thanks to the description given here
// https://community.adobe.com/t5/photoshop-ecosystem-discussions/how-to-get-animation-frame-id/m-p/8790051#M68134
std::unique_ptr<OSTypeDescriptor> Decoder::readAnimatedDataSection()
{
  read32(); // data length

  const uint32_t descVersion = read32();
  if (descVersion != 16)
    return nullptr;

  auto desc = parseDescriptor();
  if (!desc)
    return nullptr;

  using DescriptorMap = OSTypeDescriptor::DescriptorMap;
  const DescriptorMap& descrs = desc->descriptors;
  const DescriptorMap::const_iterator frameStateIter =
    descrs.find("FSts");
  const DescriptorMap::const_iterator framesIter =
    descrs.find("FrIn");

  uint32_t activeFrameIndex = 0;
  if (frameStateIter != descrs.cend() &&
    frameStateIter->second->type() == OSTypeKey::List) {
    const std::vector<std::unique_ptr<OSType>>& frameStates =
      static_cast<const OSTypeList*>(
        frameStateIter->second.get())->values;
    if (frameStates.size() == 1 &&
      frameStates[0]->type() == OSTypeKey::Descriptor) {
      const DescriptorMap& frameStatesDesc =
        static_cast<const OSTypeDescriptor*>(
          frameStates[0].get())->descriptors;
      const DescriptorMap::const_iterator activeIndexIter =
        frameStatesDesc.find("AFrm");
      if (activeIndexIter != frameStatesDesc.cend()) {
        activeFrameIndex = getTypeNumber(activeIndexIter->second);
      }
    }
  }

  if (framesIter == descrs.cend())
    return nullptr;

  const auto& absFrameList = framesIter->second;
  if (absFrameList->type() != OSTypeKey::List)
    return nullptr;

  const OSTypeList* frameList =
    static_cast<const OSTypeList*>(absFrameList.get());

  std::vector<FrameInformation> frameInfoList;
  if (!frameList->values.empty())
    frameInfoList.reserve(frameList->values.size());

  for (const auto& absFrameValue : frameList->values) {
    if (absFrameValue->type() == OSTypeKey::Descriptor) {
      const OSTypeDescriptor* absFrameDesc =
        static_cast<const OSTypeDescriptor*>(
          absFrameValue.get());
      const DescriptorMap& framesDescrs = absFrameDesc->descriptors;
      const DescriptorMap::const_iterator frameDurationIter =
        framesDescrs.find("FrDl");
      const DescriptorMap::const_iterator frameIDIter =
        framesDescrs.find("FrID");
      const DescriptorMap::const_iterator frameGAIter =
        framesDescrs.find("FrGA");

      FrameInformation frameInfo;
      if (frameDurationIter != framesDescrs.cend())
        frameInfo.duration = getTypeNumber(
          frameDurationIter->second);
      if (frameIDIter != framesDescrs.cend())
        frameInfo.id = getTypeNumber(frameIDIter->second);
      if (frameGAIter != framesDescrs.cend())
        frameInfo.ga = getTypeNumber(frameGAIter->second);

      TRACE("Frame ID: %d, Duration: %d, GA: %f\n", frameInfo.id,
        frameInfo.duration, frameInfo.ga);
      frameInfoList.push_back(std::move(frameInfo));
    }
  }

  if (m_delegate)
    m_delegate->onFramesData(frameInfoList, activeFrameIndex);
  return desc;
}

// TODO: what to do with the data read in this segment?
bool Decoder::readSectionDivider(LayerRecord& layerRecord,
                                 const uint64_t length)
{
  // 4 possible values => 0 = any other type of layer,
  // 1 = open "folder", 2 = closed "folder",
  // 3 = bounding section divider, hidden in the UI
  layerRecord.sectionType = SectionType(read32());

  if (length < 12)
    return true;

  const uint32_t signature = read32();
  if(signature != PSD_LAYER_INFO_MAGIC_NUMBER)
    throw std::runtime_error("magic number do not match in section divider");

  const uint32_t bm = read32();
  const LayerBlendMode blendMode = LayerBlendMode(bm);
  if (length < 16)
    return true;

  // Sub type. 0 = normal, 1 = scene group, affects the animation timeline.
  const uint32_t subType = read32();
  if (subType != 0 && subType != 1)
    throw std::runtime_error("invalid subtype in section divider");
  return true;
}

// TODO: Decide what to do with the information obtained
// TODO: Determine how the information relates to the timeline
bool Decoder::readLayerTMLNSection(LayerRecord& layerRecord)
{
  using DescriptorMap = OSTypeDescriptor::DescriptorMap;

  const uint32_t descVersion = read32();
  if (descVersion != 16)
    return false;

  const auto descriptor = parseDescriptor();
  if (!descriptor)
    return false;

  const DescriptorMap& descMap = descriptor->descriptors;
  const auto timeScopeIter = descMap.find("timeScope");
  if (timeScopeIter == descMap.cend() ||
    timeScopeIter->second->type() != OSTypeKey::Descriptor)
    return false;

  const DescriptorMap& timeScope = static_cast<const OSTypeDescriptor*>(
    timeScopeIter->second.get())->descriptors;
  for (const auto& timeScopeKeyValue : timeScope) {
    if (timeScopeKeyValue.second->type() != OSTypeKey::Descriptor)
      continue;
    const std::string& key = timeScopeKeyValue.first;
    const DescriptorMap& value = static_cast<const OSTypeDescriptor*>(
      timeScopeKeyValue.second.get())->descriptors;
    const auto numeratorIter = value.find("numerator");
    const auto denominatorIter = value.find("denominator");

    uint32_t numerator = 0, denominator = 0;

    if (numeratorIter != value.cend())
      numerator = getTypeNumber(numeratorIter->second);
    if (denominatorIter != value.cend())
      denominator = getTypeNumber(denominatorIter->second);
    TRACE("Numerator: %d, Denominator: %d\n", numerator, denominator);
  }
  return true;
}

// TODO: Decide what to do with the information obtained
bool Decoder::readLayerCUSTSection(LayerRecord& layerRecord)
{
  using DescriptorMap = OSTypeDescriptor::DescriptorMap;

  const uint32_t descVersion = read32();
  if (descVersion != 16)
    return false;

  const auto descriptor = parseDescriptor();
  if (!descriptor)
    return false;

  const DescriptorMap& metadataMap = descriptor->descriptors;
  const auto layerTimeIter = metadataMap.find("layerTime");
  const double layerTime =
    layerTimeIter == metadataMap.cend() ? 0.0 :
    getTypeNumber(layerTimeIter->second);
  return true;
}

bool Decoder::readLayerMLSTSection(LayerRecord& layerRecord)
{
  using DescriptorMap = OSTypeDescriptor::DescriptorMap;

  const uint32_t descVersion = read32();
  const auto descriptor = parseDescriptor();
  if (!descriptor)
    return false;

  const DescriptorMap& descMap = descriptor->descriptors;
  const auto layerIDIter = descMap.find("LaID");
  const auto layerStatesIter = descMap.find("LaSt");

  if (layerIDIter == descMap.cend())
    return false;

  const uint32_t layerID = getTypeNumber(layerIDIter->second);
  if (layerRecord.layerID != layerID ||
    layerStatesIter == descMap.cend())
    return false;

  if (layerStatesIter->second->type() != OSTypeKey::List)
    return false;

  const auto layerStates = static_cast<const OSTypeList*>(
    layerStatesIter->second.get());

  for (const auto& layerState : layerStates->values) {
    if (layerState->type() != OSTypeKey::Descriptor)
      continue;

    const DescriptorMap& layerDesc = static_cast<
      const OSTypeDescriptor*>(layerState.get())->descriptors;
    const auto frameListIter = layerDesc.find("FrLs");
    const auto layerEnabledIter = layerDesc.find("enab");
    const bool isLayerVisible =
      layerEnabledIter == layerDesc.cend() ||
      static_cast<OSTypeBoolean*>(
        layerEnabledIter->second.get())->value;

    TRACE("Layer is enabled here: %d\n", isLayerVisible);

    if (frameListIter != layerDesc.cend() &&
      frameListIter->second->type() == OSTypeKey::List) {
      const auto& frameList = static_cast<const OSTypeList*>(
        frameListIter->second.get())->values;
      for (const auto& frameIDPtr : frameList) {
        LayerRecord::FrameVisibility inFrame;
        inFrame.frameID = getTypeNumber(frameIDPtr);
        inFrame.isVisibleInFrame = isLayerVisible;
        layerRecord.inFrames.push_back(std::move(inFrame));
      }
    }
  }
  return true;
}

uint64_t Decoder::readAdditionalLayerInfo(LayerRecord& layerRecord)
{
  const uint32_t signature = read32(); // Magic ("8BIM" or "8B64")
  if (signature != PSD_LAYER_INFO_MAGIC_NUMBER &&
    signature != PSD_LAYER_INFO_MAGIC_NUMBER2)
    return 0;

  const LayerInfoKey key = static_cast<LayerInfoKey>(read32());
  uint64_t dataLength = 0;

  if ((m_header.version == Version::Psb) &&
    (key == LayerInfoKey::LMsk ||
      key == LayerInfoKey::Lr16 ||
      key == LayerInfoKey::Lr32 ||
      key == LayerInfoKey::Layr ||
      key == LayerInfoKey::Mt16 ||
      key == LayerInfoKey::Mt32 ||
      key == LayerInfoKey::Mtrn ||
      key == LayerInfoKey::Alph ||
      key == LayerInfoKey::FMsk ||
      key == LayerInfoKey::lnk2 ||
      key == LayerInfoKey::FEid ||
      key == LayerInfoKey::FXid ||
      key == LayerInfoKey::PxSD)) {
    dataLength = read64();
  }
  else {
    dataLength = read32();
  }

  const size_t fileBegin = m_file->tell();
  if (key == LayerInfoKey::lsct) {
    // Section divider setting (Photoshop 6.0)
    if (readSectionDivider(layerRecord, dataLength)) {
      TRACE("section divider read");
    }
  }
  else if (key == LayerInfoKey::cinf) {
    const uint32_t version = read32();
    if (version != 16)
      throw std::runtime_error("The version for cinf doesn't match");
    auto descriptor = parseDescriptor();
    if (descriptor) {
      TRACE("Number of desc: %d\n", version);
    }
  }
  else if (key == LayerInfoKey::luni) {
    const std::wstring layerName = getUnicodeString();
    if (!layerName.empty()) {
      TRACE("Layer name parsed: %ls\n", layerName.c_str());
    }
  }
  else if (key == LayerInfoKey::lyid) {
    layerRecord.layerID = read32();
  }
  else if (key == LayerInfoKey::SoLE) {
    const LayerInfoKey type = LayerInfoKey(read32());
    const uint32_t version = read32();
    if (type == LayerInfoKey::SoLd &&
      (version == 4 || version == 5)) {
      auto desc = parseDescriptor();
      if (desc) {
        TRACE("Descriptor name: %ls\n", desc->descriptorName.c_str());
        TRACE("Descriptor size: %zu\n", desc->descriptors.size());
      }
    }
  }
  else if (key == LayerInfoKey::Lr16 ||
    key == LayerInfoKey::Lr32 ||
    key == LayerInfoKey::Layr) {
    LayersInformation layersInfo;
    if (readLayersInfo(dataLength, layersInfo)) {
      TRACE("Number of layers read: %zu\n",
        layersInfo.layers.size());
    }
  }
  else if (key == LayerInfoKey::anFX) {
    const uint32_t descVersion = read32();
    if (descVersion == 16) {
      auto desc = parseDescriptor();
      if (desc) {
        TRACE("Descriptor name: %ls\n", desc->descriptorName.c_str());
        TRACE("Descriptor size: %zu\n", desc->descriptors.size());
      }
    }
  }
  else if (key == LayerInfoKey::shmd) {
    const uint32_t metadataCount = read32();
    for (uint32_t i = 0; i < metadataCount; ++i) {
      const uint32_t sig = read32();
      if (sig != PSD_LAYER_INFO_MAGIC_NUMBER)
        throw std::runtime_error("magic number does not match");

      const uint32_t metaKey = read32();
      read8(); // this is supposed to be a kind of "duplicate" data???
      read16(); read8(); // 3 bytes padding
      const uint32_t metaDataLength = read32();
      const size_t filePos = m_file->tell();
      if (metaKey == (uint32_t)LayerInfoKey::mlst)
        readLayerMLSTSection(layerRecord);
      else if (metaKey == (uint32_t)LayerInfoKey::cust)
        readLayerCUSTSection(layerRecord);
      else if (metaKey == (uint32_t)LayerInfoKey::tmln)
        readLayerTMLNSection(layerRecord);
      m_file->seek(filePos + metaDataLength);
    }
  }

  const uint64_t origLength = dataLength;
  if (origLength & 1)
    ++dataLength;

  TRACE(" tag block %c%c%c%c with length=%" PRId64 " (%" PRId64 ")\n",
    ((((int)key) >> 24) & 0xff),
    ((((int)key) >> 16) & 0xff),
    ((((int)key) >> 8) & 0xff),
    (((int)key) & 0xff),
    dataLength, origLength);
  TRACE("\n");

  m_file->seek(fileBegin + dataLength);
  return dataLength;
}

bool Decoder::readLayersAndMask()
{
  LayersInformation layers;
  const uint64_t length = read32or64Length();
  const uint64_t beg = m_file->tell();

  TRACE("layers length=%" PRId64 "\n", length);

  // Read layers info section
  readLayersInfo(layers);

  // Read global mask info section
  readGlobalMaskInfo(layers);

  // Read tagged blocks with more data
  if (m_file->tell() < beg+length) {
    TRACE(" Tagged blocks\n");

    LayerRecord layerRecord;
    while ((m_file->tell() - (beg+length)) > 4) {
      readAdditionalLayerInfo(layerRecord);
    }

    // TODO
  }

  if (m_delegate)
    m_delegate->onLayersAndMask(layers);

  m_file->seek(beg + length);
  return true;
}

bool Decoder::readImageData()
{
  ImageData data;

  // Compressiong method
  uint16_t compressionMethod = read16();
  data.compressionMethod = CompressionMethod(compressionMethod);

  TRACE("Image data compressionMethod=%d\n", compressionMethod);

  ImageData img;
  img.depth = m_header.depth;
  img.width = m_header.width;
  img.height = m_header.height;
  img.compressionMethod = data.compressionMethod;
  switch (m_header.nchannels) {
    case 1:
      img.channels.push_back(ChannelID::Alpha);
      break;
    case 2:
      img.channels.push_back(ChannelID::TransparencyMask);
      img.channels.push_back(ChannelID::Red);
      break;
    case 3:
      img.channels.push_back(ChannelID::Red);
      img.channels.push_back(ChannelID::Green);
      img.channels.push_back(ChannelID::Blue);
      break;
    case 4:
      img.channels.push_back(ChannelID::Red);
      img.channels.push_back(ChannelID::Green);
      img.channels.push_back(ChannelID::Blue);
      img.channels.push_back(ChannelID::Alpha);
      break;
    default:
      throw std::runtime_error("Invalid number of channels"); // TODO support custom channels
      break;
  }

  readImage(img);
  if (m_delegate)
    m_delegate->onImageData(data);
  return true;
}

bool Decoder::readLayersInfo(LayersInformation& layers)
{
  const uint64_t length = read32or64Length();
  TRACE("Layers Info length=%" PRId64 "\n", length);

  return readLayersInfo(length, layers);
}

bool Decoder::readLayersInfo(const uint64_t length, LayersInformation& layers)
{
  // Empty layers section
  if (length == 0)
    return true;

  uint64_t beg = m_file->tell();
  int16_t nlayers = read16();

  // If "nlayers" is negative the first alpha channel contains the
  // transparency data for the merged result.
  bool firstChannelIsTransparency = false;
  if (nlayers < 0) {
    nlayers = -nlayers;
    firstChannelIsTransparency = true;
  }

  TRACE("layers count=%d length=%" PRId64 " firstChannelIsTransparency=%d\n",
        nlayers, length, firstChannelIsTransparency);

  // Read layers info
  for (uint16_t i=0; i<uint16_t(nlayers); ++i) {
    LayerRecord layerRecord;
    if (!readLayerRecord(layers, layerRecord))
      throw std::runtime_error("Error reading layer record");

    // Add the layer
    layers.layers.push_back(layerRecord);
  }

  // Read transparency of merged result
  if (false && firstChannelIsTransparency) {
    uint16_t compression = read16();

    ImageData img;
    img.depth = m_header.depth;
    img.compressionMethod = CompressionMethod(compression);
    img.width = m_header.width;
    img.height = m_header.height;
    img.channels.push_back(ChannelID::TransparencyMask);
    readImage(img);
  }

  // Read channel data of each layer
  uint32_t fileBegin = m_file->tell();
  for (auto& layerRecord : layers.layers) {
    if (m_delegate)
      m_delegate->onBeginLayer(layerRecord);

    for (auto& channel : layerRecord.channels) {
      const uint16_t compression = read16();
      const int width = layerRecord.width();
      const int height = layerRecord.height();
      const uint32_t fileEnd = fileBegin + channel.length;

      TRACE("Reading channel data for layer='%s' channel=%d compression:%d width=%d height=%d\n",
            layerRecord.name.c_str(), channel.channelID,
            compression, width, height);

      ImageData img;
      img.depth = m_header.depth;
      img.compressionMethod = CompressionMethod(compression);
      img.width = width;
      img.height = height;
      img.channels.push_back(channel.channelID);
      readImage(img);

      m_file->seek(fileEnd);
      fileBegin = fileEnd;
    }
    if (m_delegate)
      m_delegate->onEndLayer(layerRecord);
  }

  m_file->seek(beg + length);
  return true;
}

bool Decoder::readLayerRecord(LayersInformation& layers,
                              LayerRecord& layerRecord)
{
  layerRecord.top = read32();
  layerRecord.left = read32();
  layerRecord.bottom = read32();
  layerRecord.right = read32();

  uint16_t nchannels = read16();
  layerRecord.channels.resize(nchannels);
  for (int i=0; i<nchannels; ++i) {
    layerRecord.channels[i].channelID = ChannelID(int16_t(read16()));
    layerRecord.channels[i].length = read32or64Length();
  }

  // Blend mode signature
  const uint32_t magic = read32();
  TRACE("LAYER magic=%c%c%c%c\n",
         ((magic >> 24) & 255),
         ((magic >> 16) & 255),
         ((magic >> 8) & 255),
         ((magic) & 255));
  if (magic != PSD_BLEND_MODE_MAGIC_NUMBER)
    throw std::runtime_error(
      "Magic number does not match for layer record");

  uint32_t bm = read32();
  layerRecord.blendMode = LayerBlendMode(bm);
  layerRecord.opacity = read8();
  layerRecord.sectionType = SectionType::Others;

  TRACE(" blendMode=%d %d %d %d\n",
         ((bm >> 24) & 255),
         ((bm >> 16) & 255),
         ((bm >> 8) & 255),
         ((bm) & 255));

  layerRecord.clipping = read8(); // clipping (0=base, 1=non-base);
  layerRecord.flags = read8();
  read8();                      // filler (zero)

  const uint32_t length = read32();
  const uint32_t beforeDataPos = m_file->tell();

  // Read mask data
  uint32_t maskLength = read32();
  m_file->seek(m_file->tell() + maskLength);

  // Read blending ranges
  uint32_t blendingRangesLength = read32();
  m_file->seek(m_file->tell() + blendingRangesLength);

  // Read layer name
  layerRecord.name = readPascalString(4);

  TRACE(" - top=%d left=%d bottom=%d right=%d; nchannels=%d\n"
         " - name=%s\n",
         layerRecord.top,
         layerRecord.left,
         layerRecord.bottom,
         layerRecord.right,
         nchannels,
         layerRecord.name.c_str());

  uint64_t filePos = m_file->tell();
  const size_t expectedPos = beforeDataPos + length;
  while (filePos < expectedPos) {
    const uint64_t bytesProcessed =
      readAdditionalLayerInfo(layerRecord);
    if (bytesProcessed == 0)
      break;
    filePos += bytesProcessed;
  }
  m_file->seek(expectedPos);
  return true;
}

bool Decoder::readGlobalMaskInfo(LayersInformation& layers)
{
  const std::size_t filePos = m_file->tell();
  uint64_t length = read32();
  TRACE("Global mask info length=%" PRId64 "\n", length);
  if (length == 0)
    return true;

  read16(); // Overlay color space
  read64(); // 4 * 2 bytes color components
  const uint16_t maskOpacity = read16();
  const uint8_t  maskKind = read8();

  if (maskOpacity > 100)
    throw std::runtime_error("Unexpected opacity for mask");

  if (maskKind != 0 && maskOpacity != 1 && maskKind != 128)
    throw std::runtime_error("Unexpected mask kind");

  layers.maskInfo.opacity = maskOpacity;
  layers.maskInfo.kind =
    static_cast<GlobalMaskInfo::MaskKind>(maskKind);

  m_file->seek(filePos + length);
  return true;
}

uint16_t Decoder::read16()
{
  int b1 = read8();
  int b2 = read8();

  if (m_file->ok()) {
    return ((b1 << 8) | b2); // Big endian
  }
  else
    return 0;
}

uint32_t Decoder::read32()
{
  int b1 = read8();
  int b2 = read8();
  int b3 = read8();
  int b4 = read8();

  if (m_file->ok()) {
    // Big endian
    return ((b1 << 24) | (b2 << 16) | (b3 << 8) | b4);
  }
  else
    return 0;
}

uint64_t Decoder::read64()
{
  uint8_t bytes[8];
  m_file->read(bytes, 8);

  if (m_file->ok()) {
    // Big endian
    return uint64_t(
      (uint64_t(bytes[0]) << 56) |
      (uint64_t(bytes[1]) << 48) |
      (uint64_t(bytes[2]) << 40) |
      (uint64_t(bytes[3]) << 32) |
      (uint64_t(bytes[4]) << 24) |
      (uint64_t(bytes[5]) << 16) |
      (uint64_t(bytes[6]) << 8) |
      uint64_t(bytes[7]));
  }
  else
    return 0;
}

uint32_t Decoder::read16or32Length()
{
  uint32_t length;
  if (m_header.version == Version::Psb)
    length = read32();
  else
    length = read16();
  return length;
}

uint64_t Decoder::read32or64Length()
{
  uint64_t length;
  if (m_header.version == Version::Psb)
    length = read64();
  else
    length = read32();
  return length;
}

std::string Decoder::readPascalString(const int alignment)
{
  const int length = read8();
  int bytes = 1;   // 1 byte because we've just read the "length" byte

  std::string result;
  for (int i=0; i<length; ++i) {
    result.push_back(read8());
    ++bytes;
  }

  // The string is padded to make the size even
  while (bytes % alignment) {
    read8();
    ++bytes;
  }

  return result;
}

bool Decoder::readImage(const ImageData& img)
{
  int scanlineSize =
    (img.depth >= 8 ?
     (img.width * (img.depth/8)):
     (img.width / img.depth + (img.width % img.depth ? 1: 0)));

  if (scanlineSize & 1)
    ++scanlineSize;

  std::vector<uint8_t> scanline(scanlineSize);

  if (m_delegate)
    m_delegate->onBeginImage(img);

  std::vector<uint32_t> byteCounts;
  if (img.compressionMethod == CompressionMethod::RLE) {
    byteCounts.resize(img.height * img.channels.size());
    for (size_t i=0; i<byteCounts.size(); ++i)
      byteCounts[i] = read16or32Length();
  }

  // Read channel by channel
  int curByteCount = 0;
  for (ChannelID chanID : img.channels) {
    TRACE("--- Channel ID=%d compression=%d depth=%d scanline=%zu ---\n",
          chanID,
          img.compressionMethod,
          img.depth,
          scanline.size());

    switch (img.compressionMethod) {

      case CompressionMethod::RawImageData:
        for (int y=0; y<img.height; ++y) {

          std::vector<uint8_t> rawData;
          rawData.reserve(img.width * (img.depth == 1 ? 1: img.depth/8));

          for (int x=0; x<img.width; ) {
            if (img.depth == 1) {
              uint8_t byte = read8();
              TRACE("%d %d%d%d%d%d%d%d%d",
                    byte,
                    (byte & 0x80) >> 7,
                    (byte & 0x40) >> 6,
                    (byte & 0x20) >> 5,
                    (byte & 0x10) >> 4,
                    (byte & 0x08) >> 3,
                    (byte & 0x04) >> 2,
                    (byte & 0x02) >> 1,
                    (byte & 0x01));
              x += 8;
              rawData.push_back(byte);
            }
            else if (img.depth == 8) {
              uint8_t byte = read8();
              TRACE(" %02x", byte);
              ++x;
              rawData.push_back(byte);
            }
            else if (img.depth == 16) {
              uint16_t word = read16();
              TRACE(" %04x", word);
              ++x;
              rawData.push_back(word & 0xff);
              rawData.push_back(word >> 8);
            }
            else if (img.depth == 32) {
              uint32_t dword = read32();
              TRACE(" %08x", dword);
              ++x;
              rawData.push_back(dword >> 24);
              rawData.push_back(dword >> 16);
              rawData.push_back(dword >> 8);
              rawData.push_back(dword);
            }
            else {
              throw std::runtime_error(
                "Unsupported raw image depth");
            }
          }
          TRACE("\n");

          if (m_delegate)
            m_delegate->onImageScanline(
              img, y, chanID,
              &rawData[0], rawData.size());
        }
        break;

      case CompressionMethod::RLE:
        switch (m_header.depth) {

          case 8: {
            auto end = scanline.end();
            for (int y=0; y<img.height; ++y, ++curByteCount) {
              auto it = scanline.begin();

              int32_t remaining = byteCounts[curByteCount];
              TRACE("   line[%d] (remaining=%d): ", y, remaining);
              while (remaining > 0) {
                int8_t n = int8_t(m_file->read8());
                TRACE("n=%d ", n);
                --remaining;

                if (n == -128) {
                  // No operation, skip this byte
                }
                else if (n >= 0 && n <= 127) {
                  for (int j=0; j<int(n)+1; ++j, ++it) {
                    if (it == end)
                      break; // throw std::runtime_error("invalid RLE data (too much individual values)");
                    *it = read8();
                  }
                  remaining -= int(n)+1;
                }
                else if (n < 0) {
                  uint8_t data = read8();
                  // TRACE("Repeat %d value %d times\n",
                  //       data, 1-int(n));
                  --remaining;
                  for (int j=1-int(n); j>0; --j, ++it) {
                    // TRACE("  %d ", j);
                    if (it == end)
                      break; // throw std::runtime_error("invalid RLE data (too much repeated values)");
                    *it = data;
                  }
                }
              }
              TRACE("\n");

              if (!m_file->ok())
                throw std::runtime_error("end-of-file not expected");

              // if (it < end)
              //   throw std::runtime_error("invalid RLE data (count too small)");

              for (; it < end; ++it)
                *it = 0;

              if (m_delegate) {
                m_delegate->onImageScanline(
                  img, y, chanID,
                  &scanline[0], scanline.size());
              }
            }
            break;
          }

        }
        break;

      case CompressionMethod::ZIPWithoutPrediction:
        // TODO
        break;

      case CompressionMethod::ZIPWithPrediction:
        // TODO
        break;
    }
  }

  if (m_delegate)
    m_delegate->onEndImage(img);

  return true;
}

} // namespace psd
