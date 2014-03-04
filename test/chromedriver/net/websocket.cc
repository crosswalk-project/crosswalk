// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/net/websocket.h"

#include <string.h>

#include "base/base64.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/memory/scoped_vector.h"
#include "base/rand_util.h"
#include "base/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "net/base/address_list.h"
#include "net/base/io_buffer.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/base/sys_addrinfo.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_util.h"
#include "net/websockets/websocket_frame.h"

#if defined(OS_WIN)
#include <Winsock2.h>
#endif

namespace {

bool ResolveHost(const std::string& host, net::IPAddressNumber* address) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* result;
  if (getaddrinfo(host.c_str(), NULL, &hints, &result))
    return false;

  for (struct addrinfo* addr = result; addr; addr = addr->ai_next) {
    if (addr->ai_family == AF_INET || addr->ai_family == AF_INET6) {
      net::IPEndPoint end_point;
      if (!end_point.FromSockAddr(addr->ai_addr, addr->ai_addrlen)) {
        freeaddrinfo(result);
        return false;
      }
      *address = end_point.address();
    }
  }
  freeaddrinfo(result);
  return true;
}

}  // namespace

WebSocket::WebSocket(const GURL& url, WebSocketListener* listener)
    : url_(url),
      listener_(listener),
      state_(INITIALIZED),
      write_buffer_(new net::DrainableIOBuffer(new net::IOBuffer(0), 0)),
      read_buffer_(new net::IOBufferWithSize(4096)) {}

WebSocket::~WebSocket() {
  CHECK(thread_checker_.CalledOnValidThread());
}

void WebSocket::Connect(const net::CompletionCallback& callback) {
  CHECK(thread_checker_.CalledOnValidThread());
  CHECK_EQ(INITIALIZED, state_);

  net::IPAddressNumber address;
  if (!net::ParseIPLiteralToNumber(url_.HostNoBrackets(), &address)) {
    if (!ResolveHost(url_.HostNoBrackets(), &address)) {
      callback.Run(net::ERR_ADDRESS_UNREACHABLE);
      return;
    }
  }
  int port = 80;
  base::StringToInt(url_.port(), &port);
  net::AddressList addresses(net::IPEndPoint(address, port));
  net::NetLog::Source source;
  socket_.reset(new net::TCPClientSocket(addresses, NULL, source));

  state_ = CONNECTING;
  connect_callback_ = callback;
  int code = socket_->Connect(base::Bind(
      &WebSocket::OnSocketConnect, base::Unretained(this)));
  if (code != net::ERR_IO_PENDING)
    OnSocketConnect(code);
}

bool WebSocket::Send(const std::string& message) {
  CHECK(thread_checker_.CalledOnValidThread());
  if (state_ != OPEN)
    return false;

  net::WebSocketFrameHeader header(net::WebSocketFrameHeader::kOpCodeText);
  header.final = true;
  header.masked = true;
  header.payload_length = message.length();
  int header_size = net::GetWebSocketFrameHeaderSize(header);
  net::WebSocketMaskingKey masking_key = net::GenerateWebSocketMaskingKey();
  std::string header_str;
  header_str.resize(header_size);
  CHECK_EQ(header_size, net::WriteWebSocketFrameHeader(
      header, &masking_key, &header_str[0], header_str.length()));

  std::string masked_message = message;
  net::MaskWebSocketFramePayload(
      masking_key, 0, &masked_message[0], masked_message.length());
  Write(header_str + masked_message);
  return true;
}

void WebSocket::OnSocketConnect(int code) {
  if (code != net::OK) {
    Close(code);
    return;
  }

  CHECK(base::Base64Encode(base::RandBytesAsString(16), &sec_key_));
  std::string handshake = base::StringPrintf(
      "GET %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Key: %s\r\n"
      "Sec-WebSocket-Version: 13\r\n"
      "Pragma: no-cache\r\n"
      "Cache-Control: no-cache\r\n"
      "\r\n",
      url_.path().c_str(),
      url_.host().c_str(),
      sec_key_.c_str());
  Write(handshake);
  Read();
}

void WebSocket::Write(const std::string& data) {
  pending_write_ += data;
  if (!write_buffer_->BytesRemaining())
    ContinueWritingIfNecessary();
}

void WebSocket::OnWrite(int code) {
  if (!socket_->IsConnected()) {
    // Supposedly if |StreamSocket| is closed, the error code may be undefined.
    Close(net::ERR_FAILED);
    return;
  }
  if (code < 0) {
    Close(code);
    return;
  }

  write_buffer_->DidConsume(code);
  ContinueWritingIfNecessary();
}

void WebSocket::ContinueWritingIfNecessary() {
  if (!write_buffer_->BytesRemaining()) {
    if (pending_write_.empty())
      return;
    write_buffer_ = new net::DrainableIOBuffer(
        new net::StringIOBuffer(pending_write_),
        pending_write_.length());
    pending_write_.clear();
  }
  int code =
      socket_->Write(write_buffer_.get(),
                     write_buffer_->BytesRemaining(),
                     base::Bind(&WebSocket::OnWrite, base::Unretained(this)));
  if (code != net::ERR_IO_PENDING)
    OnWrite(code);
}

void WebSocket::Read() {
  int code =
      socket_->Read(read_buffer_.get(),
                    read_buffer_->size(),
                    base::Bind(&WebSocket::OnRead, base::Unretained(this)));
  if (code != net::ERR_IO_PENDING)
    OnRead(code);
}

void WebSocket::OnRead(int code) {
  if (code <= 0) {
    Close(code ? code : net::ERR_FAILED);
    return;
  }

  if (state_ == CONNECTING)
    OnReadDuringHandshake(read_buffer_->data(), code);
  else if (state_ == OPEN)
    OnReadDuringOpen(read_buffer_->data(), code);

  if (state_ != CLOSED)
    Read();
}

void WebSocket::OnReadDuringHandshake(const char* data, int len) {
  handshake_response_ += std::string(data, len);
  int headers_end = net::HttpUtil::LocateEndOfHeaders(
      handshake_response_.data(), handshake_response_.size(), 0);
  if (headers_end == -1)
    return;

  const char kMagicKey[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string websocket_accept;
  CHECK(base::Base64Encode(base::SHA1HashString(sec_key_ + kMagicKey),
                           &websocket_accept));
  scoped_refptr<net::HttpResponseHeaders> headers(
      new net::HttpResponseHeaders(
          net::HttpUtil::AssembleRawHeaders(
              handshake_response_.data(), headers_end)));
  if (headers->response_code() != 101 ||
      !headers->HasHeaderValue("Upgrade", "WebSocket") ||
      !headers->HasHeaderValue("Connection", "Upgrade") ||
      !headers->HasHeaderValue("Sec-WebSocket-Accept", websocket_accept)) {
    Close(net::ERR_FAILED);
    return;
  }
  std::string leftover_message = handshake_response_.substr(headers_end);
  handshake_response_.clear();
  sec_key_.clear();
  state_ = OPEN;
  InvokeConnectCallback(net::OK);
  if (!leftover_message.empty())
    OnReadDuringOpen(leftover_message.c_str(), leftover_message.length());
}

void WebSocket::OnReadDuringOpen(const char* data, int len) {
  ScopedVector<net::WebSocketFrameChunk> frame_chunks;
  CHECK(parser_.Decode(data, len, &frame_chunks));
  for (size_t i = 0; i < frame_chunks.size(); ++i) {
    scoped_refptr<net::IOBufferWithSize> buffer = frame_chunks[i]->data;
    if (buffer.get())
      next_message_ += std::string(buffer->data(), buffer->size());
    if (frame_chunks[i]->final_chunk) {
      listener_->OnMessageReceived(next_message_);
      next_message_.clear();
    }
  }
}

void WebSocket::InvokeConnectCallback(int code) {
  net::CompletionCallback temp = connect_callback_;
  connect_callback_.Reset();
  CHECK(!temp.is_null());
  temp.Run(code);
}

void WebSocket::Close(int code) {
  socket_->Disconnect();
  if (!connect_callback_.is_null())
    InvokeConnectCallback(code);
  if (state_ == OPEN)
    listener_->OnClose();

  state_ = CLOSED;
}


