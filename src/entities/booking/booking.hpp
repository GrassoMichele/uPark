#ifndef BOOKING
#define BOOKING

#include <iostream>

class Booking{
  private:
    int id;
    std::string datetime_start;
    std::string datetime_end;
    std::string entry_time;
    std::string exit_time;
    float amount;
    int id_user;
    int id_vehicle;
    int id_parking_slot;
    std::string note;

  public:
    Booking();
    Booking(int id);
    Booking(int id, std::string datetime_start, std::string datetime_end, std::string entry_time, std::string exit_time, float amount,
            int id_user, int id_vehicle, int id_parking_slot, std::string note);

    void setId(int);
    void setEntryTime(std::string);
    void setExitTime(std::string);
    void setAmount(float);
    void setNote(std::string);

    int getId() const;
    std::string getDateTimeStart() const;
    std::string getDateTimeEnd() const;
    std::string getEntryTime() const;
    std::string getExitTime() const;
    float getAmount() const;
    int getIdUser() const;
    int getIdVehicle() const;
    int getIdParkingSlot() const;
    std::string getNote() const;

    friend bool operator== (const Booking&, const Booking&);
    friend std::ostream& operator<<(std::ostream& os, const Booking&);
};

class BookingException : public std::exception {
    std::string _message;
    public:
        BookingException(const std::string & message);
        const char * what() const throw();
};

#endif
