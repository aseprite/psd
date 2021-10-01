#include "psd.h"
#include "psd_details.h"

#include <cassert>

namespace psd {
    Encoder::Encoder(FileInterface* file, EncoderDelegate* delegate)
      : m_file{file}
      , m_delegate{delegate}
      , m_header{}
    {
    }

    bool Encoder::writePSDHeader(const PSDHeader& header) {
      constexpr const int psdMaxLen = 30'000;
      constexpr const int psbMaxLen = 300'000;

      // sanity checks before we do any writes
      if (header.nchannels < 1 || header.nchannels > 56)
        return false;
      if(header.height < 1)
        return false;
      if (header.version == Version::Psd &&
          ((header.height > psdMaxLen) || (header.width > psdMaxLen))) {
        return false;
      }
      else if (header.version == Version::Psb &&
               ((header.height > psbMaxLen) || (header.width > psbMaxLen))) {
        return false;
      }
      if (header.depth != 1 && header.depth != 8 && header.depth != 16 
          && header.depth != 32)
        return false;

      if (header.colorMode != ColorMode::Bitmap &&
          header.colorMode != ColorMode::Grayscale &&
          header.colorMode != ColorMode::Indexed &&
          header.colorMode != ColorMode::RGB &&
          header.colorMode != ColorMode::CMYK &&
          header.colorMode != ColorMode::Multichannel &&
          header.colorMode != ColorMode::Duotone &&
          header.colorMode != ColorMode::Lab)
        return false;

      write32(PSD_FILE_MAGIC_NUMBER); // the magic number '8BPS'
      write16(static_cast<std::uint16_t>(header.version));
      for (int i = 0; i < 6; ++i) // 6 reserved bytes
        write8(0);

      write16(static_cast<std::uint16_t>(header.nchannels));
      write32(header.height); // image height
      write32(header.width); // image width
      write16(header.depth); // image depth: 1,8,16 or 32
      write16(static_cast<std::uint16_t>(header.colorMode));
      
      if (m_delegate) {
        m_delegate->onHeaderWritten(header);
      }

      // the header is needed in subsequent sections
      m_header = header; 
      return true;
    }

    bool Encoder::writeColorModeData(const ColorModeData& colorModeData) {
      // sanity checks
      const bool isIndexMode = m_header.colorMode == ColorMode::Indexed;
      const bool isDuotoneMode = m_header.colorMode == ColorMode::Duotone;

      if((isIndexMode || isDuotoneMode) && colorModeData.length < 1)
        return false;
      
      if((!isIndexMode && !isDuotoneMode) && colorModeData.length != 0)
        return false;

      if( isIndexMode && colorModeData.length != 768 )
        return false;

      // start writing the data

      write32(colorModeData.length);

      // Only indexed color and duotone have color mode data. For all other 
      // modes, this section is just the 4-byte length field, which is set to zero
      if(colorModeData.length == 0)
        return true;
      
      if (isIndexMode) {
        const std::vector<psd::IndexColor>& colors = colorModeData.colors;
        assert(colors.size() == 256);
        for (const auto &color : colors) write8(color.r);
        for (const auto &color : colors) write8(color.g);
        for (const auto &color : colors) write8(color.b);
      }
      else {
        assert(!colorModeData.data.empty());
        writeRawData(&colorModeData.data[0], colorModeData.data.size());
      }
      
      if (m_delegate) {
        m_delegate->onColorModeWritten(colorModeData);
      }
      return true;
    }

    std::size_t Encoder::pascalStringSize(const std::string& str,
        const int alignment)
    {
      return 1 // sizeof the length of the string itself
          + str.size() // the length of the string
          + str.size() % alignment; // possible alignment
    }

    std::size_t Encoder::imageResourceBlockSize(const ImageResource& res,
        const int stringAlignment ) {
      return 4 // psd image block magic number -> std::uint32_t
          + 2  // resource ID -> std::uint16_t
          + pascalStringSize(res.name, stringAlignment)
          + 4  // size of arbitrary data length -> std::uint32_t
               // length of the data, if it is an odd number, it's padded.
          + res.data.empty() ? 0 : 
            ((res.data.size() & 1) ? (res.data.size() + 1): (res.data.size()));
    }

    bool Encoder::writeImageResources(const ImageResources& imageResources) {
      constexpr const int pascalStringAlignment = 2;

      // we need to cache the current file position, so we can come back to it
      // and write the total length of the image resources
      const std::uint32_t prevPos = m_file->tell();

      // advance the file 4 bytes forward so we can write the image resources
      m_file->seek(prevPos + 4);

      std::uint32_t resLength = 0;
      for (const auto& resData : imageResources.resources) {
        write32(PSD_IMAGE_BLOCK_MAGIC_NUMBER);

        write16(resData.resourceID);
        writePascalString(resData.name, 2);

        const std::size_t dataLength = resData.data.size();
        write32(dataLength);

        if (dataLength > 0)
          writeRawData(&resData.data[0], dataLength);

        // Padded to make it even
        if(dataLength&1)
          write8(0);

        resLength += imageResourceBlockSize(
          resData, pascalStringAlignment);
      }

      const std::uint32_t curPos = m_file->tell();
      // go back in time and write the length of the image resource
      m_file->seek(prevPos);
      write32(resLength);

      // return to the current timeline
      m_file->seek(curPos);

      if (m_delegate) {
        m_delegate->onImageResourcesWritten(imageResources);
      }

      return true;
    }

    void Encoder::write8(const uint8_t value) {
      m_file->write8(value);
    }

    void Encoder::write16(const uint16_t value) {
      m_file->write16(value);
    }

    void Encoder::write32(const uint32_t value) {
      m_file->write32(value);
    }

    void Encoder::write64(const uint64_t value) {
      m_file->write64(value);
    }

    void Encoder::write32or64Length(const uint64_t value) {
      if(m_header.version == Version::Psb)
        write64(value);
      else
        write32(static_cast<uint32_t>(value));
    }

    void Encoder::writeRawData(const uint8_t* data, const std::size_t len) {
      m_file->write(data, len);
    }

    void Encoder::writePascalString(const std::string& str, const int alignment) {
      write8(static_cast<uint8_t>(str.size()));

      if (!str.empty()) {
        m_file->write((uint8_t const*)&str[0], str.size());
      }
      int bytesWritten = 1 + str.size();
      while (bytesWritten % alignment) {
        write8(0);
        ++bytesWritten;
      }
    }
}