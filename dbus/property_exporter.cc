// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/property_exporter.h"

#include <string>
#include "base/bind.h"
#include "base/stl_util.h"
#include "base/values.h"
#include "dbus/message.h"
#include "dbus/property.h"

namespace {

const char kErrorName[] = "org.freedesktop.DBus.Properties.Error";

}  // namespace

namespace dbus {

PropertyExporter::PropertyExporter(ExportedObject* object,
                                   const ObjectPath& path)
    : path_(path),
      weak_factory_(this) {
  CHECK(object);
  object->ExportMethod(
      kPropertiesInterface, kPropertiesGet,
      base::Bind(&PropertyExporter::OnGet, weak_factory_.GetWeakPtr()),
      base::Bind(&PropertyExporter::OnExported, weak_factory_.GetWeakPtr()));
}

PropertyExporter::~PropertyExporter() {
  STLDeleteValues(&interfaces_);
}

void PropertyExporter::Set(const std::string& interface,
                           const std::string& property,
                           scoped_ptr<base::Value> value) {
  // TODO(cmarcelo): Support more types as we need to use them.
  if (!value->IsType(base::Value::TYPE_STRING)
      && !value->IsType(base::Value::TYPE_INTEGER)) {
    LOG(ERROR) << "PropertyExporter can only can "
               << "export String and Integer properties";
    return;
  }

  InterfacesMap::iterator it = interfaces_.find(interface);
  base::DictionaryValue* dict;
  if (it != interfaces_.end()) {
    dict = it->second;
  } else {
    dict = new DictionaryValue;
    interfaces_[interface] = dict;
  }

  dict->Set(property, value.release());

  // TODO(cmarcelo): Emit PropertyChanged signal.
}

namespace {

void AppendVariantOfValue(MessageWriter* writer, const base::Value* value) {
  switch (value->GetType()) {
    case base::Value::TYPE_STRING: {
      std::string s;
      value->GetAsString(&s);
      writer->AppendVariantOfString(s);
      break;
    }
    case base::Value::TYPE_INTEGER: {
      int n;
      value->GetAsInteger(&n);
      writer->AppendVariantOfInt32(n);
      break;
    }
    default:
      LOG(ERROR) << "Unsupported base::Value when converting to DBus VARIANT.";
  }
}

}  // namespace

void PropertyExporter::OnGet(
    MethodCall* method_call, ExportedObject::ResponseSender response_sender) {
  MessageReader reader(method_call);
  std::string interface;
  std::string property;
  if (!reader.PopString(&interface) || !reader.PopString(&property)) {
    scoped_ptr<ErrorResponse> error_response = ErrorResponse::FromMethodCall(
        method_call, kErrorName, "Error parsing arguments.");
    response_sender.Run(error_response.PassAs<Response>());
    return;
  }

  InterfacesMap::const_iterator it = interfaces_.find(interface);
  if (it == interfaces_.end()) {
    scoped_ptr<ErrorResponse> error_response = ErrorResponse::FromMethodCall(
        method_call, kErrorName,
        "Interface '" + interface + "' not found for object '"
        + path_.value() + "'.");
    response_sender.Run(error_response.PassAs<Response>());
    return;
  }

  const DictionaryValue* dict = it->second;
  const base::Value* value = NULL;
  if (!dict->Get(property, &value)) {
    scoped_ptr<ErrorResponse> error_response = ErrorResponse::FromMethodCall(
        method_call, kErrorName,
        "Property '" + property + "' of interface '" + interface
        + "' not found for object '" + path_.value() + "'.");
    response_sender.Run(error_response.PassAs<Response>());
    return;
  }

  scoped_ptr<Response> response = Response::FromMethodCall(method_call);
  MessageWriter writer(response.get());
  AppendVariantOfValue(&writer, value);
  response_sender.Run(response.Pass());
}

void PropertyExporter::OnExported(const std::string& interface_name,
                                  const std::string& method_name,
                                  bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in object '"
                 << path_.value() << "'.";
  }
}

}  // namespace dbus
