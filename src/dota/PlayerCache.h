#pragma once

#include <cstdint>
#include <unordered_map>

class PlayerCache
{
public:
	class Player
	{
	public:
		Player(uint32_t id) : id(id) {};
		uint32_t id;
		uint32_t team = 0;
		bool ready = false;
	};

	Player& get(uint32_t id) {
		if(_players.find(id) == _players.end())
			return _players.emplace(id, Player(id)).first->second;
		return _players.at(id);
	}

	void clear() {
		_players.clear();
	}

private:
	std::unordered_map<uint32_t, Player> _players;
};