#include <iostream>
#include "hourly_rate.hpp"

HourlyRate::HourlyRate(){}
HourlyRate::HourlyRate(int id) : id(id) {}
HourlyRate::HourlyRate(int id, float amount): id(id), amount(amount) {}

HourlyRate::~HourlyRate(){}


void HourlyRate::setId(int id){
    this->id = id;
}

void HourlyRate::setAmount(float amount){
    this->amount = amount;
}

int HourlyRate::getId() const{
    return id;
}

float HourlyRate::getAmount() const{
    return amount;
}

HourlyRate& HourlyRate::operator= (const HourlyRate& h1){
    id = h1.id;
    amount = h1.amount;
    return *this;
}


bool operator== ( const HourlyRate& h1, const HourlyRate& h2)
{
    return h1.id == h2.id;
}

std::ostream& operator<<(std::ostream& os, const HourlyRate& h)
{
    os << "HourlyRate - Id: " << h.id << ", Amount: " << h.amount << std::endl;
    return os;
}

HourlyRateException::HourlyRateException(const std::string & message):_message(message) {}
const char * HourlyRateException::what() const throw() {
      return _message.c_str();
}
