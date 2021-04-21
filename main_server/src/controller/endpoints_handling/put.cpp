#include "endpoints_handling.hpp"

using namespace web;
using namespace http;

// PUT users/{id}
void put_ns::users_id(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    //User wants to modify his password. He can do only this kind of update.
    if (requesting_user.getId() == requested_user_id && !json_request.at("password").is_null()){

        User requested_user = mapperU.Read(requested_user_id);
        requested_user.setPassword(json_request.at("password").as_string());

        mapperU.Update(requested_user);

        request.reply(status_codes::OK, "Password correctly updated!");
    }
    //Admin can modify disability, password, active_account
    else if(requesting_user_category == "Admin"){

      User requested_user = mapperU.Read(requested_user_id);

      if(!json_request.at("disability").is_null())
          requested_user.setDisability(json_request.at("disability").as_bool());

      if(!json_request.at("password").is_null())
          requested_user.setPassword(json_request.at("password").as_string());

      if(!json_request.at("active_account").is_null())
          requested_user.setActiveAccount(json_request.at("active_account").as_bool());

      mapperU.Update(requested_user);
      request.reply(status_codes::OK, "Account updated!");

    }
    else {
        request.reply(status_codes::Unauthorized, "You can't update info of another user!");
    }
}


// PUT user_categories/{id_user_category}
void put_ns::user_categories_id(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user = authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_user_category_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if(requesting_user_category == "Admin"){

     UserCategory requested_user_category = mapperUC.Read(requested_user_category_id);

     if(!json_request.at("id_hourly_rate").is_null())
         requested_user_category.setIdHourlyRate(json_request.at("id_hourly_rate").as_number().to_int64());

     if(!json_request.at("service_validity_start").is_null())
         requested_user_category.setServiceValidityStart(json_request.at("service_validity_start").as_string());

     if(!json_request.at("service_validity_end").is_null())
         requested_user_category.setServiceValidityEnd(json_request.at("service_validity_end").as_string());

     mapperUC.Update(requested_user_category);
     request.reply(status_codes::OK, "User category updated!");

    }
    else {
       request.reply(status_codes::Unauthorized, "Only admin can update user categories!");
    }
}


// PUT vehicle_types/{id}
void put_ns::vehicle_types_id(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_vehicle_type_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if(requesting_user_category == "Admin"){

     VehicleType requested_vehicle_type = mapperVT.Read(requested_vehicle_type_id);

     if(!json_request.at("name").is_null())
         requested_vehicle_type.setName(json_request.at("name").as_string());

     if(!json_request.at("rate_percentage").is_null())
         requested_vehicle_type.setRatePercentage(std::stof(json_request.at("rate_percentage").as_string()));

     mapperVT.Update(requested_vehicle_type);
     request.reply(status_codes::OK, "Vehicle type updated!");

    }
    else {
       request.reply(status_codes::Unauthorized, "Only admin can update vehicle types!");
    }
}


// PUT hourly_rates/{id}
void put_ns::hourly_rates_id(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_hourly_rate_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if(requesting_user_category == "Admin"){

      HourlyRate requested_hourly_rate = mapperH.Read(requested_hourly_rate_id);

      if(!json_request.at("amount").is_null())
          requested_hourly_rate.setAmount(std::stof(json_request.at("amount").as_string()));

      mapperH.Update(requested_hourly_rate);
      request.reply(status_codes::OK, "Hourly rate updated!");

    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can update hourly rate!");
    }
}


// PUT parking_lots/{id}
void put_ns::parking_lots_id(const http_request& request, const json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_parking_lot_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user_category == "Admin"){

      ParkingLot requested_parking_lot = mapperPL.Read(requested_parking_lot_id);
      const int num_parking_slots = requested_parking_lot.getNumParkingSlots();

      if(!json_request.at("name").is_null()) {
          requested_parking_lot.setName(json_request.at("name").as_string());
          mapperPL.Update(requested_parking_lot);
      }

      std::vector<ParkingSlot> parking_slots = mapperPS.Read_all();

      // parking slots of requested parking lot
      parking_slots.erase(std::remove_if(parking_slots.begin(), parking_slots.end(), [requested_parking_lot_id](const ParkingSlot& ps)
          {
              return (ps.getIdParkingLot() != requested_parking_lot_id);
          }), parking_slots.end());

      // SLOTS reserved for Disability {"1":true, "2":false}
      if(!json_request.at("slots_reserved_disability").is_null()) {

          json::object slots_reserved_disability = json_request.at("slots_reserved_disability").as_object();

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
      if(!json_request.at("slots_reserved_per_vehicle_types").is_null()) {

          json::object json_obj = json_request.at("slots_reserved_per_vehicle_types").as_object();

          std::vector<VehicleType> vehicle_types = mapperVT.Read_all();

          std::vector<int>::iterator it;

          // reading vehicle_types, for-each vehicle type(key)     "1", "2" in  {"1":[], "2":[], ....}
          for (VehicleType vt : vehicle_types) {

              try {
                  json::array slots_per_vehicle_types = json_obj.at(std::to_string(vt.getId())).as_array();
                  std::vector<int> slvt;                // slots per vehicle types

                  // putting json::array in a vector<int>
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

      // Categories allowed for parking lot
      if(!json_request.at("categories_allowed").is_null()) {

          // json : {"id_user_category":true, "id_user_category":false, ...}
          json::object user_categories_allowed = json_request.at("categories_allowed").as_object();

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
