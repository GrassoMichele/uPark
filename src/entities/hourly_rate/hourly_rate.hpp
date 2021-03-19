#ifndef HOURLY_RATE
#define HOURLY_RATE

#include <iostream>

class HourlyRate{
  private:
    int id;
    float amount = 1.00;

  public:
    HourlyRate();
    HourlyRate(int id);
    HourlyRate(int id, float amount);
    ~HourlyRate();
    void setId(int);
    void setAmount(float);
    int getId() const;
    float getAmount() const;

    HourlyRate& operator= (const HourlyRate&);

    friend bool operator== (const HourlyRate&, const HourlyRate&);
    friend std::ostream& operator<<(std::ostream& os, const HourlyRate&);
};

class HourlyRateException : public std::exception {
    std::string _message;
    public:
        HourlyRateException(const std::string & message);
        const char * what() const throw();
};

#endif
