#include <iostream>
#include <string>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <chrono>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#endif

using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::vector;
using std::thread;

vector<char> vBuffer(16 * 1024);

void GetData(asio::ip::tcp::socket& socket) 
{
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, std::size_t length) {
            if (!ec) 
            {
                cout << "\n\nRead " << length << " bytes\n\n";
                for (int i = 0; i < length; ++i)
                    cout << vBuffer[i];

                GetData(socket);

            }
        }
      );
}

int main()
{
    asio::error_code ec;
    asio::io_context context; 

    asio::io_context::work idleWork(context); //fake tasks so it keeps running

    thread thrContext = thread([&]() {
        context.run();
        }); //start the context and it won't block the main program if it has to stop to wait

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80); //get the address of somewhere in order to connect

    asio::ip::tcp::socket socket(context); //creating a socket for the implementation of the context

    socket.connect(endpoint, ec); // try and connect

    if (!ec)
    {
        cout << "Connected!" << endl;
    }
    else {
        cout << "Failed to connect: \n" << ec.message() << endl;
    }

    if (socket.is_open()) 
    {
        GetData(socket);

        string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

     
        /*program will do something else so asio deals with the data in the background*/
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20000ms);

        context.stop();
        if (thrContext.joinable()) thrContext.join();

        /*it read manually - you know, good to know*/       
        /*socket.wait(socket.wait_read);


        size_t bytes = socket.available();
        cout << "Bytes available: " << bytes << endl;

        if (bytes > 0) {
            vector<char> vBuffer(bytes);
            socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

            for (auto c : vBuffer) {
                cout << c;
            }
        }*/
    }

    std::cin.get();
    return 0;
}