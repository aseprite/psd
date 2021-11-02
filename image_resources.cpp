// Aseprite PSD Library
// Copyright (C) 2019-2021 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"
#include "psd_debug.h"
#include "psd_details.h"

#include <stdexcept>

namespace psd {

// static
const char* ImageResource::resIDString(uint16_t resID)
{
  switch (resID) {
    case 1000:
    case 1003:
    case 0x03EF:
    case 0x03EA:
    case 0x03FC:
    case 0x03FF:
    case 0x0403: return "(Obsolete)";
    case 0x03E9: return "Macintosh print manager print info record";
    case 0x03ED: return "ResolutionInfo structure";
    case 0x03EE: return "Names of the alpha channels as a series of Pascal strings";
    case 0x03F0: return "The caption as a Pascal string";
    case 0x03F1: return "Border information";
    case 0x03F2: return "Background color";
    case 0x03F3: return "Print flags";
    case 0x03F4: return "Grayscale and multichannel halftoning information";
    case 0x03F5: return "Color halftoning information";
    case 0x03F6: return "Duotone halftoning information";
    case 0x03F7: return "Grayscale and multichannel transfer function";
    case 0x03F8: return "Color transfer functions";
    case 0x03F9: return "Duotone transfer functions";
    case 0x03FA: return "Duotone image information";
    case 0x03FB: return "Effective black and white values for the dot range";
    case 0x03FD: return "EPS options";
    case 0x03FE: return "Quick Mask information";
    case 0x0400: return "Layer state information";
    case 0x0401: return "Working path (not saved)";
    case 0x0402: return "Layers group information";
    case 0x0404: return "IPTC-NAA record";
    case 0x0405: return "Image mode for raw format files";
    case 0x0406: return "JPEG quality";
    case 0x0408: return "Grid and guides information";
    case 0x0409: return "Thumbnail resource (Photoshop 4.0)";
    case 0x040A: return "Copyright flag";
    case 0x040B: return "URL";
    case 0x040C: return "Thumbnail resource";
    case 0x040D: return "Global Angle";
    case 0x040E: return "Color samplers resource";
    case 0x040F: return "ICC Profile";
    case 0x0410: return "Watermark";
    case 0x0411: return "ICC Untagged Profile";
    case 0x0412: return "Effects visible";
    case 0x0413: return "Spot Halftone";
    case 0x0414: return "Document-specific IDs seed number";
    case 0x0415: return "Unicode Alpha Names";
    case 0x0416: return "Indexed Color Table Count";
    case 0x0417: return "Transparency Index";
    case 0x0419: return "Global Altitude";
    case 0x041A: return "Slices";
    case 0x041B: return "Workflow URL";
    case 0x041C: return "Jump To XPEP";
    case 0x041D: return "Alpha Identifiers";
    case 0x041E: return "URL List";
    case 0x0421: return "Version Info";
    case 0x0422: return "EXIF data 1";
    case 0x0423: return "EXIF data 3";
    case 0x0424: return "XMP metadata";
    case 0x0425: return "Caption digest";
    case 0x0426: return "Print scale";
    case 0x0428: return "Pixel Aspect Ratio";
    case 0x0429: return "Layer Comps";
    case 0x042A: return "Alternate Duotone Colors";
    case 0x042B: return "Alternate Spot Colors";
    case 0x042D: return "Layer Selection ID(s)";
    case 0x042E: return "HDR Toning information";
    case 0x042F: return "Print info";
    case 0x0430: return "Layer Group(s)";
    case 0x0431: return "Color samplers resource";
    case 0x0432: return "Measurement Scale";
    case 0x0433: return "Timeline Information";
    case 0x0434: return "Sheet Disclosure";
    case 0x0435: return "DisplayInfo structure to support floating point colors";
    case 0x0436: return "Onion Skins";
    case 0x0438: return "Count Information";
    case 0x043A: return "Print Information";
    case 0x043B: return "Print Style";
    case 0x043C: return "Macintosh NSPrintInfo";
    case 0x043D: return "Windows DEVMODE";
    case 0x043E: return "Auto Save File Path";
    case 0x043F: return "Auto Save Format";
    case 0x0440: return "Path Selection State";
    case 2999: return "Name of clipping path";
    case 3000: return "Origin Path Info";
    case 7000: return "Image Ready variables";
    case 7001: return "Image Ready data sets";
    case 7002: return "Image Ready default selected state";
    case 7003: return "Image Ready 7 rollover expanded state";
    case 7004: return "Image Ready rollover expanded state";
    case 7005: return "Image Ready save layer settings";
    case 7006: return "Image Ready version";
    case 8000: return "Lightroom workflow";
    case 10000: return "Print flags information";
    default:
      if (resID >= 2000 && resID <= 2997) return "Path Information";
      if (resID >= 4000 && resID <= 4999) return "Plug-In resource";
      break;
  }
  return "";
}

std::string key_to_string(const uint32_t key)
{
  std::string str(4, '0');
  for (int i=0; i<4; ++i) {
    str[3-i] = char(key>>(8*i));
  }
  return str;
}

// static
bool ImageResource::resIDHasDescriptor(uint16_t resID)
{
  switch (resID) {
    case 1065:
    case 1074:
    case 1075:
    case 1076:
    case 1078:
    case 1080:
    case 1082:
    case 1083:
    case 1088:
    case 3000:
      return true;
    default:
      return false;
  }
}

bool is_valid_class_type(const uint32_t class_id)
{
  // We don't want to cast the class_id to OSTypeKey to avoid UB
  switch (class_id) {
    case (uint32_t)OSTypeKey::Alias:
    case (uint32_t)OSTypeKey::Boolean:
    case (uint32_t)OSTypeKey::ClassType:
    case (uint32_t)OSTypeKey::Descriptor:
    case (uint32_t)OSTypeKey::Double:
    case (uint32_t)OSTypeKey::Enumerated:
    case (uint32_t)OSTypeKey::GlobalClass:
    case (uint32_t)OSTypeKey::GlobalObject:
    case (uint32_t)OSTypeKey::LargeInteger:
    case (uint32_t)OSTypeKey::List:
    case (uint32_t)OSTypeKey::Long:
    case (uint32_t)OSTypeKey::RawData:
    case (uint32_t)OSTypeKey::Reference:
    case (uint32_t)OSTypeKey::UnitFloat:
    case (uint32_t)OSTypeKey::String:
      return true;
    }
  return false;
}

bool is_valid_reference_type(const uint32_t key)
{
  switch (key) {
    case (uint32_t)OSTypeKey::RefClass:
    case (uint32_t)OSTypeKey::RefEnum:
    case (uint32_t)OSTypeKey::RefIdentifier:
    case (uint32_t)OSTypeKey::RefIndex:
    case (uint32_t)OSTypeKey::RefName:
    case (uint32_t)OSTypeKey::RefOffset:
    case (uint32_t)OSTypeKey::RefProperty:
      return true;
  }
  return false;
}

bool is_valid_unit_float(const uint32_t unit)
{
  switch (unit) {
    case (std::uint32_t)OSTypeUnitFloat::Unit::Angle:
    case (std::uint32_t)OSTypeUnitFloat::Unit::Density:
    case (std::uint32_t)OSTypeUnitFloat::Unit::Distance:
    case (std::uint32_t)OSTypeUnitFloat::Unit::None:
    case (std::uint32_t)OSTypeUnitFloat::Unit::Percent:
    case (std::uint32_t)OSTypeUnitFloat::Unit::Pixel:
      return true;
    default:
      return false;
  }
}

OSTypeClassMetaType Decoder::parseDescrVariable()
{
  const uint32_t classIDLength = read32();
  OSTypeClassMetaType meta;
  if (classIDLength == 0) {
    const uint32_t key = read32();
    meta.name = key_to_string(key);
  }
  else {
    meta.name.resize(classIDLength);
    m_file->read((uint8_t*)&meta.name[0], classIDLength);
  }
  return meta;
}

std::unique_ptr<OSTypeReference> Decoder::parseReferenceType()
{
  const uint32_t nItems = read32();
  std::unique_ptr<OSTypeReference> ref(new OSTypeReference);
  for (int i = 0; i < nItems; ++i) {
    const uint32_t osTypeInt = read32();
    if (!is_valid_reference_type(osTypeInt))
      throw std::runtime_error(
        "invalid reference type while parsing references");

    const OSTypeKey osType = static_cast<OSTypeKey>(osTypeInt);
    switch (osType) {
      case OSTypeKey::RefProperty: {
        std::unique_ptr<OSTypeProperty> data(new OSTypeProperty);
        data->propName = getUnicodeString();
        data->classID = parseDescrVariable();
        data->keyID = parseDescrVariable();
        ref->refs.emplace_back(std::move(data));
        break;
      }
      case OSTypeKey::RefClass:
        ref->refs.emplace_back(parseClassType());
        break;
      case OSTypeKey::RefEnum: {
        std::unique_ptr<OSTypeEnumeratedRef> enumRef(new OSTypeEnumeratedRef);
        enumRef->refClassID = getUnicodeString();
        enumRef->classID = parseDescrVariable();
        enumRef->typeID = parseDescrVariable();
        enumRef->enumValue = parseDescrVariable();
        ref->refs.emplace_back(std::move(enumRef));
        break;
      }
      case OSTypeKey::RefOffset: {
        std::unique_ptr<OSTypeOffset> offset(new OSTypeOffset);
        offset->offsetName = getUnicodeString();
        offset->classID = parseDescrVariable();
        offset->value = read32();
        ref->refs.emplace_back(std::move(offset));
        break;
      }
      case OSTypeKey::RefIdentifier:
      case OSTypeKey::RefIndex:
      case OSTypeKey::RefName:
        throw std::runtime_error(
          "undocumented type encountered parsing reference type");
    }
  }
  return ref;
}

std::unique_ptr<OSTypeList> Decoder::parseListType()
{
  const uint32_t nLength = read32();
  std::unique_ptr<OSTypeList> list(new OSTypeList);
  for (int i = 0; i < nLength; ++i) {
    list->values.emplace_back(parseOsTypeVariable());
  }
  return list;
}

std::unique_ptr<OSTypeClass> Decoder::parseClassType()
{
  std::unique_ptr<OSTypeClass> klass(new OSTypeClass);
  klass->className = getUnicodeString();
  klass->meta = parseDescrVariable();
  return klass;
}

std::unique_ptr<OSTypeEnum> Decoder::parseEnumeratedType()
{
  std::unique_ptr<OSTypeEnum> e(new OSTypeEnum);
  e->typeID = parseDescrVariable();
  e->enumValue = parseDescrVariable();
  return e;
}

std::unique_ptr<OSTypeAlias> Decoder::parseAliasType()
{
  const uint32_t length = read32();
  m_file->seek(m_file->tell() + length);
  return std::unique_ptr<OSTypeAlias>(new OSTypeAlias);
}

std::unique_ptr<OSType> Decoder::parseOsTypeVariable()
{
  const uint32_t osTypeInt = read32();
  if (!is_valid_class_type(osTypeInt))
    throw std::runtime_error(
      "invalid class type encountered in descriptor type");

  const OSTypeKey osType = static_cast<OSTypeKey>(osTypeInt);
  std::unique_ptr<OSType> value;

  switch (osType) {
    case OSTypeKey::GlobalObject:
    case OSTypeKey::Descriptor:
      value = parseDescriptor();
      break;
    case OSTypeKey::Reference:
      value = parseReferenceType();
      break;
    case OSTypeKey::List:
      value = parseListType();
      break;
    case OSTypeKey::Double:
      value.reset(new OSTypeDouble(read64()));
      break;
    case OSTypeKey::UnitFloat: {
      const uint32_t unit = read32();
      const double v = read64();
      if (!is_valid_unit_float(unit))
        throw std::runtime_error(
          "invalid unit float in descriptor type");

      value.reset(new OSTypeUnitFloat(
          static_cast<OSTypeUnitFloat::Unit>(unit), v));
      break;
    }
    case OSTypeKey::String:
      value.reset(new OSTypeString(getUnicodeString()));
      break;
    case OSTypeKey::Enumerated:
      value = parseEnumeratedType();
      break;
    case OSTypeKey::Long:
      value.reset(new OSTypeInt(read32()));
      break;
    case OSTypeKey::LargeInteger:
      value.reset(new OSTypeLargeInt(read64()));
      break;
    case OSTypeKey::Boolean:
      value.reset(new OSTypeBoolean((bool)read8()));
      break;
    case OSTypeKey::GlobalClass:
    case OSTypeKey::ClassType:
      value = parseClassType();
      break;
    case OSTypeKey::Alias:
      value = parseAliasType();
      break;
    case OSTypeKey::RawData:
      throw std::runtime_error("raw data is unimplemented");
  }
  return value;
}

std::unique_ptr<OSTypeDescriptor> Decoder::parseDescriptor()
{
  std::unique_ptr<OSTypeDescriptor> desc(new OSTypeDescriptor);
  desc->descriptorName = getUnicodeString();
  desc->classId = parseDescrVariable();

  const uint32_t nDescriptors = read32();
  for (int i = 0; i < nDescriptors; ++i) {
    const OSTypeClassMetaType key = parseDescrVariable();
    desc->descriptors.emplace(key.name, parseOsTypeVariable());
  }

  return desc;
}

std::wstring Decoder::getUnicodeString()
{
  const uint32_t length = read32();
  std::wstring str;
  for (int i = 0; i < length; ++i) {
    wchar_t ch = read16();
    str += ch;
  }
  return str;
}

} // namespace psd
