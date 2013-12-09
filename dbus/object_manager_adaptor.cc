// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/object_manager_adaptor.h"

#include <vector>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/message.h"

namespace {

const char kDBusObjectManagerInterface[] = "org.freedesktop.DBus.ObjectManager";

}  // namespace

namespace dbus {

ObjectManagerAdaptor::ObjectManagerAdaptor(scoped_refptr<Bus> bus,
                                           const ObjectPath& manager_path)
    : weak_factory_(this),
      manager_path_(manager_path),
      manager_object_(bus->GetExportedObject(manager_path)),
      bus_(bus),
      is_exported_(false) {
  manager_object_->ExportMethod(
      kDBusObjectManagerInterface, "GetManagedObjects",
      base::Bind(&ObjectManagerAdaptor::OnGetManagedObjects,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&ObjectManagerAdaptor::OnExported,
                 weak_factory_.GetWeakPtr()));
}

ObjectManagerAdaptor::~ObjectManagerAdaptor() {
  RemoveAllManagedObjects();
  bus_->UnregisterExportedObject(manager_path_);
}

void ObjectManagerAdaptor::AddManagedObject(scoped_ptr<ManagedObject> object) {
  EmitInterfacesAdded(object.get());
  managed_objects_.push_back(object.release());
}

namespace {

struct MatchObjectPath {
 public:
  explicit MatchObjectPath(const ObjectPath& path) : path_(path) {}
  bool operator()(const ManagedObject* object) {
    return path_ == object->path();
  }
  const ObjectPath path_;
};

}  // namespace

void ObjectManagerAdaptor::RemoveManagedObject(const ObjectPath& path) {
  ScopedVector<ManagedObject>::iterator it = std::find_if(
      managed_objects_.begin(), managed_objects_.end(), MatchObjectPath(path));
  if (it == managed_objects_.end()) {
    LOG(WARNING) << "Error removing unmanaged object with path: "
                 << path.value();
    return;
  }

  EmitInterfacesRemoved(*it);

  // We need to explicitly unregister the exported object, otherwise the Bus
  // would keep it alive.
  bus_->UnregisterExportedObject(path);

  // Since this is a ScopedVector, erasing will actually destroy the value.
  managed_objects_.erase(it);
}

ManagedObject* ObjectManagerAdaptor::GetManagedObject(const ObjectPath& path) {
  ScopedVector<ManagedObject>::iterator it = std::find_if(
      managed_objects_.begin(), managed_objects_.end(), MatchObjectPath(path));
  if (it == managed_objects_.end())
    return NULL;
  return *it;
}

void ObjectManagerAdaptor::OnGetManagedObjects(
    MethodCall* method_call,
    ExportedObject::ResponseSender response_sender) {
  scoped_ptr<Response> response =
      Response::FromMethodCall(method_call);
  MessageWriter writer(response.get());

  MessageWriter dict_writer(NULL);
  writer.OpenArray("{oa{sa{sv}}}", &dict_writer);

  ScopedVector<ManagedObject>::const_iterator it;
  for (it = managed_objects_.begin(); it != managed_objects_.end(); ++it) {
    ManagedObject* object = *it;
    MessageWriter entry_writer(NULL);

    dict_writer.OpenDictEntry(&entry_writer);
    entry_writer.AppendObjectPath(object->path());
    object->AppendAllPropertiesToWriter(&entry_writer);
    dict_writer.CloseContainer(&entry_writer);
  }

  writer.CloseContainer(&dict_writer);
  response_sender.Run(response.Pass());
}

void ObjectManagerAdaptor::OnExported(const std::string& interface_name,
                               const std::string& method_name,
                               bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << manager_path_.value() << "'.";
    return;
  }

  is_exported_ = true;
}

void ObjectManagerAdaptor::RemoveAllManagedObjects() {
  ScopedVector<ManagedObject>::iterator it = managed_objects_.begin();
  for (; it != managed_objects_.end(); ++it) {
    EmitInterfacesRemoved(*it);
    bus_->UnregisterExportedObject((*it)->path());
  }

  // Since this is a ScopedVector, clearing will destroy all values.
  managed_objects_.clear();
}

void ObjectManagerAdaptor::EmitInterfacesAdded(const ManagedObject* object) {
  // We only start sending signals after the GetManagedObjects() was exported.
  // This allow us to bootstrap the list of managed objects right after
  // construction of the adaptor without sending unnecessary signals.
  if (!is_exported_)
    return;

  Signal interfaces_added(kDBusObjectManagerInterface, "InterfacesAdded");
  MessageWriter writer(&interfaces_added);

  writer.AppendObjectPath(object->path());
  object->AppendAllPropertiesToWriter(&writer);
  manager_object_->SendSignal(&interfaces_added);
}

void ObjectManagerAdaptor::EmitInterfacesRemoved(const ManagedObject* object) {
  // See comment in EmitInterfacesAdded().
  if (!is_exported_)
    return;

  Signal interfaces_removed(kDBusObjectManagerInterface, "InterfacesRemoved");
  MessageWriter writer(&interfaces_removed);

  writer.AppendObjectPath(object->path());
  object->AppendInterfacesToWriter(&writer);
  manager_object_->SendSignal(&interfaces_removed);
}

ManagedObject::ManagedObject(scoped_refptr<Bus> bus, const ObjectPath& path)
    : dbus_object_(bus->GetExportedObject(path)),
      path_(path),
      properties_(dbus_object_, path_) {}

ManagedObject::~ManagedObject() {}

ObjectPath ManagedObject::path() const {
  return path_;
};

void ManagedObject::AppendAllPropertiesToWriter(MessageWriter* writer) const {
  MessageWriter interfaces_writer(NULL);
  writer->OpenArray("{sa{sv}}", &interfaces_writer);

  std::vector<std::string> interfaces = properties_.interfaces();
  for (int i = 0; i < interfaces.size(); i++) {
    MessageWriter entry_writer(NULL);
    interfaces_writer.OpenDictEntry(&entry_writer);
    entry_writer.AppendString(interfaces[i]);
    properties_.AppendPropertiesToWriter(interfaces[i], &entry_writer);
    interfaces_writer.CloseContainer(&entry_writer);
  }

  writer->CloseContainer(&interfaces_writer);
}

void ManagedObject::AppendInterfacesToWriter(MessageWriter* writer) const {
  writer->AppendArrayOfStrings(properties_.interfaces());
}

}  // namespace dbus
