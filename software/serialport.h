//
// Created by baris on 27.08.2020.
//

#ifndef VTTYTESTER_SERIALPORT_H
#define VTTYTESTER_SERIALPORT_H

#include <cstdio>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/experimental/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio.hpp>

class SerialPort {

public:
    explicit SerialPort(const char *device = "/dev/ttyUSB0/", unsigned int baud_rate = 115200);

    std::size_t write(std::string &s);

    int read(std::string *dest);
private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;

    std::string shaStr;

};


#endif //VTTYTESTER_SERIALPORT_H
