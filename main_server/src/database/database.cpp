#include "database.hpp"

Database::Database(){
    driver = get_driver_instance();
    try{
        connection.reset(driver->connect("tcp://upark-server-db.mysql.database.azure.com", "rest_server@upark-server-db", "upark_pwd"));
        std::cout << "--- Connection to DB established! ---" << std::endl;
        db_init();
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        std::cout << "Closing ..." << std::endl;
        std::exit(1);
    }
}


DatabaseException::DatabaseException(const std::string & message):_message(message) {}
const char * DatabaseException::what() const throw() {
      return _message.c_str();
}


void Database::db_init(){
    //Align the categories of db_upark and db_university
    //Read from db_university
    try{
        stmt.reset(connection->createStatement());
        res.reset(stmt->executeQuery("SELECT uni_uc.name FROM db_university.user_categories uni_uc LEFT"
        " JOIN db_upark.user_categories up_uc ON uni_uc.name=up_uc.name WHERE up_uc.name IS NULL"));

        connection->setSchema("db_upark");
        int query_success;

        while(res->next()){
            std::cout << "New Category found: " <<  res->getString("name")  << std::endl;

            pstmt.reset(connection->prepareStatement("INSERT INTO db_upark.user_categories (name, id_hourly_rate) VALUES (?, ?)"));
            pstmt->setString(1, res->getString("name"));
            pstmt->setNull(2, 0);
            query_success = pstmt->executeUpdate();

            if(query_success==0){
                throw DatabaseException("Can't add new category");
            }
            else{
                std::cout << "-> Category added!" << std::endl;
            }
        }
        //Creation of Admin category
        pstmt.reset(connection->prepareStatement("INSERT IGNORE INTO user_categories (name, id_hourly_rate) VALUES (?, ?)"));
        pstmt->setString(1, "Admin");
        pstmt->setNull(2, 0);
        query_success = pstmt->executeUpdate();

        //Creation of user Admin
        stmt.reset(connection->createStatement());
        res.reset(stmt->executeQuery("SELECT id FROM user_categories WHERE name = 'Admin'"));
        res->next();
        int id_admin_category = res->getInt("id");

        pstmt.reset(connection->prepareStatement("INSERT IGNORE INTO db_upark.users (email, name, surname, password, wallet, disability, active_account, id_user_category) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
        pstmt->setString(1, "admin@admin");
        pstmt->setString(2, "admin");
        pstmt->setString(3, "admin");
        pstmt->setString(4, "adminadmin");
        pstmt->setDouble(5, 0.0);
        pstmt->setBoolean(6, false);
        pstmt->setBoolean(7, true);
        pstmt->setInt(8, id_admin_category);
        query_success = pstmt->executeUpdate();

        std::cout << "--- upark_db is updated! ---" << std::endl;
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException("Alignment Failed!");
    }
}


// ---SELECT---
template<>
HourlyRate Database::select_instance_object<HourlyRate>(){
    return HourlyRate(res->getInt("id"), res->getDouble("amount"));
}

template<>
UserCategory Database::select_instance_object<UserCategory>(){
    return UserCategory(res->getInt("id"), res->getString("name"), res->getInt("id_hourly_rate"), res->getString("service_validity_start"),
        res->getString("service_validity_end"));
}

template<>
User Database::select_instance_object<User>(){
    return User(res->getInt("id"), res->getString("email"), res->getString("name"), res->getString("surname"),  res->getString("password"),
        res->getDouble("wallet"), res->getBoolean("disability"), res->getBoolean("active_account"),  res->getInt("id_user_category"));
}

template<>
VehicleType Database::select_instance_object<VehicleType>(){
    return VehicleType(res->getInt("id"), res->getString("name"), res->getDouble("rate_percentage"));
}

template<>
Vehicle Database::select_instance_object<Vehicle>(){
    return Vehicle(res->getInt("id"), res->getString("license_plate"), res->getString("brand"), res->getString("model"),
        res->getInt("id_user"), res->getInt("id_vehicle_type"));
}

template<>
ParkingLot Database::select_instance_object<ParkingLot>(){
    return ParkingLot(res->getInt("id"), res->getString("name"), res->getString("street"), res->getInt("num_parking_slots"));
}

template<>
ParkingCategoriesAllowed Database::select_instance_object<ParkingCategoriesAllowed>(){
    return ParkingCategoriesAllowed(res->getInt("id"), res->getInt("id_parking_lot"), res->getInt("id_user_category"));
}

template<>
ParkingSlot Database::select_instance_object<ParkingSlot>(){
    return ParkingSlot(res->getInt("id"), res->getInt("number"), res->getInt("id_parking_lot"), res->getInt("id_vehicle_type"), res->getBoolean("reserved_disability"));
}

template<>
Booking Database::select_instance_object<Booking>(){
    return Booking(res->getInt("id"), res->getString("datetime_start"), res->getString("datetime_end"), res->getString("entry_time"),
        res->getString("exit_time"), res->getDouble("amount"), res->getInt("id_user"), res->getInt("id_vehicle"),
        res->getInt("id_parking_slot"), res->getString("note"));
}


//---user_existence_check---
bool Database::user_existence_check(const User& u, const std::string& upark_code, const std::string& category_name){
    try {
        // A user can be created in DB_upark only if present on DB_University
        pstmt.reset(connection->prepareStatement("SELECT EXISTS (SELECT * FROM db_university.upark_users UU JOIN db_university.user_categories UUC "
          "ON UU.id_user_category = UUC.id WHERE UU.email = ? AND UU.upark_code = ? AND UUC.name = ?) AS user_match"));

        pstmt->setString(1, u.getEmail());
        pstmt->setString(2, upark_code);
        pstmt->setString(3, category_name);

        res.reset(pstmt->executeQuery());
        res->next();

        if(res->getBoolean(1)){           // returns the first column of the query's result
            std::cout << "A match was found for user: " << u.getEmail() << std::endl;
            return true;
        }
        else {
            return false;
        }
    }
    catch(sql::SQLException& e){
        std::cout << e.what() << std::endl;
        throw DatabaseException("User match checking Failed!");
    }

}


//---INSERT---
template<>
void Database::insert_query(const HourlyRate& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (amount) VALUES (?)"));
    pstmt->setDouble(1, t.getAmount());
}

template<>
void Database::insert_query(const UserCategory& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (name, id_hourly_rate, service_validity_start, service_validity_end)"
    " VALUES (?, ?, ?, ?)"));
    pstmt->setString(1, t.getName());
    pstmt->setInt(2, t.getIdHourlyRate());
    pstmt->setString(3, t.getServiceValidityStart());
    pstmt->setString(4, t.getServiceValidityEnd());
}

template<>
void Database::insert_query(const User& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (email, name, surname, password, wallet, disability, active_account, id_user_category)"
    " VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    pstmt->setString(1, t.getEmail());
    pstmt->setString(2, t.getName());
    pstmt->setString(3, t.getSurname());
    pstmt->setString(4, t.getPassword());
    pstmt->setDouble(5, t.getWallet());
    pstmt->setBoolean(6, t.getDisability());
    pstmt->setBoolean(7, t.getActiveAccount());
    pstmt->setInt(8, t.getIdUserCategory());
}

template<>
void Database::insert_query(const VehicleType& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (name, rate_percentage) VALUES (?, ?)"));
    pstmt->setString(1, t.getName());
    pstmt->setDouble(2, t.getRatePercentage());
}

template<>
void Database::insert_query(const Vehicle& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (license_plate, brand, model, id_user, id_vehicle_type)"
    " VALUES (?, ?, ?, ?, ?)"));
    pstmt->setString(1, t.getLicensePlate());
    pstmt->setString(2, t.getBrand());
    pstmt->setString(3, t.getModel());
    pstmt->setInt(4, t.getIdUser());
    pstmt->setInt(5, t.getIdVehicleType());
}

template<>
void Database::insert_query(const ParkingLot& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (name, street, num_parking_slots)"
    " VALUES (?, ?, ?)"));
    pstmt->setString(1, t.getName());
    pstmt->setString(2, t.getStreet());
    pstmt->setInt(3, t.getNumParkingSlots());
}

template<>
void Database::insert_query(const ParkingCategoriesAllowed& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (id_parking_lot, id_user_category)"
    " VALUES (?, ?)"));
    pstmt->setInt(1, t.getIdParkingLot());
    pstmt->setInt(2, t.getIdUserCategory());
}

template<>
void Database::insert_query(const ParkingSlot& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (number, id_parking_lot, id_vehicle_type, reserved_disability)"
    " VALUES (?, ?, ?, ?)"));
    pstmt->setInt(1, t.getNumber());
    pstmt->setInt(2, t.getIdParkingLot());
    pstmt->setInt(3, t.getIdVehicleType());
    pstmt->setBoolean(4, t.getReservedDisability());
}

template<>
void Database::insert_query(const Booking& t){
    pstmt.reset(connection->prepareStatement("INSERT INTO " + get_relation_name(t) + " (datetime_start, datetime_end, amount, "
    " id_user, id_vehicle, id_parking_slot, note) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    pstmt->setString(1, t.getDateTimeStart());
    pstmt->setString(2, t.getDateTimeEnd());
    pstmt->setDouble(3, t.getAmount());
    pstmt->setInt(4, t.getIdUser());
    pstmt->setInt(5, t.getIdVehicle());
    pstmt->setInt(6, t.getIdParkingSlot());
    pstmt->setString(7, t.getNote());
}


//---UPDATE---
template<>
void Database::update_query(const HourlyRate& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET amount= ? WHERE id = ?"));
    pstmt->setDouble(1, t.getAmount());
    pstmt->setInt(2, t.getId());
}

template<>
void Database::update_query(const UserCategory& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET name= ?, id_hourly_rate= ?, service_validity_start= ?, service_validity_end= ?"
    " WHERE id = ?"));
    pstmt->setString(1, t.getName());
    pstmt->setInt(2, t.getIdHourlyRate());

    if (t.getServiceValidityStart() == "")
        pstmt->setNull(3, 0);
    else
        pstmt->setString(3, t.getServiceValidityStart());

    if (t.getServiceValidityEnd() == "")
        pstmt->setNull(4, 0);
    else
        pstmt->setString(4, t.getServiceValidityEnd());

    pstmt->setInt(5, t.getId());
}

template<>
void Database::update_query(const User& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET email= ?, name= ?, surname= ?, password= ?, wallet= ?, disability= ?,"
    "active_account= ?, id_user_category= ? WHERE id = ?"));
    pstmt->setString(1, t.getEmail());
    pstmt->setString(2, t.getName());
    pstmt->setString(3, t.getSurname());
    pstmt->setString(4, t.getPassword());
    pstmt->setDouble(5, t.getWallet());
    pstmt->setBoolean(6, t.getDisability());
    pstmt->setBoolean(7, t.getActiveAccount());
    pstmt->setInt(8, t.getIdUserCategory());
    pstmt->setInt(9, t.getId());
}

template<>
void Database::update_query(const VehicleType& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET name= ?, rate_percentage= ? WHERE id = ?"));
    pstmt->setString(1, t.getName());
    pstmt->setDouble(2, t.getRatePercentage());
    pstmt->setInt(3, t.getId());
}

template<>
void Database::update_query(const Vehicle& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET license_plate= ?, brand= ?, model= ?, id_user= ?, id_vehicle_type= ? WHERE id = ?"));
    pstmt->setString(1, t.getLicensePlate());
    pstmt->setString(2, t.getBrand());
    pstmt->setString(3, t.getModel());
    pstmt->setInt(4, t.getIdUser());
    pstmt->setInt(5, t.getIdVehicleType());
    pstmt->setInt(6, t.getId());
}

template<>
void Database::update_query(const ParkingLot& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET name= ?, street= ?, num_parking_slots= ? WHERE id = ?"));
    pstmt->setString(1, t.getName());
    pstmt->setString(2, t.getStreet());
    pstmt->setInt(3, t.getNumParkingSlots());
    pstmt->setInt(4, t.getId());
}

template<>
void Database::update_query(const ParkingCategoriesAllowed& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET id_parking_lot= ?, id_user_category= ? WHERE id = ?"));
    pstmt->setInt(1, t.getIdParkingLot());
    pstmt->setInt(2, t.getIdUserCategory());
    pstmt->setInt(3, t.getId());
}

template<>
void Database::update_query(const ParkingSlot& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET number= ?, id_parking_lot= ?, id_vehicle_type= ?, reserved_disability= ? WHERE id = ?"));
    pstmt->setInt(1, t.getNumber());
    pstmt->setInt(2, t.getIdParkingLot());
    pstmt->setInt(3, t.getIdVehicleType());
    pstmt->setBoolean(4, t.getReservedDisability());
    pstmt->setInt(5, t.getId());
}

template<>
void Database::update_query(const Booking& t){
    pstmt.reset(connection->prepareStatement("UPDATE " + get_relation_name(t) + " SET datetime_start = ?, datetime_end = ?, entry_time = ?,"
                  "exit_time = ?, amount = ?, id_user = ?, id_vehicle = ?, id_parking_slot = ?, note = ? WHERE id = ?"));
    pstmt->setString(1, t.getDateTimeStart());
    pstmt->setString(2, t.getDateTimeEnd());
    if (t.getEntryTime() != "")
        pstmt->setString(3, t.getEntryTime());
    else
        pstmt->setNull(3, 0);
    if (t.getExitTime() != "")
        pstmt->setString(4, t.getExitTime());
    else
        pstmt->setNull(4, 0);
    pstmt->setDouble(5, t.getAmount());
    pstmt->setInt(6, t.getIdUser());
    pstmt->setInt(7, t.getIdVehicle());
    pstmt->setInt(8, t.getIdParkingSlot());
    pstmt->setString(9, t.getNote());
    pstmt->setInt(10, t.getId());
}
