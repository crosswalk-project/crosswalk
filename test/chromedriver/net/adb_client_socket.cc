// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/net/adb_client_socket.h"

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "net/base/address_list.h"
#include "net/base/completion_callback.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/socket/tcp_client_socket.h"

namespace {

const int kBufferSize = 16 * 1024;
const char kOkayResponse[] = "OKAY";
const char kHostTransportCommand[] = "host:transport:%s";
const char kLocalAbstractCommand[] = "localabstract:%s";
const char kLocalhost[] = "127.0.0.1";

typedef base::Callback<void(int, const std::string&)> CommandCallback;
typedef base::Callback<void(int, net::StreamSocket*)> SocketCallback;

std::string EncodeMessage(const std::string& message) {
  static const char kHexChars[] = "0123456789ABCDEF";

  size_t length = message.length();
  std::string result(4, '\0');
  char b = reinterpret_cast<const char*>(&length)[1];
  result[0] = kHexChars[(b >> 4) & 0xf];
  result[1] = kHexChars[b & 0xf];
  b = reinterpret_cast<const char*>(&length)[0];
  result[2] = kHexChars[(b >> 4) & 0xf];
  result[3] = kHexChars[b & 0xf];
  return result + message;
}

class AdbTransportSocket : public AdbClientSocket {
 public:
  AdbTransportSocket(int port,
                     const std::string& serial,
                     const std::string& socket_name,
                     const SocketCallback& callback)
    : AdbClientSocket(port),
      serial_(serial),
      socket_name_(socket_name),
      callback_(callback) {
    Connect(base::Bind(&AdbTransportSocket::OnConnected,
                       base::Unretained(this)));
  }

 private:
  ~AdbTransportSocket() {}

  void OnConnected(int result) {
    if (!CheckNetResultOrDie(result))
      return;
    SendCommand(base::StringPrintf(kHostTransportCommand, serial_.c_str()),
        true, base::Bind(&AdbTransportSocket::SendLocalAbstract,
                         base::Unretained(this)));
  }

  void SendLocalAbstract(int result, const std::string& response) {
    if (!CheckNetResultOrDie(result))
      return;
    SendCommand(base::StringPrintf(kLocalAbstractCommand, socket_name_.c_str()),
        true, base::Bind(&AdbTransportSocket::OnSocketAvailable,
                         base::Unretained(this)));
  }

  void OnSocketAvailable(int result, const std::string& response) {
    if (!CheckNetResultOrDie(result))
      return;
    callback_.Run(net::OK, socket_.release());
    delete this;
  }

  bool CheckNetResultOrDie(int result) {
    if (result >= 0)
      return true;
    callback_.Run(result, NULL);
    delete this;
    return false;
  }

  std::string serial_;
  std::string socket_name_;
  SocketCallback callback_;
};

class HttpOverAdbSocket {
 public:
  HttpOverAdbSocket(int port,
                    const std::string& serial,
                    const std::string& socket_name,
                    const std::string& request,
                    const CommandCallback& callback)
    : request_(request),
      command_callback_(callback),
      body_pos_(0) {
    Connect(port, serial, socket_name);
  }

  HttpOverAdbSocket(int port,
                    const std::string& serial,
                    const std::string& socket_name,
                    const std::string& request,
                    const SocketCallback& callback)
    : request_(request),
      socket_callback_(callback),
      body_pos_(0) {
    Connect(port, serial, socket_name);
  }

 private:
  ~HttpOverAdbSocket() {
  }

  void Connect(int port,
               const std::string& serial,
               const std::string& socket_name) {
    AdbClientSocket::TransportQuery(
        port, serial, socket_name,
        base::Bind(&HttpOverAdbSocket::OnSocketAvailable,
                   base::Unretained(this)));
  }

  void OnSocketAvailable(int result,
                         net::StreamSocket* socket) {
    if (!CheckNetResultOrDie(result))
      return;

    socket_.reset(socket);

    scoped_refptr<net::StringIOBuffer> request_buffer =
        new net::StringIOBuffer(request_);

    result = socket_->Write(
        request_buffer.get(),
        request_buffer->size(),
        base::Bind(&HttpOverAdbSocket::ReadResponse, base::Unretained(this)));
    if (result != net::ERR_IO_PENDING)
      ReadResponse(result);
  }

  void ReadResponse(int result) {
    if (!CheckNetResultOrDie(result))
      return;

    scoped_refptr<net::IOBuffer> response_buffer =
        new net::IOBuffer(kBufferSize);

    result = socket_->Read(response_buffer.get(),
                           kBufferSize,
                           base::Bind(&HttpOverAdbSocket::OnResponseData,
                                      base::Unretained(this),
                                      response_buffer,
                                      -1));
    if (result != net::ERR_IO_PENDING)
      OnResponseData(response_buffer, -1, result);
  }

  void OnResponseData(scoped_refptr<net::IOBuffer> response_buffer,
                      int bytes_total,
                      int result) {
    if (!CheckNetResultOrDie(result))
      return;
    if (result == 0) {
      CheckNetResultOrDie(net::ERR_CONNECTION_CLOSED);
      return;
    }

    response_ += std::string(response_buffer->data(), result);
    int expected_length = 0;
    if (bytes_total < 0) {
      size_t content_pos = response_.find("Content-Length:");
      if (content_pos != std::string::npos) {
        size_t endline_pos = response_.find("\n", content_pos);
        if (endline_pos != std::string::npos) {
          std::string len = response_.substr(content_pos + 15,
                                             endline_pos - content_pos - 15);
          TrimWhitespace(len, TRIM_ALL, &len);
          if (!base::StringToInt(len, &expected_length)) {
            CheckNetResultOrDie(net::ERR_FAILED);
            return;
          }
        }
      }

      body_pos_ = response_.find("\r\n\r\n");
      if (body_pos_ != std::string::npos) {
        body_pos_ += 4;
        bytes_total = body_pos_ + expected_length;
      }
    }

    if (bytes_total == static_cast<int>(response_.length())) {
      if (!command_callback_.is_null())
        command_callback_.Run(body_pos_, response_);
      else
        socket_callback_.Run(net::OK, socket_.release());
      delete this;
      return;
    }

    result = socket_->Read(response_buffer.get(),
                           kBufferSize,
                           base::Bind(&HttpOverAdbSocket::OnResponseData,
                                      base::Unretained(this),
                                      response_buffer,
                                      bytes_total));
    if (result != net::ERR_IO_PENDING)
      OnResponseData(response_buffer, bytes_total, result);
  }

  bool CheckNetResultOrDie(int result) {
    if (result >= 0)
      return true;
    if (!command_callback_.is_null())
      command_callback_.Run(result, std::string());
    else
      socket_callback_.Run(result, NULL);
    delete this;
    return false;
  }

  scoped_ptr<net::StreamSocket> socket_;
  std::string request_;
  std::string response_;
  CommandCallback command_callback_;
  SocketCallback socket_callback_;
  size_t body_pos_;
};

class AdbQuerySocket : AdbClientSocket {
 public:
  AdbQuerySocket(int port,
                 const std::string& query,
                 const CommandCallback& callback)
      : AdbClientSocket(port),
        current_query_(0),
        callback_(callback) {
    if (Tokenize(query, "|", &queries_) == 0) {
      CheckNetResultOrDie(net::ERR_INVALID_ARGUMENT);
      return;
    }
    Connect(base::Bind(&AdbQuerySocket::SendNextQuery,
                       base::Unretained(this)));
  }

 private:
  ~AdbQuerySocket() {
  }

  void SendNextQuery(int result) {
    if (!CheckNetResultOrDie(result))
      return;
    std::string query = queries_[current_query_];
    if (query.length() > 0xFFFF) {
      CheckNetResultOrDie(net::ERR_MSG_TOO_BIG);
      return;
    }
    bool is_void = current_query_ < queries_.size() - 1;
    SendCommand(query, is_void,
        base::Bind(&AdbQuerySocket::OnResponse, base::Unretained(this)));
  }

  void OnResponse(int result, const std::string& response) {
    if (++current_query_ < queries_.size()) {
      SendNextQuery(net::OK);
    } else {
      callback_.Run(result, response);
      delete this;
    }
  }

  bool CheckNetResultOrDie(int result) {
    if (result >= 0)
      return true;
    callback_.Run(result, std::string());
    delete this;
    return false;
  }

  std::vector<std::string> queries_;
  size_t current_query_;
  CommandCallback callback_;
};

}  // namespace

// static
void AdbClientSocket::AdbQuery(int port,
                               const std::string& query,
                               const CommandCallback& callback) {
  new AdbQuerySocket(port, query, callback);
}

#if defined(DEBUG_DEVTOOLS)
static void UseTransportQueryForDesktop(const SocketCallback& callback,
                                        net::StreamSocket* socket,
                                        int result) {
  callback.Run(result, socket);
}
#endif  // defined(DEBUG_DEVTOOLS)

// static
void AdbClientSocket::TransportQuery(int port,
                                     const std::string& serial,
                                     const std::string& socket_name,
                                     const SocketCallback& callback) {
#if defined(DEBUG_DEVTOOLS)
  if (serial.empty()) {
    // Use plain socket for remote debugging on Desktop (debugging purposes).
    net::IPAddressNumber ip_number;
    net::ParseIPLiteralToNumber(kLocalhost, &ip_number);

    int tcp_port = 0;
    if (!base::StringToInt(socket_name, &tcp_port))
      tcp_port = 9222;

    net::AddressList address_list =
        net::AddressList::CreateFromIPAddress(ip_number, tcp_port);
    net::TCPClientSocket* socket = new net::TCPClientSocket(
        address_list, NULL, net::NetLog::Source());
    socket->Connect(base::Bind(&UseTransportQueryForDesktop, callback, socket));
    return;
  }
#endif  // defined(DEBUG_DEVTOOLS)
  new AdbTransportSocket(port, serial, socket_name, callback);
}

// static
void AdbClientSocket::HttpQuery(int port,
                                const std::string& serial,
                                const std::string& socket_name,
                                const std::string& request_path,
                                const CommandCallback& callback) {
  new HttpOverAdbSocket(port, serial, socket_name, request_path,
      callback);
}

// static
void AdbClientSocket::HttpQuery(int port,
                                const std::string& serial,
                                const std::string& socket_name,
                                const std::string& request_path,
                                const SocketCallback& callback) {
  new HttpOverAdbSocket(port, serial, socket_name, request_path,
      callback);
}

AdbClientSocket::AdbClientSocket(int port)
    : host_(kLocalhost), port_(port) {
}

AdbClientSocket::~AdbClientSocket() {
}

void AdbClientSocket::Connect(const net::CompletionCallback& callback) {
  net::IPAddressNumber ip_number;
  if (!net::ParseIPLiteralToNumber(host_, &ip_number)) {
    callback.Run(net::ERR_FAILED);
    return;
  }

  net::AddressList address_list =
      net::AddressList::CreateFromIPAddress(ip_number, port_);
  socket_.reset(new net::TCPClientSocket(address_list, NULL,
                                         net::NetLog::Source()));
  int result = socket_->Connect(callback);
  if (result != net::ERR_IO_PENDING)
    callback.Run(result);
}

void AdbClientSocket::SendCommand(const std::string& command,
                                  bool is_void,
                                  const CommandCallback& callback) {
  scoped_refptr<net::StringIOBuffer> request_buffer =
      new net::StringIOBuffer(EncodeMessage(command));
  int result = socket_->Write(request_buffer.get(),
                              request_buffer->size(),
                              base::Bind(&AdbClientSocket::ReadResponse,
                                         base::Unretained(this),
                                         callback,
                                         is_void));
  if (result != net::ERR_IO_PENDING)
    ReadResponse(callback, is_void, result);
}

void AdbClientSocket::ReadResponse(const CommandCallback& callback,
                                   bool is_void,
                                   int result) {
  if (result < 0) {
    callback.Run(result, "IO error");
    return;
  }
  scoped_refptr<net::IOBuffer> response_buffer =
      new net::IOBuffer(kBufferSize);
  result = socket_->Read(response_buffer.get(),
                         kBufferSize,
                         base::Bind(&AdbClientSocket::OnResponseHeader,
                                    base::Unretained(this),
                                    callback,
                                    is_void,
                                    response_buffer));
  if (result != net::ERR_IO_PENDING)
    OnResponseHeader(callback, is_void, response_buffer, result);
}

void AdbClientSocket::OnResponseHeader(
    const CommandCallback& callback,
    bool is_void,
    scoped_refptr<net::IOBuffer> response_buffer,
    int result) {
  if (result <= 0) {
    callback.Run(result == 0 ? net::ERR_CONNECTION_CLOSED : result,
                 "IO error");
    return;
  }

  std::string data = std::string(response_buffer->data(), result);
  if (result < 4) {
    callback.Run(net::ERR_FAILED, "Response is too short: " + data);
    return;
  }

  std::string status = data.substr(0, 4);
  if (status != kOkayResponse) {
    callback.Run(net::ERR_FAILED, data);
    return;
  }

  data = data.substr(4);

  if (!is_void) {
    int payload_length = 0;
    int bytes_left = -1;
    if (data.length() >= 4 &&
        base::HexStringToInt(data.substr(0, 4), &payload_length)) {
      data = data.substr(4);
      bytes_left = payload_length - result + 8;
    } else {
      bytes_left = -1;
    }
    OnResponseData(callback, data, response_buffer, bytes_left, 0);
  } else {
    callback.Run(net::OK, data);
  }
}

void AdbClientSocket::OnResponseData(
    const CommandCallback& callback,
    const std::string& response,
    scoped_refptr<net::IOBuffer> response_buffer,
    int bytes_left,
    int result) {
  if (result < 0) {
    callback.Run(result, "IO error");
    return;
  }

  bytes_left -= result;
  std::string new_response =
      response + std::string(response_buffer->data(), result);
  if (bytes_left == 0) {
    callback.Run(net::OK, new_response);
    return;
  }

  // Read tail
  result = socket_->Read(response_buffer.get(),
                         kBufferSize,
                         base::Bind(&AdbClientSocket::OnResponseData,
                                    base::Unretained(this),
                                    callback,
                                    new_response,
                                    response_buffer,
                                    bytes_left));
  if (result > 0)
    OnResponseData(callback, new_response, response_buffer, bytes_left, result);
  else if (result != net::ERR_IO_PENDING)
    callback.Run(net::OK, new_response);
}
