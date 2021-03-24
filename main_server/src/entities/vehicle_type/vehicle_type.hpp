#ifndef VEHICLE_TYPE
#define VEHICLE_TYPE

#include <iostream>

class VehicleType{

  private:
    int id;
    std::string name;
    float rate_percentage = 1.0;

  public:
    VehicleType();
    VehicleType(int id);
    VehicleType(int id, std::string name, float rate_percentage);
    void setId(int);
    void setName(std::string);
    void setRatePercentage(float);
    int getId() const;
    std::string getName() const;
    float getRatePercentage() const;

    VehicleType& operator= (const VehicleType&);

    friend bool operator== ( const VehicleType&, const VehicleType&);
    friend std::ostream& operator<<(std::ostream& os, const VehicleType&);
};

class VehicleTypeException : public std::exception {
    std::string _message;
    public:
        VehicleTypeException(const std::string & message);
        const char * what() const throw();
};

#endif
