#include "endpoints_handling.hpp"
#include <algorithm>
#include <string>

using namespace web;
using namespace http;


// GET ping
void get_ns::ping(const web::http::http_request& request) {
    json::value response = json::value::object();
    response["status"] = json::value::string("pong!");
    request.reply(status_codes::OK, response);
}

//GET user_categories
void get_ns::user_categories(const web::http::http_request& request){
    //response is a list of json object
    json::value response;
    int i=0;

    std::vector<UserCategory> user_categories = mapperUC.Read_all();
    for(UserCategory uc : user_categories){
        json::value uc_json= json::value::object(true);   // keep_order=true
        uc_json["id"] = json::value::number(uc.getId());
        uc_json["name"] = json::value::string(uc.getName());
        uc_json["id_hourly_rate"] = json::value::number(uc.getIdHourlyRate());
        uc_json["service_validity_start"] = json::value::string(uc.getServiceValidityStart());
        uc_json["service_validity_end"] = json::value::string(uc.getServiceValidityEnd());

        response[i++]=uc_json;
    }
        request.reply(status_codes::OK, response);
}

// GET login
void get_ns::login(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user) {

    json::value response = json::value::object(true);

    response["message"] = json::value::string("Welcome " + authenticated_user.getName() + "!");

    response["id"] = json::value::number(authenticated_user.getId());
    response["email"] = json::value::string(authenticated_user.getEmail());
    response["name"] = json::value::string(authenticated_user.getName());
    response["surname"] = json::value::string(authenticated_user.getSurname());
    //to change with Base64
    response["password"] = json::value::string(authenticated_user.getPassword());

    std::stringstream decimal_value;
    decimal_value << std::fixed << std::setprecision(2) << authenticated_user.getWallet();
    response["wallet"] = json::value::string(decimal_value.str());

    response["disability"] = json::value::boolean(authenticated_user.getDisability());
    response["active_account"] = json::value::boolean(authenticated_user.getActiveAccount());
    response["id_user_category"] = json::value::number(authenticated_user.getIdUserCategory());
    request.reply(status_codes::OK, response);
}

// GET users/{id}
void get_ns::users_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

        User requested_user = mapperU.Read(requested_user_id);

        json::value response = json::value::object(true);
        response["id"] = json::value::number(requested_user.getId());
        response["email"] = json::value::string(requested_user.getEmail());
        response["name"] = json::value::string(requested_user.getName());
        response["surname"] = json::value::string(requested_user.getSurname());
        //to change with Base64
        response["password"] = json::value::string(requested_user.getPassword());

        // cast double to string because json rounding problem on numbers
        std::stringstream decimal_value;
        decimal_value << std::fixed << std::setprecision(2) << requested_user.getWallet();
        response["wallet"] = json::value::string(decimal_value.str());

        response["disability"] = json::value::boolean(requested_user.getDisability());
        response["active_account"] = json::value::boolean(requested_user.getActiveAccount());
        response["id_user_category"] = json::value::number(requested_user.getIdUserCategory());
        request.reply(status_codes::OK, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "You can't get info of another user!");
    }
}

// GET users
void get_ns::users(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    if (requesting_user_category == "Admin") {
        std::vector<User> users = mapperU.Read_all();

        //response is a list of json object
        json::value response;
        int i=0;

        for(User u : users){
            json::value u_json= json::value::object(true);   // keep_order=true
            u_json["id"] = json::value::number(u.getId());
            u_json["email"] = json::value::string(u.getEmail());
            u_json["name"] = json::value::string(u.getName());
            u_json["surname"] = json::value::string(u.getSurname());
            u_json["password"] = json::value::string(u.getPassword());

            // cast double to string because json rounding problem on numbers
            std::stringstream decimal_value;
            decimal_value << std::fixed << std::setprecision(2) << u.getWallet();
            u_json["wallet"] = json::value::string(decimal_value.str());

            u_json["disability"] = json::value::boolean(u.getDisability());
            u_json["active_account"] = json::value::boolean(u.getActiveAccount());
            u_json["id_user_category"] = json::value::number(u.getIdUserCategory());

            response[i++]=u_json;
        }
            request.reply(status_codes::OK, response);
    }
    else {
        request.reply(status_codes::Unauthorized,"You're not admin!");
    }
}

// GET vehicle_types
void get_ns::vehicle_types(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    std::vector<VehicleType> vehicle_types = mapperVT.Read_all();

    //response is a list of json object
    json::value response;
    int i=0;

    for(VehicleType vt : vehicle_types){
        json::value vt_json= json::value::object(true);   // keep_order=true
        vt_json["id"] = json::value::number(vt.getId());
        vt_json["name"] = json::value::string(vt.getName());

        // cast double to string because json rounding problem on numbers
        std::stringstream rate_value;
        rate_value << std::fixed << std::setprecision(2) << vt.getRatePercentage();
        vt_json["rate_percentage"] = json::value::string(rate_value.str());

        response[i++]=vt_json;
    }
        request.reply(status_codes::OK, response);
}

// GET hourly_rates
void get_ns::hourly_rates(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    std::vector<HourlyRate> hourly_rates = mapperH.Read_all();

    //response is a list of json object
    json::value response;
    int i=0;

    for(HourlyRate h : hourly_rates){
        json::value h_json= json::value::object(true);   // keep_order=true
        h_json["id"] = json::value::number(h.getId());

        // cast double to string because json rounding problem on numbers
        std::stringstream amount;
        amount << std::fixed << std::setprecision(2) << h.getAmount();
        h_json["amount"] = json::value::string(amount.str());

        response[i++]=h_json;
    }
        request.reply(status_codes::OK, response);
}

// GET vehicles
void get_ns::vehicles(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    if (requesting_user_category == "Admin") {
        std::vector<Vehicle> vehicles = mapperV.Read_all();

        //response is a list of json object
        json::value response;
        int i=0;

        for(Vehicle v : vehicles){
            json::value v_json= json::value::object(true);   // keep_order=true
            v_json["id"] = json::value::number(v.getId());
            v_json["license_plate"] = json::value::string(v.getLicensePlate());
            v_json["brand"] = json::value::string(v.getBrand());
            v_json["model"] = json::value::string(v.getModel());
            v_json["id_user"] = json::value::number(v.getIdUser());
            v_json["id_vehicle_type"] = json::value::number(v.getIdVehicleType());

            response[i++]=v_json;
        }
            request.reply(status_codes::OK, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "You can't get all vehicles!");
    }
}

// GET users/{id_user}/vehicles
void get_ns::users_id_vehicles(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    json::value response = json::value::object(true);

    if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

        std::vector<Vehicle> vehicles = mapperV.Read_all();

        //response is a list of json object
        json::value response;
        int i=0;

        for(Vehicle v : vehicles){
            if (v.getIdUser() != requested_user_id)
                continue;
            json::value v_json= json::value::object(true);   // keep_order=true
            v_json["id"] = json::value::number(v.getId());
            v_json["license_plate"] = json::value::string(v.getLicensePlate());
            v_json["brand"] = json::value::string(v.getBrand());
            v_json["model"] = json::value::string(v.getModel());
            v_json["id_user"] = json::value::number(v.getIdUser());
            v_json["id_vehicle_type"] = json::value::number(v.getIdVehicleType());

            response[i++]=v_json;
        }
            request.reply(status_codes::OK, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "You can't get vehicles of another user!");
    }
}

// GET parking_lot
void get_ns::parking_lot(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    std::vector<ParkingLot> parking_lots = mapperPL.Read_all();

    //response is a list of json object
    json::value response;
    int i=0;

    //adding filter to discriminate from admin which can view all the parking lots
    //and user which can view only parking lots allowed for his category
    User requesting_user=authenticated_user;
    std::string requesting_user_category_name = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requesting_user_category_id = requesting_user.getIdUserCategory();

    std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();

    parking_categories_allowed.erase(std::remove_if(parking_categories_allowed.begin(), parking_categories_allowed.end(), [requesting_user_category_id](const ParkingCategoriesAllowed& pca)
        {
            return (pca.getIdUserCategory() != requesting_user_category_id);
        }), parking_categories_allowed.end());

    std::vector<ParkingCategoriesAllowed>::iterator it;

    for(ParkingLot pl : parking_lots){

        it = std::find_if(std::begin(parking_categories_allowed), std::end(parking_categories_allowed), [pl](const ParkingCategoriesAllowed& pca)
        {
            return (pl.getId() == pca.getIdParkingLot());
        });

        if (requesting_user_category_name == "Admin" || it != std::end(parking_categories_allowed)){
            json::value b_json= json::value::object(true);   // keep_order=true
            b_json["id"] = json::value::number(pl.getId());
            b_json["name"] = json::value::string(pl.getName());
            b_json["street"] = json::value::string(pl.getStreet());
            b_json["num_parking_slots"] = json::value::number(pl.getNumParkingSlots());

            response[i++]=b_json;
        }
    }

    request.reply(status_codes::OK, response);
}

// GET parking_lot/{id_parking_lot}/categories_allowed
void get_ns::parking_lot_id_categories_allowed(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category_name = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_parking_lot_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);


    if (requesting_user_category_name == "Admin"){

        std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();

        parking_categories_allowed.erase(std::remove_if(parking_categories_allowed.begin(), parking_categories_allowed.end(), [requested_parking_lot_id](const ParkingCategoriesAllowed& pca)
            {
                return (pca.getIdParkingLot() != requested_parking_lot_id);
            }), parking_categories_allowed.end());

        json::value response;
        int i=0;

        for(ParkingCategoriesAllowed pca : parking_categories_allowed){
            json::value pca_json= json::value::object(true);   // keep_order=true
            pca_json["id"] = json::value::number(pca.getId());
            pca_json["id_user_category"] = json::value::number(pca.getIdUserCategory());

            response[i++]=pca_json;
        }

        request.reply(status_codes::OK, response);
    }
    else{
          request.reply(status_codes::Unauthorized,"Only admin can get parking categories allowed for this parking lot!");
    }
}

// GET parking_lot/{id_parking_lot}/parking_slots
void get_ns::parking_lot_id_parking_slots(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    int requested_parking_lot_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);
    std::vector<ParkingSlot> parking_slots = mapperPS.Read_all();

    parking_slots.erase(std::remove_if(parking_slots.begin(), parking_slots.end(), [requested_parking_lot_id](const ParkingSlot& ps)
        {
            return (ps.getIdParkingLot() != requested_parking_lot_id);
        }), parking_slots.end());

    json::value response;
    int i=0;

    for(ParkingSlot ps : parking_slots){
        json::value ps_json= json::value::object(true);   // keep_order=true
        ps_json["id"] = json::value::number(ps.getId());
        ps_json["number"] = json::value::number(ps.getNumber());
        ps_json["id_parking_lot"] = json::value::number(ps.getIdParkingLot());
        ps_json["id_vehicle_type"] = json::value::number(ps.getIdVehicleType());
        ps_json["reserved_disability"] = json::value::boolean(ps.getReservedDisability());

        response[i++] = ps_json;
    }

    request.reply(status_codes::OK, response);
}

// GET bookings?since=xxxx-xx-xx&until=xxxx-xx-xx&id_user=x&id_parking_lot=x
void get_ns::bookings(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category_name = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    std::map<utility::string_t, utility::string_t> query = uri::split_query(uri::decode(request.relative_uri().query()));

    std::string bookings_since = query.count("since") == 1 ? query["since"] : "null";
    std::string bookings_until = query.count("until") == 1 ? query["until"] : "null";

    int bookings_id_user = query.count("id_user") == 1 ? std::stoi(query["id_user"]) : 0;
    int bookings_id_parking_lot = query.count("id_parking_lot") == 1 ? std::stoi(query["id_parking_lot"]) : 0;

    if (requesting_user_category_name != "Admin" && bookings_id_user!=0 && bookings_id_user!=requesting_user.getId()) {
        request.reply(status_codes::Unauthorized,"You can't get bookings of another user!");
        return;
    }

    std::vector<Booking> bookings = mapperB.Read_all();

    //response is a list of json object
    json::value response;
    int i=0;
    std::string time_format = "YYYY-mm-dd";


    //since time management
    time_t filter_b_start_time;
    if (bookings_since != "null") {
        struct tm filter_b_start_struct;

        if (bookings_since.length() != time_format.length()){            //if booking since format contains also time YYYY-mm-ddTHH:MM:SS
            std::replace(bookings_since.begin(), bookings_since.end(), 'T', ' ');
            std::replace(bookings_since.begin(), bookings_since.end(), '_', ':');
        }
        else{
            bookings_since += " 00:00:00";
        }

        std::istringstream f_bdts(bookings_since);
        f_bdts >> std::get_time(&filter_b_start_struct, "%Y-%m-%d %T");
        filter_b_start_time = timegm(&filter_b_start_struct);
    }


    //until time management
    time_t filter_b_end_time;
    if (bookings_until != "null") {
        struct tm filter_b_end_struct;
        bool no_time_provided = false;

        if (bookings_until.length() != time_format.length()){
            std::replace(bookings_until.begin(), bookings_until.end(), 'T', ' ');
            std::replace(bookings_until.begin(), bookings_until.end(), '_', ':');
        }
        else{
            no_time_provided = true;
            bookings_until += " 00:00:00";
        }

        std::istringstream f_bdte(bookings_until);
        f_bdte >> std::get_time(&filter_b_end_struct, "%Y-%m-%d %T");
        filter_b_end_time = timegm(&filter_b_end_struct);

        //include entire last day
        if (no_time_provided)
            filter_b_end_time += (3600 * 24);
    }

    for(Booking b : bookings){
        //if (requesting_user_category_name == "Admin" || b.getIdUser() == requesting_user.getId()){
        // filtering on user_id required
        if (bookings_id_user != 0 && b.getIdUser() != bookings_id_user)
            continue;

        if (bookings_id_parking_lot != 0 && mapperPS.Read(b.getIdParkingSlot()).getIdParkingLot() != bookings_id_parking_lot)
            continue;

        if (bookings_since != "null") {
            struct tm existing_b_end_struct;
            std::istringstream bdts(b.getDateTimeEnd());
            bdts >> std::get_time(&existing_b_end_struct, "%Y-%m-%d %T");
            time_t existing_b_end_time = timegm(&existing_b_end_struct);

            if (difftime(existing_b_end_time, filter_b_start_time) <= 0)
                continue;
        }

        if (bookings_until != "null") {
            struct tm existing_b_start_struct;
            std::istringstream bdts(b.getDateTimeStart());
            bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");
            time_t existing_b_start_time = timegm(&existing_b_start_struct);

            if (difftime(existing_b_start_time, filter_b_end_time) >= 0)
                continue;
        }

        json::value b_json= json::value::object(true);   // keep_order=true
        b_json["id"] = json::value::number(b.getId());
        b_json["datetime_start"] = json::value::string(b.getDateTimeStart());
        b_json["datetime_end"] = json::value::string(b.getDateTimeEnd());
        b_json["entry_time"] = json::value::string(b.getEntryTime());
        b_json["exit_time"] = json::value::string(b.getExitTime());

        std::stringstream decimal_value;
        decimal_value << std::fixed << std::setprecision(2) << b.getAmount();
        b_json["amount"] = json::value::string(decimal_value.str());

        b_json["id_user"] = json::value::number(b.getIdUser());
        b_json["id_vehicle"] = json::value::number(b.getIdVehicle());
        b_json["id_parking_slot"] = json::value::number(b.getIdParkingSlot());
        b_json["note"] = json::value::string(b.getNote());

        response[i++]=b_json;
        //}
    }

    request.reply(status_codes::OK, response);
}

// GET bookings/{id}
void get_ns::bookings_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_booking_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    Booking requested_booking = mapperB.Read(requested_booking_id);

    if (requesting_user.getId() == requested_booking.getIdUser() || requesting_user_category == "Admin"){

        json::value response = json::value::object(true);

        response["id"] = json::value::number(requested_booking.getId());
        response["datetime_start"] = json::value::string(requested_booking.getDateTimeStart());
        response["datetime_end"] = json::value::string(requested_booking.getDateTimeEnd());
        response["entry_time"] = json::value::string(requested_booking.getEntryTime());
        response["exit_time"] = json::value::string(requested_booking.getExitTime());

        std::stringstream decimal_value;
        decimal_value << std::fixed << std::setprecision(2) << requested_booking.getAmount();
        response["amount"] = json::value::string(decimal_value.str());

        response["id_user"] = json::value::number(requested_booking.getIdUser());
        response["id_vehicle"] = json::value::number(requested_booking.getIdVehicle());
        response["id_parking_slot"] = json::value::number(requested_booking.getIdParkingSlot());
        response["note"] = json::value::string(requested_booking.getNote());

        request.reply(status_codes::OK, response);
    }
    else {
        request.reply(status_codes::Unauthorized, "You can't get booking of another user!");
    }
}
