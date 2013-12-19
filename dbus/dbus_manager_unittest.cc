// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"
#include "dbus/object_proxy.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/dbus/dbus_manager.h"
#include "xwalk/dbus/test_client.h"

using xwalk::DBusManager;

const char kTestServiceName[] = "org.crosswalkproject.test_service1";
const char kTestInterface[] = "org.crosswalkproject.test_service.Interface1";
const char kTestMethod[] = "Method";
const dbus::ObjectPath kTestObjectPath("/test/path1");

void EmptyCallback() {}

// Uses a DBusManager to expose an object with a method. Provide hooks (on_*
// callbacks), so that calling code can track whether the object was exposed and
// the method was called.
class ExportObjectService {
 public:
  ExportObjectService()
      : on_exported(base::Bind(EmptyCallback)),
        on_method_called(base::Bind(EmptyCallback)),
        manager_(),
        dbus_object_(NULL) {}

  void Initialize() {
    scoped_refptr<dbus::Bus> bus = manager_.session_bus();
    dbus_object_ = bus->GetExportedObject(kTestObjectPath);
    dbus_object_->ExportMethod(
        kTestInterface, kTestMethod,
        base::Bind(&ExportObjectService::OnMethodCalled,
                   base::Unretained(this)),
        base::Bind(&ExportObjectService::OnExported,
                   base::Unretained(this)));
    bus->RequestOwnership(
        kTestServiceName,
        dbus::Bus::REQUIRE_PRIMARY,
        base::Bind(&ExportObjectService::OnOwnershipCallback,
                   base::Unretained(this)));
  }

  base::Closure on_exported;
  base::Closure on_method_called;

 private:
  void OnOwnershipCallback(const std::string& service_name, bool success) {
    ASSERT_TRUE(success)
        << "Couldn't get ownership of D-Bus service name:" << service_name;
    on_exported.Run();
  }

  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success) {
    ASSERT_TRUE(success);
  }

  void OnMethodCalled(dbus::MethodCall* method_call,
                      dbus::ExportedObject::ResponseSender response_sender) {
    on_method_called.Run();
  }

  DBusManager manager_;
  dbus::ExportedObject* dbus_object_;
};

// Test whether we can expose an object using DBusManager.
TEST(DBusManagerTest, ExportObject) {
  base::MessageLoop message_loop;
  base::RunLoop run_loop;
  ExportObjectService test_service;
  test_service.on_exported = base::Bind(run_loop.QuitClosure());
  test_service.Initialize();
  run_loop.Run();
}

// Call the method exposed by ExportObjectService.
class CallMethodClient : public dbus::TestClient {
 public:
  void CallTestMethod() {
    dbus::ObjectProxy* object_proxy =
        bus_->GetObjectProxy(kTestServiceName, kTestObjectPath);

    dbus::MethodCall method_call(kTestInterface, kTestMethod);
    object_proxy->CallMethod(&method_call,
                             dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                             dbus::ObjectProxy::EmptyResponseCallback());
  }
};

// Tests whether we can call a method exported by a bus created in DBusManager.
TEST(DBusManagerTest, CallMethod) {
  base::MessageLoop message_loop;
  base::RunLoop run_loop;

  CallMethodClient test_client;
  ExportObjectService test_service;
  test_service.on_method_called = base::Bind(run_loop.QuitClosure());
  test_service.on_exported = base::Bind(&CallMethodClient::CallTestMethod,
                                        base::Unretained(&test_client));
  test_service.Initialize();

  run_loop.Run();
}
