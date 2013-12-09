// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_DBUS_OBJECT_MANAGER_ADAPTOR_H_
#define XWALK_DBUS_OBJECT_MANAGER_ADAPTOR_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "dbus/exported_object.h"
#include "xwalk/dbus/property_exporter.h"

namespace dbus {

class ManagedObject;

// Implementation of the org.freedesktop.DBus.ObjectManager interface. Users
// should add and remove managed objects through this object, which will emit
// the appropriate InterfacesAdded and InterfacesRemoved signals.
//
// It takes care of creating the manager object, so if more interfaces will be
// exported at that path, use manager_object().
class ObjectManagerAdaptor {
 public:
  ObjectManagerAdaptor(scoped_refptr<Bus> bus, const ObjectPath& manager_path);
  virtual ~ObjectManagerAdaptor();

  void AddManagedObject(scoped_ptr<ManagedObject> object);
  void RemoveManagedObject(const ObjectPath& path);
  ManagedObject* GetManagedObject(const ObjectPath& path);

  ExportedObject* manager_object() const { return manager_object_; }
  Bus* bus() const { return bus_; }

 private:
  void OnGetManagedObjects(
      MethodCall* method_call, ExportedObject::ResponseSender response_sender);

  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  void RemoveAllManagedObjects();
  void EmitInterfacesAdded(const ManagedObject* object);
  void EmitInterfacesRemoved(const ManagedObject* object);

  base::WeakPtrFactory<ObjectManagerAdaptor> weak_factory_;

  // TODO(cmarcelo): Exported object already contains path, so would be nice if
  // we could make a public method exposing it and do not store it here.
  ObjectPath manager_path_;
  ExportedObject* manager_object_;
  scoped_refptr<Bus> bus_;

  ScopedVector<ManagedObject> managed_objects_;
  bool is_exported_;

  DISALLOW_COPY_AND_ASSIGN(ObjectManagerAdaptor);
};

// Base class for objects managed by the ObjectManagerAdaptor. This object will
// be destroyed when removed from the manager or when the manager is destroyed.
class ManagedObject {
 public:
  ManagedObject(scoped_refptr<Bus> bus, const ObjectPath& path);
  virtual ~ManagedObject();

  ObjectPath path() const;
  void AppendAllPropertiesToWriter(MessageWriter* writer) const;
  void AppendInterfacesToWriter(MessageWriter* writer) const;

  ExportedObject* dbus_object() { return dbus_object_; }
  PropertyExporter* properties() { return &properties_; }

 private:
  // TODO(cmarcelo): Exported object already contains path, so would be nice if
  // we could make a public method exposing it and do not store it here.
  ObjectPath path_;
  ExportedObject* dbus_object_;
  PropertyExporter properties_;

  DISALLOW_COPY_AND_ASSIGN(ManagedObject);
};

}  // namespace dbus

#endif  // XWALK_DBUS_OBJECT_MANAGER_ADAPTOR_H_
