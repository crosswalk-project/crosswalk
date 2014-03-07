// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/guid.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"
#include "base/sync_socket.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/net/port_server.h"
#include "net/base/sys_addrinfo.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_LINUX)
#include <fcntl.h>  // NOLINT
#include <sys/socket.h>  // NOLINT
#include <sys/un.h>  // NOLINT
#endif

namespace {

void SetOnCall(bool* called) {
  *called = true;
}

}  // namespace

TEST(PortReservationTest, Normal) {
  bool called = false;
  {
    PortReservation r(base::Bind(&SetOnCall, &called), 100);
  }
  ASSERT_TRUE(called);
}

TEST(PortReservationTest, Leak) {
  bool called = false;
  {
    PortReservation r(base::Bind(&SetOnCall, &called), 100);
    r.Leak();
  }
  ASSERT_FALSE(called);
}

TEST(PortReservationTest, MultipleLeaks) {
  bool called = false;
  {
    PortReservation r(base::Bind(&SetOnCall, &called), 100);
    r.Leak();
    r.Leak();
  }
  ASSERT_FALSE(called);
}

#if defined(OS_LINUX)
namespace {

void RunServerOnThread(const std::string& path,
                       const std::string& response,
                       base::WaitableEvent* listen_event,
                       std::string* request) {
  int server_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  ASSERT_GE(server_sock_fd, 0);
  ASSERT_GE(fcntl(server_sock_fd, F_SETFL, O_NONBLOCK), 0);
  base::SyncSocket server_sock(server_sock_fd);

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  memcpy(addr.sun_path, &path[0], path.length());
  ASSERT_EQ(0,
            bind(server_sock_fd,
                 reinterpret_cast<struct sockaddr*>(&addr),
                 sizeof(sa_family_t) + path.length()));
  ASSERT_EQ(0, listen(server_sock_fd, 1));
  listen_event->Signal();

  struct sockaddr_un  cli_addr;
  socklen_t clilen = sizeof(cli_addr);
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(2);
  int client_sock_fd = -1;
  while (base::TimeTicks::Now() < deadline && client_sock_fd < 0) {
    client_sock_fd = accept(
        server_sock_fd, reinterpret_cast<struct sockaddr*>(&cli_addr), &clilen);
  }
  ASSERT_GE(client_sock_fd, 0);
  base::SyncSocket sock(client_sock_fd);
  do {
    char c = 0;
    size_t rv = sock.Receive(&c, 1);
    if (!rv)
      break;
    request->push_back(c);
  } while (sock.Peek());
  sock.Send(response.c_str(), response.length());
}

std::string GenerateRandomPath() {
  std::string path = base::GenerateGUID();
  if (!path.empty()) {
    std::string pre_path;
    pre_path.push_back(0);  // Linux abstract namespace.
    path = pre_path + path;
  }
  return path;
}

}  // namespace

class PortServerTest : public testing::Test {
 public:
  PortServerTest() : thread_("server") {
    EXPECT_TRUE(thread_.Start());
  }

  void RunServer(const std::string& path,
                 const std::string& response,
                 std::string* request) {
    base::WaitableEvent listen_event(false, false);
    thread_.message_loop()->PostTask(
        FROM_HERE,
        base::Bind(
            &RunServerOnThread, path, response, &listen_event, request));
    ASSERT_TRUE(listen_event.TimedWait(base::TimeDelta::FromSeconds(5)));
  }

 private:
  base::Thread thread_;
};

TEST_F(PortServerTest, Reserve) {
  std::string path = GenerateRandomPath();
  PortServer server(path);

  std::string request;
  RunServer(path, "12345\n", &request);

  int port = 0;
  scoped_ptr<PortReservation> reservation;
  Status status = server.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();
  ASSERT_EQ(port, 12345);
}

TEST_F(PortServerTest, ReserveResetReserve) {
  std::string path = GenerateRandomPath();
  PortServer server(path);

  std::string request;
  RunServer(path, "12345\n", &request);

  int port = 0;
  scoped_ptr<PortReservation> reservation;
  Status status = server.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();
  ASSERT_EQ(port, 12345);

  reservation.reset();
  status = server.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();
  ASSERT_EQ(port, 12345);
}

TEST_F(PortServerTest, ReserveReserve) {
  std::string path = GenerateRandomPath();
  PortServer server(path);

  std::string request;
  RunServer(path, "12345\n", &request);

  int port = 0;
  scoped_ptr<PortReservation> reservation;
  Status status = server.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();
  ASSERT_EQ(port, 12345);

  RunServer(path, "12346\n", &request);
  status = server.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();
  ASSERT_EQ(port, 12346);
}
#endif

TEST(PortManagerTest, Reserve) {
  PortManager mgr(15000, 16000);
  int port = 0;
  scoped_ptr<PortReservation> reservation;
  Status status = mgr.ReservePort(&port, &reservation);
  ASSERT_EQ(kOk, status.code()) << status.message();

  ASSERT_GE(port, 15000);
  ASSERT_LE(port, 16000);
  ASSERT_TRUE(reservation);
}
