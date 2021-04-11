// Aseprite PSD Library
// Copyright (C) 2019 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "psd.h"

namespace psd {

StdioFileInterface::StdioFileInterface(FILE* file)
  : m_file(file)
  , m_ok(true)
{
}

bool StdioFileInterface::ok() const
{
  return m_ok;
}

size_t StdioFileInterface::tell()
{
  return ftell(m_file);
}

void StdioFileInterface::seek(size_t absPos)
{
  fseek(m_file, absPos, SEEK_SET);
}

uint8_t StdioFileInterface::read8()
{
  int value = fgetc(m_file);
  if (value != EOF)
    return value;

  m_ok = false;
  return 0;
}

bool StdioFileInterface::read(uint8_t* buf, uint32_t size)
{
  return (fread(buf, 1, size, m_file) == size);
}

void StdioFileInterface::write8(uint8_t value)
{
  fputc(value, m_file);
}

bool StdioFileInterface::write(const uint8_t* buf, uint32_t size)
{
  return (fwrite(buf, 1, size, m_file) == size);
}

} // namespace psd
