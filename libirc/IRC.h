#pragma once 
#include <queue>
#include <memory>
#include <string_view>
#include "IRCBase.h"
#include "ksignals.h"

namespace IRC
{
	class IRC : public IRCBase
	{
	public:
		IRC(net::io_context& io) : IRCBase(io) { Raw.connect([&](auto msg) { Parse(msg); }); }

		// prefix, args, postfix
		ksignals::Event<void(const std::string_view&, const std::vector<std::string_view>&, const std::string_view&)> Message;
	private:
		void Parse(const std::string_view& message);
	};

}