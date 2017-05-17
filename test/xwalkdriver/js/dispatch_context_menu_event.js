// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Enum for modifier keys, same as DevTools protocol.
 * @enum {number}
 */
var ModifierMask = {
  ALT: 1 << 0,
  CTRL: 1 << 1,
  META: 1 << 2,
  SHIFT: 1 << 3,
};

/**
 * Dispatches a context menu event at the given location.
 *
 * @param {number} x The X coordinate to dispatch the event at.
 * @param {number} y The Y coordinate to dispatch the event at.
 * @param {modifiers} modifiers The modifiers to use for the event.
 */
function dispatchContextMenuEvent(x, y, modifiers) {
  var event = new MouseEvent(
      'contextmenu',
      {view: window,
       bubbles: true,
       cancelable: true,
       screenX: x,
       screenY: y,
       clientX: x,
       clientY: y,
       ctrlKey: modifiers & ModifierMask.CTRL,
       altKey: modifiers & ModifierMask.ALT,
       shiftKey: modifiers & ModifierMask.SHIFT,
       metaKey: modifiers & ModifierMask.META,
       button: 2});

  var elem = document.elementFromPoint(x, y);
  if (!elem) {
    throw new Error('cannot right click outside the visible bounds of the ' +
                    'document: (' + x + ', ' + y + ')');
  }
  elem.dispatchEvent(event);
}
