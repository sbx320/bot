#pragma once
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
class RecurringTimer
{
public:
    using timer_t = boost::asio::steady_timer;
    using duration_t = std::chrono::steady_clock::duration;
    using func_t = std::function<void(boost::system::error_code)>;
    RecurringTimer(boost::asio::io_service& io);

    void SetCallback(func_t func);
    void Start(const duration_t& interval);
    void Stop();
    bool Running() { return _timer.get() != nullptr; }

private:
    void Expired(const boost::system::error_code& ec);

    func_t _func;
    duration_t _interval;
    boost::asio::io_service& _io;
    std::unique_ptr<timer_t> _timer;
};