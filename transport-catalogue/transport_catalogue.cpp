#include "transport_catalogue.h"

#include <algorithm>

#include <set>

#include "libs/geo.h"

size_t TransportCatalogue::Hasher::operator()(const Stop *_stop) const
{
    static const size_t number = 17;
    const size_t result =
            static_cast<size_t>(_stop->latitude_ * number) +
            static_cast<size_t>(_stop->longitude_ * std::pow(number, 2))
            + std::hash<std::string_view>{}(_stop->name_);

    return result;
}

size_t TransportCatalogue::Hasher::operator()(const Bus *_key) const
{
    size_t result = std::hash<std::string_view>{}(_key->name_);

    for (const auto &stop : _key->route_)
    {
        result += std::hash<std::string_view>{}(stop);
    }

    return result;
}

size_t TransportCatalogue::Hasher::operator()(std::pair<Stop *, Stop *> _key) const
{
    const size_t number = 37;
    const size_t result = _key.first->latitude_ * std::pow(number, 1) +
            _key.first->longitude_ * std::pow(number, 2) +
            _key.second->latitude_ * std::pow(number, 3) +
            _key.second->longitude_ * std::pow(number, 4) +
            std::hash<std::string_view>{}(_key.first->name_) +
            std::hash<std::string_view>{}(_key.second->name_);

    return result;
}

void TransportCatalogue::addStop(Stop &&_new_stop,
                                 const std::vector<std::pair<std::string_view, double> > &_distances_to_stops) noexcept
{
    static size_t id_counter = 0;

    stops_.emplace_back(std::move(_new_stop));
    Stop *ptr_stop = &stops_[stops_.size() - 1];
    stopname_to_stops_[ptr_stop->name_] = ptr_stop;
    ptr_stop->id_ = id_counter++;
    stop_to_buses_.insert({ptr_stop->name_, std::set<std::string_view>{}});

    if (!_distances_to_stops.empty())
    {
        for (auto const &[namestop, distance] : _distances_to_stops)
        {
            this->distances_between_stops_[{ptr_stop->name_, namestop}] = distance;
        }
    }
}

TransportCatalogue::Stop *TransportCatalogue::findStop(std::string_view _name) const
{
    if (stopname_to_stops_.find(_name) != stopname_to_stops_.end())
    {
        return stopname_to_stops_.at(_name);
    }
    return nullptr;
}

const TransportCatalogue::Stop *TransportCatalogue::findStopById(size_t _id) const
{
    if (_id < stops_.size())
    {
        return &stops_.at(_id);
    }
    return nullptr;
}

void TransportCatalogue::addBus(Bus &&_new_bus) noexcept
{
    Bus bus(std::move(_new_bus));
    std::pair<std::string_view, std::string_view> pair_stops;
    double geo_distance = 0.0;
    for (size_t index = 0; index < bus.route_.size() - 1; ++index)
    {
        Stop *stop = stopname_to_stops_.at(bus.route_.at(index));
        Stop *next = stopname_to_stops_.at(bus.route_.at(index + 1));
        pair_stops = {stop->name_, next->name_};

        geo_distance +=
                geo::ComputeDistance({stop->latitude_, stop->longitude_},
                                     {next->latitude_, next->longitude_});

        double distance = distances_between_stops_[pair_stops];
        if (distance == 0.0)
        {
            distance = distances_between_stops_[{next->name_, stop->name_}];
            distances_between_stops_[pair_stops] = distance;
        }
        bus.route_length_ += distance;

        stop_to_buses_.at(stop->name_).insert(bus.name_);
    }
    stop_to_buses_.at(*(bus.route_.end() - 1)).insert(bus.name_);
    if (!bus.is_circul_)
    {
        for (size_t index = bus.route_.size() - 1; index != 0; --index)
        {
            Stop *stop = stopname_to_stops_.at(bus.route_.at(index));
            Stop *next = stopname_to_stops_.at(bus.route_.at(index - 1));
            pair_stops = {stop->name_, next->name_};

            geo_distance +=
                    geo::ComputeDistance({stop->latitude_, stop->longitude_},
                                         {next->latitude_, next->longitude_});

            double distance = distances_between_stops_[pair_stops];
            if (distance == 0.0)
            {
                distance = distances_between_stops_[{next->name_, stop->name_}];
                distances_between_stops_[pair_stops] = distance;
            }
            bus.route_length_ += distance;
        }
    }

    bus.curvature_ = bus.route_length_ / geo_distance;
    std::set<std::string_view> set_buf;

    for (const auto &stop : bus.route_)
    {
        set_buf.insert(stop);
    }

    bus.number_unique_stops_ = set_buf.size();

    buses_.emplace_back(std::move(bus));
    Bus &added_bus = buses_[buses_.size() - 1];
    busname_to_buses_[added_bus.name_] = &added_bus;
}

TransportCatalogue::Bus *TransportCatalogue::findBus(std::string_view _name) const
{
    if (busname_to_buses_.find(_name) != busname_to_buses_.end())
    {
        return busname_to_buses_.at(_name);
    }
    return nullptr;
}

const std::set<std::string_view> &
TransportCatalogue::getNameBuses(std::string_view _name) const
{
    return stop_to_buses_.at(_name);
}

auto TransportCatalogue::getSortedBuses() const -> std::vector<const Bus *>
{
    std::vector<const Bus *> result;
    result.reserve(buses_.size());
    for (const auto &bus : buses_)
    {
        result.push_back(&bus);
    }
    std::sort(result.begin(), result.end(),
              [](const Bus *_lhs, const Bus *_rhs)
    {
        return _lhs->name_ < _rhs->name_;
    });
    return result;
}

std::vector<const TransportCatalogue::Stop *> TransportCatalogue::getSortedUsedStops() const
{
    std::vector<const Stop *> result;
    result.reserve(stops_.size());
    for (const auto &[stop, buses] : stop_to_buses_)
    {
        if (!buses.empty())
        {
            result.push_back(findStop(stop));
        }
    }
    std::sort(result.begin(), result.end(),
              [](const Stop *_lhs, const Stop *_rhs)
    {
        return _lhs->name_ < _rhs->name_;
    });
    return result;
}

size_t TransportCatalogue::getCountStops() const
{
    return stops_.size();
}

std::optional<double>
TransportCatalogue::getDistancesBetweenStops(const std::pair<std::string_view, std::string_view> &_key) const
{
    if (distances_between_stops_.count(_key) != 0U)
    {
        return distances_between_stops_.at(_key);
    }

    return std::nullopt;
}
