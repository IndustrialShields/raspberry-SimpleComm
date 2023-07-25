#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <stdint.h>

#define WRITE_FUN std::size_t write(const uint8_t* data, std::size_t count)
#define READ_FUN int read(void)
#define AVAILABLE_FUN std::size_t available(void)

class Stream {
public:
  virtual WRITE_FUN  = 0;
  virtual READ_FUN  = 0;
  virtual AVAILABLE_FUN = 0;
};

class FileStream : public Stream {
public:
  explicit FileStream(const char* file_name);
  explicit inline FileStream(const std::string& file_name) : FileStream(file_name.c_str()) {};
  ~FileStream();
  
private:
  std::fstream* _file_stream;

public:
  WRITE_FUN final;
  READ_FUN final;
  AVAILABLE_FUN final;
  void flush(void);
};

class SockStream : public Stream {
public:
  explicit SockStream(int socket_fd);
  
private:
  int _socket_fd;
  std::vector<uint8_t> _socket_rx_buff;
  std::queue<uint8_t> _rx_queue;

public:
  WRITE_FUN final;
  READ_FUN final;
  AVAILABLE_FUN final;
  void flush(void);
};
