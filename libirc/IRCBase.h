#pragma once 
#include <queue>
#include <memory>
#include <sstream>
#include "../libsteam/libsteam/NetTS.h"
#include "ksignals.h"

namespace IRC
{
	class IRCBase
	{
	public:
		IRCBase(net::io_context& io) : _socket(std::make_unique<net::ip::tcp::socket>(io)), _io(io) { _buffer.reserve(1024); }

		void Connect(net::ip::tcp::resolver::results_type& endpoint);
		void Disconnect();
		bool IsConnected();
		void SendRaw(const std::string message);
	

		ksignals::Event<void(std::string_view)> Raw;
		ksignals::Event<void()> Connected;
		ksignals::Event<void(std::error_code)> Disconnected;
	private:
		void Disconnect(const std::error_code& ec);
		void Read();
		void Write();

		std::queue<std::string> _outBuffer;
		std::unique_ptr<net::ip::tcp::socket> _socket;
        std::string _buffer;
        const std::string _delimiter = "\r\n";
		bool _connected = false;
    protected:
        net::io_context& _io;
	};

}