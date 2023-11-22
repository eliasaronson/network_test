#pragma once
#include "sockpp/tcp_connector.h"

class tcp_client {
  public:
    tcp_client(std::string host, int port);
    ~tcp_client();
    std::vector<double> read();
    void write(const std::vector<double>& data, bool end = false, bool flag = false);
    void write(std::string data);

  private:
    sockpp::tcp_connector* conn = nullptr;
};

