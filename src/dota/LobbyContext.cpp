#include "Dota.h"
#include "irc/RD2LIRC.h"
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

template<class InputIt1, class InputIt2,
    class OutputIt, class Compare>
    OutputIt set_pair_intersection(InputIt1 first1, InputIt1 last1,
        InputIt2 first2, InputIt2 last2,
        OutputIt d_first, Compare comp)
{
    while (first1 != last1 && first2 != last2) {
        if (comp(*first1, *first2)) {
            ++first1;
        }
        else {
            if (!comp(*first2, *first1)) {
                *d_first++ = std::make_pair(*first1++, *first2++);
            }
            else
                ++first2;
        }
    }
    return d_first;
}

void LobbyContext::Update(const proto::dota::CSODOTALobby* newLobby)
{
    using namespace proto::dota;
    if (_oldLobby == nullptr)
    {
        _oldLobby = std::make_unique<proto::dota::CSODOTALobby>(*newLobby);
        OnJoin();
        return;
    }

    if (newLobby == nullptr)
    {
        OnLeave();
        _oldLobby = nullptr;
        return;
    }

    std::vector<proto::dota::CDOTALobbyMember> joinedPlayers;
    std::vector<proto::dota::CDOTALobbyMember> leftPlayers;
    std::vector<proto::dota::CDOTALobbyMember> commonPlayers;
    std::vector<std::pair<proto::dota::CDOTALobbyMember, proto::dota::CDOTALobbyMember>> oldPlayers;

    auto memberCompare = [](const auto& a, const auto& b)
    {
        return a.id() < b.id();
    };

    auto oldMembers = _oldLobby->members();
    std::sort(oldMembers.begin(), oldMembers.end(), memberCompare);
    auto newMembers = newLobby->members();
    std::sort(newMembers.begin(), newMembers.end(), memberCompare);

	if (oldMembers.size() != newMembers.size())
		PlayerCountChange(newMembers.size());

    std::set_difference(
        oldMembers.begin(), oldMembers.end(),
        newMembers.begin(), newMembers.end(),
        std::inserter(leftPlayers, leftPlayers.begin()),
        memberCompare
    );

    std::set_difference(
		newMembers.begin(), newMembers.end(), 
		oldMembers.begin(), oldMembers.end(),
        std::inserter(joinedPlayers, joinedPlayers.begin()),
        memberCompare
    );

    set_pair_intersection(
        oldMembers.begin(), oldMembers.end(),
        newMembers.begin(), newMembers.end(),
        std::inserter(oldPlayers, oldPlayers.begin()),
        memberCompare
    );


    for (const auto& member : joinedPlayers)
    {
        auto id = steam::SteamID::ToAccountID(member.id());
        PlayerJoin(id);
        PlayerTeamChange(id, DOTA_GC_TEAM::DOTA_GC_TEAM_NOTEAM, member.team());
    }

    for (const auto& member : leftPlayers)
    {
        auto id = steam::SteamID::ToAccountID(member.id());
        PlayerLeave(id);
    }

    for (const auto& member : oldPlayers)
    {
        auto id = steam::SteamID::ToAccountID(member.first.id());
        if (member.first.team() != member.second.team())
            PlayerTeamChange(id, member.first.team(), member.second.team());
    }

    for (const auto& member : _oldLobby->members())
    {
        auto iter = std::find_if(newLobby->members().begin(), newLobby->members().end(),
            [&](const auto& old)
        {
            return member.id() == old.id();
        });
        if (iter == newLobby->members().end())
        {
            PlayerLeave(steam::SteamID::ToAccountID(member.id()));
        }
    }

    if (_oldLobby->game_state() != newLobby->game_state())
    {
        OnGameStateChange(_oldLobby->game_state(), newLobby->game_state());
    }


    _oldLobby = std::make_unique<proto::dota::CSODOTALobby>(*newLobby);
}