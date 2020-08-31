#include "iostream"
#include "serialport.h"

SerialPort::SerialPort(const char* device, unsigned int baud_rate) : io(), serial(io)
{
    try{
        serial.open(device);

        if(!serial.is_open())
            std::cerr << "Failed connection" << std::endl;
        else {
            serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
            serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
            serial.set_option(boost::asio::serial_port_base::character_size(8));
            serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));

            const int timeout = 100;
            ::setsockopt(serial.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof timeout);
            ::setsockopt(serial.native_handle(), SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof timeout);

        }
    } catch (const boost::system::system_error& err) {
        std::cerr << "Serial port error: " << err.what() << std::endl;
    }
}

int SerialPort::read(std::string *dest)
{
    boost::asio::streambuf buff;

    try{
        boost::asio::read_until(serial, buff, "\r\n");
    } catch (const boost::system::system_error& err) {
        std::cerr << "Failed to read: " << err.what() << std::endl;
    }

    *dest = std::string( (std::istreambuf_iterator<char>(&buff)), std::istreambuf_iterator<char>() );

    return 500;
}

std::size_t SerialPort::write(std::string& s)
{
    size_t written = 0;

    try{
        written = boost::asio::write(serial,boost::asio::buffer(s.c_str(),s.size()));
    } catch (const boost::system::system_error& err) {
        std::cerr << "Failed to write: " << err.what() << std::endl;
    }

    return written;
}