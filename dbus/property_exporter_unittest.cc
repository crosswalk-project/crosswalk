// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/property_exporter.h"

#include "base/bind.h"
#include "base/run_loop.h"
#include "base/values.h"
#include "dbus/bus.h"
#include "dbus/property.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/dbus/dbus_manager.h"
#include "xwalk/dbus/test_client.h"

const char kTestServiceName[] = "org.crosswalkproject.test_properties";
const char kTestInterface[] = "org.crosswalkproject.test_properties.Interface";
const char kTestProperty[] = "Property";
const dbus::ObjectPath kTestObjectPath("/object_with_properties");

class ExportObjectWithPropertiesService {
 public:
  ExportObjectWithPropertiesService()
      : manager_(kTestServiceName),
        dbus_object_(NULL) {}

  void Initialize(const base::Closure& on_initialized_callback) {
    on_initialized_callback_ = on_initialized_callback;
    manager_.Initialize(
        base::Bind(&ExportObjectWithPropertiesService::OnInitialized,
                   base::Unretained(this)));
  }

 private:
  void OnInitialized() {
    dbus_object_ =
        manager_.session_bus()->GetExportedObject(kTestObjectPath);
    properties_.reset(
        new dbus::PropertyExporter(dbus_object_, kTestObjectPath));
    scoped_ptr<base::Value> value(base::Value::CreateStringValue("Pass"));
    properties_->Set(kTestInterface, kTestProperty, value.Pass());
    on_initialized_callback_.Run();
  }

  xwalk::DBusManager manager_;
  dbus::ExportedObject* dbus_object_;
  scoped_ptr<dbus::PropertyExporter> properties_;
  base::Closure on_initialized_callback_;
};

// Use dbus/property.h to access the property exposed above.
class GetPropertyClient : public dbus::TestClient {
 public:
  struct Properties : public dbus::PropertySet {
    dbus::Property<std::string> property;
    Properties(dbus::ObjectProxy* object_proxy,
               const PropertyChangedCallback callback)
        : dbus::PropertySet(object_proxy, kTestInterface, callback) {
      RegisterProperty(kTestProperty, &property);
    }
  };

  explicit GetPropertyClient(const base::Closure& on_get_property_callback)
      : on_get_property_callback_(on_get_property_callback) {
    object_proxy_ = bus_->GetObjectProxy(kTestServiceName, kTestObjectPath);
    properties_.reset(
        new Properties(object_proxy_,
                       base::Bind(&GetPropertyClient::OnPropertyChanged,
                                  base::Unretained(this))));
    properties_->ConnectSignals();
  }

  void GetProperty() {
    properties_->property.Get(
        base::Bind(&GetPropertyClient::GetCallback, base::Unretained(this)));
  }

  std::string result() const { return result_; }

 private:
  void OnPropertyChanged(const std::string& property_name) {}
  void GetCallback(bool success) {
    if (success)
      result_ = properties_->property.value();
    on_get_property_callback_.Run();
  }

  dbus::ObjectProxy* object_proxy_;
  scoped_ptr<Properties> properties_;
  base::Closure on_get_property_callback_;
  std::string result_;
};

// Get a property exported using PropertyExporter helper class.
TEST(PropertyExporterTest, Simple) {
  base::MessageLoop message_loop;
  ExportObjectWithPropertiesService test_service;

  base::RunLoop run_loop;
  GetPropertyClient test_client(run_loop.QuitClosure());

  // After initialize we call GetProperty in the client that will try to get a
  // property and then run the QuitClosure() passed above, exiting the run loop.
  test_service.Initialize(base::Bind(&GetPropertyClient::GetProperty,
                                     base::Unretained(&test_client)));
  run_loop.Run();

  ASSERT_EQ(test_client.result(), "Pass");
}
