// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/geolocation_override_manager.h"
#include "chrome/test/chromedriver/chrome/geoposition.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/stub_devtools_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct Command {
  Command() {}
  Command(const std::string& method, const base::DictionaryValue& params)
      : method(method) {
    this->params.MergeDictionary(&params);
  }
  Command(const Command& command) {
    *this = command;
  }
  Command& operator=(const Command& command) {
    method = command.method;
    params.Clear();
    params.MergeDictionary(&command.params);
    return *this;
  }
  ~Command() {}

  std::string method;
  base::DictionaryValue params;
};

class RecorderDevToolsClient : public StubDevToolsClient {
 public:
  RecorderDevToolsClient() {}
  virtual ~RecorderDevToolsClient() {}

  // Overridden from StubDevToolsClient:
  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE {
    commands_.push_back(Command(method, params));
    return Status(kOk);
  }

  std::vector<Command> commands_;
};

void AssertGeolocationCommand(const Command& command,
                              const Geoposition& geoposition) {
  ASSERT_EQ("Page.setGeolocationOverride", command.method);
  double latitude, longitude, accuracy;
  ASSERT_TRUE(command.params.GetDouble("latitude", &latitude));
  ASSERT_TRUE(command.params.GetDouble("longitude", &longitude));
  ASSERT_TRUE(command.params.GetDouble("accuracy", &accuracy));
  ASSERT_EQ(geoposition.latitude, latitude);
  ASSERT_EQ(geoposition.longitude, longitude);
  ASSERT_EQ(geoposition.accuracy, accuracy);
}

}  // namespace

TEST(GeolocationOverrideManager, OverrideSendsCommand) {
  RecorderDevToolsClient client;
  GeolocationOverrideManager manager(&client);
  Geoposition geoposition = {1, 2, 3};
  manager.OverrideGeolocation(geoposition);
  ASSERT_EQ(1u, client.commands_.size());
  ASSERT_NO_FATAL_FAILURE(
     AssertGeolocationCommand(client.commands_[0], geoposition));

  geoposition.latitude = 5;
  manager.OverrideGeolocation(geoposition);
  ASSERT_EQ(2u, client.commands_.size());
  ASSERT_NO_FATAL_FAILURE(
      AssertGeolocationCommand(client.commands_[1], geoposition));
}

TEST(GeolocationOverrideManager, SendsCommandOnConnect) {
  RecorderDevToolsClient client;
  GeolocationOverrideManager manager(&client);
  ASSERT_EQ(0u, client.commands_.size());
  ASSERT_EQ(kOk, manager.OnConnected(&client).code());

  Geoposition geoposition = {1, 2, 3};
  manager.OverrideGeolocation(geoposition);
  ASSERT_EQ(1u, client.commands_.size());
  ASSERT_EQ(kOk, manager.OnConnected(&client).code());
  ASSERT_EQ(2u, client.commands_.size());
  ASSERT_NO_FATAL_FAILURE(
      AssertGeolocationCommand(client.commands_[1], geoposition));
}

TEST(GeolocationOverrideManager, SendsCommandOnNavigation) {
  RecorderDevToolsClient client;
  GeolocationOverrideManager manager(&client);
  base::DictionaryValue main_frame_params;
  ASSERT_EQ(kOk,
            manager.OnEvent(&client, "Page.frameNavigated", main_frame_params)
                .code());
  ASSERT_EQ(0u, client.commands_.size());

  Geoposition geoposition = {1, 2, 3};
  manager.OverrideGeolocation(geoposition);
  ASSERT_EQ(1u, client.commands_.size());
  ASSERT_EQ(kOk,
            manager.OnEvent(&client, "Page.frameNavigated", main_frame_params)
                .code());
  ASSERT_EQ(2u, client.commands_.size());
  ASSERT_NO_FATAL_FAILURE(
      AssertGeolocationCommand(client.commands_[1], geoposition));

  base::DictionaryValue sub_frame_params;
  sub_frame_params.SetString("frame.parentId", "id");
  ASSERT_EQ(
      kOk,
      manager.OnEvent(&client, "Page.frameNavigated", sub_frame_params).code());
  ASSERT_EQ(2u, client.commands_.size());
}
