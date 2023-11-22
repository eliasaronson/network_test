#include "test.h"
#include <string>
#include <iostream>
#include <fstream>

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

    /*printf("res s: %s\n", buf2);
    std::cout << "res d: ";
    for (size_t i = 0; i < size / sizeof(double); ++i) {
        std::cout << ((double*)&buf2[2])[i] << ", ";
    }
    std::cout << "\n\r";*/

    return std::vector<double>(&buf2[0], &buf[size]);
}

void tcp_client::write(const std::vector<double>& data, bool end, bool flag) {
    const size_t n = data.size() * sizeof(double) + 2;
    char send_data[4 + n];
    send_data[0] = static_cast<char>(n & 0xFF);
    send_data[1] = static_cast<char>((n >> 8) & 0xFF);
    send_data[2] = static_cast<char>((n >> 16) & 0xFF);
    send_data[3] = static_cast<char>((n >> 24) & 0xFF);

    send_data[4] = end;
    send_data[5] = flag;

    memcpy(&send_data[6], &data[0], data.size() * sizeof(double));

    conn->write_n(send_data, 4 + n);
}

void tcp_client::write(std::string data) {
    const size_t n = data.size();
    char send_data[4 + n];
    send_data[0] = static_cast<char>(n & 0xFF);
    send_data[1] = static_cast<char>((n >> 8) & 0xFF);
    send_data[2] = static_cast<char>((n >> 16) & 0xFF);
    send_data[3] = static_cast<char>((n >> 24) & 0xFF);

    memcpy(&send_data[4], &data[0], data.size());

    conn->write_n(send_data, 4 + n);
}

int main() {
    std::string command =
        "curl --verbose -H 'Content-Type: application/json' -d '\{\"secret\": \"secret\", \"socket_type\": \"tcp\"}' -X POST http://localhost:8080/connect";

    char buffer[128];
    std::string result = "";

    // Open pipe to file
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        printf("popen failed!\n");
        return 0;
    }

    // read till end of process:
    while (!feof(pipe)) {
        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

    pclose(pipe);
    printf("Connecting to TCP port: %s\n\n", result.data());

    tcp_client client = tcp_client("localhost", stoi(result));

    printf("Reading data response and sending secret\n");
    client.read();
    client.write("secret");
    client.read();

    int n_runs = 1000000;
    std::vector<double> dummy_data = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    std::vector<std::chrono::nanoseconds> times;

    printf("\nStarting timing loop.\n");
    std::chrono::time_point<std::chrono::high_resolution_clock> prev_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n_runs; ++i) {
        client.write(dummy_data);
        client.read();

        const auto update_end = std::chrono::high_resolution_clock::now();
        const auto update_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(update_end - prev_time);

        times.push_back(update_duration);

        prev_time = std::chrono::high_resolution_clock::now();
    }

    // Calculate results
    std::chrono::nanoseconds t_sum(0);

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();

    std::ofstream resfile;
    resfile.open("resfile.csv");
    for (auto time : times) {
        long long int tt = time.count();
        t_sum += time;

        min = min > tt ? tt : min;
        max = max < tt ? tt : max;
        resfile << time.count() << ", ";
    }
    resfile.close();
    printf("time μ: %zu ns, max: %f ns, min: %f ns", t_sum.count() / (times.size() - 1), max, min);
}
