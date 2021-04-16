#include "upark_controller.hpp"
#include "endpoints_handling/endpoints_handling.hpp"

#include <iomanip>
#include <sstream>
#include <ctime>

using namespace web;
using namespace http;

Database db;
DataMapper<HourlyRate> mapperH;
DataMapper<UserCategory> mapperUC;
DataMapper<User> mapperU;
DataMapper<VehicleType> mapperVT;
DataMapper<Vehicle> mapperV;
DataMapper<ParkingLot> mapperPL;
DataMapper<ParkingCategoriesAllowed> mapperPCA;
DataMapper<ParkingSlot> mapperPS;
DataMapper<Booking> mapperB;


void UParkController::initHttpMethodHandlers() {
    listener.support(methods::POST, std::bind(&UParkController::handlePost, this, std::placeholders::_1));
    listener.support(methods::GET, std::bind(&UParkController::handleGet, this, std::placeholders::_1));
    listener.support(methods::PUT, std::bind(&UParkController::handlePut, this, std::placeholders::_1));
    listener.support(methods::DEL, std::bind(&UParkController::handleDelete, this, std::placeholders::_1));
}

//------------------------------------------------------------------------------

void UParkController::handlePost(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {

        // POST users/signup
        if (path[0] == "users" && path[1] == "signup" && path.size() == 2) {
            // request - json: {"email":"x", "name":"x", "surname":"x", "password":"x", "id_user_category":id, "upark_code":"x"}
            handler_json(request, &post_ns::users_signup, User());       // User() because authentication is not required!
        }

        // POST users/add_money
        else if (path[0] == "users" && path[2] == "add_money" && path.size() == 3) {
            //request - json: {"amount":x}
            handler_auth(request, true, &post_ns::users_add_money);
        }

        //POST vehicle_types
        else if (path[0] == "vehicle_types" && path.size() == 1) {
            //request - json: {"name": "x", "rate_percentage": "x"}
            handler_auth(request, true, &post_ns::vehicle_types);
        }

        // POST hourly_rates
        else if (path[0] == "hourly_rates" && path.size() == 1) {
            //request - json: {"amount": "x"}
            handler_auth(request, true, &post_ns::hourly_rates);
        }

        // POST users/{id_user}/vehicles
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 3) {
            //request - json: {"license_plate":"x", "brand":"x", "model":"x", "id_vehicle_type":x}
            handler_auth(request, true, &post_ns::users_id_vehicles);
        }

       // POST parking_lot
       else if (path[0] == "parking_lots" && path.size() == 1) {
            //request - json: {"name":"x", "street":"x", "num_parking_slots":x}
            handler_auth(request, true, &post_ns::parking_lot);
        }

        //POST users/{id_user}/bookings
        else if (path[0] == "users" && path[2] == "bookings" && path.size() == 3) {
            //request - json: {"datetime_start": "x" , "datetime_end": "x", "id_vehicle": x, "id_parking_slot": x}
            handler_auth(request, true, &post_ns::users_id_bookings);
        }

        // POST crossing
        else if (path[0] == "crossing" && path.size() == 1) {
            //request - json: {"license_plate": "x", "id_parking_lot":x, "crossing_type": "entry"/"exit", "auth_token":"x"}
            handler_json(request, &post_ns::crossing, User());
        }

        else{
            request.reply(status_codes::NotFound);
        }
    }
    else{
        request.reply(status_codes::NotFound);
    }
}

//------------------------------------------------------------------------------

void UParkController::handleGet(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {

        // GET ping
        if (path[0] == "ping" && path.size() == 1) {
            get_ns::ping(request);
        }

        //GET user_categories
        else if (path[0] == "user_categories" && path.size() == 1) {
            get_ns::user_categories(request);
        }

        // GET login
        else if (path[0] == "login" && path.size() == 1) {
            handler_auth(request, false, &get_ns::login);
        }

        // GET users/{id}
        else if (path[0] == "users" && path.size() == 2) {
            handler_auth(request, false, &get_ns::users_id);
        }

       // GET users
        else if (path[0] == "users" && path.size() == 1) {
            handler_auth(request, false, &get_ns::users);
        }

        // GET vehicle_types
        else if (path[0] == "vehicle_types" && path.size() == 1) {
            handler_auth(request, false, &get_ns::vehicle_types);
        }
        // GET hourly_rates
        else if (path[0] == "hourly_rates" && path.size() == 1) {
            handler_auth(request, false, &get_ns::hourly_rates);
        }

        // GET vehicles
        else if (path[0] == "vehicles" && path.size() == 1) {
            handler_auth(request, false, &get_ns::vehicles);
        }

        // GET users/{id_user}/vehicles
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 3) {
            handler_auth(request, false, &get_ns::users_id_vehicles);
        }

        // GET parking_lot
        else if (path[0] == "parking_lots" && path.size() == 1) {
            handler_auth(request, false, &get_ns::parking_lot);
        }

        // GET parking_lot/{id_parking_lot}/categories_allowed
        else if (path[0] == "parking_lots" && path[2] == "categories_allowed" && path.size() == 3) {
            handler_auth(request, false, &get_ns::parking_lot_id_categories_allowed);
        }

        // GET parking_lot/{id_parking_lot}/parking_slots
        else if (path[0] == "parking_lots" && path[2] == "parking_slots" && path.size() == 3) {
            handler_auth(request, false, &get_ns::parking_lot_id_parking_slots);
        }

        // GET bookings?since=yyyy-mm-ddThh_mm_ss&until=yyyy-mm-ddThh_mm_ss&id_user=x&id_parking_lot=x
        else if (path[0] == "bookings" && path.size() == 1) {
            handler_auth(request, false, &get_ns::bookings);
        }

        // GET bookings/{id}
        else if (path[0] == "bookings" && path.size() == 2) {
            handler_auth(request, false, &get_ns::bookings_id);
        }

        else{
            request.reply(status_codes::NotFound);
        }
    }
    else{
        request.reply(status_codes::NotFound);
    }
}

//------------------------------------------------------------------------------

void UParkController::handlePut(http_request request) {

    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {
        // PUT users/{id}
        if (path[0] == "users" && path.size() == 2){
            //JSON: {"disability":true, "password":null ,"active_account":true}
            handler_auth(request, true, &put_ns::users_id);
        }
        // PUT vehicle_types/{id}
        else if (path[0] == "vehicle_types" && path.size() == 2){
            //JSON: {"name": "x", "rate_percentage": null}
            handler_auth(request, true, &put_ns::vehicle_types_id);
        }

        // PUT hourly_rates/{id}
        else if (path[0] == "hourly_rates" && path.size() == 2){
            //JSON: {"amount": "x"}
            handler_auth(request, true, &put_ns::hourly_rates_id);
        }

        // PUT parking_lots/{id}
        else if (path[0] == "parking_lots" && path.size() == 2){
            //JSON: {"name": "x", "slots_reserved_disability":{"number":true, "number":false}, "slots_reserved_per_vehicle_types": {"id_vehicle_type":[numbers], ... ,}, "categories_allowed":{"id_user_categories":true, "id_user_categories":false}
            handler_auth(request, true, &put_ns::parking_lots_id);
        }

        // PUT user_categories/{id_user_category}
        else if (path[0] == "user_categories" && path.size() == 2){
            //JSON: {"id_hourly_rate": x, "service_validity_start": "yyyy-mm-dd hh_mm_ss", "service_validity_end": "yyyy-mm-dd hh_mm_ss"}
            handler_auth(request, true, &put_ns::user_categories_id);
        }

        else {
            request.reply(status_codes::NotFound);
        }
    }
    else{
        request.reply(status_codes::NotFound);
    }
}

//------------------------------------------------------------------------------

void UParkController::handleDelete(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {

        // DELETE users/{id}
        if (path[0] == "users" && path.size() == 2){
            handler_auth(request, false, &delete_ns::users_id);
        }

        // DELETE vehicle_types/{id}
        else if (path[0] == "vehicle_types" && path.size() == 2){
            handler_auth(request, false, &delete_ns::vehicle_types_id);
        }

        // DELETE hourly_rates/{id}
        else if (path[0] == "hourly_rates" && path.size() == 2){
            handler_auth(request, false, &delete_ns::hourly_rates_id);
        }

        // DELETE users/{id_user}/vehicles/{id_vehicle}
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 4){
            handler_auth(request, false, &delete_ns::users_id_vehicles_id);
        }

        // DELETE parking_lots/{id}
        else if (path[0] == "parking_lots" && path.size() == 2){
            handler_auth(request, false, &delete_ns::parking_lots_id);
        }

        // DELETE bookings/{id}
        else if (path[0] == "bookings" && path.size() == 2){
            handler_auth(request, false, &delete_ns::bookings_id);
        }

        else {
            request.reply(status_codes::NotFound);
        }
    }
    else{
        request.reply(status_codes::NotFound);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void UParkController::handler_json(const http_request& request, std::function<void(const http_request&, const json::value&, const User&)> handler, const User& authenticated_user) {
    request.extract_json().
    then([=](pplx::task<json::value> requestTask) {                       // task based continuation to handle exception
        try {
            json::value json_request = requestTask.get();               //inside try to return value or handle a possible exception

            handler(request, json_request, authenticated_user);
        }
        catch(DataMapperException & e) {
            request.reply(status_codes::InternalError, e.what());
        }
        catch(json::json_exception & e) {
            request.reply(status_codes::BadRequest, "Json body errors!");
        }
        catch(...) {
            request.reply(status_codes::BadRequest);
        }
    });
}


void UParkController::handler_auth(const http_request& request, bool json_body, std::function<void(const http_request&, const json::value&, const User&)> handler) {

    pplx::create_task(std::bind(userAuthentication, request))
    .then([=](pplx::task<std::tuple<bool, User>> resultTask)
    {
        try {
            std::tuple<bool, User> result = resultTask.get();

            if (std::get<0>(result) == true){

                if (json_body == true) {
                    handler_json(request, handler, std::get<1>(result));    // get<1>(result) = authenticated User
                }
                else {
                    handler(request, json::value(), std::get<1>(result));
                }
            }
            else {
                request.reply(status_codes::Unauthorized,"User doesn't exist or credentials are wrong!");
            }
        }
        catch(DataMapperException & e) {
            request.reply(status_codes::InternalError, e.what());
        }
        catch(UserException& e) {
            request.reply(status_codes::NotFound, e.what());
        }
        catch(...) {
            request.reply(status_codes::BadRequest);
        }
    });
}


std::tuple<bool, User> UParkController::userAuthentication(http_request request){
    http_headers headers = request.headers();

    if (request.headers().find("Authorization") == headers.end())
        throw UserException("No authorization header in http request.");

    utility::string_t auth_header = headers["Authorization"];
    std::string::size_type credentials_position = auth_header.find("Basic");

    if (credentials_position == std::string::npos)            //If not found
        throw UserException("No credentials found in authorization header.");

    std::string base64_creds = auth_header.substr(credentials_position + std::string("Basic").length() + 1);

    if (base64_creds.empty())
        throw UserException("Credentials are empty.");

    std::vector<unsigned char> bytes = utility::conversions::from_base64(base64_creds);
    std::string credentials(bytes.begin(), bytes.end());
    std::string::size_type colon_position = credentials.find(":");

    if (colon_position == std::string::npos)
        throw UserException("No password in authorization header.");

    std::string email = credentials.substr(0, colon_position);
    std::string password = credentials.substr(colon_position + 1, credentials.size() - (colon_position + 1));


    std::vector<User> users = mapperU.Read_all();

    for(User u : users){
        if (u.getEmail() == email && u.getPassword() == password){
            return std::make_tuple(true, u);
        }
    }

    return std::make_tuple(false, User());
}
