#include "parking_categories_allowed.hpp"

ParkingCategoriesAllowed::ParkingCategoriesAllowed() {}
ParkingCategoriesAllowed::ParkingCategoriesAllowed(int id) : id(id) {}
ParkingCategoriesAllowed::ParkingCategoriesAllowed(int id, int id_parking_lot, int id_user_category) :
      id(id), id_parking_lot(id_parking_lot), id_user_category(id_user_category) {}

void ParkingCategoriesAllowed::setId(int id) {
    this->id = id;
}

void ParkingCategoriesAllowed::setIdParkingLot(int id_parking_lot){
    this->id_parking_lot = id_parking_lot;
}

void ParkingCategoriesAllowed::setIdUserCategory(int id_user_category){
    this->id_user_category = id_user_category;
}

int ParkingCategoriesAllowed::getId() const{
    return id;
}

int ParkingCategoriesAllowed::getIdParkingLot() const{
    return id_parking_lot;
}

int ParkingCategoriesAllowed::getIdUserCategory() const{
    return id_user_category;
}

bool operator== (const ParkingCategoriesAllowed& pca1, const ParkingCategoriesAllowed& pca2)
{
    return pca1.id == pca2.id;
}

std::ostream& operator<<(std::ostream& os, const ParkingCategoriesAllowed& pca)
{
    os << "Parking Categories Allowed - Id parking lot: " << pca.id_parking_lot << ", Id user category: " << pca.id_user_category << std::endl;
    return os;
}

ParkingCategoriesAllowedException::ParkingCategoriesAllowedException(const std::string & message):_message(message) {}
const char * ParkingCategoriesAllowedException::what() const throw() {
      return _message.c_str();
}
