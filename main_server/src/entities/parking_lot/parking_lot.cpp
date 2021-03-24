#include "parking_lot.hpp"

ParkingLot::ParkingLot(){}
ParkingLot::ParkingLot(int id) : id(id) {}
ParkingLot::ParkingLot(int id,  std::string name, std::string street, int num_parking_slots):
                  id(id), name(name), street(street), num_parking_slots(num_parking_slots) {}

void ParkingLot::setId(int id){
    this->id = id;
}

void ParkingLot::setName(std::string name){
    this->name = name;
}

void ParkingLot::setStreet(std::string street){
    this->street = street;
}

void ParkingLot::setNumParkingSlots(int num_parking_slots){
    this->num_parking_slots = num_parking_slots;
}

int ParkingLot::getId() const{
    return id;
}

std::string ParkingLot::getName() const{
    return name;
}

std::string ParkingLot::getStreet() const{
    return street;
}

int ParkingLot::getNumParkingSlots() const{
    return num_parking_slots;
}

ParkingLot& ParkingLot::operator= (const ParkingLot& p1){
    id = p1.id;
    name = p1.name;
    street = p1.street;
    num_parking_slots = p1.num_parking_slots;
    return *this;
}


bool operator== (const ParkingLot& p1, const ParkingLot& p2)
{
    return p1.id == p2.id;
}

std::ostream& operator<<(std::ostream& os, const ParkingLot& p)
{
    os << "ParkingLot - Id: " << p.id << ", Name: " << p.name << ", Street: " << p.street << ", Num parking slots: "
      << p.num_parking_slots << std::endl;
    return os;
}

ParkingLotException::ParkingLotException(const std::string & message):_message(message) {}
const char * ParkingLotException::what() const throw() {
      return _message.c_str();
}
