{
  'rules': [
    {
      'rule_name': 'xwalk_js2c',
      'extension': 'js',
      'msvs_external_rule': 1,
      'inputs': [
        'tools/generate_api.py',
      ],
      'outputs': [
        '<(SHARED_INTERMEDIATE_DIR)/<(RULE_INPUT_ROOT).cc'
      ],
      'process_outputs_as_sources': 1,
      'action': [
        'python',
        '<@(_inputs)',
        '<(RULE_INPUT_PATH)',
        'kSource_<(RULE_INPUT_ROOT)',
        '<@(_outputs)',
      ],
      'message': 'Generating code from <(RULE_INPUT_PATH)',
    },
  ],
}
