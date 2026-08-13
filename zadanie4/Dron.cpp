//
// Created by mf on 8/12/26.
//

#include "Dron.h"
#include <iostream>

Dron::Dron() {
    std::cout<<"Inicjalizacja bazy danych...\n";
    int dbStatus = sqlite3_open("OperationLogs",&db);
    if (dbStatus != SQLITE_OK) {
        std::cerr<<"[BLAD] Nie mozna otworzyc bazy, kod bledu: "<<sqlite3_errmsg(db)<<"\n";
        exit(1);
    }

    const char* create_table_sql =
        "CREATE TABLE IF NOT EXISTS OperationLogs ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "Phase TEXT,"
        "Battery REAL,"
        "Latitude REAL,"
        "Longitude REAL,"
        "Altitude REAL);";

    char *error_message = nullptr;
    dbStatus = sqlite3_exec(db,create_table_sql,nullptr,nullptr,&error_message);
    if (dbStatus != SQLITE_OK) {
        std::cerr<<"[BLAD] Nie udalo sie utowrzyc tabeli, kod bledu: "<<error_message<<"\n";
        sqlite3_free(error_message);
        exit(1);
    }

    std::cout<<"Tabel stworzona pomyslnie\n";

    ConnectionResult connect = mavsdk.add_udp_connection("udp://:14540");
    if (connect != ConnectionResult::Success) {
        std::cerr<<"[BLAD] Brak polaczenia, kod bledu: "<<connect<<"\n";
        exit(1);
    }
    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(seconds(1));
        systems = mavsdk.systems();
    }

    system = systems.at(0);
    telemetry = std::make_unique<Telemetry>(system);
    action = std::make_unique<Action>(system);
    offboard = std::make_unique<Offboard>(system);
    mission = std::make_unique<Mission>(system);


    telemetry->subscribe_battery([this](Telemetry::Battery battery) {
       if (battery.remaining_percent< 0.12f) {
           const char* sqlS = "INSERT INTO OperationLogs (Phase) VALUES ('CRITICAL_RTL');";
           char *error_message = nullptr;
           int status = sqlite3_exec(db,sqlS,nullptr,nullptr,&error_message);
           if (status != SQLITE_OK) {
               std::cerr<<"[BLAD] Nie mozna zapisac danych "<<error_message<<"\n";
               sqlite3_free(error_message);
               exit(1);
           }
           Action::Result return_result = action->return_to_launch();
           if (return_result == Action::Result::Success) exit(1);
       }
    });

    telemetry->set_rate_position(1.0);
    telemetry->subscribe_position([this](Telemetry::Position position) {
        szerokosc = position.latitude_deg;
        wysokosc = position.relative_altitude_m;
        dlguosc = position.longitude_deg;
    });

    while (!telemetry->health().is_armable) {
        sleep_for(seconds(1));
    }

}

Dron::~Dron() {
    sqlite3_close(db);
}

void Dron::uzbrojenie() {
    Action::Result armResult = action->arm();
    if (armResult != Action::Result::Success) {
        std::cerr<<"[BLAD] Nie mozna uzbroic\n";
        exit(1);
    }

    std::string sql2 = "INSERT INTO OperationLogs (Phase,Latitude,Longitude,Altitude) VALUES ('START'," +
                              std::to_string(szerokosc) + ", " +
                              std::to_string(dlguosc) + ", " +
                              std::to_string(wysokosc) + ");";
    char* errorr = nullptr;
    int status2 = sqlite3_exec(db,sql2.c_str(),nullptr,nullptr,&errorr);

    if (status2 != SQLITE_OK) {
        std::cerr<<"BLAD: "<<sqlite3_errmsg(db);
        sqlite3_free(errorr);
        exit(1);
    }

}

void Dron::fazaPierwsza() {

    Mission::MissionPlan plan{};
    Mission::MissionItem item{};
    item.latitude_deg = 47.3980398;
    item.longitude_deg = 8.5450150;
    item.relative_altitude_m = 25.0;
    item.speed_m_s = 8.0;
    item.is_fly_through = false;

    plan.mission_items.push_back(item);
    Mission::Result uload= mission->upload_mission(plan);
    if (uload != Mission::Result::Success) {
        exit(1);
    }
    Mission::Result start = mission->start_mission();
    if (start != Mission::Result::Success) {
        exit(1);
    }

    std::string sql = "INSERT INTO OperationLogs (Phase,Latitude,Longitude,Altitude) VALUES ('TRANSIT'," +

                              std::to_string(szerokosc) + ", " +
                              std::to_string(dlguosc) + ", " +
                              std::to_string(wysokosc) + ");";
    char* errorr = nullptr;
    int status = sqlite3_exec(db,sql.c_str(),nullptr,nullptr,&errorr);

    if (status != SQLITE_OK) {
        std::cerr<<"BLAD: "<<sqlite3_errmsg(db);
        sqlite3_free(errorr);
        exit(1);
    }

    while (! mission->is_mission_finished().second) {
        sleep_for(seconds(1));
    }
}

void Dron::fazaDruga() {

    std::string sql2 = "INSERT INTO OperationLogs (Phase,Latitude,Longitude,Altitude) VALUES ('SCANNING'," +

                             std::to_string(szerokosc) + ", " +
                             std::to_string(dlguosc) + ", " +
                             std::to_string(wysokosc) + ");";
    char* errorr = nullptr;
    int status2 = sqlite3_exec(db,sql2.c_str(),nullptr,nullptr,&errorr);

    if (status2 != SQLITE_OK) {
        std::cerr<<"BLAD: "<<sqlite3_errmsg(db);
        sqlite3_free(errorr);
        exit(1);
    }
    Offboard::VelocityBodyYawspeed setPoint{};
    setPoint.forward_m_s = 0.0;
    setPoint.down_m_s = 0.0f;
    setPoint.right_m_s = 0.0;
    setPoint.yawspeed_deg_s = 0.0;

    Offboard::Result set = offboard->set_velocity_body(setPoint);
    if (set != Offboard::Result::Success) {
        exit(1);
    }
    Offboard::Result startResult = offboard->start();
    if (startResult!= Offboard::Result::Success) {
        exit(1);
    }
    setPoint.forward_m_s = 0.0;
    setPoint.down_m_s = 2.0f;
    setPoint.right_m_s = 0.0;
    setPoint.yawspeed_deg_s = 0.0;
    set = offboard->set_velocity_body(setPoint);
    if (set != Offboard::Result::Success) {
        exit(1);
    }

    sleep_for(seconds(5));

    setPoint.forward_m_s = 0.0;
    setPoint.down_m_s = 0.0;
    setPoint.right_m_s = 0.0;
    setPoint.yawspeed_deg_s = 60.0f;

    set = offboard->set_velocity_body(setPoint);
    if (set != Offboard::Result::Success) {
        exit(1);
    }
    sleep_for(seconds(6));

    Offboard::Result stop = offboard->stop();
    if (stop != Offboard::Result::Success) {
        exit(1);
    }
}

void Dron::ladwanie() {

    std::string sql2 = "INSERT INTO OperationLogs (Phase,Latitude,Longitude,Altitude) VALUES ('Land',"+
                             std::to_string(szerokosc) + ", " +
                             std::to_string(dlguosc) + ", " +
                             std::to_string(wysokosc) + ");";
    char* errorr = nullptr;
    int status2 = sqlite3_exec(db,sql2.c_str(),nullptr,nullptr,&errorr);

    if (status2 != SQLITE_OK) {
        std::cerr<<"BLAD: "<<sqlite3_errmsg(db);
        sqlite3_free(errorr);
        exit(1);
    }
    Action::Result land = action->land();
    if (land != Action::Result::Success) {
        exit(1);
    }

    while (telemetry->in_air()) {
        sleep_for(seconds(1));
    }
}
