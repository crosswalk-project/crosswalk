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

  void SetStringProperty(const std::string& property,
                         const std::string& value) {
    scoped_ptr<base::Value> v(base::Value::CreateStringValue(value));
    properties_->Set(kTestInterface, property, v.Pass());
  }

 private:
  void OnInitialized() {
    dbus_object_ =
        manager_.session_bus()->GetExportedObject(kTestObjectPath);
    properties_.reset(
        new dbus::PropertyExporter(dbus_object_, kTestObjectPath));
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
    dbus::Property<std::string> other_property;
    Properties(dbus::ObjectProxy* object_proxy,
               const PropertyChangedCallback callback)
        : dbus::PropertySet(object_proxy, kTestInterface, callback) {
      RegisterProperty("Property", &property);
      RegisterProperty("OtherProperty", &other_property);
    }
  };

  explicit GetPropertyClient(base::MessageLoop* message_loop)
      : message_loop_(message_loop),
        update_count_(0) {
    object_proxy_ = bus_->GetObjectProxy(kTestServiceName, kTestObjectPath);
    properties_.reset(
        new Properties(object_proxy_,
                       base::Bind(&GetPropertyClient::OnPropertyChanged,
                                  base::Unretained(this))));
    properties_->ConnectSignals();
  }

  void WaitForUpdates(int count) {
    while (update_count_ < count)
      message_loop_->Run();
    update_count_ -= count;
  }

  Properties* properties() { return properties_.get(); }

 private:
  void OnPropertyChanged(const std::string& property_name) {
    update_count_++;
    message_loop_->Quit();
  }

  dbus::ObjectProxy* object_proxy_;
  scoped_ptr<Properties> properties_;
  base::MessageLoop* message_loop_;
  int update_count_;
};

void CheckSuccessCallback(bool success) {
  ASSERT_TRUE(success);
}

// Get a property exported using PropertyExporter helper class.
TEST(PropertyExporterTest, Get) {
  base::MessageLoop message_loop;
  ExportObjectWithPropertiesService test_service;
  GetPropertyClient test_client(&message_loop);

  // Will run message loop until service is initialized.
  test_service.Initialize(base::Bind(&base::MessageLoop::Quit,
                                     base::Unretained(&message_loop)));
  message_loop.Run();

  test_service.SetStringProperty("Property", "Pass");

  // We didn't Get from D-Bus yet, so value won't match.
  ASSERT_NE(test_client.properties()->property.value(), "Pass");

  test_client.properties()->property.Get(base::Bind(&CheckSuccessCallback));
  test_client.WaitForUpdates(1);

  ASSERT_EQ(test_client.properties()->property.value(), "Pass");
}

// Get two properties exported.
TEST(PropertyExporterTest, GetTwo) {
  base::MessageLoop message_loop;
  ExportObjectWithPropertiesService test_service;
  GetPropertyClient test_client(&message_loop);

  // Will run message loop until service is initialized.
  test_service.Initialize(base::Bind(&base::MessageLoop::Quit,
                                     base::Unretained(&message_loop)));
  message_loop.Run();

  test_service.SetStringProperty("Property", "Pass");
  test_service.SetStringProperty("OtherProperty", "Pass");

  // We didn't Get from D-Bus yet, so values won't match.
  ASSERT_NE(test_client.properties()->property.value(), "Pass");
  ASSERT_NE(test_client.properties()->other_property.value(), "Pass");

  test_client.properties()->property.Get(base::Bind(&CheckSuccessCallback));
  test_client.properties()->other_property.Get(
      base::Bind(&CheckSuccessCallback));
  test_client.WaitForUpdates(2);

  ASSERT_EQ(test_client.properties()->property.value(), "Pass");
  ASSERT_EQ(test_client.properties()->other_property.value(), "Pass");
}

// Get a property, change it in the service and get it again.
TEST(PropertyExporterTest, GetChangeGet) {
  base::MessageLoop message_loop;
  ExportObjectWithPropertiesService test_service;
  GetPropertyClient test_client(&message_loop);

  // Will run message loop until service is initialized.
  test_service.Initialize(base::Bind(&base::MessageLoop::Quit,
                                     base::Unretained(&message_loop)));
  message_loop.Run();

  test_service.SetStringProperty("Property", "Pass 1");

  // We didn't Get from D-Bus yet, so values won't match.
  ASSERT_NE(test_client.properties()->property.value(), "Pass 1");

  // Get Pass 1.
  test_client.properties()->property.Get(base::Bind(&CheckSuccessCallback));
  test_client.WaitForUpdates(1);
  ASSERT_EQ(test_client.properties()->property.value(), "Pass 1");

  // Set Pass 2.
  test_service.SetStringProperty("Property", "Pass 2");

  // Get Pass 2.
  test_client.properties()->property.Get(base::Bind(&CheckSuccessCallback));
  test_client.WaitForUpdates(1);
  ASSERT_EQ(test_client.properties()->property.value(), "Pass 2");
}

// Test GetAll interface with multiple properties.
TEST(PropertyExporterTest, GetAll) {
  base::MessageLoop message_loop;
  ExportObjectWithPropertiesService test_service;
  GetPropertyClient test_client(&message_loop);

  // Will run message loop until service is initialized.
  test_service.Initialize(base::Bind(&base::MessageLoop::Quit,
                                     base::Unretained(&message_loop)));
  message_loop.Run();

  test_service.SetStringProperty("Property", "Pass");
  test_service.SetStringProperty("OtherProperty", "Pass");

  ASSERT_NE(test_client.properties()->property.value(), "Pass");
  ASSERT_NE(test_client.properties()->other_property.value(), "Pass");

  test_client.properties()->GetAll();
  test_client.WaitForUpdates(2);

  ASSERT_EQ(test_client.properties()->property.value(), "Pass");
  ASSERT_EQ(test_client.properties()->other_property.value(), "Pass");
}
