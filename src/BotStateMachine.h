#pragma once

#include <string>
#include "irc/RD2LIRC.h"

class BotStateMachine
{
public:
    enum class BotState
    {
        IDLE,
        STOPPING,
        NOSTEAM,
        NODOTA,
        READY,
        INLOBBY,
        LOBBYRUNNING
    };

    inline static const char* StateName(BotState state)
    {
        switch (state)
        {
        case BotState::IDLE:
            return "IDLE";
        case BotState::NOSTEAM:
            return "NOSTEAM";
        case BotState::NODOTA:
            return "NODOTA";
        case BotState::READY:
            return "READY";
        case BotState::INLOBBY:
            return "INLOBBY";
        case BotState::LOBBYRUNNING:
            return "LOBBYRUNNING";
        default:
            return "thisshouldneverhappen";
        }
    }

private:
	BotState _State = BotState::IDLE;
public:
	ksignals::Event<void(BotState newstate)> StateChange;
    void TransitionTo(BotState state); 
	BotState State() { return _State; }

};