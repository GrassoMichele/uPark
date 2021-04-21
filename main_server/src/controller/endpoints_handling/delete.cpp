#include "endpoints_handling.hpp"

using namespace web;
using namespace http;


// DELETE users/{id}
void delete_ns::users_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user_category == "Admin") {
        mapperU.Delete(requested_user_id);
        request.reply(status_codes::OK, "User deleted");
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can delete users!");
    }
}

// DELETE vehicle_types/{id}
void delete_ns::vehicle_types_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_vehicle_type_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user_category == "Admin") {
        mapperVT.Delete(requested_vehicle_type_id);
        request.reply(status_codes::OK, "Vehicle type deleted");
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can delete vehicle types!");
    }
}

// DELETE hourly_rates/{id}
void delete_ns::hourly_rates_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_hourly_rate_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user_category == "Admin") {
        mapperH.Delete(requested_hourly_rate_id);
        request.reply(status_codes::OK, "Hourly rate deleted");
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can delete hourly rates!");
    }
}

// DELETE users/{id_user}/vehicles/{id_vehicle}
void delete_ns::users_id_vehicles_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();

    int requested_user_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);
    int requested_vehicle_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[3]);

    if (requesting_user.getId() == requested_user_id || requesting_user_category == "Admin"){
        mapperV.Delete(requested_vehicle_id);
        request.reply(status_codes::OK, "Vehicle deleted!");
    }
    else {
        request.reply(status_codes::Unauthorized, "Only owner or admin can delete vehicles!");
    }
}

// DELETE parking_lots/{id}
void delete_ns::parking_lots_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user=authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_parking_lot_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

    if (requesting_user_category == "Admin") {

      // Get parking slots of requested parking lot
      std::vector<ParkingSlot> parking_slots = mapperPS.Read_all();

      parking_slots.erase(std::remove_if(parking_slots.begin(), parking_slots.end(), [requested_parking_lot_id](const ParkingSlot& ps)
          {
              return (ps.getIdParkingLot() != requested_parking_lot_id);
          }), parking_slots.end());


      // Get all bookings associated with requested parking lot
      std::vector<Booking> bookings = mapperB.Read_all();
      bookings.erase(std::remove_if(bookings.begin(), bookings.end(), [&parking_slots](const Booking& b)
          {
              std::vector<ParkingSlot>::iterator it;
              it = std::find_if(parking_slots.begin(), parking_slots.end(), [&b](const ParkingSlot& ps) {
                  return (b.getIdParkingSlot() == ps.getId());
              });

              // if iterator == end means that booking slot wasn't found among the slots of the parking lot
              // so the booking can be erased because it's of no interest.
              return (it == parking_slots.end());

          }), bookings.end());

      // Delete bookings
      for (Booking b : bookings){
          mapperB.Delete(b.getId());
      }


      // Delete parking slots
      for (ParkingSlot ps : parking_slots){
          mapperPS.Delete(ps.getId());
      }


      // Delete all records in parkig_lots_user_categories_allowed of the requested parking_lot
      std::vector<ParkingCategoriesAllowed> parking_categories_allowed = mapperPCA.Read_all();

      parking_categories_allowed.erase(std::remove_if(parking_categories_allowed.begin(), parking_categories_allowed.end(), [requested_parking_lot_id](const ParkingCategoriesAllowed& pca)
          {
              return (pca.getIdParkingLot() != requested_parking_lot_id);
          }), parking_categories_allowed.end());

      for (ParkingCategoriesAllowed pca : parking_categories_allowed){
          mapperPCA.Delete(pca.getId());
      }

      // Delete of the requested parking_lot
      mapperPL.Delete(requested_parking_lot_id);

      request.reply(status_codes::OK, "Parking lot deleted!");
    }
    else {
        request.reply(status_codes::Unauthorized, "Only admin can delete parking lots!");
    }
}

// DELETE bookings/{id}
void delete_ns::bookings_id(const web::http::http_request& request, const web::json::value& json_request, const User& authenticated_user){
    User requesting_user = authenticated_user;
    std::string requesting_user_category = mapperUC.Read(requesting_user.getIdUserCategory()).getName();
    int requested_booking_id = std::stoi(uri::split_path(uri::decode(request.relative_uri().path()))[1]);

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
