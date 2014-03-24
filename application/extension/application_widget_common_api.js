var errors = {
  '1': { type: 'INDEX_SIZE_ERR', name: 'IndexSizeError', message: '' },
  '2': { type: 'DOMSTRING_SIZE_ERR', name: 'DOMStringSizeError', message: '' },
  '3': { type: 'HIERARCHY_REQUEST_ERR', name: 'HierarchyRequestError', message: '' },
  '4': { type: 'WRONG_DOCUMENT_ERR', name: 'WrongDocumentError', message: '' },
  '5': { type: 'INVALID_CHARACTER_ERR', name: 'InvalidCharacterError', message: '' },
  '6': { type: 'NO_DATA_ALLOWED_ERR', name: 'NoDataAllowedError', message: '' },
  '7': { type: 'NO_MODIFICATION_ALLOWED_ERR', name: 'NoModificationAllowedError', message: '' },
  '8': { type: 'NOT_FOUND_ERR', name: 'NotFoundError', message: '' },
  '9': { type: 'NOT_SUPPORTED_ERR', name: 'Not_supportedError', message: '' },
  '10': { type: 'INUSE_ATTRIBUTE_ERR', name: 'InuseAttributeError', message: '' },
  '11': { type: 'INVALID_STATE_ERR', name: 'InvalidStateError', message: '' },
  '12': { type: 'SYNTAX_ERR', name: 'SyntaxError', message: '' },
  '13': { type: 'INVALID_MODIFICATION_ERR', name: 'InvalidModificationError', message: '' },
  '14': { type: 'NAMESPACE_ERR', name: 'NamespaceError', message: '' },
  '15': { type: 'INVALID_ACCESS_ERR', name: 'InvalidAccessError', message: '' },
  '16': { type: 'VALIDATION_ERR', name: 'ValidationError', message: '' },
  '17': { type: 'TYPE_MISMATCH_ERR', name: 'TypeMismatchError', message: '' },
  '18': { type: 'SECURITY_ERR', name: 'SecurityError', message: '' },
  '19': { type: 'NETWORK_ERR', name: 'NetworkError', message: '' },
  '20': { type: 'ABORT_ERR', name: 'AbortError', message: '' },
  '21': { type: 'URL_MISMATCH_ERR', name: 'UrlMismatchError', message: '' },
  '22': { type: 'QUOTA_EXCEEDED_ERR', name: 'QuotaExceededError', message: '' },
  '23': { type: 'TIMEOUT_ERR', name: 'TimeoutError', message: '' },
  '24': { type: 'INVALID_NODE_TYPE_ERR', name: 'InvalidNodeTypeError', message: '' },
  '25': { type: 'DATA_CLONE_ERR', name: 'DataCloneError', message: '' },
};

var CustomDOMException = function(code, message) {
  var _code, _message, _name;

  if (typeof code !== 'number') {
    throw TypeError('Wrong argument type for Exception.');
  } else if ((code in errors) === false) {
    throw TypeError('Unknown exception code: ' + code);
  } else {
    _code = code;
    _name = errors[_code].name;
    if (typeof message === 'string') {
      _message = message;
    } else {
      _message = errors[_code].message;
    }
  }

  var props = {};
  var newException;

  try {
    document.removeChild({})
  } catch (e) {
    newException = Object.create(e)
  }

  var proto = newException.__proto__;

  props = Object.getOwnPropertyDescriptor(proto, "name");
  props.value = _name;
  Object.defineProperty(newException, "name", props);

  props = Object.getOwnPropertyDescriptor(proto, "code");
  props.value = _code;
  Object.defineProperty(newException, "code", props);

  props = Object.getOwnPropertyDescriptor(proto, "message");
  props.value = _message;
  Object.defineProperty(newException, "message", props);

  props.value = function() {
    return _name + ": " + _message;
  }
  Object.defineProperty(newException, "toString", props);

  return newException;
}

for (var value in errors) {
  Object.defineProperty(CustomDOMException, errors[value].type,
      { value: parseInt(value) });
}

exports.CustomDOMException = CustomDOMException;
