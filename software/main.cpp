#include <iostream>
#include <cstdio>
#include "serialport.h"

std::string result;
SerialPort *sp;

int main() {

    try {
        sp = new SerialPort("/dev/ttyv0",115200);
    } catch (std::exception &ex) {
        std::cerr << "Serial Port Error: " << ex.what() << std::endl;
    }
    std::cout << "VTTY Tester" << std::endl;
    std::string line;

    size_t i = 0;

    std::cout << ">";

    while(true)
    {
        std::getline(std::cin, line);

        try {
            line += '\n';
            sp->write(line);

            if(sp->read(&result)>0)
            {
                std::cout << "Hash: " << result << std::endl;
            }

            std::cout << ">";

        } catch (std::exception &ex) {
            std::cerr << ex.what() << std::endl;
        }
    }

    delete sp;
    return 0;
}