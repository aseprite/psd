// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"

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

} // namespace psd
