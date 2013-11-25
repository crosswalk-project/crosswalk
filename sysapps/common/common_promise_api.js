// Copyright (c) 2013 unscriptable.com / John Hann. All rights reserved.
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

Promise.prototype = {
  then: function(onFulfilled, onRejected) {
    this._thens.push({ fulfill: onFulfilled, reject: onRejected });
    return this;
  },
  fulfill: function(value) {
    this._done('fulfill', value);
  },
  reject: function(error) {
    this._done('reject', error);
  },
  _done: function(which, arg) {
    // Cover and sync func `then()`.
    this.then = which === 'fulfill' ?
      function(fulfill, reject) { fulfill && fulfill(arg); return this; } :
      function(fulfill, reject) { reject && reject(arg); return this; };
    // Disallow multiple calls.
    this.fulfill = this.reject =
      function() { throw new Error('Promise already completed.'); }
    // Complete all async `then()`s.
    var then, i = 0;
    while (then = this._thens[i++]) {
      then[which] && then[which](arg);
    }
    delete this._thens;
  }
};

function Promise() {
  this._thens = [];
};

exports.Promise = Promise;
