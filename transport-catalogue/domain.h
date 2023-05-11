#ifndef DOMAIN_H
#define DOMAIN_H

#include <string>
#include <set>
#include <vector>

namespace domain
{

struct Stop
{
    Stop() = default;

    Stop(std::string_view _name,
         std::string_view _latitude,
         std::string_view _longitude);

    ~Stop() = default;

    Stop(const Stop &other) = default;

    Stop(Stop &&other) noexcept;

    Stop &operator=(const Stop &other) = default;

    bool operator==(const Stop &other) const;

    std::size_t id_;
    std::string_view name_;
    double latitude_;
    double longitude_;
};


struct Bus
{
    Bus() = default;

    ~Bus() = default;

    Bus(const Bus &other);

    Bus(Bus &&other) noexcept;

    Bus &operator=(const Bus &other);

    bool operator==(const Bus &other) const;

    std::string_view name_;
    std::vector<std::string_view> route_;
    bool is_circul_ = false;
    size_t number_unique_stops_ = 0;
    long double route_length_ = 0.0;
    double curvature_ = 0.0;
};

struct BusStat
{
    BusStat() = default;

    std::string_view name_;
    size_t number_stops_ = 0;
    size_t number_unique_stops_ = 0;
    double route_length_ = 0.0;
    double curvature_ = 0.0;
    bool is_circul_ = false;
};

struct StopStat
{
    StopStat() = default;

    std::string_view name_;
    std::set<std::string_view> buses_;
    bool is_exist_ = false;
};

struct BusRouteInfo
{
    std::string_view name;
    int span_count = 0;
    double time = 0.0;
};

struct WaitInfo
{
    std::string_view name;
    double time = 0.0;
};

} // namespace domain
#endif // DOMAIN_H
