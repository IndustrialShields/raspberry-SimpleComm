#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <SimpleComm.h>

using namespace std;

#define PORT 8888
const uint8_t slaveAddress = 1;

void receive_packet(SockStream& rx_stream) {
  SimpleComm.begin();
  
  SimplePacket packet;
  while (1) {
    if (SimpleComm.receive(rx_stream, packet)) {
      double value = packet.getDouble();
      cout << "RX: Received value: " << value << endl;
      break;
    }
  }
}

void send_packet(SockStream& tx_stream) {
  SimpleComm.begin();

  SimplePacket packet;
  packet.setData(3.9);
  SimpleComm.send(tx_stream, packet, slaveAddress);
}

int main() {
  cout << "Forking..." << endl;
  pid_t c_pid = fork();
  if (c_pid == -1) {
    cerr << "Error when forking" << endl;
    return -1;
  }
  else if (c_pid > 0) { // Transmitter
    int socket_desc, client_socket_desc;
    struct sockaddr_in server, client;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
      cerr << "TX: Could not create the main socket" << endl;
      return 1;
    }
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    //Bind
    if(bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
      cout << "TX: Bind failed" << endl;
      return 1;
    }
    cout << "TX: Server created" << endl;
    
    // Listen
    listen(socket_desc, 1);
    cout << "TX: Waiting for a connection..." << endl;
    socklen_t l = sizeof(struct sockaddr_in);
    client_socket_desc = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&l);
    if (client_socket_desc < 0) {
      cerr << "TX: Failed to accept client" << endl;
      return 1;
    }

    cout << "TX: Connection successful" << endl;

    SockStream tx_stream = SockStream(client_socket_desc);
    send_packet(tx_stream);
    wait(&c_pid);

    cout << "TX: The child has finished" << endl;

    close(socket_desc); close(client_socket_desc);
    return 0;
  }
  else { // Receiver
    cout << "RX: Waiting 5 seconds before connecting..." << endl;
    prctl(PR_SET_PDEATHSIG, SIGHUP); // Kill the child if the parent dies
    int socket_desc;
    sleep(5); // Wait 5 seconds for the transmitter to bind the socket
    cout << "RX: Awaken" << endl;
    struct sockaddr_in server;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
      cout << "RX: Could not create socket" << endl;
      return 1;
    }
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    server.sin_port = htons(PORT);
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
      cout << "RX: Connection error" << endl;
      return 1;
    }

    SockStream rx_stream = SockStream(socket_desc);
    receive_packet(rx_stream);

    close(socket_desc);
    return 0;
  }

  return 1;
}
