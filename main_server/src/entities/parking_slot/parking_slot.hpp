#ifndef PARKING_SLOT
#define PARKING_SLOT

#include <iostream>

class ParkingSlot{
    private:
        int id;
        int number;
        int id_parking_lot;
        int id_vehicle_type;
        bool reserved_disability;

    public:
        ParkingSlot();
        ParkingSlot(int id);
        ParkingSlot(int id, int number, int id_parking_lot, int id_vehicle_type, bool reserved_disability);

        void setId(int);
        // void setNumber(int);
        // void setIdParkingLot(int);
        void setIdVehicleType(int);
        void setReservedDisability(bool);

        int getId() const;
        int getNumber() const;
        int getIdParkingLot() const;
        int getIdVehicleType() const;
        bool getReservedDisability() const;

        friend bool operator== (const ParkingSlot&, const ParkingSlot&);
        friend std::ostream& operator<<(std::ostream& os, const ParkingSlot&);
};

// class ParkingSlotException : public std::exception {
//     std::string _message;
//     public:
//         ParkingSlotException(const std::string & message);
//         const char * what() const throw();
// };

#endif
