#pragma once

#include <Dota.h>
#include "LobbyContext.h"
#include "BotStateMachine.h"
#include "PlayerCache.h"
#include <RecurringTimer.h>

class Dota : public steam::Dota
{
public:
	Dota(net::io_context& io, const std::string& user, const std::string& password);
    ~Dota() = default;
    void Reconnect();
    void GetPlayerMMR(uint32_t aid);
    void SendLobby(const std::string& msg);
	void SendLobbyUpdate(int count);
    LobbyContext _lobbyContext;
	struct
	{
		ksignals::Event<void(std::string_view fp)> Firstpick;
		ksignals::Event<void(std::string_view gm)> Gamemode;
		ksignals::Event<void(std::string_view nameteam, int slot)> Kick;
		ksignals::Event<void(std::string_view nameteam, int slot)> Teamkick;
		ksignals::Event<void(std::string_view server)> Server;
		ksignals::Event<void()> Swap;
		ksignals::Event<void(std::string_view option)> Start;
	} Events;

	class RD2LIRC* irc;
	BotStateMachine state;
	PlayerCache playerCache;
private:
	void RunCommand(const std::vector<std::string_view>& args);
	
    int64_t _lobbyChatId;
    std::string _user;
    std::string _password;
    bool _connected = false;
    steam::RecurringTimer _reconnectTimer;
    
};