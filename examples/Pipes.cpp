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

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <SimpleComm.h>

using namespace std;

static const char* pipe_name = "/tmp/pipe.fifo";
static const uint8_t slaveAddress = 1;

void receive_packet(FileStream& stream) {
  SimpleComm.begin();

  SimplePacket packet;
  while (1) {
    if (SimpleComm.receive(stream, packet)) {
      int value = packet.getInt();
      cout << "Received value: " << value << endl;
      break;
    }
  }
}

void send_packet(FileStream& stream) {
  SimpleComm.begin();

  SimplePacket packet;
  packet.setData(3);
  SimpleComm.send(stream, packet, slaveAddress);
  stream.flush();
}

int main() {
   unlink(pipe_name);
   if (mkfifo(pipe_name, 0666) == -1) {
     cerr << "Failed to create named pipe. errno: " << errno << endl;
     return 1;
   }
   
   cout << "Forking..." << endl;
   pid_t c_pid = fork();
   if (c_pid == -1) {
     cerr << "Error when forking" << endl;
     return -1;
   }
   else if (c_pid > 0) {
     FileStream stream = FileStream(pipe_name);
     send_packet(stream);
     wait(&c_pid);
     unlink(pipe_name);
     return 0;
   }
   else {
     FileStream stream = FileStream(pipe_name);
     receive_packet(stream);
   }
}
