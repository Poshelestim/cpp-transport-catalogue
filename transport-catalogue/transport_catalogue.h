#pragma once
#include <cmath>

#include <deque>
#include <set>
#include <unordered_map>

#include "input_reader.h"

struct Bus
{
    Bus() = default;

    ~Bus();

    Bus(const Bus &other);

    Bus(Bus &&other) noexcept;

    Bus operator=(const Bus &other);

    bool operator==(const Bus &other) const;

    std::string_view name_;
    std::vector<std::string_view> route_;
    bool is_circul_ = false;
    size_t number_unique_stops_ = 0;
    long double route_length_ = 0.0;
    double curvature_ = 0.0;
};

struct Stop
{
    Stop() = default;

    Stop(std::string_view _name,
         std::string_view _latitude,
         std::string_view _longitude);

    ~Stop();

    Stop(const Stop &other);

    Stop(Stop &&other) noexcept;

    Stop operator=(const Stop &other);

    bool operator==(const Stop &other) const;

    std::string_view name_;
    double latitude_;
    double longitude_;
};

class TransportCatalogue
{
public:

    struct Hasher
    {
        size_t operator()(const std::pair<std::string_view, std::string_view> &_key) const;

        size_t operator()(std::pair<Stop *, Stop *> _key) const;

        size_t operator()(const Stop *_key) const;

        size_t operator()(const Bus *_key) const;

    };

    struct BusInfo
    {
        BusInfo() = default;

        std::string_view name_ = "";
        size_t number_stops_ = 0;
        size_t number_unique_stops_ = 0;
        double route_length_ = 0.0;
        double curvature_ = 0.0;
        bool is_circul_ = false;
    };

    struct StopInfo
    {
        StopInfo() = default;

        std::string_view name_ = "";
        std::set<std::string_view> buses_;
        bool is_exist = false;
    };

    TransportCatalogue() = default;

    ~TransportCatalogue() = default;

    TransportCatalogue(const TransportCatalogue &other) = delete;

    void addStop(Stop &&_new_stop,
                 const std::vector<std::pair<std::string_view, double> > &distances_to_stops) noexcept;

    Stop* findStop(std::string_view _name);

    void addBus(Bus &&_new_bus) noexcept;

    Bus* findBus(std::string_view _name);

    BusInfo getBusInfo(std::string_view _name);

    StopInfo getStopInfo(std::string_view _name);

private:

    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop *> stopname_to_stops_;

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus *> busname_to_buses_;

    std::unordered_map<std::pair<std::string_view, std::string_view>, double, Hasher> distances_between_stops_;

    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;
};
