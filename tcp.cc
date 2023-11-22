#include "tcp.h"
#include <string>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

using namespace boost::python;

tcp_client::tcp_client(std::string host, int port) {
    conn = new sockpp::tcp_connector({host, (in_port_t)port});

    if (!(*conn)) {
        std::cerr << "Error connecting to server at " << host << ":" << port << " -> " << conn->last_error_str();
    }
}

tcp_client::~tcp_client() {
    if (conn != nullptr) {
        delete conn;
        conn = nullptr;
    }
}

std::vector<double> tcp_client::read() {
    char buf[4];
    auto status = conn->read_n(buf, sizeof(buf));

    auto size = static_cast<size_t>(static_cast<unsigned char>(buf[0]))
        + (static_cast<size_t>(static_cast<unsigned char>(buf[1]) << 8))
        + (static_cast<size_t>(static_cast<unsigned char>(buf[2]) << 16))
        + (static_cast<size_t>(static_cast<unsigned char>(buf[3]) << 24));

    char buf2[size];
    status = conn->read_n(buf2, size);

    for (int i = 0; i < size; ++i) {
        std::cout << buf2[i] << " ";
    }
    std::cout << "\n\r";

    return std::vector<double>(&buf2[0], &buf[size]);
}

void tcp_client::write(std::vector<double> data) {
    const size_t n = data.size();
    char size[4] {
        static_cast<char>(n & 0xFF),
        static_cast<char>((n >> 8) & 0xFF),
        static_cast<char>((n >> 16) & 0xFF),
        static_cast<char>((n >> 24) & 0xFF)};

    conn->write_n(size, sizeof(size));
    conn->write_n(data.data(), n);
}

BOOST_PYTHON_MODULE(tcp_client) {
    class_<std::vector<double>>("XVec").def(vector_indexing_suite<std::vector<double>>());

    class_<tcp_client>("tcp_client", init<std::string, int>())
        .def("read", &tcp_client::read)
        .def("write", &tcp_client::write);
}
