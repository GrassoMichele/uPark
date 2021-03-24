#include "user_category.hpp"

UserCategory::UserCategory(){}
UserCategory::UserCategory(int id) : id(id) {}
UserCategory::UserCategory(int id, std::string name, int id_hourly_rate, std::string service_validity_start,
                          std::string service_validity_end) : id(id),
                                                              name(name),
                                                              id_hourly_rate(id_hourly_rate),
                                                              service_validity_start(service_validity_start),
                                                              service_validity_end(service_validity_end)
                                                              {}

void UserCategory::setId(int id){
    this->id = id;
}

void UserCategory::setName(std::string name){
  this->name = name;
}

void UserCategory::setIdHourlyRate(int id_hourly_rate){
    this->id_hourly_rate = id_hourly_rate;
}

void UserCategory::setServiceValidityStart(std::string service_validity_start){
    this->service_validity_start = service_validity_start;
}

void UserCategory::setServiceValidityEnd(std::string service_validity_end){
    this->service_validity_end = service_validity_end;
}


int UserCategory::getId() const{
    return id;
}

std::string UserCategory::getName() const{
    return name;
}

int UserCategory::getIdHourlyRate() const{
    return id_hourly_rate;
}

std::string UserCategory::getServiceValidityStart() const{
    return service_validity_start;
}

std::string UserCategory::getServiceValidityEnd() const{
    return service_validity_end;
}


bool operator== (const UserCategory &uc1, const UserCategory &uc2)
{
    return uc1.id == uc2.id;
}

std::ostream& operator<<(std::ostream& os, const UserCategory& uc)
{
    os << "UserCategory - Id: " << uc.id << ", Name: " << uc.name << ", Id Hourly rate: " << uc.id_hourly_rate
        << ", Service validity start: " << uc.service_validity_start << ", Service validity end: " << uc.service_validity_end <<std::endl;
    return os;
}

UserCategoryException::UserCategoryException(const std::string & message):_message(message) {}
const char * UserCategoryException::what() const throw() {
      return _message.c_str();
}
