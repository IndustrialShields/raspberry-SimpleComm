/*
   Copyright (c) 2023 Boot&Work Corp., S.L. All rights reserved

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

using namespace std;

#include "Stream.h"
#define WRITE_FUN_C(sclass) std::size_t sclass::write(const uint8_t* data, std::size_t count)
#define READ_FUN_C(sclass) int sclass::read(void)
#define AVAILABLE_FUN_C(sclass) std::size_t sclass::available(void)

#include <iostream>
#include <vector>
#include <sys/socket.h>

FileStream::FileStream(const char* file_name) {
  _file_stream = new fstream(file_name, ios::in | ios::out | ios::binary);
}

WRITE_FUN_C(FileStream) {
  _file_stream->write((const char*) data, count);
  return count;
}

READ_FUN_C(FileStream) {
  int c = _file_stream->get();
  return c;
}

AVAILABLE_FUN_C(FileStream) {
  size_t s = _file_stream->rdbuf()->in_avail();
  return s;
}

void FileStream::flush(void) { _file_stream->flush();  } 

FileStream::~FileStream() {
  delete _file_stream;
}

SockStream::SockStream(int socket_fd) {
  _socket_fd = socket_fd;
  _socket_rx_buff = vector<uint8_t>(128);
}

WRITE_FUN_C(SockStream) {
  ssize_t s = send(_socket_fd, data, count, MSG_DONTWAIT);
  if (s == -1) return 0; // An error has occurred during write
  return s;
}

READ_FUN_C(SockStream) {
  uint8_t ret = _rx_queue.front();
  _rx_queue.pop();
  return ret;
}

AVAILABLE_FUN_C(SockStream) {
  ssize_t received = recv(_socket_fd, &_socket_rx_buff[0], _socket_rx_buff.size(), MSG_DONTWAIT);
  if (received != -1) { // Probably the call would be blocked
    for (ssize_t c = 0; c < received; c++) {
      _rx_queue.push(_socket_rx_buff[c]);
    }
  }
  return _rx_queue.size();
}
