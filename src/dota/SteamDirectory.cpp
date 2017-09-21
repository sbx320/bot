#include "SteamDirectory.h"
#include <cstdint>
#include <json.hpp>
using json = nlohmann::json;
#include <queue>
#include <streambuf>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

using namespace boost::asio;
using namespace boost::beast;

std::pair<std::string, std::string> SteamDirectory::GetServer()
{
    static std::queue<std::pair<std::string, std::string>> _cached;
    std::cout << "Requesting Server \n";
    if (!_cached.empty())
    {
        std::pair<std::string, std::string> result = _cached.back();
        _cached.pop();
        return result;
    }
    std::cout << "Updating cache\n";
    // Else update the cache
    try
    {
        std::string const host = "api.steampowered.com";
        io_service ios;
        ip::tcp::resolver r{ ios };
        ip::tcp::socket sock{ ios };
        connect(sock,
            r.resolve(ip::tcp::resolver::query{ host, "80" }));

        // Send HTTP request using beast
        http::request<http::empty_body> req;
        req.method(http::verb::get);
        req.target("/ISteamDirectory/GetCMList/v1/?cellid=0");
        req.version = 11;
		
        req.set("Host", host);
        req.set("User-Agent", "rd2lbot - See rd2l.com or message /id/sbx320 for info");
        req.prepare_payload();
        http::write(sock, req);

        // Receive and print HTTP response using beast
        http::response<http::string_body> res;
		flat_buffer b;
		http::read(sock, b, res);

        json data = json::parse(res.body);
        for (auto&& jsonentry : data["response"]["serverlist"])
        {
            std::string entry = jsonentry;
            auto offset = entry.find_first_of(':');
            _cached.emplace(entry.substr(0, offset), entry.substr(offset + 1));
        }

        std::cout << "Cached Now has " << _cached.size() << "\n";
        return GetServer();
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
        std::terminate();
    }
}