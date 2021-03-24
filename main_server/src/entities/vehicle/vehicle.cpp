#include "vehicle.hpp"

Vehicle::Vehicle(){}
Vehicle::Vehicle(int id) : id(id) {}
Vehicle::Vehicle(int id, std::string license_plate, std::string brand, std::string model,
                 int id_user, int id_vehicle_type) :  id(id),
                                                      license_plate(license_plate),
                                                      brand(brand),
                                                      model(model),
                                                      id_user(id_user),
                                                      id_vehicle_type(id_vehicle_type)
                                                      {}

void Vehicle::setId(int id){
  this->id = id;
}

void Vehicle::setLicensePlate(std::string license_plate){
  this->license_plate = license_plate;
}

void Vehicle::setIdUser(int id_user){
    this->id_user = id_user;
}

int Vehicle::getId() const{
    return id;
}

std::string Vehicle::getLicensePlate() const{
    return license_plate;
}

std::string Vehicle::getBrand() const{
    return brand;
}

std::string Vehicle::getModel() const{
    return model;
}

int Vehicle::getIdUser() const{
    return id_user;
}

int Vehicle::getIdVehicleType() const{
    return id_vehicle_type;
}

bool operator== (const Vehicle &v1, const Vehicle &v2)
{
    return v1.id == v2.id;
}

std::ostream& operator<<(std::ostream& os, const Vehicle& v)
{
    os << "Vehicle - Id: " << v.id << ", License Plate: " << v.license_plate << ", Brand: " << v.brand
        << ", Model: " << v.model << ", Id User: " << v.id_user << ", Id Vehicle Type: " << v.id_vehicle_type  << std::endl;
    return os;
}

VehicleException::VehicleException(const std::string & message):_message(message) {}
const char * VehicleException::what() const throw() {
      return _message.c_str();
}
