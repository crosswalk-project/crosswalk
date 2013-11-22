// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_DBUS_PROPERTY_EXPORTER_H_
#define XWALK_DBUS_PROPERTY_EXPORTER_H_

#include <map>
#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "dbus/exported_object.h"
#include "dbus/object_path.h"

namespace base {
class DictionaryValue;
class Value;
}

namespace dbus {

class MessageWriter;

// Exports org.freedesktop.DBus.Properties interface for the given
// ExportedObject. Properties should be set directly into the exporter object
// using the function Set().
class PropertyExporter {
 public:
  PropertyExporter(dbus::ExportedObject* object, const dbus::ObjectPath& path);
  ~PropertyExporter();

  void Set(const std::string& interface,
           const std::string& property,
           scoped_ptr<base::Value>);

  // TODO(cmarcelo): We need some callback to indicate when all the methods
  // were exported.

  void AppendPropertiesToWriter(const std::string& interface,
                                MessageWriter* writer);

 private:
  void OnGet(dbus::MethodCall* method_call,
             dbus::ExportedObject::ResponseSender response_sender);
  void OnGetAll(dbus::MethodCall* method_call,
                dbus::ExportedObject::ResponseSender response_sender);
  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  typedef std::map<std::string, base::DictionaryValue*> InterfacesMap;
  InterfacesMap interfaces_;

  dbus::ObjectPath path_;
  base::WeakPtrFactory<PropertyExporter> weak_factory_;
};

}  // namespace dbus

#endif  // XWALK_DBUS_PROPERTY_EXPORTER_H_
