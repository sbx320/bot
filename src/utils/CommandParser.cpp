#include <string>
#include <vector>

std::string_view stringUntil(const std::string_view& from, const char c, size_t offset = 0)
{
	auto pos = from.find_first_of(c, offset);
    if (pos == from.npos)
        return from.substr(offset);

    return from.substr(offset, pos - offset);
}

/*
	Parses a command string like 
	foo "bar baz" foobar
	into a vector
		{ "foo", "bar baz" "foobar" }
*/
std::vector<std::string_view> ParseCommand(const std::string_view& cmd)
{
	std::vector<std::string_view> vec;
	bool insideString = false;
	std::size_t offset = 0;
	for (std::size_t i = 0; i < cmd.length(); ++i)
	{
		auto c = cmd[i];
		if (c == '"')
		{
			if (insideString)
			{
				vec.push_back(cmd.substr(offset, i - offset));
				insideString = false;

				// Skip next space if any
				++i;
				offset = i + 1;
			}
			else
			{
				offset = i + 1;
				insideString = true;
			}
		}

		if (c == ' ' && !insideString)
		{
			vec.push_back(cmd.substr(offset, i - offset));
			offset = i+1;
		}
	}
	if (cmd.back() != '"')
	{
		vec.push_back(cmd.substr(offset));
	}

	return vec;
}

