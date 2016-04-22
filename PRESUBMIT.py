# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Copyright (c) 2016 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit checks for Crosswalk.

These checks are performed automatically by our Trybots when a patch is sent
for review. Most of them come from Chromium's own PRESUBMIT.py script.
"""

import cpplint
import errno
import os
import sys


_LICENSE_HEADER_RE = (
  r'(.*? Copyright (\(c\) )?2[\d]{3}.+\n)+'
  r'.*? Use of this source code is governed by a BSD-style license that '
  r'can be\n'
  r'.*? found in the LICENSE file\.(?: \*/)?\n'
)


def _CheckChangeLintsClean(input_api, output_api):
  class PrefixedFileInfo(cpplint.FileInfo):
    def RepositoryName(self):
      fullname = self.FullName()
      repo_pos = fullname.find('xwalk/')
      if repo_pos == -1:
        # Something weird happened, bail out.
        return [output_api.PresubmitError(
            'Cannot find "xwalk/" in %s.' % fullname)]
      return fullname[repo_pos:]
  input_api.cpplint.FileInfo = PrefixedFileInfo
  source_filter = lambda filename: input_api.FilterSourceFile(
    filename, white_list=(r'.+\.(cc|h)$',))
  return input_api.canned_checks.CheckChangeLintsClean(
    input_api, output_api, source_filter)


def CheckChangeOnUpload(input_api, output_api):
  # We need to prepend to sys.path, otherwise we may import depot_tools's own
  # PRESUBMIT.py.
  sys.path = [os.path.dirname(input_api.PresubmitLocalPath())] + sys.path
  import PRESUBMIT as cr

  results = []
  results.extend(_CheckChangeLintsClean(input_api, output_api))
  results.extend(cr._CheckNoIOStreamInHeaders(input_api, output_api))
  results.extend(cr._CheckNoUNIT_TESTInSourceFiles(input_api, output_api))
  results.extend(cr._CheckDCHECK_IS_ONHasBraces(input_api, output_api))
  results.extend(cr._CheckNoNewWStrings(input_api, output_api))
  results.extend(cr._CheckNoPragmaOnce(input_api, output_api))
  results.extend(cr._CheckNoTrinaryTrueFalse(input_api, output_api))
  results.extend(
    cr._CheckNoAuraWindowPropertyHInHeaders(input_api, output_api))
  results.extend(cr._CheckForVersionControlConflicts(input_api, output_api))
  results.extend(cr._CheckPatchFiles(input_api, output_api))
  results.extend(cr._CheckNoAbbreviationInPngFileName(input_api, output_api))
  results.extend(cr._CheckForInvalidOSMacros(input_api, output_api))
  results.extend(cr._CheckForInvalidIfDefinedMacros(input_api, output_api))
  results.extend(cr._CheckNoDeprecatedCSS(input_api, output_api))
  results.extend(cr._CheckSingletonInHeaders(input_api, output_api))
  results.extend(
    input_api.canned_checks.CheckGNFormatted(input_api, output_api))

  # The following checks should be enabled only after we fix all violations.
  # results.extend(input_api.canned_checks.PanProjectChecks(
  #   input_api, output_api, license_header=_LICENSE_HEADER_RE,
  #   project_name='Crosswalk', owners_check=False))
  # results.extend(
  #   input_api.canned_checks.CheckPatchFormatted(input_api, output_api))
  # results.extend(
  #   input_api.canned_checks.CheckChangeHasOnlyOneEol(input_api, output_api))
  # results.extend(
  #   input_api.canned_checks.CheckChangeTodoHasOwner(input_api, output_api))
  # results.extend(
  #   cr._CheckNoProductionCodeUsingTestOnlyFunctions(input_api, output_api))
  # results.extend(cr._CheckNoBannedFunctions(input_api, output_api))
  # results.extend(cr._CheckIncludeOrder(input_api, output_api))
  results.extend(
    input_api.canned_checks.CheckChangeHasNoTabs(
      input_api,
      output_api,
      source_file_filter=lambda x: x.LocalPath().endswith('.grd')))
  # results.extend(cr._CheckSpamLogging(input_api, output_api))
  # results.extend(cr._CheckNoDeprecatedJS(input_api, output_api))
  # results.extend(cr._CheckForIPCRules(input_api, output_api))
  results.extend(cr._CheckForWindowsLineEndings(input_api, output_api))
  # results.extend(cr._AndroidSpecificOnUploadChecks(input_api, output_api))

  # Some checks input_api.PresubmitLocalPath() returns Chromium's root
  # directory, so we need to fake it.
  input_api._current_presubmit_path = os.path.dirname(
    input_api.PresubmitLocalPath())
  results.extend(cr._CheckParseErrors(input_api, output_api))
  results.extend(cr._CheckFilePermissions(input_api, output_api))
  # Our DEPS rules need to be adjusted before we can enable this check.
  # results.extend(cr._CheckUnwantedDependencies(input_api, output_api))

  return results


# We do not use Chromium's commit queue, so the checks for mode uploading and
# committing should be the same.
CheckChangeOnCommit = CheckChangeOnUpload
