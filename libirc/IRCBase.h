#pragma once 
#include <queue>
#include <memory>
#include <sstream>
#include <boost/asio.hpp>
#include "ksignals.h"
namespace IRC
{
	class IRCBase
	{
	public:
		IRCBase(boost::asio::io_service& io) : _socket(std::make_unique<boost::asio::ip::tcp::socket>(io)), _io(io) {}

		void Connect(boost::asio::ip::tcp::resolver::iterator& endpoint);
		void Disconnect();
		bool IsConnected();
		void SendRaw(const std::string message);
	

		ksignals::Event<void(std::string_view)> Raw;
		ksignals::Event<void()> Connected;
		ksignals::Event<void(boost::system::error_code)> Disconnected;
	private:
		void Disconnect(const boost::system::error_code& ec);
		void Read();
		void Write();

		std::queue<std::string> _outBuffer;
		std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
        boost::asio::streambuf _buffer;
        const std::string _delimiter = "\r\n";
		bool _connected = false;
    protected:
        boost::asio::io_service& _io;
	};

}