#include "booking.hpp"

Booking::Booking(){}
Booking::Booking(int id) : id(id) {}
Booking::Booking(int id, std::string datetime_start, std::string datetime_end, std::string entry_time, std::string exit_time,
                  float amount, int id_user, int id_vehicle, int id_parking_slot, std::string note) :
                                                              id(id),
                                                              datetime_start(datetime_start),
                                                              datetime_end(datetime_end),
                                                              entry_time(entry_time),
                                                              exit_time(exit_time),
                                                              amount(amount),
                                                              id_user(id_user),
                                                              id_vehicle(id_vehicle),
                                                              id_parking_slot(id_parking_slot),
                                                              note(note)
                                                              {}

void Booking::setId(int id){
    this->id = id;
}

void Booking::setEntryTime(std::string entry_time){
    this->entry_time = entry_time;
}

void Booking::setExitTime(std::string exit_time){
    this->exit_time = exit_time;
}

void Booking::setAmount(float amount){
    this->amount = amount;
}

void Booking::setNote(std::string note) {
    this->note = note;
}


int Booking::getId() const{
    return id;
}

std::string Booking::getDateTimeStart() const {
    return datetime_start;
}

std::string Booking::getDateTimeEnd() const {
    return datetime_end;
}

std::string Booking::getEntryTime() const {
    return entry_time;
}

std::string Booking::getExitTime() const {
    return exit_time;
}

float Booking::getAmount() const {
    return amount;
}

int Booking::getIdUser() const {
    return id_user;
}

int Booking::getIdVehicle() const {
    return id_vehicle;
}

int Booking::getIdParkingSlot() const {
    return id_parking_slot;
}

std::string Booking::getNote() const {
    return note;
}


bool operator== (const Booking &b1, const Booking &b2)
{
    return b1.id == b2.id;
}


std::ostream& operator<<(std::ostream& os, const Booking& b)
{
    os << "Booking - Id: " << b.id << ", Datetime start: " << b.datetime_start << ", Datetime end: " << b.datetime_end
        << ", Entry time: " << b.entry_time << ", Exit time: " << b.exit_time << ", Amount: " << b.amount << ", Id User: "
        << b.id_user << ", Id Vehicle: " << b.id_vehicle << ", Id Parking Slot: " << b.id_parking_slot << ", Note: "
        << b.note << std::endl;
    return os;
}

BookingException::BookingException(const std::string & message):_message(message) {}
const char * BookingException::what() const throw() {
      return _message.c_str();
}
