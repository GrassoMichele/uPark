#ifndef VEHICLE
#define VEHICLE

#include <iostream>


class Vehicle{

  private:
    int id;
    std::string license_plate;
    std::string brand;
    std::string model;
    int id_user;
    int id_vehicle_type;

  public:
    Vehicle();
    Vehicle(int id);
    Vehicle(int id, std::string license_plate, std::string brand, std::string model, int id_user, int id_vehicle_type);

    void setId(int);
    void setLicensePlate(std::string);
    void setIdUser(int);

    int getId() const;
    std::string getLicensePlate() const;
    std::string getBrand() const;
    std::string getModel() const;
    int getIdUser() const;
    int getIdVehicleType() const;

    friend bool operator== (const Vehicle&, const Vehicle&);
    friend std::ostream& operator<<(std::ostream& os, const Vehicle&);
};

class VehicleException : public std::exception {
    std::string _message;
    public:
        VehicleException(const std::string & message);
        const char * what() const throw();
};

#endif
