#ifndef USER
#define USER

#include <iostream>
#include <algorithm>
#include <vector>

class User{

  private:
    int id;
    std::string email;
    std::string name;
    std::string surname;
    std::string password;
    float wallet;
    bool disability;
    bool active_account;
    int id_user_category;

  public:
    User();
    User(int id);
    User(int id, std::string email, std::string name, std::string surname, std::string password,
      float wallet, bool disability, bool active_account, int id_user_category);

    void setId(int);
    void setEmail(std::string);
    void setName(std::string);
    void setSurname(std::string);
    void setPassword(std::string);
    void setWallet(float);
    void setDisability(bool);
    void setActiveAccount(bool);
    void setIdUserCategory(int);

    int getId() const;
    std::string getEmail() const;
    std::string getName() const;
    std::string getSurname() const;
    std::string getPassword() const;
    float getWallet() const;
    bool getDisability() const;
    bool getActiveAccount() const;
    int getIdUserCategory() const;

    friend bool operator== (const User&, const User&);
    friend std::ostream& operator<<(std::ostream& os, const User&);
};

class UserException : public std::exception {
    std::string _message;
    public:
        UserException(const std::string & message);
        const char * what() const throw();
};

#endif
