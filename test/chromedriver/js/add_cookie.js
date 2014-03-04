// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
* Test whether the given domain is valid for a cookie.
*
* @param {string} domain Domain for a cookie.
* @return {boolean} True if the domain is valid, otherwise false.
*/
function isDomainValid(domain) {
  var dummyCookie = 'ChromeDriverwjers908fljsdf37459fsdfgdfwru=';

  document.cookie = dummyCookie + '; domain=' + domain;
  if (document.cookie.indexOf(dummyCookie) != -1) {
    // Expire the dummy cookie if it is added successfully.
    document.cookie = dummyCookie + '; Max-Age=0';
    return true;
  }
  return false;
}

/**
* Add the given cookie to the current web page.
*
* If path is not specified, default to '/'.
* If domain is not specified, default to document.domain, otherwise remove its
* port number.
*
* Validate name, value, domain and path of the cookie in the same way as the
* method CanonicalCookie::Create in src/net/cookies/canonical_cookie.cc. Besides
* the following requirements, name, value, domain and path of the cookie should
* not start or end with ' ' or '\t', and should not contain '\n', '\r', or '\0'.
* <ul>
* <li>name: no ';' or '='
* <li>value: no ';'
* <li>path: starts with '/', no ';'
* </ul>
*
* @param {!Object} cookie An object representing a Cookie JSON Object as
*     specified in https://code.google.com/p/selenium/wiki/JsonWireProtocol.
*/
function addCookie(cookie) {
  function isNameInvalid(value) {
    return /(^[ \t])|([;=\n\r\0])|([ \t]$)/.test(value);
  }
  function isValueInvalid(value) {
    return /(^[ \t])|([;\n\r\0])|([ \t]$)/.test(value);
  }
  function isPathInvalid(path) {
    return path[0] != '/' || /([;\n\r\0])|([ \t]$)/.test(path);
  }

  var name = cookie['name'];
  if (!name || isNameInvalid(name))
    throw new Error('name of cookie is missing or invalid:"' + name + '"');

  var value = cookie['value'] || '';
  if (isValueInvalid(value))
    throw new Error('value of cookie is invalid:"' + value + '"');

  var domain = cookie['domain'];
  // Remove the port number from domain.
  if (domain) {
    var domain_parts = domain.split(':');
    if (domain_parts.length > 2)
      throw new Error('domain of cookie has too many colons');
    else if (domain_parts.length == 2)
      domain = domain_parts[0];
  }
  // Validate domain.
  if (domain && (isValueInvalid(domain) || !isDomainValid(domain))) {
    var error = new Error();
    error.code = 24;  // Error code for InvalidCookieDomain.
    error.message = 'invalid domain:"' + domain + '"';
    throw error;
  }

  var path = cookie['path'];
  if (path && isPathInvalid(path))
    throw new Error('path of cookie is invalid:"' + path + '"');

  var newCookie = name + '=' + value;
  newCookie += '; path=' + (path || '/');
  newCookie += '; domain=' + (domain || document.domain);
  if (cookie['expiry']) {
    var expiredDate = new Date(cookie['expiry'] * 1000);
    newCookie += '; expires=' + expiredDate.toUTCString();
  }
  if (cookie['secure'])
    newCookie += '; secure';

  document.cookie = newCookie;
}
