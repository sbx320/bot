#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include "dota/Dota.h"
#include "irc/RD2LIRC.h"

int main(int argc, const char* argv[]) 
{
	if (argc < 7)
	{
		printf("Usage: ./bot [steamUser] [steamPassword] [irc host] [irc port] [irc name] [irc password]\n");
		return 1;
	}

	std::string steamUser = argv[1];
	std::string steamPassword = argv[2];
	std::string ircHost = argv[3];
	std::string ircPort = argv[4];
	std::string ircUser = argv[5];
	std::string ircPassword = argv[6];

	boost::asio::io_service io;
    printf("Initializing...\n");
	Dota dota(io, steamUser, steamPassword);
	RD2LIRC irc(io, ircHost, ircPort, ircUser, ircPassword);
	dota.irc = &irc;
	irc.dota = &dota;
	printf("Starting...\n");
    io.run();
	printf("Exiting...\n");
    return 0;
}
