#include "user.hpp"

User::User(){}
User::User(int id) : id(id) {}
User::User(int id, std::string email, std::string name, std::string surname, std::string password,
  float wallet, bool disability, bool active_account, int id_user_category) :
                                                                              id(id),
                                                                              email(email),
                                                                              name(name),
                                                                              surname(surname),
                                                                              password(password),
                                                                              wallet(wallet),
                                                                              disability(disability),
                                                                              active_account(active_account),
                                                                              id_user_category(id_user_category)
                                                                              {}

void User::setId(int id){
    this->id = id;
}

// void User::setEmail(std::string email){
//     this->email = email;
// }
//
// void User::setName(std::string name){
//     this->name = name;
// }
//
// void User::setSurname(std::string surname){
//     this->surname = surname;
// }

void User::setPassword(std::string password){
    this->password = password;
}

void User::setWallet(float wallet){
    this->wallet = wallet;
}

void User::setDisability(bool disability){
    this->disability = disability;
}

void User::setActiveAccount(bool active_account){
    this->active_account = active_account;
}

// void User::setIdUserCategory(int id_user_category){
//     this->id_user_category = id_user_category;
// }


int User::getId() const{
    return id;
}

std::string User::getEmail() const{
  return email;
}

std::string User::getName() const{
    return name;
}

std::string User::getSurname() const{
    return surname;
}

std::string User::getPassword() const{
    return password;
}

float User::getWallet() const{
    return wallet;
}

bool User::getDisability() const{
    return disability;
}

bool User::getActiveAccount() const{
    return active_account;
}

int User::getIdUserCategory() const{
    return id_user_category;
}


bool operator== (const User &u1, const User &u2)
{
    return u1.id == u2.id;
}

std::ostream& operator<< (std::ostream& os, const User& u)
{
    os << "User - Id: " << u.id << ", Email: " << u.email << ", Name: " << u.name << ", Surname: " << u.surname <<
    ", Password: " << u.password <<", Wallet: " << u.wallet <<", Disability: " << u.disability <<", Active Account: " << u.active_account <<
    ", Id User Category: " << u.id_user_category << std::endl;
    return os;
}

UserException::UserException(const std::string & message):_message(message) {}
const char * UserException::what() const throw() {
      return _message.c_str();
}
