# SimpleComm library for Raspberry devices

## Gettings started

### Installing
1. Download the [library](http://www.github.com/IndustrialShields/raspberry-SimpleComm.git) from the GitHub as a "ZIP" file.
2. Uncompress the file and open the directory with the terminal.
3. Type the command `make install`, and you are done.

### Statically linking the library
Programs that want to include the SimpleComm library inside the final binary can be compiled with `g++ program.cpp -o program -I src -L out/ -lraspberry-SimpleComm -static`

### Dynamically linking the library

Programs that want to use the SimpleComm library dynamically (installed system-wise) can be compiled with `g++ program.cpp -o program -L /usr/local/lib/ -lraspberry-SimpleComm -I /usr/local/include/raspberry-SimpleComm`

**WARNING: If you change the SimplePacketConfig.h header when dynamically linking, you will need to reinstall the library!**

## Substituting the Stream class
The **Stream** class is fundamental for communication in Arduino, as it's an abstraction that enables the standardized handling of data streams across various communication interfaces. Because SimpleComm was designed for Arduino, it heavily relies on the Stream Class as its core communication abstraction. To make the library compatible with Linux and retain its functionality seamlessly, we focused on adapting the Stream class from the Arduino world into the Linux world.

Basically, the Stream class is an abstract class that implements three virtual functions:
* ```std::size_t write(const uint8_t* data, std::size_t count)```
* ```int read(void)```
* ```std::size_t available(void)```

These functions have the same signature as the ones used by the Stream Class, so the SimpleComm library can use all the derived classes from it without any modifications.

### FileStream
The **FileStream** class is a specialized class that extends the capabilities of the Stream class, specifically designed to implement essential functions like `write()`, `read()`, and `available()` for **C++ fstream** objects. This adaptation enables users to use file-based communication methods, such as pipes (IPC) or other devices like UART and RS-485.

A FileStream object can be created using either a C-String or a String Object that contains the name of the file:
```c++
FileStream stream = FileStream("/dev/ttySC0");
// ...
// ...
uint8_t destination = 0;
SimpleComm.send(stream, packet, destination);
```

### SockStream
**SockStream** is a specialized class that extends the Stream class to implement the `write()`, `read()`, and `available()` functionsd for **C Sockets**. This class makes possible using SimpleComm with sockets, like for example TCP/IP or UNIX sockets.

A SockStream object can only be created using the socket file descriptor:
```c++
int socket_desc = socket(AF_INET , SOCK_STREAM , 0);
// connect(...)
SockStream stream = SockStream(socket_desc);
// ..
uint8_t destination = 0;
SimpleComm.send(stream, packet, destination);
```

## Reference

```c++
#include <SimpleComm.h>
```

The **SimpleComm** module sends and receives data through any supported Stream: RS-485, RS-232, Files, Sockets... It is enough flexible to support different kind of communication typologies: Ad-hoc, Master-Slave, Client-Server, and so on, using an easy to use API. 

The **SimplePacket** class encapsulates the data into a packet.

The `setData` function fills up the packet with the desired data for sending it to a remote device. It is possible to fill up the packet with different types of data: bool, char, unsigned char, int, unsigned int, long, unsigned long, double, string and even custom data types.

```c++
int num = 1234;
SimplePacket packet1;
packet1.setData(num);
```

```c++
SimplePacket packet2;
packet2.setData("text data");
```

```c++
typedef struct {
    int a;
    char b;
} customType;
customType customVar;
customVar.a = 1234;
customVar.b = 'B';
SimplePacket packet3;
packet3.setData(customVar, sizeof(customVar));
```

There are also getter functions, which return the data contained in the packet, depending on the data type. If length is specified, it returns the data length in bytes.

```c++
int num = packet1.getInt();
```

```c++
const char *str = packet2.getString();
```

```c++
const customType *var = (const customType*) packet3.getData();
```

The **SimpleComm** class is the interface for sending and receiving packets through the desired stream.

The `begin(byte)` function enables the communication system and sets the devices identifier/address. Each device has its own address which identifies it. Devices receive packets sent to them, using their address, but not to others.

```c++
uint8_t address = 1234;
SimpleComm.begin(address);
```

It is possible to send packets to a device with an address, and optionally define the packet type.

```c++
SockStream stream = SockStream(client_socket_descriptor);
// ...
// ...
uint8_t destination = 87;
SimpleComm.send(stream, packet, destination);
SimpleComm.send(stream, packet, destination, 0x33);
```

To send packets to all devices it is possible to send a broadcast packet with the destination field set to 0 (zero).

```c++
SockStream stream = SockStream(client_socket_descriptor);
// ...
// ...
uint8_t destination = 0;
SimpleComm.send(stream, packet, destination);
```

The `receive` function receives a packet from another device, using the stream. It returns true if a packet is really received.

```c++
FileStream stream = FileStream(/dev/ttySC0);
// ...
// ...
SimplePacket rxPacket;
if (SimpleComm.receive(stream, rxPacket)) {
    // A packet is received
}
```

It is also possible to receive all the packets (sniffer mode) by setting the SimpleComm address to 0 (zero). This mode is useful when implementing a gateway node between Ethernet and RS-485.

```c++
uint8_t address = 0;
SimpleComm.begin(address);
```

## Compatibility between architectures
This library relies on standard C++ types (e.g., unsigned long, int) which can function correctly if the communicating architectures maintain consistent type sizes. However, problems may arise if you try to communicate different CPU architectures, such as ESP32 and Arduino. The C++ types that are defined in each architecture have different sizes, which will cause communication errors.

To ensure proper communication between different architectures and to address potential type size issues, the library provides a solution through the "SimplePacketConfig.h" header file. This file allows users to customise the types used in the library, thereby fixing the size of the types for proper communication:
* If you uncomment `#define UNIVERSAL_CPP` the types used will be the minimum size according to the C++ standard.
* If you uncomment `#define CUSTOM_TYPES`, the types used will be the size of what you define.
