#include "IRCBase.h"

void IRC::IRCBase::Connect(net::ip::tcp::resolver::results_type & endpoint)
{
    net::async_connect(*_socket, endpoint,
		[&](const std::error_code& ec, auto)
	{
		if (ec)
		{
			Disconnected(ec);
			return;
		}
		Connected();
		Read();
	});
}

void IRC::IRCBase::Disconnect()
{
	Disconnect(std::error_code());
}

bool IRC::IRCBase::IsConnected()
{
	return _connected;
}

void IRC::IRCBase::SendRaw(const std::string message)
{
	_outBuffer.push(message);

	// If the write worker isn't running, launch it now
	if (_outBuffer.size() == 1) 
	{
		Write();
	}
}

void IRC::IRCBase::Write()
{
	auto& message = _outBuffer.front();
    net::async_write(*_socket, net::buffer(message.data(), message.length()),
		[&](std::error_code ec, std::size_t)
	{
		if (ec)
		{
			Disconnected(ec);
			return;
		}

		// TODO: Check written length
		_outBuffer.pop();
		if (!_outBuffer.empty())
			Write();
	});
}

void IRC::IRCBase::Disconnect(const std::error_code & ec)
{
	_socket = std::make_unique<net::ip::tcp::socket>(_io);
	_connected = false;
	Disconnected(ec);
}

void IRC::IRCBase::Read()
{
    net::async_read_until(*_socket, net::dynamic_buffer(_buffer), _delimiter,
        [&](std::error_code ec, std::size_t length)
    {
        if (ec)
        {
            Disconnected(ec);
            return;
        }
        if (length == 0)
        {
            Read();
            return;
        }

        // Read line to a string_view
        std::string_view line(_buffer.data(), length-2);
		
        // Invoke Raw
        Raw(line);

        // Discard the line
		_buffer = _buffer.substr(length);
		
		// Read next
        Read();
    });
}
