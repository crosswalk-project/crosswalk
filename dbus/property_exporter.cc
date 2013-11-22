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
  object->ExportMethod(
      kPropertiesInterface, kPropertiesGetAll,
      base::Bind(&PropertyExporter::OnGetAll, weak_factory_.GetWeakPtr()),
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

void AppendVariantOfValue(MessageWriter* writer, const base::Value& value) {
  switch (value.GetType()) {
    case base::Value::TYPE_STRING: {
      std::string s;
      value.GetAsString(&s);
      writer->AppendVariantOfString(s);
      break;
    }
    case base::Value::TYPE_INTEGER: {
      int n;
      value.GetAsInteger(&n);
      writer->AppendVariantOfInt32(n);
      break;
    }
    default:
      LOG(ERROR) << "Unsupported base::Value when converting to DBus VARIANT.";
  }
}

scoped_ptr<Response> CreateParseError(MethodCall* method_call) {
  scoped_ptr<ErrorResponse> error_response = ErrorResponse::FromMethodCall(
      method_call, kErrorName, "Error parsing arguments.");
  return error_response.PassAs<Response>();
}

scoped_ptr<Response> CreateInterfaceNotFoundError(
    MethodCall* method_call, const std::string& interface,
    const dbus::ObjectPath& path) {
  scoped_ptr<ErrorResponse> error_response = ErrorResponse::FromMethodCall(
      method_call, kErrorName,
      "Interface '" + interface + "' not found for object '"
      + path.value() + "'.");
  return error_response.PassAs<Response>();
}

}  // namespace

void PropertyExporter::AppendPropertiesToWriter(const std::string& interface,
                                                MessageWriter* writer) {
  InterfacesMap::iterator it = interfaces_.find(interface);
  if (it == interfaces_.end())
    return;

  MessageWriter dict_writer(NULL);
  writer->OpenArray("{sv}", &dict_writer);

  for (base::DictionaryValue::Iterator dict_it(*it->second);
       !dict_it.IsAtEnd();
       dict_it.Advance()) {
    MessageWriter entry_writer(NULL);
    dict_writer.OpenDictEntry(&entry_writer);
    entry_writer.AppendString(dict_it.key());
    AppendVariantOfValue(&entry_writer, dict_it.value());
    dict_writer.CloseContainer(&entry_writer);
  }

  writer->CloseContainer(&dict_writer);
}

void PropertyExporter::OnGet(
    MethodCall* method_call, ExportedObject::ResponseSender response_sender) {
  MessageReader reader(method_call);
  std::string interface;
  std::string property;
  if (!reader.PopString(&interface) || !reader.PopString(&property)) {
    scoped_ptr<Response> error_response = CreateParseError(method_call);
    response_sender.Run(error_response.Pass());
    return;
  }

  InterfacesMap::const_iterator it = interfaces_.find(interface);
  if (it == interfaces_.end()) {
    scoped_ptr<Response> error_response =
        CreateInterfaceNotFoundError(method_call, interface, path_);
    response_sender.Run(error_response.Pass());
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
  AppendVariantOfValue(&writer, *value);
  response_sender.Run(response.Pass());
}

void PropertyExporter::OnGetAll(
    MethodCall* method_call, ExportedObject::ResponseSender response_sender) {
  MessageReader reader(method_call);
  std::string interface;
  if (!reader.PopString(&interface)) {
    scoped_ptr<Response> error_response = CreateParseError(method_call);
    response_sender.Run(error_response.Pass());
    return;
  }

  InterfacesMap::const_iterator it = interfaces_.find(interface);
  if (it == interfaces_.end()) {
    scoped_ptr<Response> error_response =
        CreateInterfaceNotFoundError(method_call, interface, path_);
    response_sender.Run(error_response.Pass());
    return;
  }

  scoped_ptr<Response> response = Response::FromMethodCall(method_call);
  MessageWriter writer(response.get());
  AppendPropertiesToWriter(interface, &writer);
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
