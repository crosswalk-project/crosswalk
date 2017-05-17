// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function getElementRegion(element) {
  // Check that node type is element.
  if (element.nodeType != 1)
    throw new Error(element + ' is not an element');

  // We try 2 methods to determine element region. Try the first client rect,
  // and then the bounding client rect.
  // SVG is one case that doesn't have a first client rect.
  var clientRects = element.getClientRects();
  if (clientRects.length == 0) {
    var box = element.getBoundingClientRect();
    if (element.tagName.toLowerCase() == 'area') {
      var coords = element.coords.split(',');
      if (element.shape.toLowerCase() == 'rect') {
        if (coords.length != 4)
          throw new Error('failed to detect the region of the area');
        var leftX = Number(coords[0]);
        var topY = Number(coords[1]);
        var rightX = Number(coords[2]);
        var bottomY = Number(coords[3]);
        return {
            'left': leftX,
            'top': topY,
            'width': rightX - leftX,
            'height': bottomY - topY
        };
      } else if (element.shape.toLowerCase() == 'circle') {
        if (coords.length != 3)
          throw new Error('failed to detect the region of the area');
        var centerX = Number(coords[0]);
        var centerY = Number(coords[1]);
        var radius = Number(coords[2]);
        return {
            'left': Math.max(0, centerX - radius),
            'top': Math.max(0, centerY - radius),
            'width': radius * 2,
            'height': radius * 2
        };
      } else if (element.shape.toLowerCase() == 'poly') {
        if (coords.length < 2)
          throw new Error('failed to detect the region of the area');
        var minX = Number(coords[0]);
        var minY = Number(coords[1]);
        var maxX = minX;
        var maxY = minY;
        for (i = 2; i < coords.length; i += 2) {
          var x = Number(coords[i]);
          var y = Number(coords[i + 1]);
          minX = Math.min(minX, x);
          minY = Math.min(minY, y);
          maxX = Math.max(maxX, x);
          maxY = Math.max(maxY, y);
        }
        return {
            'left': minX,
            'top': minY,
            'width': maxX - minX,
            'height': maxY - minY
        };
      } else {
        throw new Error('shape=' + element.shape + ' is not supported');
      }
    }
    return {
        'left': 0,
        'top': 0,
        'width': box.width,
        'height': box.height
    };
  } else {
    var clientRect = clientRects[0];
    var box = element.getBoundingClientRect();
    return {
        'left': clientRect.left - box.left,
        'top': clientRect.top - box.top,
        'width': clientRect.right - clientRect.left,
        'height': clientRect.bottom - clientRect.top
    };
  }
}
