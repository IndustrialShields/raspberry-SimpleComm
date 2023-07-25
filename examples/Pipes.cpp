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
