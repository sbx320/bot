#include "IRCBase.h"

void IRC::IRCBase::Connect(boost::asio::ip::tcp::resolver::iterator & endpoint)
{
    boost::asio::async_connect(*_socket.get(), endpoint,
		[&](const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator)
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
	Disconnect(boost::system::error_code());
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
    boost::asio::async_write(*_socket, boost::asio::buffer(message.data(), message.length()),
		[&](boost::system::error_code ec, std::size_t)
	{
		if (ec)
		{
			Disconnected(ec);
			return;
		}

		// TODO: Check written length
		_outBuffer.pop();
		if (_outBuffer.size() > 0)
			Write();
	});
}

void IRC::IRCBase::Disconnect(const boost::system::error_code & ec)
{
	_socket = std::make_unique<boost::asio::ip::tcp::socket>(_io);
	_connected = false;
	Disconnected(ec);
}

void IRC::IRCBase::Read()
{
    boost::asio::async_read_until(*_socket, _buffer, _delimiter,
        [&](boost::system::error_code ec, std::size_t length)
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
        std::string line;
        std::istream istream(&_buffer);
        std::getline(istream, line);
        std::string_view view = line;
        
        // Invoke Raw
        Raw(view.substr(0, view.size() - 1));

        // Read next
        Read();
    });
}
