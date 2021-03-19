#ifndef PARKING_CATEGORIES_ALLOWED
#define PARKING_CATEGORIES_ALLOWED

#include <iostream>

class ParkingCategoriesAllowed{
  private:
    int id;
    int id_parking_lot;
    int id_user_category;

  public:
    ParkingCategoriesAllowed();
    ParkingCategoriesAllowed(int id);
    ParkingCategoriesAllowed(int id, int id_parking_lot, int id_user_category);

    void setId(int);
    void setIdParkingLot(int);
    void setIdUserCategory(int);

    int getId() const;
    int getIdParkingLot() const;
    int getIdUserCategory() const;

    friend bool operator== (const ParkingCategoriesAllowed&, const ParkingCategoriesAllowed&);
    friend std::ostream& operator<<(std::ostream& os, const ParkingCategoriesAllowed&);
};

class ParkingCategoriesAllowedException : public std::exception {
    std::string _message;
    public:
        ParkingCategoriesAllowedException(const std::string & message);
        const char * what() const throw();
};

#endif
