#ifndef ENDPOINTS_HANDLING
#define ENDPOINTS_HANDLING

#include <iostream>
#include <string>
#include <cpprest/http_msg.h>
#include <pplx/pplxtasks.h>
#include <iomanip>

#include "../../database/data_mapper.hpp"

#ifdef _WIN32
#define timegm _mkgmtime          //timegm is a unix function, windows equivalent is _mkgmtime
#endif

extern Database db;
extern DataMapper<HourlyRate> mapperH;
extern DataMapper<UserCategory> mapperUC;
extern DataMapper<User> mapperU;
extern DataMapper<VehicleType> mapperVT;
extern DataMapper<Vehicle> mapperV;
extern DataMapper<ParkingLot> mapperPL;
extern DataMapper<ParkingCategoriesAllowed> mapperPCA;
extern DataMapper<ParkingSlot> mapperPS;
extern DataMapper<Booking> mapperB;


namespace post_ns {
    // POST users/signup
    void users_signup(const web::http::http_request&, const web::json::value&, const User&);

    // POST users/add_money
    void users_add_money(const web::http::http_request&, const web::json::value&, const User&);

    //POST vehicle_types
    void vehicle_types(const web::http::http_request&, const web::json::value&, const User&);

    // POST hourly_rates
    void hourly_rates(const web::http::http_request&, const web::json::value&, const User&);

    // POST users/{id_user}/vehicles
    void users_id_vehicles(const web::http::http_request&, const web::json::value&, const User&);

    // POST parking_lot
    void parking_lot(const web::http::http_request&, const web::json::value&, const User&);

    // POST users/{id_user}/bookings
    void users_id_bookings(const web::http::http_request&, const web::json::value&, const User&);
    
    // POST crossing
    void crossing(const web::http::http_request&, const web::json::value&, const User&);
}

namespace get_ns {
    // GET ping
    void ping(const web::http::http_request&);

    //GET user_categories
    void user_categories(const web::http::http_request&);

    // GET login
    void login(const web::http::http_request&, const web::json::value&, const User&);

    // GET users/{id}
    void users_id(const web::http::http_request&, const web::json::value&, const User&);

    // GET users
    void users(const web::http::http_request&, const web::json::value&, const User&);

    // GET vehicle_types
    void vehicle_types(const web::http::http_request&, const web::json::value&, const User&);

    // GET hourly_rates
    void hourly_rates(const web::http::http_request&, const web::json::value&, const User&);

    // GET vehicles
    void vehicles(const web::http::http_request&, const web::json::value&, const User&);

    // GET users/{id_user}/vehicles
    void users_id_vehicles(const web::http::http_request&, const web::json::value&, const User&);

    // GET parking_lot
    void parking_lot(const web::http::http_request&, const web::json::value&, const User&);

    // GET parking_lot/{id_parking_lot}/categories_allowed
    void parking_lot_id_categories_allowed(const web::http::http_request&, const web::json::value&, const User&);

    // GET parking_lot/{id_parking_lot}/parking_slots
    void parking_lot_id_parking_slots(const web::http::http_request&, const web::json::value&, const User&);

    // GET bookings?since=xxxx-xx-xx&until=xxxx-xx-xx&id_user=x&id_parking_lot=x
    void bookings(const web::http::http_request&, const web::json::value&, const User&);

    // GET bookings/{id}
    void bookings_id(const web::http::http_request&, const web::json::value&, const User&);
}

namespace put_ns {
    // PUT users/{id}
    void users_id(const web::http::http_request&, const web::json::value&, const User&);

    // PUT vehicle_types/{id}
    void vehicle_types_id(const web::http::http_request&, const web::json::value&, const User&);

    // PUT hourly_rates/{id}
    void hourly_rates_id(const web::http::http_request&, const web::json::value&, const User&);

    // PUT parking_lots/{id}
    void parking_lots_id(const web::http::http_request&, const web::json::value&, const User&);
}

namespace delete_ns {
    // DELETE users/{id}
    void users_id(const web::http::http_request&, const web::json::value&, const User&);

    // DELETE vehicle_types/{id}
    void vehicle_types_id(const web::http::http_request&, const web::json::value&, const User&);

    // DELETE hourly_rates/{id}
    void hourly_rates_id(const web::http::http_request&, const web::json::value&, const User&);

    // DELETE users/{id_user}/vehicles/{id_vehicle}
    void users_id_vehicles_id(const web::http::http_request&, const web::json::value&, const User&);

    // DELETE parking_lots/{id}
    void parking_lots_id(const web::http::http_request&, const web::json::value&, const User&);

    // DELETE bookings/{id}
    void bookings_id(const web::http::http_request&, const web::json::value&, const User&);
}

#endif
