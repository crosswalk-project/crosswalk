# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is meant to be included into a target to provide a rule to generate
# a Maven POM XML file that can later be used in Maven repositories.
# It takes a template POM and substitutes the values @ARTIFACT_ID@ and
# @ARTIFACT_VERSION@ with the variables provided to this file.
#
# Usage:
# {
#   'target_name': 'my_artifact_pom_gen',
#   'type': 'none',
#   'variables': {
#     'pom_input': 'path/to/my_artifact.pom.xml.in',
#     'pom_output': '<(PRODUCT_DIR)/my_artifact.pom.xml',
#     'artifact_id': 'foo_bar_canary',
#     'artifact_version': '1.2.3.4',
#   }
#   'includes': ['path/to/this/gypi/file'],
# }
#
# Required variables:
#  pom_input - Path to the POM file that will be processed.
#  pom_output - Path to the generated POM file.
#  artifact_id - Value for the <artifactId> field in the POM file.
#  artifact_version - Value for the <version> field in the POM file.

{
  'actions': [
    {
      'action_name': 'generate_pom_file',
      'message': 'Generating <(pom_output)',
      'variables': {
        'version_py': '<(DEPTH)/build/util/version.py',
      },
      'inputs': [
        '<(version_py)',
        '<(pom_input)',
      ],
      'outputs': [
        '<(pom_output)',
      ],
      'action': [
        'python', '<(version_py)', '-i', '<(pom_input)',
                  '-o', '<(pom_output)',
                  '-e', 'ARTIFACT_ID="<(artifact_id)"',
                  '-e', 'ARTIFACT_VERSION="<(artifact_version)"',
      ]
    },
  ],
}
