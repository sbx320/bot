#include "RD2LIRC.h"
#include "dota/Dota.h"
#include "utils/CommandParser.h"
#include <iostream>
#include "BotStateMachine.h"
#include "utils/CommandHelper.h"

RD2LIRC::RD2LIRC(net::io_context & io, const std::string& host, const std::string& port, const std::string& user, const std::string& password) :
	IRC::IRC(io)
{
	_nick = user;
	Message.connect([&](const std::string_view& from, const std::vector<std::string_view>& params, const std::string_view& msg)
	{
		if (params.size() < 1)
			return;

        if (params[0] == "366") // End of Names List
        {
			Events.Botinfo();
            return;
        }
		
		if (params[0] == "PING")
		{
            std::string message = "PONG :"; 
            message += msg;
            message +="\r\n";
			SendRaw(message);
			return;
		}

		if (params[0] == "PRIVMSG" && params[1] == "#rd2l")
		{
			if (msg.size() > 1) 
			{
				if (msg[0] == '@' || msg[0] == '!')
					HandleCommand(from, msg);
			}
		}
	});
    
	net::ip::tcp::resolver resolver(_io);
	auto ircendpoint = resolver.resolve(host, port);
	Connect(ircendpoint);

	Connected.connect([&]
	{
		std::string NICK = "NICK ";
		NICK += _nick;
		NICK += "\r\n";
		std::string USER = "USER ";
		USER += _nick;
		USER += " 0 * :RD2L Bot\r\n";

		std::cout << "[IRC] Connected! Logging in...\n";
		std::string passStr = "PASS " + password + "\r\n";
		SendRaw(passStr);
		SendRaw(NICK);
		SendRaw(USER);
	});

	Disconnected.connect([&](const std::error_code& ec)
	{
		std::cout << "[IRC] Disconnected..." << ec.message() << '\n';
		auto ircendpoint = resolver.resolve(host, port);
		Connect(ircendpoint);
	});

	Raw.connect([&](const std::string_view& message)
	{
		std::cout << message << '\n';
	});

	// TODO: Refactor to split command parsing and command handling?
	Events.Botinfo.connect([&]
	{
		Send("!state " + std::string(BotStateMachine::StateName(dota->state.State())));

		if (dota->state.State() == BotStateMachine::BotState::INLOBBY || dota->state.State() == BotStateMachine::BotState::LOBBYRUNNING)
		{
			if (dota->Lobby)
			{
				Send("!lobby-update " + std::to_string(dota->Lobby->members_size())
					+ " " + std::to_string(dota->Lobby->server_region())
					+ " " + std::to_string(dota->_lobbyContext.Inhouse ? 1 : 0)
				);
			}
		}
	});

	Events.Start.connect([&]
	{
		if (dota->state.State() != BotStateMachine::BotState::IDLE)
			return;

		dota->state.TransitionTo(BotStateMachine::BotState::NOSTEAM);
		dota->Reconnect();
	});

	Events.Stop.connect([&]
	{
		dota->state.TransitionTo(BotStateMachine::BotState::IDLE);
		dota->Disconnect();
	});

	Events.CreateLobby.connect([&](std::string_view name, std::string_view password, int ticket, int server, bool inhouse)
	{
		dota->CreateLobby(std::string(name), std::string(password), ticket, server);
		dota->_lobbyContext.Inhouse = inhouse;
	});

	Events.KillLobby.connect([&]
	{
		if (!dota->Lobby)
			return;

		for (auto&& member : dota->Lobby->members())
		{
			dota->KickPlayerFromLobby(steam::SteamID::ToAccountID(member.id()));
		}

		dota->LeaveLobby();
	});

	Events.Invite.connect([&](uint32_t id)
	{
		dota->InvitePlayer(id);
	});
	Events.GetMMR.connect([&](uint32_t id)
	{
		dota->GetPlayerMMR(id);
	});

	Events.Team.connect([&](uint32_t id, uint32_t team)
	{
		auto& player = dota->playerCache.get(id);
		player.team = team;
		player.ready = true;
	});
}

void RD2LIRC::Send(const std::string message)
{
    SendRaw("PRIVMSG #rd2l :" + message + "\r\n");
}

void RD2LIRC::RunCommand(const std::vector<std::string_view>& args)
{
	command::RunCommand(args,
		"!botinfo", &Events.Botinfo,
		"!stop", &Events.Stop,
		"!kill-lobby", &Events.KillLobby,
		"!getmmr", &Events.GetMMR,
		"!invite", &Events.Invite,
		"!team", &Events.Team,
		"!create-lobby", &Events.CreateLobby,
		"!start", &Events.Start
	);
}

void RD2LIRC::HandleCommand(const std::string_view & source, const std::string_view & msg)
{
	size_t offset = 0;
	if (msg[0] == '@')
	{
        offset = 1;
		auto to = stringUntil(msg, ' ', offset);
		if (to != _nick)
			return;
		offset += to.size() + 1;
	}
	else if (msg[0] != '!')
		return;

    auto message = msg.substr(offset);
    auto params = ParseCommand(message);
    if (params.size() < 1)
        return;

	try {
		RunCommand(params);
	}
	catch (...)
	{
		// Commands may throw argument conversion errors
	}
	
}

