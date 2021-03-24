#ifndef PARKING_LOT
#define PARKING_LOT

#include <iostream>

class ParkingLot{
  private:
    int id;
    std::string name;
    std::string street;
    int num_parking_slots = 100;

  public:
    ParkingLot();
    ParkingLot(int id);
    ParkingLot(int id, std::string name, std::string street, int num_parking_slots);

    void setId(int);
    void setName(std::string);
    void setStreet(std::string);
    void setNumParkingSlots(int);

    int getId() const;
    std::string getName() const;
    std::string getStreet() const;
    int getNumParkingSlots() const;

    ParkingLot& operator= (const ParkingLot&);

    friend bool operator== ( const ParkingLot&, const ParkingLot&);
    friend std::ostream& operator<<(std::ostream& os, const ParkingLot&);
};

class ParkingLotException : public std::exception {
    std::string _message;
    public:
        ParkingLotException(const std::string & message);
        const char * what() const throw();
};

#endif
