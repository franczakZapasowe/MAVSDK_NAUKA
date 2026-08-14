//
// Created by mf on 8/14/26.
//

#include "Dron.h"

#include <iostream>

Dron::Dron() {
    int status = sqlite3_open("bazda.db", &db);
    if (status != SQLITE_OK) {
        std::cerr<<"Nie mozna otworzyc pliku blad "<<sqlite3_errmsg(db)<<std::endl;
        exit(1);
    }

    const char* sqlInit =
        "CREATE TABLE IF NOT EXISTS FlightAudit("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "ViolationType TEXT,"
        "Latitude REAL,"
        "Longitude REAL,"
        "Altitude REAL);";

    char *error = nullptr;
    status = sqlite3_exec(db, sqlInit, nullptr, nullptr, &error);
    if (status != SQLITE_OK) {
        std::cerr<<"Blad inicjalizacji tabeli: "<<error<<std::endl;
        sqlite3_free(error);
        exit(1);
    }

    std::cout<<"Tabela stworzona pomyslnie\n";

    ConnectionResult connect = mavsdk.add_any_connection("udp://:14540");
    if (connect!= ConnectionResult::Success) {
        std::cerr<<"Connection failed: "<<connect<<std::endl;
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
       if (battery.remaining_percent < 0.3f) {
           awarja = true;
           std::string sql = "INSERT INTO FlightAudit (ViolationType,Latitude,Longitude,Altitude) VALUES ('LOW_BATERY',"+
                        std::to_string(szerokosc) + ", " +
                            std::to_string(dlguosc) + ", " +
                                std::to_string(wysokosc)+");";

           char *error = nullptr;
           int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
           if (status != SQLITE_OK) {
               sqlite3_free(error);
               exit(1);
           }
       }
    });

    telemetry->subscribe_position([this](Telemetry::Position position) {
       if (position.relative_altitude_m>20.0f) {
           // const char *sqlAltitude = "INSERT INTO FlightAudit (ViolationType) VALUES ('ALT_BREACH');";
           // char *error = nullptr;
           // int status = sqlite3_exec(db, sqlAltitude, nullptr, nullptr, &error);
           // if (status != SQLITE_OK) {
           //     std::cerr<<"Nie mozna zapisac do bazy blad: "<<error<<std::endl;
           //     sqlite3_free(error);
           //     exit(1);
           // }
           awarja = true;
           std::string sql = "INSERT INTO FlightAudit (ViolationType,Latitude,Longitude,Altitude) VALUES ('LOW_BATERY',"+
                        std::to_string(szerokosc) + ", " +
                            std::to_string(dlguosc) + ", " +
                                std::to_string(wysokosc)+");";

           char *error = nullptr;
           int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
           if (status != SQLITE_OK) {
               sqlite3_free(error);
               exit(1);
           }


           Action::Result ladowanie = action->land();
           if (ladowanie!=Action::Result::Success) {
               exit(1);
           }
       }

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

void Dron::fazaPierwsza() {

    Action::Result arm = action->arm();

    Action::Result start = action->takeoff();
    if (start!=Action::Result::Success) {
        exit(1);
    }
    sleep_for(seconds(5));

    std::string sql = "INSERT INTO FlightAudit (ViolationType,Latitude,Longitude,Altitude) VALUES ('NONE',"+
                        std::to_string(szerokosc) + ", " +
                            std::to_string(dlguosc) + ", " +
                                std::to_string(wysokosc)+");";

    char *error = nullptr;
    int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
    if (status != SQLITE_OK) {
        sqlite3_free(error);
        exit(1);
    }

    Offboard::VelocityBodyYawspeed setPoint{};
    setPoint.down_m_s = 0.0f;
    setPoint.forward_m_s = 0.0f;
    setPoint.right_m_s = 0.0f;
    setPoint.yawspeed_deg_s = 0.0f;

    Offboard::Result init = offboard->set_velocity_body(setPoint);
    if (init!=Offboard::Result::Success) {
        exit(1);
    }
    Offboard::Result start_result = offboard->start();
    if (start_result!=Offboard::Result::Success) {
        exit(1);
    }

    setPoint.forward_m_s = 4.0f;
    init = offboard->set_velocity_body(setPoint);
    if (init!=Offboard::Result::Success) {
        exit(1);
    }
    sleep_for(seconds(4));

    setPoint.forward_m_s = 0.0f;
    init = offboard->set_velocity_body(setPoint);
    if (init!=Offboard::Result::Success) {
        exit(1);
    }
    sleep_for(seconds(2));

    Offboard::Result stopRe = offboard->stop();
    if (stopRe!=Offboard::Result::Success) {
        exit(1);
    }
}

void Dron::fazaDruga() {
    Mission::MissionPlan plan_misji{};
    Mission::MissionItem item1{};
    item1.latitude_deg = 47.3983000;
    item1.longitude_deg = 8.5453000;
    item1.relative_altitude_m = 15.0f;
    item1.speed_m_s = 10.0f;
    item1.is_fly_through= true;

    Mission::MissionItem item2{};
    item2.latitude_deg = 47.3985000;
    item2.longitude_deg = 8.5458000;
    item2.relative_altitude_m = 10.0f;
    item2.speed_m_s = 10.0f;
    item2.is_fly_through= false;

    plan_misji.mission_items.push_back(item1);
    plan_misji.mission_items.push_back(item2);

    Mission::Result wynik = mission->upload_mission(plan_misji);
    if (wynik!=Mission::Result::Success) {
        exit(1);
    }

    Mission::Result res = mission->start_mission();
    if (res!=Mission::Result::Success) exit(1);

    while (!mission->is_mission_finished().second && awarja==false) {
        sleep_for(seconds(1));
    }

    std::string sql = "INSERT INTO FlightAudit (ViolationType,Latitude,Longitude,Altitude) VALUES ('NONE',"+
                       std::to_string(szerokosc) + ", " +
                           std::to_string(dlguosc) + ", " +
                               std::to_string(wysokosc)+");";

    char *error = nullptr;
    int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
    if (status != SQLITE_OK) {
        sqlite3_free(error);
        exit(1);
    }
}

void Dron::koniecMisji() {
    Action::Result land = action->land();
    if (land!=Action::Result::Success) exit(1);
    while (telemetry->in_air())sleep_for(seconds(1));
    action->disarm();

    std::string sql = "INSERT INTO FlightAudit (ViolationType,Latitude,Longitude,Altitude) VALUES ('KoniecMisji',"+
                      std::to_string(szerokosc) + ", " +
                          std::to_string(dlguosc) + ", " +
                              std::to_string(wysokosc)+");";

    char *error = nullptr;
    int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
    if (status != SQLITE_OK) {
        sqlite3_free(error);
        exit(1);
    }

}
