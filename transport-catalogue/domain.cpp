#include "domain.h"

namespace domain
{

Bus::Bus(const Bus &other)
{
    this->name_ = other.name_;
    this->route_ = other.route_;

    this->is_circul_ = other.is_circul_;
    this->number_unique_stops_ = other.number_unique_stops_;
    this->route_length_ = other.route_length_;
    this->curvature_ = other.curvature_;
}

Bus::Bus(Bus &&other) noexcept
{
    std::swap(this->name_, other.name_);
    std::swap(this->route_, other.route_);
    std::swap(this->curvature_, other.curvature_);
    std::swap(this->is_circul_, other.is_circul_);
    std::swap(this->route_length_, other.route_length_);
    std::swap(this->number_unique_stops_, other.number_unique_stops_);
}

Bus &Bus::operator =(const Bus &other)
{
    this->route_length_ = other.route_length_;
    this->name_ = other.name_;
    this->is_circul_ = other.is_circul_;
    this->route_ = other.route_;
    this->number_unique_stops_ = other.number_unique_stops_;
    return *this;
}

bool Bus::operator==(const Bus &other) const
{
    const double EPSILON = 1.0;
    return std::abs(this->route_length_ - other.route_length_) < EPSILON &&
            this->name_ == other.name_ &&
            this->is_circul_ == other.is_circul_ &&
            this->route_ == other.route_ &&
            this->number_unique_stops_ == other.number_unique_stops_;
}

Stop::Stop(std::string_view _name,
           std::string_view _latitude,
           std::string_view _longitude) :
    id_(0),
    name_(_name),
    latitude_(std::stod({_latitude.data(), _latitude.size()})),
    longitude_(std::stod({_longitude.data(), _longitude.size()}))
{

}

Stop::Stop(Stop &&other) noexcept
{
    std::swap(this->name_, other.name_);
    std::swap(this->latitude_, other.latitude_);
    std::swap(this->longitude_, other.longitude_);
}


bool Stop::operator==(const Stop &other) const
{
    const double EPSILON = 1;
    return std::abs(this->latitude_ - other.latitude_) < EPSILON &&
            std::abs(this->longitude_ - other.longitude_) < EPSILON &&
            this->name_ == other.name_;
}

} // namespace domain
