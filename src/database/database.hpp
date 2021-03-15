#ifndef DATABASE
#define DATABASE

#include <utility>
#include <string>
#include <vector>
#include <mysql/jdbc.h>

#include "../entities/hourly_rate/hourly_rate.hpp"
#include "../entities/user_category/user_category.hpp"
#include "../entities/user/user.hpp"
#include "../entities/vehicle_type/vehicle_type.hpp"
#include "../entities/vehicle/vehicle.hpp"
#include "../entities/parking_lot/parking_lot.hpp"
#include "../entities/parking_categories_allowed/parking_categories_allowed.hpp"
#include "../entities/parking_slot/parking_slot.hpp"
#include "../entities/booking/booking.hpp"


class Database {
    sql::Driver *driver;
    std::unique_ptr<sql::Connection> connection;
    std::unique_ptr<sql::PreparedStatement> pstmt;
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;

    public:
        Database();

        template<typename T> std::vector<T> select_all();
        template<typename T> T select(const int& id);
        template<typename T> int insert(const T& t);
        template<typename T> void update(const T& h);
        template<typename T> void delete_f(const int& id);
        template<typename T> void full_delete_f();

        template<typename T> T select_instance_object();
        template<typename T> void insert_query(const T& t);
        template<typename T> void update_query(const T& t);

        void db_init();

        bool user_existence_check(const User&, const std::string& upark_code, const std::string& category_name);

};

template<class T>
std::string get_relation_name(T t) {
    if (std::is_same<T, HourlyRate>::value){
        return "hourly_rates";
    }
    else if (std::is_same<T, UserCategory>::value) {
        return "user_categories";
    }
    else if (std::is_same<T, User>::value){
      return "users";
    }
    else if (std::is_same<T, VehicleType>::value){
      return "vehicle_types";
    }
    else if (std::is_same<T, Vehicle>::value){
      return "vehicles";
    }
    else if (std::is_same<T, ParkingLot>::value){
      return "parking_lots";
    }
    else if (std::is_same<T, ParkingCategoriesAllowed>::value){
      return "parking_lots_user_categories_allowed";
    }
    else if (std::is_same<T, ParkingSlot>::value){
      return "parking_slots";
    }
    else if (std::is_same<T, Booking>::value){
      return "bookings";
    }


    //else if (std::is_same<T,>::value) ...
}

class DatabaseException : public std::exception {
   std::string _message;
public:
   DatabaseException(const std::string & message);
   const char * what() const throw();
};


template<class T>
std::vector<T> Database::select_all(){

    T t;
    std::vector<T> db_results;

    std::string relation_name = get_relation_name(t);

    pstmt.reset(connection->prepareStatement("SELECT * FROM  " + relation_name));
    res.reset(pstmt->executeQuery());

    int num_rows = res->rowsCount();

    if (num_rows!=0){
        while(res->next()){

            t = select_instance_object<T>();

            db_results.push_back(t);
        }
    }
    else {
        std::cout << "No items in " + relation_name << std::endl;
    }

    return db_results;
}


template<class T>
T Database::select(const int& id){

    T t;
    std::string relation_name = get_relation_name(t);

    pstmt.reset(connection->prepareStatement("SELECT * FROM  " + relation_name + " WHERE id = ?"));
    pstmt->setInt(1, id);
    res.reset(pstmt->executeQuery());

    int num_rows = res->rowsCount();

    if (num_rows!=0){
        res->next();

        t = select_instance_object<T>();

        return t;
    }
    else {
        throw DatabaseException(relation_name + " not found in DB!");
    }
}


template<class T>
int Database::insert(const T& t){
    std::string relation_name = get_relation_name(t);
    int id_created;

    try {
        insert_query(t);
        pstmt->execute();
        stmt.reset(connection->createStatement());
        res.reset(stmt->executeQuery("SELECT id FROM " + relation_name + " ORDER BY id DESC LIMIT 1"));
        res->next();
        id_created = res->getInt("id");

        return id_created;
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException(relation_name + " can't be added in DB!");
    }
}


template<class T>
void Database::update(const T& t){
    std::string relation_name = get_relation_name(t);

    try {
        update_query(t);

        int success = pstmt->executeUpdate();
        if(success==0){
            throw DatabaseException(relation_name + " with id " + std::to_string(t.getId()) + " can't be updated!");
        }
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException(relation_name + " can't be updated in DB!");
    }
}

template<class T>
void Database::delete_f(const int& id) {
    std::string relation_name = get_relation_name(T());
    try {

        pstmt.reset(connection->prepareStatement("DELETE FROM " + relation_name + " WHERE id = ?"));
        pstmt->setInt(1, id);

        int success = pstmt->executeUpdate();
        if(success==0){
            throw DatabaseException(relation_name + " with id " + std::to_string(id) + " can't be deleted because not found in DB!");
        }
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException(relation_name + " with id " + std::to_string(id) + " can't be deleted in DB!");
    }
}

template<class T>
void Database::full_delete_f() {
    std::string relation_name = get_relation_name(T());
    try {

        pstmt.reset(connection->prepareStatement("DELETE FROM " + relation_name + " WHERE 1"));

        int success = pstmt->executeUpdate();
        if(success==0){
            throw DatabaseException(relation_name + " can't be deleted because not found in DB!");
        }
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException(relation_name + " can't be deleted in DB!");
    }
}

#endif
