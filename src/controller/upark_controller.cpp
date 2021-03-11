#include "upark_controller.hpp"

#include "../database/data_mapper.hpp"

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

void UParkController::handlePost(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {

        if (path[0] == "users" && path[1] == "signup") {
            std::cout << "Ci sono" << '\n';
            request.extract_json().
            then([=](pplx::task<json::value> requestTask) {                       // task based continuation to handle exception
                try {
                    json::value signup_request = requestTask.get();                     //inside try to return value or handle a possible exception

                    // json: //{"email":"x", "name":"x", "surname":"x", "password":"x", "id_user_category":id, "upark_code":"x"}


                    User u(
                          0,
                          signup_request.at("email").as_string(),
                          signup_request.at("name").as_string(),
                          signup_request.at("surname").as_string(),
                          signup_request.at("password").as_string(),
                          0.0,                                                // wallet
                          false,                                              // disability
                          true,                                               // active account
                          signup_request.at("id_user_category").as_integer()
                    );

                    json::value response;

                    // get category name
                    std::string user_category = mapperUC.Read(u.getIdUserCategory()).getName();
                    std::cout << "Ho letto la categoria" << '\n';

                    // A user can be created in DB_upark only if present on DB_University
                    bool user_exists = db.user_existence_check(u, signup_request.at("upark_code").as_string(), user_category);
                    if (user_exists){
                        std::cout << "Sono nell'IF" << '\n';
                        int id = mapperU.Create(u);
                        std::cout << "Ho creato" << '\n';
                        response["message"] = json::value::string("User correctly created!");     //provare a togliere
                        response["id"] = json::value::number(id);
                        request.reply(status_codes::Created, response);
                    }
                    else {
                        std::cout << "User " << u.getEmail() << " not found in University DB! Some information could be wrong. Please contact university." << '\n';
                        response["message"] = json::value::string("User " + u.getEmail() + " not found in University DB! Some information could be wrong. Please contact university.!");
                        request.reply(status_codes::BadRequest, response);
                    }
                }
                catch(DataMapperException & e) {
                    request.reply(status_codes::InternalError, e.what());
                }
                catch(json::json_exception & e) {
                    request.reply(status_codes::BadRequest);
                }
                catch(...) {
                    request.reply(status_codes::BadRequest);
                }
            });
        }
    }
}

void UParkController::handleGet(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {

        if (path[0] == "ping") {
            json::value response = json::value::object();
            response["status"] = json::value::string("pong!");
            request.reply(status_codes::OK, response);
        }

        else {
            request.reply(status_codes::NotFound);
        }
    }
}

void UParkController::handlePut(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImplemented(methods::PUT));
}

void UParkController::handleDelete(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImplemented(methods::DEL));
}

json::value UParkController::responseNotImplemented(const http::method & method) {
    json::value response = json::value::object();
    response["serviceName"] = json::value::string("uPark server");
    response["http_method"] = json::value::string(method);
    return response;
}
