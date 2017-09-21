#pragma once
#include <boost/asio.hpp>
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
#include <dota/gcsystemmsgs.pb.h>
#include <dota/dota_gcmessages_msgid.pb.h>
#include <../libsteam/vendor/ksignals.h>

class LobbyContext
{
public:
    ksignals::Event<void()> OnJoin;
    ksignals::Event<void(proto::dota::DOTA_GameState, proto::dota::DOTA_GameState)> OnGameStateChange;
    ksignals::Event<void()> OnLeave;
    ksignals::Event<void(uint32_t)> PlayerJoin;
    ksignals::Event<void(uint32_t)> PlayerLeave;
    ksignals::Event<void(uint32_t, proto::dota::DOTA_GC_TEAM, proto::dota::DOTA_GC_TEAM)> PlayerTeamChange;
	ksignals::Event<void(uint32_t)> PlayerCountChange;
    bool Inhouse;
    void Update(const proto::dota::CSODOTALobby* newLobby);
    inline void Reset() { _oldLobby = nullptr; }
private:
    std::unique_ptr<proto::dota::CSODOTALobby> _oldLobby;
};