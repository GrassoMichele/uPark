#include "endpoints_handling.hpp"

using namespace web;
using namespace http;


// POST users/signup
void post_ns::users_signup(const http_request& request, const json::value& json_request, const User& authenticated_user) {
    // request - json: {"email":"x", "name":"x", "surname":"x", "password":"x", "id_user_category":id, "upark_code":"x"}
    json::value response = json::value::object(true);

    User u(
          0,
          json_request.at("email").as_string(),
          json_request.at("name").as_string(),
          json_request.at("surname").as_string(),
          json_request.at("password").as_string(),
          0.0,                                                // wallet
          false,                                              // disability
          true,                                               // active account
          json_request.at("id_user_category").as_integer()
    );

    // get category name
    std::string user_category = mapperUC.Read(u.getIdUserCategory()).getName();

    // A user can be created in DB_upark only if present on DB_University
    bool user_exists = db.user_existence_check(u, json_request.at("upark_code").as_string(), user_category);
    if (user_exists){
        int id = mapperU.Create(u);
        response["message"] = json::value::string("User correctly created!");     //provare a togliere
        response["id"] = json::value::number(id);
        request.reply(status_codes::Created, response);
    }
    else {
        request.reply(status_codes::BadRequest, "User " + u.getEmail() + " not found in University DB! Some information could be wrong. Please contact university!");
    }
}

// POST users/add_money
void post_ns::users_add_money(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);
    User requested_user = mapperU.Read(requested_user_id);

    bool payment_accepted = false;
    float payment_amount = json_request.at("amount").as_number().to_double();

    //User wants to modify his wallet.
    if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

        if (requesting_user.getId() == requested_user_id) {

            if (payment_amount > 0) {
                // payment method, have to be implemented...
                payment_accepted = true;
            }

            if (!payment_accepted || payment_amount <= 0){
                request.reply(status_codes::PaymentRequired, "Payment was not successful!");
                return;
            }
        }

        requested_user.setWallet(requested_user.getWallet() + payment_amount);
        mapperU.Update(requested_user);

        request.reply(status_codes::Accepted, "Payment accepted, wallet updated!");
      }
      else {
          request.reply(status_codes::Unauthorized, "You can't update wallet of another user!");
      }
}

//POST vehicle_types
void post_ns::vehicle_types(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    if (requesting_user_category == "Admin"){

        json::value response = json::value::object(true);

        VehicleType vt(
            0,
            json_request.at("name").as_string(),
            std::stof(json_request.at("rate_percentage").as_string())
        );

        int vehicle_type_id = mapperVT.Create(vt);
        response["message"] = json::value::string("Vehicle type successfully added!");     //provare a togliere
        response["id"] = json::value::number(vehicle_type_id);

        request.reply(status_codes::Created, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can add vehicle types!");
    }
}

// POST hourly_rates
void post_ns::hourly_rates(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user) {
    User requesting_user = authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    if (requesting_user_category == "Admin"){

        json::value response = json::value::object(true);

        HourlyRate h(
            0,
            std::stof(json_request.at("amount").as_string())
        );

        int hourly_rate_id = mapperH.Create(h);
        response["message"] = json::value::string("Hourly rate successfully added!");     //provare a togliere
        response["id"] = json::value::number(hourly_rate_id);

        request.reply(status_codes::Created, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can add hourly rates!");
    }
}

// POST users/{id_user}/vehicles
void post_ns::users_id_vehicles(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    json::value response = json::value::object(true);

    if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

        Vehicle v(
            0,
            json_request.at("license_plate").as_string(),
            json_request.at("brand").as_string(),
            json_request.at("model").as_string(),
            requested_user_id,
            json_request.at("id_vehicle_type").as_number().to_int64()
        );

        int vehicle_id = mapperV.Create(v);
        response["message"] = json::value::string("Vehicle successfully added!");     //provare a togliere
        response["id"] = json::value::number(vehicle_id);

        request.reply(status_codes::Created, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "You can't add vehicle to another user !");
    }
}

// POST parking_lot
void post_ns::parking_lot(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    json::value response = json::value::object(true);

    int num_parking_slots = json_request.at("num_parking_slots").as_number().to_int64();
    int vehicle_type_id = json_request.at("vehicle_type_id").as_number().to_int64();

    if (requesting_user_category == "Admin"){

        ParkingLot pl(
            0,
            json_request.at("name").as_string(),
            json_request.at("street").as_string(),
            num_parking_slots
        );

        int parking_lot_id = mapperPL.Create(pl);

        // categories allowed
        for (UserCategory uc : mapperUC.Read_all()) {     // default: all categories
            if (uc.getName() == "Admin")
                continue;
            ParkingCategoriesAllowed pca(
                0,
                parking_lot_id,
                uc.getId()
            );
            mapperPCA.Create(pca);
        }

        // slots creation
        for (int i = 0; i < num_parking_slots; i++) {
            ParkingSlot ps(
                0,
                i+1,                    // parking slot number starts from 1
                parking_lot_id,
                vehicle_type_id,        // default slots vehicle type chosen by user at parking lot creation time
                false                   // default: not disability
            );
            mapperPS.Create(ps);
        }

        response["message"] = json::value::string("Parking lot with " + std::to_string(num_parking_slots) + " slots successfully added!");
        response["id"] = json::value::number(parking_lot_id);

        request.reply(status_codes::Created, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can add parking lots!");
    }
}


bool booking_time_overlapping_check(const time_t, const time_t, const int, const int, const Booking&);


// POST users/{id_user}/bookings
void post_ns::users_id_bookings(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;

    //check requesting_user = requested_user
    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user.getId() != requested_user_id){
        request.reply(status_codes::Unauthorized, "You can't creat bookings for another user!");
        return;
    }

    //active_account check
    if (requesting_user.getActiveAccount() != true) {
        request.reply(status_codes::Unauthorized, "OPS! Account is not active, contact uPark admin.");
        return;
    }

    //wallet not empty check
    if (requesting_user.getWallet() <= 0) {
        request.reply(status_codes::BadRequest, "OPS! Wallet is empty, add some money first!");
        return;
    }

    //vehicle user ownership check
    int id_vehicle = json_request.at("id_vehicle").as_number().to_int64();
    Vehicle vehicle = mapperV.Read(id_vehicle);

    if (vehicle.getIdUser() != requesting_user.getId()){
        request.reply(status_codes::BadRequest, "You don't own this vehicle.");
        return;
    }

    //check if the cathegory of the user is allowed to the parking lot
    int id_parking_slot = json_request.at("id_parking_slot").as_number().to_int64();

    ParkingSlot parking_slot = mapperPS.Read(id_parking_slot);
    int id_parking_lot = parking_slot.getIdParkingLot();

    std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();
    std::vector<ParkingCategoriesAllowed>::iterator it;

    it = std::find_if(std::begin(parking_categories_allowed), std::end(parking_categories_allowed), [id_parking_lot, &requesting_user](const ParkingCategoriesAllowed& pca)
    {
        return (requesting_user.getIdUserCategory() == pca.getIdUserCategory() && id_parking_lot == pca.getIdParkingLot());
    });

    if (it == std::end(parking_categories_allowed)){
        request.reply(status_codes::BadRequest, "Selected parking lot is not available for the requesting_user category!");
        return;
    }

    //check if the vehicle type is the same of the parking slot vehicle type allowed
    if(parking_slot.getIdVehicleType() != vehicle.getIdVehicleType()){
        request.reply(status_codes::BadRequest, "Selected parking slot isn't of the same type of selected vehicle type!");
        return;
    }

    //check if the selected slot is disability reserved and user can occupy that
    if(parking_slot.getReservedDisability() == true && requesting_user.getDisability() == false){
        request.reply(status_codes::BadRequest, "Selected parking slot is reserved only for Disabled people!");
        return;
    }

    //check that datetime_start is not in the past
    std::string datetime_start = json_request.at("datetime_start").as_string();
    std::string datetime_end = json_request.at("datetime_end").as_string();

    time_t current_datetime = time(0);
    struct tm now = *gmtime(&current_datetime);
    now.tm_sec = 0;

    struct tm datetime_start_struct;
    std::istringstream dts(datetime_start);
    dts >> std::get_time(&datetime_start_struct, "%Y-%m-%d %T");

    int multiple_quarter = datetime_start_struct.tm_min % 15;

    if (multiple_quarter != 0) {                                    // datetime_start isn't a multiple of quarter of an hour
        time_t datetime_start_time = timegm(&datetime_start_struct);
        datetime_start_time += (15 - multiple_quarter) * 60;        // adding minutes to reach the quarter
        datetime_start_struct = *(gmtime(&datetime_start_time));
    }

    //is it a future date?
    double seconds = difftime(timegm(&datetime_start_struct), timegm(&now));

    if (seconds <= 0) {
        request.reply(status_codes::BadRequest, "It's impossible to set a booking for a date in the past!");
        return;
    }

    struct tm datetime_end_struct;
    std::istringstream dte(datetime_end);
    dte >> std::get_time(&datetime_end_struct, "%Y-%m-%d %T");

    seconds = difftime(timegm(&datetime_end_struct),timegm(&datetime_start_struct));

    //is datetime_end previous to datetime_start?
    if (seconds <= 0) {
        request.reply(status_codes::BadRequest, "Datetime_end is previous or equals to datetime_start!");
        return;
    }

    // A quarter approssimation to datetime_end
    multiple_quarter = ((int) (seconds/60)) % 15;
    if (multiple_quarter != 0) {
        time_t datetime_end_time = timegm(&datetime_end_struct);
        datetime_end_time += (15 - multiple_quarter) * 60;
        datetime_end_struct = *(gmtime(&datetime_end_time));
    }

    time_t datetime_start_time = timegm(&datetime_start_struct);
    time_t datetime_end_time = timegm(&datetime_end_struct);

    //datetime_start is free of bookings in the requested slot (included 15 minutes of delay for the removing of vehicle)
    std::vector<Booking> bookings = mapperB.Read_all();

    //filtering booking for selected parking slot
    bookings.erase(std::remove_if(bookings.begin(), bookings.end(), [id_parking_slot](const Booking& b)
        {
            return (b.getIdParkingSlot() != id_parking_slot);
        }), bookings.end());

    std::vector<Booking>::iterator it_b;

    it_b = std::find_if(std::begin(bookings), std::end(bookings), [&datetime_start_time, &datetime_end_time, &requested_user_id, &id_vehicle](const Booking& b)
    {
        return booking_time_overlapping_check(datetime_start_time, datetime_end_time, requested_user_id, id_vehicle, b);
    });

    if (it_b != std::end(bookings)){
        request.reply(status_codes::BadRequest, "Booking conflicts with an existing one or you're not using the same vehicle to extend an existing booking!");
        return;
    }

    // check that the user does not have another reservation on the same park (different park slot) in overlapping time intervals.
    bookings = mapperB.Read_all();

    //filtering booking for selected parking lot and user
    bookings.erase(std::remove_if(bookings.begin(), bookings.end(), [requested_user_id, id_parking_lot](const Booking& b)
        {
            return (b.getIdUser() != requested_user_id || mapperPS.Read(b.getIdParkingSlot()).getIdParkingLot() != id_parking_lot);
        }), bookings.end());


    it_b = std::find_if(std::begin(bookings), std::end(bookings), [&datetime_start_time, &datetime_end_time, &requested_user_id, &id_vehicle](const Booking& b)
    {
        return booking_time_overlapping_check(datetime_start_time, datetime_end_time, requested_user_id, id_vehicle, b);
    });

    if (it_b != std::end(bookings)){
        request.reply(status_codes::BadRequest, "User has another reservation on the same parking lot (different park slot) in overlapping time intervals.");
        return;
    }

    // Calculate booking amount
    // hourly_rate * ratepercentage * duration
    UserCategory user_category = mapperUC.Read(requesting_user.getIdUserCategory());
    int id_hourly_rate = user_category.getIdHourlyRate();
    HourlyRate hourly_rate = mapperH.Read(id_hourly_rate);
    float hourly_rate_amount = hourly_rate.getAmount();

    VehicleType vehicle_type = mapperVT.Read(vehicle.getIdVehicleType());
    float rate_percentage = vehicle_type.getRatePercentage();

    double time_interval_seconds = difftime(datetime_end_time, datetime_start_time);

    // amount_parziale = hourly_rate/3600 * durata_in_secondi
    float booking_amount = time_interval_seconds * (hourly_rate_amount/3600) * rate_percentage;

    //check if user's wallet has enough money
    if (requesting_user.getWallet() < booking_amount) {
        request.reply(status_codes::BadRequest, "OPS! You don't have enough money, please add first!");
        return;
    }

    requesting_user.setWallet(requesting_user.getWallet() - booking_amount);
    mapperU.Update(requesting_user);

    // update admin Wallet
    std::vector<User> users = mapperU.Read_all();
    std::vector<User>::iterator it_u;
    it_u = std::find_if(std::begin(users), std::end(users), [](const User& u)
    {
        return (mapperUC.Read(u.getIdUserCategory()).getName() == "Admin");
    });

    if (it_u != std::end(users)) {
        User admin = *it_u;
        admin.setWallet(admin.getWallet() + booking_amount);
        mapperU.Update(admin);
    }

    //Booking creation handling
    std::ostringstream os;
    os << std::put_time(&datetime_start_struct, "%F %T");
    std::string str_datetime_start = os.str();

    os.str("");

    os << std::put_time(&datetime_end_struct, "%F %T");
    std::string str_datetime_end = os.str();

    Booking b(
        0,
        str_datetime_start,
        str_datetime_end,
        "",                         //entry_time
        "",                         //exit_time
        booking_amount,             //amount
        requested_user_id,
        id_vehicle,                 //id_vehicle
        id_parking_slot,            //id_parking_slot
        ""                          //note
    );

    int id = mapperB.Create(b);

    json::value response = json::value::object(true);
    response["message"] = json::value::string("Booking correctly created!");
    response["id"] = json::value::number(id);

    response["datetime_start"] = json::value::string(str_datetime_start);

    response["datetime_end"] = json::value::string(str_datetime_end);

    std::stringstream decimal_value;
    decimal_value << std::fixed << std::setprecision(2) << booking_amount;
    response["amount"] = json::value::string(decimal_value.str());

    request.reply(status_codes::Created, response);
}

// POST crossing
void post_ns::crossing(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    if (json_request.at("auth_token").as_string() == "UHJvY2Vzc2luZ1NlcnZlcg=="){   // "ProcessingServer" in base64

        std::string license_plate = json_request.at("license_plate").as_string();
        int id_parking_lot = json_request.at("id_parking_lot").as_number().to_int64();
        std::string crossing_type = json_request.at("crossing_type").as_string();

        json::value response = json::value::object(true);

        std::vector<Booking> bookings = mapperB.Read_all();
        std::vector<Booking>::iterator it;

        time_t current_datetime = time(0);

        it = std::find_if(std::begin(bookings), std::end(bookings), [current_datetime, license_plate, id_parking_lot, crossing_type](const Booking& b)
        {
            struct tm existing_b_start_struct;
            std::istringstream bdts(b.getDateTimeStart());
            bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");

            struct tm existing_b_end_struct;
            std::istringstream bdte(b.getDateTimeEnd());
            bdte >> std::get_time(&existing_b_end_struct, "%Y-%m-%d %T");

            // check on booking for user's vehicle
            if (b.getIdVehicle() != 0 && mapperV.Read(b.getIdVehicle()).getLicensePlate() != license_plate)
                return false;

            // check on parking lot
            else if (b.getIdParkingSlot() != 0 && mapperPS.Read(b.getIdParkingSlot()).getIdParkingLot() != id_parking_lot)
                return false;

            // check on time, entry allowed 5 minutes before booking starts.
            else if (crossing_type == "entry" && difftime(current_datetime, timegm(&existing_b_start_struct) - (5*60)) >= 0 && difftime(current_datetime, timegm(&existing_b_end_struct)) < 0)
                return true;

            else if (crossing_type == "exit" && difftime(current_datetime, timegm(&existing_b_start_struct) - (5*60)) > 0 && difftime(current_datetime, timegm(&existing_b_end_struct) + (15*60)) <= 0)     // after 5 minutes tow truck removes vehicle, within 10 minutes for a total of 15 minutes. After this time vehicle can't be inside park.
                return true;
            else
                return false;
        });

        if (it != std::end(bookings)){
            std::ostringstream os;
            os << std::put_time(gmtime(&current_datetime), "%T");
            std::string current_datetime_str = os.str();

            if (crossing_type == "entry") {
                it->setEntryTime(current_datetime_str);
                mapperB.Update(*it);
                response["open"] = json::value::boolean(true);
                response["message"] = json::value::string("Entry allowed!");
                request.reply(status_codes::OK, response);
            }
            // crossing_type == "exit"
            else {
                struct tm found_b_end_struct;
                std::istringstream bdte(it->getDateTimeEnd());
                bdte >> std::get_time(&found_b_end_struct, "%Y-%m-%d %T");

                if (difftime(current_datetime, timegm(&found_b_end_struct) + (5*60)) > 0)
                    it->setNote("You have been fined!");

                it->setExitTime(current_datetime_str);
                mapperB.Update(*it);
                response["open"] = json::value::boolean(true);
                response["message"] = json::value::string("Exit allowed!");
                request.reply(status_codes::OK, response);
            }
        }
        else {
          response["open"] = json::value::boolean(false);
          response["message"] = json::value::string("Ops! Can't find appropriate booking for the vehicle.");
          request.reply(status_codes::InternalError, response);
        }
    }
    else {
        request.reply(status_codes::Unauthorized, "You're not processing server!");
    }
}


// function that checks for booking overlapping temporally
bool booking_time_overlapping_check(const time_t datetime_start_time, const time_t datetime_end_time, const int requested_user_id, const int id_vehicle, const Booking& b) {
    //existing booking start_time ad end_time from string to time_t
    struct tm existing_b_start_struct;
    std::istringstream bdts(b.getDateTimeStart());
    bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");
    time_t existing_b_start_time = timegm(&existing_b_start_struct);

    struct tm existing_b_end_struct;
    std::istringstream bdte(b.getDateTimeEnd());
    bdte >> std::get_time(&existing_b_end_struct, "%Y-%m-%d %T");
    time_t existing_b_end_time = timegm(&existing_b_end_struct);

    int time_between_bookings = requested_user_id == b.getIdUser() ? 0 : (15*60);             // 0 to allow same user to make contiguous bookings

    //cheking if requested booking (-15 minutes) starts before the ending of temporally earlier bookings or requested booking is contained inside an existing one
    if (difftime((datetime_start_time - time_between_bookings), existing_b_end_time) < 0 && difftime(existing_b_start_time, datetime_start_time) < 0){
        return true;
    }
    //cheking if requested booking (+15 minutes) ends before the start of temporally subsequent bookings or requested booking contains an existing ones
    else if (difftime((datetime_end_time + time_between_bookings), existing_b_start_time) > 0 && difftime(datetime_start_time, existing_b_start_time) < 0 ){
        return true;
    }
    //cheking if existing booking is overlapping with the requested booking
    else if (difftime(existing_b_start_time, datetime_start_time) == 0 && difftime(existing_b_end_time, datetime_end_time) == 0){
        return true;
    }
    // next two checks are for avoid that the a user can make contiguous bookings with different vehicles
    else if (time_between_bookings==0 && difftime(datetime_start_time, existing_b_end_time) == 0 && difftime(existing_b_start_time, datetime_start_time) < 0){
        if (id_vehicle != b.getIdVehicle())
            return true;
    }
    else if (time_between_bookings==0 && difftime(datetime_end_time, existing_b_start_time) == 0 && difftime(datetime_start_time, existing_b_start_time) < 0 ){
        if (id_vehicle != b.getIdVehicle())
            return true;
    }
    return false;
}
