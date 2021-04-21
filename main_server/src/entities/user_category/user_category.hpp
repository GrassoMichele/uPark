#ifndef USER_CATEGORY
#define USER_CATEGORY

#include <iostream>

class UserCategory{
    private:
        int id;
        std::string name;
        int id_hourly_rate;
        std::string service_validity_start;
        std::string service_validity_end;

    public:
        UserCategory();
        UserCategory(int id);
        UserCategory(int id, std::string name, int id_hourly_rate, std::string service_validity_start, std::string service_validity_end);

        void setId(int);
        // void setName(std::string);
        void setIdHourlyRate(int);
        void setServiceValidityStart(std::string);
        void setServiceValidityEnd(std::string);

        int getId() const;
        std::string getName() const;
        int getIdHourlyRate() const;
        std::string getServiceValidityStart() const;
        std::string getServiceValidityEnd() const;

        friend bool operator== (const UserCategory&, const UserCategory&);
        friend std::ostream& operator<<(std::ostream& os, const UserCategory&);
};

// class UserCategoryException : public std::exception {
//     std::string _message;
//     public:
//         UserCategoryException(const std::string & message);
//         const char * what() const throw();
// };

#endif
