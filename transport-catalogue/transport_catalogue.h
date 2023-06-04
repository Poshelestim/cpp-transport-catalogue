#pragma once
#include <cmath>

#include <deque>
#include <optional>
#include <unordered_set>
#include <unordered_map>

#include "domain.h"

class TransportCatalogue
{
    using Bus = domain::Bus;
    using Stop = domain::Stop;
    using CatalogueBuses = std::unordered_map<std::string_view, Bus *>;
    using StopToBuses = std::unordered_map<std::string_view, std::set<std::string_view>>;

public:

    struct Hasher
    {
        size_t operator()(std::pair<Stop *, Stop *> _key) const;

        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &_key) const
        {
            return std::hash<T1>()(_key.first) + 37 * std::hash<T2>()(_key.second);
        }

        size_t operator()(const Stop *_key) const;

        size_t operator()(const Bus *_key) const;

    };

    TransportCatalogue() = default;

    ~TransportCatalogue() = default;

    TransportCatalogue(const TransportCatalogue &other) = delete;

    void addStop(Stop &&_new_stop,
                 const std::vector<std::pair<std::string_view, double> > &_distances_to_stops) noexcept;

    Stop *findStop(std::string_view _name) const;

    const Stop *findStopById(size_t _id) const;

    void addBus(Bus &&_new_bus) noexcept;

    Bus *findBus(std::string_view _name) const;

    const std::set<std::string_view> &getNameBuses(std::string_view _name) const;

    std::vector<const Bus *> getSortedBuses() const;

    std::vector<const Stop *> getSortedUsedStops() const;

    size_t getCountStops() const;

    std::optional<double>
    getDistancesBetweenStops(const std::pair<std::string_view, std::string_view> &_key) const;

    const std::deque<Stop> &getAllStops() const;

    const std::deque<Bus> &getAllBuses() const;

    const std::unordered_map<std::pair<std::string_view, std::string_view>, double, Hasher>
    &getAllDistances() const;

    void appendDistancesBetweenStops(const std::pair<std::string_view, std::string_view> &stops,
                                     double distance);
private:

    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop *> stopname_to_stops_;

    std::deque<Bus> buses_;
    CatalogueBuses busname_to_buses_;

    std::unordered_map<std::pair<std::string_view, std::string_view>, double, Hasher> distances_between_stops_;

    StopToBuses stop_to_buses_;
};
