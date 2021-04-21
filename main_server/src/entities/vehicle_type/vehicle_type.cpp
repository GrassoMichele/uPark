#include "vehicle_type.hpp"

VehicleType::VehicleType(){}
VehicleType::VehicleType(int id) : id(id) {}
VehicleType::VehicleType(int id, std::string name, float rate_percentage): id(id), name(name), rate_percentage(rate_percentage) {}


void VehicleType::setId(int id){
    this->id = id;
}

void VehicleType::setName(std::string name){
    this->name = name;
}

void VehicleType::setRatePercentage(float rate_percentage){
    this->rate_percentage = rate_percentage;
}

int VehicleType::getId() const{
    return id;
}

std::string VehicleType::getName() const{
    return name;
}

float VehicleType::getRatePercentage() const{
    return rate_percentage;
}

VehicleType& VehicleType::operator= (const VehicleType& vt1){
    id = vt1.id;
    name = vt1.name;
    rate_percentage = vt1.rate_percentage;
    return *this;
}


bool operator== (const VehicleType& vt1, const VehicleType& vt2)
{
    return vt1.id == vt2.id;
}

std::ostream& operator<<(std::ostream& os, const VehicleType& vt)
{
    os << "VehicleType - Id: " << vt.id << ", name: " << vt.name << ", rate_percentage: " << vt.rate_percentage << std::endl;
    return os;
}

// VehicleTypeException::VehicleTypeException(const std::string & message):_message(message) {}
// const char * VehicleTypeException::what() const throw() {
//       return _message.c_str();
// }
