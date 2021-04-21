#include "parking_slot.hpp"

ParkingSlot::ParkingSlot(){}
ParkingSlot::ParkingSlot(int id) : id(id) {}
ParkingSlot::ParkingSlot(int id, int number, int id_parking_lot, int id_vehicle_type, bool reserved_disability):
                                                                                id(id),
                                                                                number(number),
                                                                                id_parking_lot(id_parking_lot),
                                                                                id_vehicle_type(id_vehicle_type),
                                                                                reserved_disability(reserved_disability)
                                                                                {}

void ParkingSlot::setId(int id){
    this->id = id;
}

// void ParkingSlot::setNumber(int number){
//     this->number = number;
// }
//
// void ParkingSlot::setIdParkingLot(int id_parking_lot){
//     this->id_parking_lot = id_parking_lot;
// }

void ParkingSlot::setIdVehicleType(int id_vehicle_type){
    this->id_vehicle_type = id_vehicle_type;
}

void ParkingSlot::setReservedDisability(bool reserved_disability){
    this->reserved_disability = reserved_disability;
}


int ParkingSlot::getId() const{
    return id;
}

int ParkingSlot::getNumber() const{
    return number;
}

int ParkingSlot::getIdParkingLot() const{
    return id_parking_lot;
}

int ParkingSlot::getIdVehicleType() const{
    return id_vehicle_type;
}

bool ParkingSlot::getReservedDisability() const{
    return reserved_disability;
}


bool operator== (const ParkingSlot& ps1, const ParkingSlot& ps2)
{
    return ps1.id == ps2.id;
}

std::ostream& operator<<(std::ostream& os, const ParkingSlot& ps)
{
    os << "ParkingSlot - Id: " << ps.id << ", Number: " << ps.number << ", Id Parking lot: "
    << ps.id_parking_lot << ", Id Vehicle Type: " << ps.id_vehicle_type << ", Reserved disability: " << ps.reserved_disability << std::endl;
    return os;
}

// ParkingSlotException::ParkingSlotException(const std::string & message):_message(message) {}
// const char * ParkingSlotException::what() const throw() {
//       return _message.c_str();
// }
