# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import BaseHTTPServer
import os
import threading


class Responder(object):
  """Sends a HTTP response. Used with TestWebServer."""

  def __init__(self, handler):
    self._handler = handler

  def SendResponse(self, body):
    """Sends OK response with body."""
    self.SendHeaders(len(body))
    self.SendBody(body)

  def SendResponseFromFile(self, path):
    """Sends OK response with the given file as the body."""
    with open(path, 'r') as f:
      self.SendResponse(f.read())

  def SendHeaders(self, content_length=None):
    """Sends headers for OK response."""
    self._handler.send_response(200)
    if content_length:
      self._handler.send_header('Content-Length', content_length)
    self._handler.end_headers()

  def SendError(self, code):
    """Sends response for the given HTTP error code."""
    self._handler.send_error(code)

  def SendBody(self, body):
    """Just sends the body, no headers."""
    self._handler.wfile.write(body)


class Request(object):
  """An HTTP request."""

  def __init__(self, handler):
    self._handler = handler

  def GetPath(self):
    return self._handler.path


class _BaseServer(BaseHTTPServer.HTTPServer):
  """Internal server that throws if timed out waiting for a request."""

  def __init__(self, on_request, server_cert_and_key_path=None):
    """Starts the server.

    It is an HTTP server if parameter server_cert_and_key_path is not provided.
    Otherwise, it is an HTTPS server.

    Args:
      server_cert_and_key_path: path to a PEM file containing the cert and key.
                                if it is None, start the server as an HTTP one.
    """
    class _Handler(BaseHTTPServer.BaseHTTPRequestHandler):
      """Internal handler that just asks the server to handle the request."""

      def do_GET(self):
        if self.path.endswith('favicon.ico'):
          self.send_error(404)
          return
        on_request(Request(self), Responder(self))

      def log_message(self, *args, **kwargs):
        """Overriddes base class method to disable logging."""
        pass

    BaseHTTPServer.HTTPServer.__init__(self, ('127.0.0.1', 0), _Handler)

    if server_cert_and_key_path is not None:
      self._is_https_enabled = True
      self._server.socket = ssl.wrap_socket(
          self._server.socket, certfile=server_cert_and_key_path,
          server_side=True)
    else:
      self._is_https_enabled = False

  def handle_timeout(self):
    """Overridden from SocketServer."""
    raise RuntimeError('Timed out waiting for http request')

  def GetUrl(self):
    """Returns the base URL of the server."""
    postfix = '://127.0.0.1:%s' % self.server_port
    if self._is_https_enabled:
      return 'https' + postfix
    return 'http' + postfix


class WebServer(object):
  """An HTTP or HTTPS server that serves on its own thread.

  Serves files from given directory but may use custom data for specific paths.
  """

  def __init__(self, root_dir, server_cert_and_key_path=None):
    """Starts the server.

    It is an HTTP server if parameter server_cert_and_key_path is not provided.
    Otherwise, it is an HTTPS server.

    Args:
      root_dir: root path to serve files from. This parameter is required.
      server_cert_and_key_path: path to a PEM file containing the cert and key.
                                if it is None, start the server as an HTTP one.
    """
    self._root_dir = os.path.abspath(root_dir)
    self._server = _BaseServer(self._OnRequest, server_cert_and_key_path)
    self._thread = threading.Thread(target=self._server.serve_forever)
    self._thread.daemon = True
    self._thread.start()
    self._path_data_map = {}
    self._path_data_lock = threading.Lock()

  def _OnRequest(self, request, responder):
    path = request.GetPath().split('?')[0]

    # Serve from path -> data map.
    self._path_data_lock.acquire()
    try:
      if path in self._path_data_map:
        responder.SendResponse(self._path_data_map[path])
        return
    finally:
      self._path_data_lock.release()

    # Serve from file.
    path = os.path.normpath(
        os.path.join(self._root_dir, *path.split('/')))
    if not path.startswith(self._root_dir):
      responder.SendError(403)
      return
    if not os.path.exists(path):
      responder.SendError(404)
      return
    responder.SendResponseFromFile(path)

  def SetDataForPath(self, path, data):
    self._path_data_lock.acquire()
    try:
      self._path_data_map[path] = data
    finally:
      self._path_data_lock.release()

  def GetUrl(self):
    """Returns the base URL of the server."""
    return self._server.GetUrl()

  def Shutdown(self):
    """Shuts down the server synchronously."""
    self._server.shutdown()
    self._thread.join()


class SyncWebServer(object):
  """WebServer for testing.

  Incoming requests are blocked until explicitly handled.
  This was designed for single thread use. All requests should be handled on
  the same thread.
  """

  def __init__(self):
    self._server = _BaseServer(self._OnRequest)
    # Recognized by SocketServer.
    self._server.timeout = 10
    self._on_request = None

  def _OnRequest(self, request, responder):
    self._on_request(responder)
    self._on_request = None

  def Respond(self, on_request):
    """Blocks until request comes in, then calls given handler function.

    Args:
      on_request: Function that handles the request. Invoked with single
          parameter, an instance of Responder.
    """
    if self._on_request:
      raise RuntimeError('Must handle 1 request at a time.')

    self._on_request = on_request
    while self._on_request:
      # Don't use handle_one_request, because it won't work with the timeout.
      self._server.handle_request()

  def RespondWithContent(self, content):
    """Blocks until request comes in, then handles it with the given content."""
    def SendContent(responder):
      responder.SendResponse(content)
    self.Respond(SendContent)

  def GetUrl(self):
    return self._server.GetUrl()
