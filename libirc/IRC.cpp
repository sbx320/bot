#include "IRC.h"
#include <string_view>

std::string_view viewUntil(const std::string_view& sv, char c, std::size_t& offset)
{
    auto postfix = sv.find_first_of(c, offset);
    if (postfix == sv.npos)
    {
        auto result = sv.substr(offset);
        offset = sv.npos;
        return result;
    }
    else
    {
        auto result = sv.substr(offset, postfix - offset);
        offset = postfix + 1;
        return result;
    }
}


void IRC::IRC::Parse(const std::string_view & message)
{
    std::size_t offset = 0;
    std::string_view prefix;
    if (message[0] == ':')
    {
        offset = 1;
        prefix = viewUntil(message, ' ', offset);
    }

    std::vector<std::string_view> params;
    std::string_view postfix;
    while (offset != message.npos)
    {
        auto oldOffset = offset;
        auto param = viewUntil(message, ' ', offset);
        if (param.length() > 0 && param[0] == ':')
        {
            postfix = message.substr(oldOffset+1);
            break;
        }

        params.push_back(param);
    }

    Message(prefix, params, postfix);
}

