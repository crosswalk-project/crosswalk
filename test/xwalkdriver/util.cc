// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/util.h"

#include <list>

#include "base/base64.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/files/scoped_temp_dir.h"
#include "base/format_macros.h"
#include "base/rand_util.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/third_party/icu/icu_utf.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/ui_events.h"
#include "xwalk/test/xwalkdriver/xwalk/web_view.h"
#include "xwalk/test/xwalkdriver/xwalk/zip.h"
#include "xwalk/test/xwalkdriver/key_converter.h"

std::string GenerateId() {
  uint64 msb = base::RandUint64();
  uint64 lsb = base::RandUint64();
  return base::StringPrintf("%016" PRIx64 "%016" PRIx64, msb, lsb);
}

namespace {

Status FlattenStringArray(const base::ListValue* src, string16* dest) {
  string16 keys;
  for (size_t i = 0; i < src->GetSize(); ++i) {
    string16 keys_list_part;
    if (!src->GetString(i, &keys_list_part))
      return Status(kUnknownError, "keys should be a string");
    for (size_t j = 0; j < keys_list_part.size(); ++j) {
      if (CBU16_IS_SURROGATE(keys_list_part[j])) {
        return Status(kUnknownError,
                      "XwalkDriver only supports characters in the BMP");
      }
    }
    keys.append(keys_list_part);
  }
  *dest = keys;
  return Status(kOk);
}

}  // namespace

Status SendKeysOnWindow(
    WebView* web_view,
    const base::ListValue* key_list,
    bool release_modifiers,
    int* sticky_modifiers) {
  string16 keys;
  Status status = FlattenStringArray(key_list, &keys);
  if (status.IsError())
    return status;
  std::list<KeyEvent> events;
  int sticky_modifiers_tmp = *sticky_modifiers;
  status = ConvertKeysToKeyEvents(
      keys, release_modifiers, &sticky_modifiers_tmp, &events);
  if (status.IsError())
    return status;
  status = web_view->DispatchKeyEvents(events);
  if (status.IsOk())
    *sticky_modifiers = sticky_modifiers_tmp;
  return status;
}

bool Base64Decode(const std::string& base64,
                  std::string* bytes) {
  std::string copy = base64;
  // Some WebDriver client base64 encoders follow RFC 1521, which require that
  // 'encoded lines be no more than 76 characters long'. Just remove any
  // newlines.
  RemoveChars(copy, "\n", &copy);
  return base::Base64Decode(copy, bytes);
}

namespace {

Status UnzipArchive(const base::FilePath& unzip_dir,
                    const std::string& bytes) {
  base::ScopedTempDir dir;
  if (!dir.CreateUniqueTempDir())
    return Status(kUnknownError, "unable to create temp dir");

  base::FilePath archive = dir.path().AppendASCII("temp.zip");
  int length = bytes.length();
  if (file_util::WriteFile(archive, bytes.c_str(), length) != length)
    return Status(kUnknownError, "could not write file to temp dir");

  if (!zip::Unzip(archive, unzip_dir))
    return Status(kUnknownError, "could not unzip archive");
  return Status(kOk);
}

// Stream for writing binary data.
class DataOutputStream {
 public:
  DataOutputStream() {}
  ~DataOutputStream() {}

  void WriteUInt16(uint16 data) {
    WriteBytes(&data, sizeof(data));
  }

  void WriteUInt32(uint32 data) {
    WriteBytes(&data, sizeof(data));
  }

  void WriteString(const std::string& data) {
    WriteBytes(data.c_str(), data.length());
  }

  void WriteBytes(const void* bytes, int size) {
    if (!size)
      return;
    size_t next = buffer_.length();
    buffer_.resize(next + size);
    memcpy(&buffer_[next], bytes, size);
  }

  const std::string& buffer() const { return buffer_; }

 private:
  std::string buffer_;
};

// Stream for reading binary data.
class DataInputStream {
 public:
  DataInputStream(const char* data, int size)
      : data_(data), size_(size), iter_(0) {}
  ~DataInputStream() {}

  bool ReadUInt16(uint16* data) {
    return ReadBytes(data, sizeof(*data));
  }

  bool ReadUInt32(uint32* data) {
    return ReadBytes(data, sizeof(*data));
  }

  bool ReadString(std::string* data, int length) {
    if (length < 0)
      return false;
    // Check here to make sure we don't allocate wastefully.
    if (iter_ + length > size_)
      return false;
    data->resize(length);
    if (length == 0)
      return true;
    return ReadBytes(&(*data)[0], length);
  }

  bool ReadBytes(void* bytes, int size) {
    if (iter_ + size > size_)
      return false;
    memcpy(bytes, &data_[iter_], size);
    iter_ += size;
    return true;
  }

  int remaining() const { return size_ - iter_; }

 private:
  const char* data_;
  int size_;
  int iter_;
};

// A file entry within a zip archive. This may be incomplete and is not
// guaranteed to be able to parse all types of zip entries.
// See http://www.pkware.com/documents/casestudies/APPNOTE.TXT for the zip
// file format.
struct ZipEntry {
  // The given bytes must contain the whole zip entry and only the entry,
  // although the entry may include a data descriptor.
  static bool FromBytes(const std::string& bytes, ZipEntry* zip,
                        std::string* error_msg) {
    DataInputStream stream(bytes.c_str(), bytes.length());

    uint32 signature;
    if (!stream.ReadUInt32(&signature) || signature != kFileHeaderSignature) {
      *error_msg = "invalid file header signature";
      return false;
    }
    if (!stream.ReadUInt16(&zip->version_needed)) {
      *error_msg = "invalid version";
      return false;
    }
    if (!stream.ReadUInt16(&zip->bit_flag)) {
      *error_msg = "invalid bit flag";
      return false;
    }
    if (!stream.ReadUInt16(&zip->compression_method)) {
      *error_msg = "invalid compression method";
      return false;
    }
    if (!stream.ReadUInt16(&zip->mod_time)) {
      *error_msg = "invalid file last modified time";
      return false;
    }
    if (!stream.ReadUInt16(&zip->mod_date)) {
      *error_msg = "invalid file last modified date";
      return false;
    }
    if (!stream.ReadUInt32(&zip->crc)) {
      *error_msg = "invalid crc";
      return false;
    }
    uint32 compressed_size;
    if (!stream.ReadUInt32(&compressed_size)) {
      *error_msg = "invalid compressed size";
      return false;
    }
    if (!stream.ReadUInt32(&zip->uncompressed_size)) {
      *error_msg = "invalid compressed size";
      return false;
    }
    uint16 name_length;
    if (!stream.ReadUInt16(&name_length)) {
      *error_msg = "invalid name length";
      return false;
    }
    uint16 field_length;
    if (!stream.ReadUInt16(&field_length)) {
      *error_msg = "invalid field length";
      return false;
    }
    if (!stream.ReadString(&zip->name, name_length)) {
      *error_msg = "invalid name";
      return false;
    }
    if (!stream.ReadString(&zip->fields, field_length)) {
      *error_msg = "invalid fields";
      return false;
    }
    if (zip->bit_flag & 0x8) {
      // Has compressed data and a separate data descriptor.
      if (stream.remaining() < 16) {
        *error_msg = "too small for data descriptor";
        return false;
      }
      compressed_size = stream.remaining() - 16;
      if (!stream.ReadString(&zip->compressed_data, compressed_size)) {
        *error_msg = "invalid compressed data before descriptor";
        return false;
      }
      if (!stream.ReadUInt32(&signature) ||
          signature != kDataDescriptorSignature) {
        *error_msg = "invalid data descriptor signature";
        return false;
      }
      if (!stream.ReadUInt32(&zip->crc)) {
        *error_msg = "invalid crc";
        return false;
      }
      if (!stream.ReadUInt32(&compressed_size)) {
        *error_msg = "invalid compressed size";
        return false;
      }
      if (compressed_size != zip->compressed_data.length()) {
        *error_msg = "compressed data does not match data descriptor";
        return false;
      }
      if (!stream.ReadUInt32(&zip->uncompressed_size)) {
        *error_msg = "invalid compressed size";
        return false;
      }
    } else {
      // Just has compressed data.
      if (!stream.ReadString(&zip->compressed_data, compressed_size)) {
        *error_msg = "invalid compressed data";
        return false;
      }
      if (stream.remaining() != 0) {
        *error_msg = "leftover data after zip entry";
        return false;
      }
    }
    return true;
  }

  // Returns bytes for a valid zip file that just contains this zip entry.
  std::string ToZip() {
    // Write zip entry with no data descriptor.
    DataOutputStream stream;
    stream.WriteUInt32(kFileHeaderSignature);
    stream.WriteUInt16(version_needed);
    stream.WriteUInt16(bit_flag);
    stream.WriteUInt16(compression_method);
    stream.WriteUInt16(mod_time);
    stream.WriteUInt16(mod_date);
    stream.WriteUInt32(crc);
    stream.WriteUInt32(compressed_data.length());
    stream.WriteUInt32(uncompressed_size);
    stream.WriteUInt16(name.length());
    stream.WriteUInt16(fields.length());
    stream.WriteString(name);
    stream.WriteString(fields);
    stream.WriteString(compressed_data);
    uint32 entry_size = stream.buffer().length();

    // Write central directory.
    stream.WriteUInt32(kCentralDirSignature);
    stream.WriteUInt16(0x14);  // Version made by. Unused at version 0.
    stream.WriteUInt16(version_needed);
    stream.WriteUInt16(bit_flag);
    stream.WriteUInt16(compression_method);
    stream.WriteUInt16(mod_time);
    stream.WriteUInt16(mod_date);
    stream.WriteUInt32(crc);
    stream.WriteUInt32(compressed_data.length());
    stream.WriteUInt32(uncompressed_size);
    stream.WriteUInt16(name.length());
    stream.WriteUInt16(fields.length());
    stream.WriteUInt16(0);  // Comment length.
    stream.WriteUInt16(0);  // Disk number where file starts.
    stream.WriteUInt16(0);  // Internal file attr.
    stream.WriteUInt32(0);  // External file attr.
    stream.WriteUInt32(0);  // Offset to file.
    stream.WriteString(name);
    stream.WriteString(fields);
    uint32 cd_size = stream.buffer().length() - entry_size;

    // End of central directory.
    stream.WriteUInt32(kEndOfCentralDirSignature);
    stream.WriteUInt16(0);  // num of this disk
    stream.WriteUInt16(0);  // disk where cd starts
    stream.WriteUInt16(1);  // number of cds on this disk
    stream.WriteUInt16(1);  // total cds
    stream.WriteUInt32(cd_size);  // size of cd
    stream.WriteUInt32(entry_size);  // offset of cd
    stream.WriteUInt16(0);  // comment len

    return stream.buffer();
  }

  static const uint32 kFileHeaderSignature;
  static const uint32 kDataDescriptorSignature;
  static const uint32 kCentralDirSignature;
  static const uint32 kEndOfCentralDirSignature;
  uint16 version_needed;
  uint16 bit_flag;
  uint16 compression_method;
  uint16 mod_time;
  uint16 mod_date;
  uint32 crc;
  uint32 uncompressed_size;
  std::string name;
  std::string fields;
  std::string compressed_data;
};

const uint32 ZipEntry::kFileHeaderSignature = 0x04034b50;
const uint32 ZipEntry::kDataDescriptorSignature = 0x08074b50;
const uint32 ZipEntry::kCentralDirSignature = 0x02014b50;
const uint32 ZipEntry::kEndOfCentralDirSignature = 0x06054b50;

Status UnzipEntry(const base::FilePath& unzip_dir,
                  const std::string& bytes) {
  ZipEntry entry;
  std::string zip_error_msg;
  if (!ZipEntry::FromBytes(bytes, &entry, &zip_error_msg))
    return Status(kUnknownError, zip_error_msg);
  std::string archive = entry.ToZip();
  return UnzipArchive(unzip_dir, archive);
}

}  // namespace

Status UnzipSoleFile(const base::FilePath& unzip_dir,
                     const std::string& bytes,
                     base::FilePath* file) {
  std::string archive_error, entry_error;
  Status status = UnzipArchive(unzip_dir, bytes);
  if (status.IsError()) {
    Status entry_status = UnzipEntry(unzip_dir, bytes);
    if (entry_status.IsError()) {
      return Status(kUnknownError, base::StringPrintf(
          "archive error: (%s), entry error: (%s)",
          status.message().c_str(), entry_status.message().c_str()));
    }
  }

  base::FileEnumerator enumerator(unzip_dir, false /* recursive */,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
  base::FilePath first_file = enumerator.Next();
  if (first_file.empty())
    return Status(kUnknownError, "contained 0 files");

  base::FilePath second_file = enumerator.Next();
  if (!second_file.empty())
    return Status(kUnknownError, "contained multiple files");

  *file = first_file;
  return Status(kOk);
}
