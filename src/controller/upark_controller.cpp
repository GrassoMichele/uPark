#include "upark_controller.hpp"

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
//---
        if (path[0] == "users" && path[1] == "signup") {
            request.extract_json().
            then([=](pplx::task<json::value> requestTask) {                       // task based continuation to handle exception
                try {
                    json::value signup_request = requestTask.get();               //inside try to return value or handle a possible exception

                    // json: {"email":"x", "name":"x", "surname":"x", "password":"x", "id_user_category":id, "upark_code":"x"}

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

                    json::value response = json::value::object(true) ;


                    // get category name
                    std::string user_category = mapperUC.Read(u.getIdUserCategory()).getName();

                    // A user can be created in DB_upark only if present on DB_University
                    bool user_exists = db.user_existence_check(u, signup_request.at("upark_code").as_string(), user_category);
                    if (user_exists){
                        int id = mapperU.Create(u);
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
                    request.reply(status_codes::BadRequest, "Json body errors!");
                }
                catch(...) {
                    request.reply(status_codes::BadRequest);
                }
            });
        }

//---
        if (path[0] == "users" && path[2] == "add-money" && path.size() == 3)  {

              //JSON: {"amount":x}
              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value update_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  int requested_user_id = std::stoi(path[1]);
                                  User requested_user = mapperU.Read(requested_user_id);

                                  bool payment_accepted = false;
                                  float payment_amount = update_request.at("amount").as_number().to_double();

                                  //User wants to modify his wallet.
                                  if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

                                      if (requesting_user.getId() == requested_user_id) {

                                          if (payment_amount > 0) {
                                              // payment method, have to be implemented...
                                              payment_accepted = true;
                                          }

                                          if (!payment_accepted || payment_amount <= 0)
                                              request.reply(status_codes::PaymentRequired, "Payment was not successful!");
                                              return;
                                      }

                                      requested_user.setWallet(requested_user.getWallet() + payment_amount);
                                      mapperU.Update(requested_user);

                                      request.reply(status_codes::Accepted, "Payment accepted, wallet updated!");
                                  }
                                  else {
                                      request.reply(status_codes::Unauthorized, "You can't update wallet of another user!");
                                  }
                              }
                              catch(json::json_exception & e) {
                                  request.reply(status_codes::BadRequest, "Json body errors!");
                              }
                          });
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
              });
        }

//---

    }
}

//------------------------------------------------------------------------------

void UParkController::handleGet(http_request request) {
    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {
//---
        if (path[0] == "ping") {
            json::value response = json::value::object();
            response["status"] = json::value::string("pong!");
            request.reply(status_codes::OK, response);
        }
//---

        else if (path[0] == "user_categories") {

            std::vector<UserCategory> user_categories = mapperUC.Read_all();

            //response is a list of json object
            json::value response;
            int i=0;

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
//---

        else if (path[0] == "login") {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask) {

                try {

                    std::tuple<bool, User> result = resultTask.get();


                    if (std::get<0>(result) == true){         // if user which requests the API is present in DB_upark (email-password match)
                        User u=std::get<1>(result);
                        json::value response = json::value::object(true);

                        response["message"] = json::value::string("Welcome " + u.getName() + "!");

                        response["id"] = json::value::number(u.getId());
                        response["email"] = json::value::string(u.getEmail());
                        response["name"] = json::value::string(u.getName());
                        response["surname"] = json::value::string(u.getSurname());
                        //to change with Base64
                        response["password"] = json::value::string(u.getPassword());
                        response["wallet"] = json::value::number(u.getWallet());
                        response["disability"] = json::value::boolean(u.getDisability());
                        response["active_account"] = json::value::boolean(u.getActiveAccount());
                        response["id_user_category"] = json::value::number(u.getIdUserCategory());
                        request.reply(status_codes::OK, response);
                    }
                    else {
                        request.reply(status_codes::Unauthorized, "User doesn't exist or credentials are wrong!");
                    }
                }
                catch(UserException& e) {
                    request.reply(status_codes::Unauthorized, e.what());
                }
            });
        }
//---

        else if (path[0] == "account_info" && path.size() == 2) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_user_id = std::stoi(path[1]);

                        if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

                            User requested_user = mapperU.Read(requested_user_id);

                            json::value response = json::value::object(true);
                            response["id"] = json::value::number(requested_user.getId());
                            response["email"] = json::value::string(requested_user.getEmail());
                            response["name"] = json::value::string(requested_user.getName());
                            response["surname"] = json::value::string(requested_user.getSurname());
                            //to change with Base64
                            response["password"] = json::value::string(requested_user.getPassword());
                            response["wallet"] = json::value::number(requested_user.getWallet());
                            response["disability"] = json::value::boolean(requested_user.getDisability());
                            response["active_account"] = json::value::boolean(requested_user.getActiveAccount());
                            response["id_user_category"] = json::value::number(requested_user.getIdUserCategory());
                            request.reply(status_codes::OK, response);
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "You can't get info of another user!");
                        }
                    }
                    else {
                        request.reply(status_codes::Unauthorized,"User doesn't exist or credentials are wrong!");
                    }
                }
                catch(UserException& e) {
                    request.reply(status_codes::NotFound, e.what());
                }
                });
        }
//---
        else {
            request.reply(status_codes::NotFound);
        }
    }
}

//------------------------------------------------------------------------------

void UParkController::handlePut(http_request request) {

    std::vector< utility::string_t > path = requestPath(request);

    if (!path.empty()) {
//---
        if (path[0] == "users" && path.size() == 2){

            //JSON: {"disability":true, "password":null ,"active_account":true}
            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        request.extract_json().
                        then([=](pplx::task<json::value> requestTask) {

                            try {
                                json::value update_request = requestTask.get();

                                User requesting_user=std::get<1>(result);
                                std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                                int requested_user_id = std::stoi(path[1]);

                                //User wants to modify his password. He can do only this kind of update.
                                if (requesting_user.getId() == requested_user_id && !update_request.at("password").is_null()){

                                    User requested_user = mapperU.Read(requested_user_id);
                                    requested_user.setPassword(update_request.at("password").as_string());

                                    mapperU.Update(requested_user);

                                    request.reply(status_codes::OK, "Password correctly updated!");
                                }
                                //Admin can modify disability, password, active_account
                                else if(requesting_user_category == "Admin"){

                                  User requested_user = mapperU.Read(requested_user_id);

                                  if(!update_request.at("disability").is_null())
                                      requested_user.setDisability(update_request.at("disability").as_bool());

                                  if(!update_request.at("password").is_null())
                                      requested_user.setPassword(update_request.at("password").as_string());

                                  if(!update_request.at("active_account").is_null())
                                      requested_user.setActiveAccount(update_request.at("active_account").as_bool());

                                  mapperU.Update(requested_user);
                                  request.reply(status_codes::OK, "Account updated!");

                                }
                                else {
                                    request.reply(status_codes::Unauthorized, "You can't update info of another user!");
                                }
                            }
                            catch(json::json_exception & e) {
                                request.reply(status_codes::BadRequest, "Json body errors!");
                                return;
                            }
                        });
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
            });
        }
//---
    }
}

//------------------------------------------------------------------------------

void UParkController::handleDelete(http_request request) {
    request.reply(status_codes::NotImplemented, responseNotImplemented(methods::DEL));
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

json::value UParkController::responseNotImplemented(const http::method & method) {
    json::value response = json::value::object();
    response["serviceName"] = json::value::string("uPark server");
    response["http_method"] = json::value::string(method);
    return response;
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
