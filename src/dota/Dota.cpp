#include "Dota.h"
#include "irc/RD2LIRC.h"
#include <iostream>
#include "utils/CommandParser.h"
#include "BotStateMachine.h"
#include "SteamDirectory.h"
#include <dota/dota_gcmessages_common.pb.h>
#include <dota/dota_gcmessages_client.pb.h>
#include <dota/dota_gcmessages_client_chat.pb.h>
#include <dota/dota_gcmessages_client_match_management.pb.h>
#include <steam/steammessages_base.pb.h>
#include <steam/steammessages_clientserver.pb.h>
#include <steam/steammessages_clientserver_2.pb.h>
#include <steam/steammessages_clientserver_login.pb.h>
#include <dota/gcsystemmsgs.pb.h>
#include <dota/dota_gcmessages_msgid.pb.h>
#include "utils/CommandHelper.h"
#include <optional>

#include "LobbyContext.h"

Dota::Dota(net::io_context& io, const std::string& user, const std::string& password)
    : steam::Dota(io), _reconnectTimer(io)
{
	_user = user;
	_password = password;
    _reconnectTimer.SetCallback([&](auto)
    {
        Reconnect();
    });

	state.StateChange.connect([&](BotStateMachine::BotState newstate)
	{
		irc->Send("!state " + std::string(BotStateMachine::StateName(newstate)));
	});

	Connected.connect([&]()
	{
		std::cout << "[Steam] Connected! Logging in...\n";
		_reconnectTimer.Stop();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		LogOn(_user, _password);
	});

	LoggedOn.connect([&](const proto::steam::CMsgClientLogonResponse& msg)
	{
		if (msg.eresult() != 1)
		{
			std::cout << "[Steam] Unable to logon" + std::to_string(msg.eresult()) + "\n";
			return;
		}

		std::cout << "[Steam] Successfully logged on to Steam!\n";

		SetPersonaState(steam::EPersonaState::Online);

		state.TransitionTo(BotStateMachine::BotState::NODOTA);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		Start();
	});

	Disconnected.connect([&](auto ec)
	{
		std::cout << "[Steam] Disconnected!\n";

		if (state.State() != BotStateMachine::BotState::IDLE)
		{
			state.TransitionTo(BotStateMachine::BotState::NOSTEAM);
			using namespace std::literals::chrono_literals;
			_reconnectTimer.Start(10s);
		}
		_lobbyContext.Reset();
	});

	LoggedOff.connect([&](const proto::steam::CMsgClientLoggedOff& msg)
	{
		std::cout << "[Steam] Logged off..." << msg.eresult() << "\n";
		_lobbyContext.Reset();
		Disconnect();
	});


	LobbyChanged.connect([&](auto newLobby)
	{
		_lobbyContext.Update(newLobby);
	});


	ProfileCardsReceived.connect([&](const proto::dota::CMsgDOTAProfileCard& cards)
	{
		irc->Send("!player-mmr " + std::to_string(cards.account_id()) + " " + std::to_string(cards.rank_tier()));
	});

	GCConnected.connect([&]
	{
		state.TransitionTo(BotStateMachine::BotState::READY);
	});

	GCDisconnected.connect([&]
	{
		state.TransitionTo(BotStateMachine::BotState::NODOTA);
	});

	ChatMessage.connect([&](uint32_t accountid, auto& name, auto& msg)
	{
		if (msg.size() == 0 || msg[0] != '!')
			return;

		auto params = ParseCommand(msg);
		if (params.size() < 1)
			return;

		RunCommand(params);
	});

	ChatJoined.connect([&](uint64_t id, proto::dota::DOTAChatChannelType_t type)
	{
		if (type == proto::dota::DOTAChannelType_Lobby)
		{
			_lobbyChatId = id;
		}
	});

	_lobbyContext.OnLeave.connect([&]
	{
		state.TransitionTo(BotStateMachine::BotState::READY);
		playerCache.clear();
		std::cout << "left lobby\n";
	});

	_lobbyContext.OnJoin.connect([&]
	{
		JoinChatChannel("Lobby_" + std::to_string(Lobby->lobby_id()), proto::dota::DOTAChatChannelType_t::DOTAChannelType_Lobby);
		state.TransitionTo(BotStateMachine::BotState::INLOBBY);
		std::cout << "joined lobby\n";
		SendLobbyUpdate(Lobby->members_size());
		KickPlayerFromLobbyTeam(steam::SteamID::ToAccountID(steamID));
		if (Lobby->game_state() >= proto::dota::DOTA_GameState::DOTA_GAMERULES_STATE_HERO_SELECTION) {
			std::cout << "leaving lobby\n";
			LeaveLobby();
		}
	});

	_lobbyContext.PlayerJoin.connect([&](auto id)
	{
		if(!playerCache.get(id).ready)
			irc->Send("!get-team " + std::to_string(id));
	});

	_lobbyContext.OnGameStateChange.connect([&](auto from, auto to)
	{
		/*if (from >= proto::dota::DOTA_GameState::DOTA_GAMERULES_STATE_HERO_SELECTION) {
			std::cout << "leaving lobby\n";
			LeaveLobby();
		}*/
		
		printf("state: %d -> %d", from, to);
	});

	_lobbyContext.PlayerCountChange.connect([&](auto count)
	{
		SendLobbyUpdate(count);
	});

	Events.Firstpick.connect([&](std::string_view pick)
	{
		proto::dota::DOTA_CM_PICK fp;
		if (pick == "radiant")
		{
			fp = proto::dota::DOTA_CM_GOOD_GUYS;
			SendLobby("Set firstpick to radiant");
		}
		else if (pick == "dire")
		{
			fp = proto::dota::DOTA_CM_BAD_GUYS;
			SendLobby("Set firstpick to dire");
		}
		else if (pick == "random")
		{
			fp = proto::dota::DOTA_CM_RANDOM;
			SendLobby("Set firstpick to random");
		}
		else
		{
			SendLobby("Invalid firstpick choice!");
			SendLobby("Valid: random, radiant, dire!");
			return;
		}

		SetLobbyFirstpick(fp);
	});

	Events.Gamemode.connect([&](std::string_view gamemode)
	{
		if (gamemode == "cm")
		{
			SetLobbyGamemode(2);
			SendLobby("Set gamemode to Captains Mode");
			return;
		}
		if (gamemode == "ap")
		{
			SetLobbyGamemode(1);
			SendLobby("Set gamemode to All Pick");
			return;
		}
		if (gamemode == "cd")
		{
			SetLobbyGamemode(16);
			SendLobby("Set gamemode to Captains Draft");
			return;
		}
		SendLobby("Invalid gamemode choice");
		SendLobby("Gamemodes: cm, ap, cd");
	});

	auto KickHelper = [](proto::dota::CSODOTALobby* lobby, std::string_view nameteam, int id) -> std::optional<proto::dota::CDOTALobbyMember>
	{
		// Method A: Kick via team + slot number
		proto::dota::DOTA_GC_TEAM team = proto::dota::DOTA_GC_TEAM_NOTEAM;
		if (nameteam == "radiant")
			team = proto::dota::DOTA_GC_TEAM_GOOD_GUYS;
		else if (nameteam == "dire")
			team = proto::dota::DOTA_GC_TEAM_BAD_GUYS;

		if (team != proto::dota::DOTA_GC_TEAM_NOTEAM)
		{
			if (auto iter = std::find_if(lobby->members().begin(), lobby->members().end(), [&](auto member) {
				return member.slot() == id && member.team() == team;
			}); iter != lobby->members().end())
			{
				return *iter;
			}
			return std::nullopt;
		}

		// Method B: Kick via name
		std::vector<proto::dota::CDOTALobbyMember> matches;
		std::copy_if(lobby->members().begin(), lobby->members().end(), std::back_inserter(matches),
			[&](auto member)
		{
			if (member.name().size() < nameteam.size())
				return false;

			return member.name().substr(0, nameteam.size()) == nameteam;
		});

		if (matches.size() != 1)
			return std::nullopt;
		return matches[0];
	};


	Events.Kick.connect([&](std::string_view nameteam, int id)
	{
		if (auto player = KickHelper(Lobby.get(), nameteam, id); player.has_value())
		{
			KickPlayerFromLobby(steam::SteamID::ToAccountID(player->id()));
			SendLobby("Kicked " + player->name());
		}
		else {
			SendLobby("Error: No such player");
		}
	});

	Events.Teamkick.connect([&](std::string_view nameteam, int id)
	{
		if (auto player = KickHelper(Lobby.get(), nameteam, id); player.has_value())
		{
			KickPlayerFromLobbyTeam(steam::SteamID::ToAccountID(player->id()));
			SendLobby("Teamkicked " + player->name());
		}
		else {
			SendLobby("Error: No such player");
		}
	});

	Events.Server.connect([&](std::string_view server)
	{
		if (server == "use")
		{
			SetLobbyServer(2);
			SendLobby("Set server to US East");
			return;
		}
		if (server == "usw")
		{
			SetLobbyServer(1);
			SendLobby("Set server to US West");
			return;
		}
		if (server == "eu" || server == "euw" || server == "lux")
		{
			SetLobbyServer(3);
			SendLobby("Set server to EU West");
			return;
		}
		SendLobby("Invalid server choice");
		SendLobby("Servers: use, usw, euw");
	});


	Events.Swap.connect([&]
	{
		SendLobby("Team Swapped");
		FlipLobbyTeams();
	});

	Events.Start.connect([&](std::string_view option)
	{
		bool standin = false;
		bool force = false;
		if (option == "standin")
			standin = true;
		if (option == "force")
			force = true;
		
		auto& lobby = Lobby;
		if (!force)
		{
			int count = 0;
			for (auto& member : lobby->members())
			{
				if (member.team() == proto::dota::DOTA_GC_TEAM_BAD_GUYS || member.team() == proto::dota::DOTA_GC_TEAM_GOOD_GUYS)
					count++;
			}

			if (false && count != 10)
			{
				SendLobby("Cannot start with less than 10 players");
				return;
			}

			if (!standin)
			{
				bool abort = false;
				std::map<uint32_t, std::vector<proto::dota::CDOTALobbyMember>> radiant;
				std::map<uint32_t, std::vector<proto::dota::CDOTALobbyMember>> dire;
				int radiantLastTeam = 0;
				int direLastTeam = 0;
				for (auto& member : lobby->members())
				{
					if (member.team() == proto::dota::DOTA_GC_TEAM_GOOD_GUYS ||
						member.team() == proto::dota::DOTA_GC_TEAM_BAD_GUYS)
					{
						auto player = playerCache.get(steam::SteamID::ToAccountID(member.id()));
						if (!player.ready) {
							SendLobby("Information for " + member.name() + " has not been loaded yet");
							abort = true;
							continue;
						}

						if (member.team() == proto::dota::DOTA_GC_TEAM_GOOD_GUYS)
							radiant[player.team].push_back(member);
						else 
							dire[player.team].push_back(member);
					}
				}
				if (abort)
					return;

				auto displayTeams = [&](auto& team)
				{
					for (auto&[team, players] : team)
					{
						std::string playerstr = "-> ";
						for (auto& player : players)
							playerstr += player.name() + ", ";

						SendLobby(playerstr.substr(0, playerstr.length() - 2));
					}
				};

				if (radiant.size() >= 2)
				{
					if (radiant.size() > 2 || radiant[0].size() > 1 && radiant[1].size() > 1)
					{
						SendLobby("Dire has more than one standin! The following players are on the same team:");
						displayTeams(radiant);
						abort = true;
					}
				}
				if (dire.size() >= 2)
				{
					if (dire.size() > 2 || dire[0].size() > 1 && dire[1].size() > 1)
					{
						SendLobby("Dire has more than one standin! The following players are on the same team:");
						displayTeams(dire);
						abort = true;
					}
				}

				if (abort)
					return;

			}
		}

		LaunchLobby();
	});
}

void Dota::SendLobbyUpdate(int count)
{
	irc->Send("!lobby-update " + std::to_string(count-1)
		+ " " + std::to_string(Lobby->server_region())
		+ " " + std::to_string(_lobbyContext.Inhouse ? 1 : 0)
	);
}

void Dota::RunCommand(const std::vector<std::string_view>& args)
{
	command::RunCommandOptional(args,
		"!fp", &Events.Firstpick,
		"!firstpick", &Events.Firstpick,
		"!gm", &Events.Gamemode,
		"!gamemode", &Events.Gamemode,
		"!k", &Events.Kick,
		"!kick", &Events.Kick,
		"!tk", &Events.Teamkick,
		"!teamkick", &Events.Teamkick,
		"!server", &Events.Server,
		"!swap", &Events.Swap,
		"!start", &Events.Start
	);
}

void Dota::Reconnect()
{
    std::cout << "[Steam] Reconnecting...!\n";
    
    auto remote = SteamDirectory::GetServer();

    std::cout << "[Steam] Selected Server " << remote.first << ":" << remote.second << "\n";
    net::ip::tcp::resolver resolver(_io);
    auto dotaendpoint = resolver.resolve(remote.first, remote.second);
    Connect(dotaendpoint);
}

void Dota::GetPlayerMMR(uint32_t aid)
{
    RequestProfileCards(aid);
}

void Dota::SendLobby(const std::string & msg)
{
    SendChatMessage(_lobbyChatId, msg);
}