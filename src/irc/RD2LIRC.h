#pragma once
#include <future>
#include "IRC.h"
#include "ksignals.h"

class RD2LIRC : public IRC::IRC
{
public:
	RD2LIRC(net::io_context& io, const std::string& host, const std::string& port, const std::string& user, const std::string& password);
    void Send(const std::string message);

	class Dota* dota;
private:
	struct
	{
		ksignals::Event<void()> Botinfo;
		ksignals::Event<void()> Start;
		ksignals::Event<void()> Stop;
		ksignals::Event<void(std::string_view name, std::string_view password, int ticket, int server, bool inhouse)> CreateLobby;
		ksignals::Event<void()> KillLobby;
		ksignals::Event<void(uint32_t id)> Invite;
		ksignals::Event<void(uint32_t id)> GetMMR;
		ksignals::Event<void(uint32_t id, uint32_t team)> Team;
	} Events;

	void RunCommand(const std::vector<std::string_view>& args);
	void HandleCommand(const std::string_view& source, const std::string_view& msg);
    std::string _nick;

	std::vector<std::pair<uint32_t, std::future<uint32_t>>> _pendingTeam;
};