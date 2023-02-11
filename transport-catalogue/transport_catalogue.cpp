#include "transport_catalogue.h"

#include <algorithm>

#include <set>

#include "libs/geo.h"

TransportCatalogue::Bus::Bus(std::string_view _query)
{
    uint64_t colon = _query.rfind(':');
    if (colon != std::string_view::npos)
    {
        name_ = _query.substr(0, colon);
        name_.remove_prefix(1);
        _query.remove_prefix(colon + 1);
        char sep = '>';

        if (_query.find('-') != std::string_view::npos)
        {
            is_circul_ = true;
            sep = '-';
        }

        uint64_t sep_pos = _query.find_first_of(sep);
        while (!_query.empty())
        {
            std::string_view namestop;
            if (sep_pos != std::string_view::npos)
            {
                namestop = _query.substr(_query.find_first_not_of(' '), sep_pos - 1);
                if (namestop[namestop.size() - 1] == ' ')
                {
                    namestop.remove_suffix(1);
                }
                route_.emplace_back(namestop);
                _query = _query.substr(_query.find_first_of(sep) + 1);
                sep_pos = _query.find_first_of(sep);
                continue;
            }
            namestop = _query.substr(_query.find_first_not_of(' '));
            if (namestop[namestop.size() - 1] == ' ')
            {
                namestop.remove_suffix(1);
            }
            route_.emplace_back(namestop);
            _query = "";
        }
    }
}

TransportCatalogue::Bus::~Bus()
{
    this->route_.clear();
}

TransportCatalogue::Bus::Bus(const Bus &other)
{
    this->name_ = other.name_;
    this->route_ = other.route_;

    this->is_circul_ = other.is_circul_;
    this->number_unique_stops_ = other.number_unique_stops_;
    this->route_length_ = other.route_length_;
    this->curvature_ = other.curvature_;
}

TransportCatalogue::Bus::Bus(Bus &&other) noexcept
{
    std::swap(this->name_, other.name_);
    std::swap(this->route_, other.route_);
    std::swap(this->curvature_, other.curvature_);
    std::swap(this->is_circul_, other.is_circul_);
    std::swap(this->route_length_, other.route_length_);
    std::swap(this->number_unique_stops_, other.number_unique_stops_);
}

TransportCatalogue::Bus TransportCatalogue::Bus::operator=(const Bus &other)
{
    this->route_length_ = other.route_length_;
    this->name_ = other.name_;
    this->is_circul_ = other.is_circul_;
    this->route_ = other.route_;
    this->number_unique_stops_ = other.number_unique_stops_;
    return *this;
}

bool TransportCatalogue::Bus::operator==(const Bus &other) const
{
    const double EPSILON = 1.0;
    return std::abs(this->route_length_ - other.route_length_) < EPSILON &&
            this->name_ == other.name_ &&
            this->is_circul_ == other.is_circul_ &&
            this->route_ == other.route_ &&
            this->number_unique_stops_ == other.number_unique_stops_;
}

TransportCatalogue::Stop::Stop(std::string_view _query)
{
    uint64_t colon = _query.rfind(':');
    if (colon != std::string_view::npos)
    {
        name_ = _query.substr(0, colon);
        name_.remove_prefix(1);
        _query.remove_prefix(colon + 1);
        _query = _query.substr(_query.find_first_not_of(' '));
        colon = _query.find(',');
        if (colon != std::string_view::npos)
        {
            std::string_view latitude = _query.substr(0, colon);
            latitude_ = std::stod({latitude.data(), latitude.size()});
        }
        _query = _query.substr(_query.find(','));
        _query.remove_prefix(1);
        std::string_view longitude = _query.substr(_query.find_first_not_of(' '));
        longitude_ = std::stod({longitude.data(), longitude.size()});
    }
}

TransportCatalogue::Stop::Stop(std::string_view _name,
                               std::string_view _latitude,
                               std::string_view _longitude) :
    name_(_name),
    latitude_(std::stod({_latitude.data(), _latitude.size()})),
    longitude_(std::stod({_longitude.data(), _longitude.size()}))
{

}

TransportCatalogue::Stop::~Stop()
{

}

TransportCatalogue::Stop::Stop(const Stop &other) :
    name_(other.name_),
    latitude_(other.latitude_),
    longitude_(other.longitude_)
{

}

TransportCatalogue::Stop::Stop(Stop &&other) noexcept
{
    this->name_.swap(other.name_);
    this->latitude_ = other.latitude_;
    this->longitude_ = other.longitude_;

    other.latitude_ = 0.0;
    other.longitude_ = 0.0;
}

TransportCatalogue::Stop TransportCatalogue::Stop::operator=(const Stop &other)
{
    this->name_ = other.name_;
    this->latitude_ = other.latitude_;
    this->longitude_ = other.longitude_;
    return *this;
}

bool TransportCatalogue::Stop::operator==(const Stop &other) const
{
    const double EPSILON = 1;
    return std::abs(this->latitude_ - other.latitude_) < EPSILON &&
            std::abs(this->longitude_ - other.longitude_) < EPSILON &&
            this->name_ == other.name_;
}

size_t TransportCatalogue::Hasher::operator()(const Stop *_stop) const
{
    const size_t number = 17;
    size_t result =
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
    size_t result = _key.first->latitude_ * std::pow(number, 1) +
            _key.first->longitude_ * std::pow(number, 2) +
            _key.second->latitude_ * std::pow(number, 3) +
            _key.second->longitude_ * std::pow(number, 4) +
            std::hash<std::string_view>{}(_key.first->name_) +
            std::hash<std::string_view>{}(_key.second->name_);

    return result;
}

size_t TransportCatalogue::Hasher::operator()(const std::pair<std::string_view, std::string_view> &_key) const
{
    return std::hash<std::string_view>{}(_key.first) +
    std::hash<std::string_view>{}(_key.second);
}

void TransportCatalogue::addStop(std::string_view _query)
{
    std::string_view _query_for_stop = _query;
    uint64_t comma = _query_for_stop.rfind(',');
    bool has_stops = comma != _query_for_stop.find(',');
    if (has_stops)
    {
        _query_for_stop = _query_for_stop.substr(0, comma);
    }
    Stop new_stop(_query_for_stop);
    stops_.emplace_back(std::move(new_stop));
    Stop *ptr_stop = &stops_[stops_.size() - 1];
    stopname_to_stops_[ptr_stop->name_] = ptr_stop;
    stop_to_buses_.insert({ptr_stop->name_, std::set<std::string_view>{}});

    if (has_stops)
    {
        _query = _query.substr(_query.find(',') + 1);
        _query = _query.substr(_query.find(',') + 1);
        while (!_query.empty())
        {
            std::string_view distance = _query.substr(1, _query.find('m') - 1);
            std::string_view namestop = _query.substr(_query.find("to") + 3);
            namestop = namestop.substr(0, namestop.find_first_of(','));
            this->distances_between_stops_[{ptr_stop->name_, namestop}] =
                    std::stol(distance.data());
            comma = _query.find(',');
            if (comma != std::string_view::npos)
            {
                _query = _query.substr(comma + 1);
                continue;
            }
            _query = "";
        }
    }
}


TransportCatalogue::Stop *TransportCatalogue::findStop(std::string_view _name)
{
    if (stopname_to_stops_.find(_name) != stopname_to_stops_.end())
    {
        return stopname_to_stops_.at(_name);
    }
    return nullptr;
}

void TransportCatalogue::addBus(std::string_view _query)
{
    Bus bus(_query);
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

    if (bus.is_circul_)
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
    std::set<std::string_view> set_buf(bus.route_.begin(), bus.route_.end());
    bus.number_unique_stops_ = set_buf.size();

    buses_.emplace_back(std::move(bus));
    Bus *ptr_bus = &buses_[buses_.size() - 1];
    busname_to_buses_[ptr_bus->name_] = ptr_bus;
}

TransportCatalogue::Bus *TransportCatalogue::findBus(std::string_view _name)
{
    if (busname_to_buses_.find(_name) != busname_to_buses_.end())
    {
        return busname_to_buses_.at(_name);
    }
    return nullptr;
}

TransportCatalogue::BusInfo
TransportCatalogue::getBusInfo(std::string_view _name)
{
    Bus *ptr_bus = findBus(_name);
    BusInfo busInfo;
    busInfo.name_ = _name;
    if (ptr_bus != nullptr)
    {
        busInfo.number_stops_ = !ptr_bus->is_circul_ ?
                    ptr_bus->route_.size() : ptr_bus->route_.size() * 2 - 1;
        busInfo.number_unique_stops_ = ptr_bus->number_unique_stops_;
        busInfo.route_length_ = ptr_bus->route_length_;
        busInfo.is_circul_ = ptr_bus->is_circul_;
        busInfo.curvature_ = ptr_bus->curvature_;
    }
    return busInfo;
}

TransportCatalogue::StopInfo
TransportCatalogue::getStopInfo(std::string_view _name)
{
    StopInfo stopInfo;
    Stop *ptr_stop = findStop(_name);
    stopInfo.name_ = _name;
    if (ptr_stop != nullptr)
    {
        stopInfo.buses_ = this->stop_to_buses_.at(_name);
        stopInfo.is_exist = true;
    }
    return stopInfo;
}
