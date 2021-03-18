#include "upark_controller.hpp"

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
//---
        // POST users/signup
        if (path[0] == "users" && path[1] == "signup" && path.size() == 2) {
            // request - json: {"email":"x", "name":"x", "surname":"x", "password":"x", "id_user_category":id, "upark_code":"x"}

            request.extract_json().
            then([=](pplx::task<json::value> requestTask) {                       // task based continuation to handle exception
                try {
                    json::value signup_request = requestTask.get();               //inside try to return value or handle a possible exception

                    json::value response = json::value::object(true);

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
                        request.reply(status_codes::BadRequest, "User " + u.getEmail() + " not found in University DB! Some information could be wrong. Please contact university!");
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
        // POST users/add_money
        else if (path[0] == "users" && path[2] == "add_money" && path.size() == 3) {
              //request - json: {"amount":x}

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
        // POST vehicle_types
        else if (path[0] == "vehicle_types" && path.size() == 1) {
              //request - json: {"name": "x", "rate_percentage": "x"}

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value create_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  if (requesting_user_category == "Admin"){

                                      json::value response = json::value::object(true);

                                      //std::cout << std::stof(create_request.at("rate_percentage").as_string()) << '\n';

                                      VehicleType vt(
                                          0,
                                          create_request.at("name").as_string(),
                                          std::stof(create_request.at("rate_percentage").as_string())
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
                              catch(DataMapperException& e) {
                                  request.reply(status_codes::InternalError, e.what());
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
                  catch(UserException& e) {
                      request.reply(status_codes::NotFound, e.what());
                  }
              });
        }
//---
        // POST hourly_rates
        else if (path[0] == "hourly_rates" && path.size() == 1) {
              //request - json: {"amount": "x"}

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value create_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  if (requesting_user_category == "Admin"){

                                      json::value response = json::value::object(true);

                                      HourlyRate h(
                                          0,
                                          std::stof(create_request.at("amount").as_string())
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
                              catch(DataMapperException& e) {
                                  request.reply(status_codes::InternalError, e.what());
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
                  catch(UserException& e) {
                      request.reply(status_codes::NotFound, e.what());
                  }
              });
        }
//---
        // POST users/{id_user}/vehicles
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 3) {
              //request - json: {"license_plate":"x", "brand":"x", "model":"x", "id_vehicle_type":x}

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value create_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  int requested_user_id = std::stoi(path[1]);

                                  json::value response = json::value::object(true);

                                  if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){

                                      Vehicle v(
                                          0,
                                          create_request.at("license_plate").as_string(),
                                          create_request.at("brand").as_string(),
                                          create_request.at("model").as_string(),
                                          requested_user_id,
                                          create_request.at("id_vehicle_type").as_number().to_int64()
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
                              catch(DataMapperException& e) {
                                  request.reply(status_codes::InternalError, e.what());
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
                  catch(UserException& e) {
                      request.reply(status_codes::NotFound, e.what());
                  }
              });
        }
//---
        // POST parking_lot
        else if (path[0] == "parking_lots" && path.size() == 1) {
              //request - json: {"name":"x", "street":"x", "num_parking_slots":x}

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value create_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  json::value response = json::value::object(true);

                                  int num_parking_slots = create_request.at("num_parking_slots").as_number().to_int64();

                                  if (requesting_user_category == "Admin"){

                                      ParkingLot pl(
                                          0,
                                          create_request.at("name").as_string(),
                                          create_request.at("street").as_string(),
                                          num_parking_slots
                                      );

                                      int parking_lot_id = mapperPL.Create(pl);

                                      // categories allowed
                                      for (UserCategory uc : mapperUC.Read_all()) {     // default: all categories
                                          if (uc.getName() == "admin")
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
                                              1,                      // default vehicle: type car
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
                              catch(DataMapperException& e) {
                                  request.reply(status_codes::InternalError, e.what());
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
                  catch(UserException& e) {
                      request.reply(status_codes::NotFound, e.what());
                  }
              });
        }
//---
        // POST users/{id_user}/bookings
        else if (path[0] == "users" && path[2] == "bookings" && path.size() == 3) {
              //request - json: {"datetime_start": "x" , "datetime_end": "x", "id_vehicle": x, "id_parking_slot": x}

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                          try {
                              json::value create_request = requestTask.get();

                              User requesting_user=std::get<1>(result);

                              //check requesting_user = requested_user
                              int requested_user_id = std::stoi(path[1]);

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
                              int id_vehicle = create_request.at("id_vehicle").as_number().to_int64();
                              Vehicle vehicle = mapperV.Read(id_vehicle);

                              if (vehicle.getIdUser() != requesting_user.getId()){
                                  request.reply(status_codes::BadRequest, "You don't own this vehicle.");
                                  return;
                              }

                              //check if the cathegory of the user is allowed to the parking lot
                              int id_parking_slot = create_request.at("id_parking_slot").as_number().to_int64();

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
                              #ifdef _WIN32
                              #define timegm _mkgmtime          //timegm is a unix function, windows equivalent is _mkgmtime
                              #endif

                              std::string datetime_start = create_request.at("datetime_start").as_string();
                              std::string datetime_end = create_request.at("datetime_end").as_string();

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

                              it_b = std::find_if(std::begin(bookings), std::end(bookings), [&datetime_start_time, &datetime_end_time](const Booking& b)
                              {
                                  //existing booking start_time ad end_time from string to time_t
                                  struct tm existing_b_start_struct;
                                  std::istringstream bdts(b.getDateTimeStart());
                                  bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");
                                  time_t existing_b_start_time = timegm(&existing_b_start_struct);

                                  struct tm existing_b_end_struct;
                                  std::istringstream bdte(b.getDateTimeEnd());
                                  bdte >> std::get_time(&existing_b_end_struct, "%Y-%m-%d %T");
                                  time_t existing_b_end_time = timegm(&existing_b_end_struct);

                                  //cheking if requested booking (-15 minutes) starts before the ending of temporally earlier bookings or requested booking is contained inside an existing one
                                  if (difftime((datetime_start_time - (15 * 60)), existing_b_end_time) < 0 && difftime(existing_b_start_time, datetime_start_time) < 0){
                                      return true;
                                  }
                                  //cheking if requested booking (+15 minutes) ends before the start of temporally subsequent bookings or requested booking contains an existing ones
                                  else if (difftime((datetime_end_time + (15 * 60)), existing_b_start_time) > 0 && difftime(datetime_start_time, existing_b_start_time) < 0 ){
                                      return true;
                                  }
                                  //cheking if existing booking is overlapping with the requested booking
                                  else if (difftime(existing_b_start_time, datetime_start_time) == 0 && difftime(existing_b_end_time, datetime_end_time) == 0){
                                      return true;
                                  }
                                  else{
                                      return false;
                                  }
                              });

                              if (it_b != std::end(bookings)){
                                  request.reply(status_codes::BadRequest, "Booking conflicts with an existing one!");
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
                          catch(DataMapperException& e) {
                              request.reply(status_codes::InternalError, e.what());
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
              catch(UserException& e) {
                  request.reply(status_codes::NotFound, e.what());
              }
          });
        }
//---
        // POST crossing
        else if (path[0] == "crossing" && path.size() == 1) {
              //request - json: {"license_plate": "x", "id_parking_lot":x, "crossing_type": "entry"/"exit", "auth_token":"x"}

              request.extract_json().
              then([=](pplx::task<json::value> requestTask) {

                  try {
                      json::value crossing_request = requestTask.get();

                      if (crossing_request.at("auth_token").as_string() == "UHJvY2Vzc2luZ1NlcnZlcg=="){   // "ProcessingServer" in base64

                          std::string license_plate = crossing_request.at("license_plate").as_string();
                          int id_parking_lot = crossing_request.at("id_parking_lot").as_number().to_int64();
                          std::string crossing_type = crossing_request.at("crossing_type").as_string();

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
                  catch(DataMapperException& e) {
                      request.reply(status_codes::InternalError, e.what());
                  }
                  catch(json::json_exception & e) {
                      request.reply(status_codes::BadRequest, "Json body errors!");
                  }
              });
        }
//---
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
//---
        // GET ping
        if (path[0] == "ping" && path.size() == 1) {
            json::value response = json::value::object();
            response["status"] = json::value::string("pong!");
            request.reply(status_codes::OK, response);
        }
//---
        // GET user_categories
        else if (path[0] == "user_categories" && path.size() == 1) {

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
        // GET login
        else if (path[0] == "login" && path.size() == 1) {

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
        // GET users/{id}
        else if (path[0] == "users" && path.size() == 2) {

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
        // GET users
        else if (path[0] == "users" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
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
        // GET vehicle_types
        else if (path[0] == "vehicle_types" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

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
        // GET hourly_rates
        else if (path[0] == "hourly_rates" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

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
        // GET hourly_rates
        else if (path[0] == "vehicles" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
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
        // GET users/{id_user}/vehicles
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 3) {

              pplx::create_task(std::bind(userAuthentication, request))
              .then([=](pplx::task<std::tuple<bool, User>> resultTask)
              {
                  try {
                      std::tuple<bool, User> result = resultTask.get();

                      if (std::get<0>(result) == true){

                          request.extract_json().
                          then([=](pplx::task<json::value> requestTask) {

                              try {
                                  json::value create_request = requestTask.get();

                                  User requesting_user=std::get<1>(result);
                                  std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                                  int requested_user_id = std::stoi(path[1]);

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
                              catch(DataMapperException& e) {
                                  request.reply(status_codes::InternalError, e.what());
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
                  catch(UserException& e) {
                      request.reply(status_codes::NotFound, e.what());
                  }
              });
        }
//---
        // GET parking_lot
        else if (path[0] == "parking_lots" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        std::vector<ParkingLot> parking_lots = mapperPL.Read_all();

                        //response is a list of json object
                        json::value response;
                        int i=0;

                        //adding filter to discriminate from admin which can view all the parking lots
                        //and user which can view only parking lots allowed for his category
                        User requesting_user=std::get<1>(result);
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
        // GET parking_lot/{id_parking_lot}/categories_allowed
        else if (path[0] == "parking_lots" && path[2] == "categories_allowed" && path.size() == 3) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category_name = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_parking_lot_id = std::stoi(path[1]);


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
        // GET parking_lot/{id_parking_lot}/parking_slots
        else if (path[0] == "parking_lots" && path[2] == "parking_slots" && path.size() == 3) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        int requested_parking_lot_id = std::stoi(path[1]);

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
        // GET bookings?since=xxxx-xx-xx&until=xxxx-xx-xx&id_user=x&id_parking_lot=x
        else if (path[0] == "bookings" && path.size() == 1) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category_name = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                        std::map<utility::string_t, utility::string_t> query = requestQuery(request);

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

                        //adding filter to discriminate from admin which can view all the bookings
                        //and user which can view only his own bookings.

                        for(Booking b : bookings){

                            if (requesting_user_category_name == "Admin" || b.getIdUser() == requesting_user.getId()){

                                // filtering on user_id required
                                if (bookings_id_user != 0 && b.getIdUser() != bookings_id_user)
                                    continue;

                                if (bookings_id_parking_lot != 0 && mapperPS.Read(b.getIdParkingSlot()).getIdParkingLot() != bookings_id_parking_lot)
                                    continue;

                                if (bookings_since != "null") {
                                    struct tm existing_b_start_struct;
                                    std::istringstream bdts(b.getDateTimeStart());
                                    bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");
                                    time_t existing_b_start_time = timegm(&existing_b_start_struct);

                                    struct tm filter_b_start_struct;
                                    std::istringstream f_bdts(bookings_since + " 00:00:00");
                                    f_bdts >> std::get_time(&filter_b_start_struct, "%Y-%m-%d %T");
                                    time_t filter_b_start_time = timegm(&filter_b_start_struct);

                                    if (difftime(existing_b_start_time, filter_b_start_time) < 0)
                                        continue;
                                }

                                if (bookings_until != "null") {
                                    struct tm existing_b_start_struct;
                                    std::istringstream bdts(b.getDateTimeStart());
                                    bdts >> std::get_time(&existing_b_start_struct, "%Y-%m-%d %T");
                                    time_t existing_b_start_time = timegm(&existing_b_start_struct);

                                    struct tm filter_b_end_struct;
                                    std::istringstream f_bdte(bookings_until + " 00:00:00");
                                    f_bdte >> std::get_time(&filter_b_end_struct, "%Y-%m-%d %T");
                                    time_t filter_b_end_time = timegm(&filter_b_end_struct);
                                    // include entire last day
                                    filter_b_end_time += (3600 * 24);

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
                            }
                        }

                        request.reply(status_codes::OK, response);
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
        // GET bookings/{id}
        else if (path[0] == "bookings" && path.size() == 2) {

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_booking_id = std::stoi(path[1]);

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
//---
        // PUT users/{id}
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
        // PUT vehicle_types/{id}
        else if (path[0] == "vehicle_types" && path.size() == 2){

            //JSON: {"name": "x", "rate_percentage": null}
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
                                int requested_vehicle_type_id = std::stoi(path[1]);

                                if(requesting_user_category == "Admin"){

                                  VehicleType requested_vehicle_type = mapperVT.Read(requested_vehicle_type_id);

                                  if(!update_request.at("name").is_null())
                                      requested_vehicle_type.setName(update_request.at("name").as_string());

                                  if(!update_request.at("rate_percentage").is_null())
                                      requested_vehicle_type.setRatePercentage(std::stof(update_request.at("rate_percentage").as_string()));

                                  mapperVT.Update(requested_vehicle_type);
                                  request.reply(status_codes::OK, "Vehicle type updated!");

                                }
                                else {
                                    request.reply(status_codes::Unauthorized, "Only admin can update vehicle types!");
                                }
                            }
                            catch(DataMapperException & e) {
                                request.reply(status_codes::InternalError, e.what());
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
                catch(UserException& e) {
                    request.reply(status_codes::NotFound, e.what());
                }
            });
        }
//---
        // PUT hourly_rates/{id}
        else if (path[0] == "hourly_rates" && path.size() == 2){

            //JSON: {"amount": "x"}
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
                                int requested_hourly_rate_id = std::stoi(path[1]);

                                if(requesting_user_category == "Admin"){

                                  HourlyRate requested_hourly_rate = mapperH.Read(requested_hourly_rate_id);

                                  if(!update_request.at("amount").is_null())
                                      requested_hourly_rate.setAmount(std::stof(update_request.at("amount").as_string()));

                                  mapperH.Update(requested_hourly_rate);
                                  request.reply(status_codes::OK, "Hourly rate updated!");

                                }
                                else {
                                    request.reply(status_codes::Unauthorized, "Only admin can update hourly rate!");
                                }
                            }
                            catch(DataMapperException & e) {
                                request.reply(status_codes::InternalError, e.what());
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
                catch(UserException& e) {
                    request.reply(status_codes::NotFound, e.what());
                }
            });
        }
//---
        // PUT parking_lots/{id}
        else if (path[0] == "parking_lots" && path.size() == 2){

            //JSON: {"name": "x", "slots_reserved_disability":{"number":true, "number":false}, "slots_reserved_per_vehicle_types": {"id_vehicle_type":[numbers], ... ,}, "categories_allowed":{"id_user_categories":true, "id_user_categories":false}
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
                                int requested_parking_lot_id = std::stoi(path[1]);

                                if (requesting_user_category == "Admin"){

                                  ParkingLot requested_parking_lot = mapperPL.Read(requested_parking_lot_id);
                                  const int num_parking_slots = requested_parking_lot.getNumParkingSlots();

                                  if(!update_request.at("name").is_null()) {
                                      requested_parking_lot.setName(update_request.at("name").as_string());
                                      mapperPL.Update(requested_parking_lot);
                                  }

                                  std::vector<ParkingSlot> parking_slots = mapperPS.Read_all();

                                  // parking slots of requested parking lot
                                  parking_slots.erase(std::remove_if(parking_slots.begin(), parking_slots.end(), [requested_parking_lot_id](const ParkingSlot& ps)
                                      {
                                          return (ps.getIdParkingLot() != requested_parking_lot_id);
                                      }), parking_slots.end());

                                  // SLOTS reserved for Disability {"1":true, "2":false}
                                  if(!update_request.at("slots_reserved_disability").is_null()) {

                                      json::object slots_reserved_disability = update_request.at("slots_reserved_disability").as_object();

                                      for (ParkingSlot ps : parking_slots) {

                                          bool requested_disability = false;
                                          bool actual_disability = false;

                                          try {
                                              requested_disability = slots_reserved_disability.at(std::to_string(ps.getNumber())).as_bool();
                                              actual_disability = ps.getReservedDisability();

                                              // update only if new value != old value
                                              if (requested_disability != actual_disability) {
                                                  ps.setReservedDisability(requested_disability);
                                                  mapperPS.Update(ps);
                                              }
                                          }
                                          catch (json::json_exception& e) {
                                              std::cout << "No disability info for slot number " << ps.getNumber() << ". Going on..."<< std::endl;
                                          }
                                      }
                                  }

                                  // SLOTS Reserved per vehicle type
                                  if(!update_request.at("slots_reserved_per_vehicle_types").is_null()) {

                                      json::object json_obj = update_request.at("slots_reserved_per_vehicle_types").as_object();

                                      std::vector<VehicleType> vehicle_types = mapperVT.Read_all();

                                      std::vector<int>::iterator it;

                                      //reading vehicle_types, for-each vehicle type(key)     "1", "2" in  {"1":[], "2":[], ....}
                                      for (VehicleType vt : vehicle_types) {

                                          try {
                                              json::array slots_per_vehicle_types = json_obj.at(std::to_string(vt.getId())).as_array();
                                              std::vector<int> slvt;                // slots per vehicle types

                                              //putting json::array in a vector<int>
                                              for (int i = 0; i < slots_per_vehicle_types.size(); i++){
                                                  slvt.push_back(slots_per_vehicle_types.at(i).as_number().to_int64());
                                              }

                                              // slvt = [1,2,3,4]           number : 1 2 3 4
                                              for (ParkingSlot ps : parking_slots) {
                                                it = std::find(std::begin(slvt), std::end(slvt), ps.getNumber());
                                                    if ((it != std::end(slvt)) && (ps.getIdVehicleType() != vt.getId())){
                                                        ps.setIdVehicleType(vt.getId());
                                                        mapperPS.Update(ps);
                                                    }
                                              }
                                          }
                                          catch(json::json_exception& e) {
                                              std::cout << "Vehicle type key missing, going on..." << '\n';
                                          }
                                      }
                                  }

                                  //Categories allowed for parking lot
                                  if(!update_request.at("categories_allowed").is_null()) {

                                      // json : {"id_user_category":true, "id_user_category":false, ...}
                                      json::object user_categories_allowed = update_request.at("categories_allowed").as_object();

                                      std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();

                                      parking_categories_allowed.erase(std::remove_if(parking_categories_allowed.begin(), parking_categories_allowed.end(), [requested_parking_lot_id](const ParkingCategoriesAllowed& pca)
                                          {
                                              return (pca.getIdParkingLot() != requested_parking_lot_id);
                                          }), parking_categories_allowed.end());

                                      // Looping throw all cathegories
                                      for (UserCategory us : mapperUC.Read_all()) {

                                          bool requested_user_category;

                                          // try to find the id_user_category in json (key)
                                          try {
                                              requested_user_category = user_categories_allowed.at(std::to_string(us.getId())).as_bool();

                                              std::vector<ParkingCategoriesAllowed>::iterator it;
                                              it = std::find_if(std::begin(parking_categories_allowed), std::end(parking_categories_allowed), [us](const ParkingCategoriesAllowed& pca)
                                              {
                                                  return (us.getId() == pca.getIdUserCategory());
                                              });

                                              if (requested_user_category == true && it == std::end(parking_categories_allowed)) {    // if PCA record is not present in DB, and admin wants to have it, we create it.
                                                  ParkingCategoriesAllowed pca (
                                                      0,
                                                      requested_parking_lot_id,
                                                      us.getId()
                                                  );
                                                  mapperPCA.Create(pca);
                                              }
                                              else if (requested_user_category == false && it != std::end(parking_categories_allowed)) {    // if PCA record is present in DB, and admin wants to remove it, we delete it.
                                                  mapperPCA.Delete((*it).getId());
                                              }
                                          }
                                          catch (json::json_exception& e) {
                                              std::cout << "No user category with key " << us.getId() << " found. Going on..."<< std::endl;
                                          }
                                      }
                                  }

                                  request.reply(status_codes::OK, "Parking lot updated!");

                                }
                                else {
                                    request.reply(status_codes::Unauthorized, "Only admin can update parking lots!");
                                }
                            }
                            catch(DataMapperException & e) {
                                request.reply(status_codes::InternalError, e.what());
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

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_user_id = std::stoi(path[1]);

                        if (requesting_user_category == "Admin") {
                            mapperU.Delete(requested_user_id);
                            request.reply(status_codes::OK, "User deleted");
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "Only admin can delete users!");
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
            });
        }
//---
        // DELETE vehicle_types/{id}
        else if (path[0] == "vehicle_types" && path.size() == 2){

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_vehicle_type_id = std::stoi(path[1]);

                        if (requesting_user_category == "Admin") {
                            mapperVT.Delete(requested_vehicle_type_id);
                            request.reply(status_codes::OK, "Vehicle type deleted");
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "Only admin can delete vehicle types!");
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
            });
        }
//---
        // DELETE hourly_rates/{id}
        else if (path[0] == "hourly_rates" && path.size() == 2){

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_hourly_rate_id = std::stoi(path[1]);

                        if (requesting_user_category == "Admin") {
                            mapperH.Delete(requested_hourly_rate_id);
                            request.reply(status_codes::OK, "Hourly rate deleted");
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "Only admin can delete hourly rates!");
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
            });
        }
//---
        // DELETE users/{id_user}/vehicles/{id_vehicle}
        else if (path[0] == "users" && path[2] == "vehicles" && path.size() == 4){

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

                        int requested_user_id = std::stoi(path[1]);
                        int requested_vehicle_id = std::stoi(path[3]);

                        if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){
                            mapperV.Delete(requested_vehicle_id);
                            request.reply(status_codes::OK, "Vehicle deleted!");
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "Only owner or admin can delete vehicles!");
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
            });
        }
//---
        // DELETE parking_lots/{id}
        else if (path[0] == "parking_lots" && path.size() == 2){

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_parking_lot_id = std::stoi(path[1]);

                        if (requesting_user_category == "Admin") {

                          //Delete parking slots of requested parking lot
                          std::vector<ParkingSlot> parking_slots = mapperPS.Read_all();

                          parking_slots.erase(std::remove_if(parking_slots.begin(), parking_slots.end(), [requested_parking_lot_id](const ParkingSlot& ps)
                              {
                                  return (ps.getIdParkingLot() != requested_parking_lot_id);
                              }), parking_slots.end());

                          for (ParkingSlot ps : parking_slots){
                              mapperPS.Delete(ps.getId());
                          }

                          //Delete all records in parkig_lots_user_categories_allowed of the requested parking_lot
                          std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();

                          parking_categories_allowed.erase(std::remove_if(parking_categories_allowed.begin(), parking_categories_allowed.end(), [requested_parking_lot_id](const ParkingCategoriesAllowed& pca)
                              {
                                  return (pca.getIdParkingLot() != requested_parking_lot_id);
                              }), parking_categories_allowed.end());

                          for (ParkingCategoriesAllowed pca : parking_categories_allowed){
                              mapperPCA.Delete(pca.getId());
                          }

                          //Delete of the requested parking_lot
                          mapperPL.Delete(requested_parking_lot_id);

                          request.reply(status_codes::OK, "Parking lot deleted!");
                        }
                        else {
                            request.reply(status_codes::Unauthorized, "Only admin can delete parking lots!");
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
            });
        }
//---
        // DELETE bookings/{id}
        else if (path[0] == "bookings" && path.size() == 2){

            pplx::create_task(std::bind(userAuthentication, request))
            .then([=](pplx::task<std::tuple<bool, User>> resultTask)
            {
                try {
                    std::tuple<bool, User> result = resultTask.get();

                    if (std::get<0>(result) == true){

                        User requesting_user=std::get<1>(result);
                        std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
                        int requested_booking_id = std::stoi(path[1]);

                        Booking booking = mapperB.Read(requested_booking_id);

                        // check on booking ownership
                        if (requesting_user_category != "Admin" && booking.getIdUser() != requesting_user.getId()) {
                            request.reply(status_codes::Unauthorized, "You can't delete booking of another user");
                            return;
                        }

                        // check if delete request is at least 2 hours before booking start time
                        time_t current_datetime = time(0);
                        struct tm now = *gmtime(&current_datetime);
                        now.tm_sec = 0;

                        struct tm datetime_start_struct;
                        std::istringstream dts(booking.getDateTimeStart());
                        dts >> std::get_time(&datetime_start_struct, "%Y-%m-%d %T");

                        double seconds = difftime(timegm(&datetime_start_struct), timegm(&now));

                        if (seconds < (3600*2)) {     // 2 hours
                            request.reply(status_codes::BadRequest, "Impossible to delete a booking with less than two hours notice!");
                            return;
                        }

                        // refund
                        User booking_user = mapperU.Read(booking.getIdUser());
                        booking_user.setWallet(booking_user.getWallet() + booking.getAmount());
                        mapperU.Update(booking_user);

                        // update admin Wallet
                        std::vector<User> users = mapperU.Read_all();
                        std::vector<User>::iterator it;
                        it = std::find_if(std::begin(users), std::end(users), [](const User& u)
                        {
                            return (mapperUC.Read(u.getIdUserCategory()).getName() == "Admin");
                        });

                        if (it != std::end(users)) {
                            User admin = *it;
                            admin.setWallet(admin.getWallet() - booking.getAmount());
                            mapperU.Update(admin);
                        }

                        // booking deleting
                        mapperB.Delete(requested_booking_id);

                        std::stringstream decimal_value;
                        decimal_value << std::fixed << std::setprecision(2) << booking.getAmount();
                        request.reply(status_codes::OK, "Booking correctly deleted! User has been refunded for " + decimal_value.str());
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
