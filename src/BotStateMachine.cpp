#include "BotStateMachine.h"

void BotStateMachine::TransitionTo(BotState state)
{
    _State = state;
	StateChange(state);
}