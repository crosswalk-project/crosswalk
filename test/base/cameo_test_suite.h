// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_TEST_BASE_CAMEO_TEST_SUITE_H_
#define CAMEO_TEST_BASE_CAMEO_TEST_SUITE_H_

#include <string>

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/test/content_test_suite_base.h"

namespace base {
class StatsTable;
}

class CameoTestSuite : public content::ContentTestSuiteBase {
 public:
  CameoTestSuite(int argc, char** argv);
  virtual ~CameoTestSuite();

 protected:
  virtual void Initialize() OVERRIDE;
  virtual void Shutdown() OVERRIDE;

  virtual content::ContentClient* CreateClientForInitialization() OVERRIDE;

  std::string stats_filename_;
  scoped_ptr<base::StatsTable> stats_table_;
};

#endif  // CAMEO_TEST_BASE_CAMEO_TEST_SUITE_H_
