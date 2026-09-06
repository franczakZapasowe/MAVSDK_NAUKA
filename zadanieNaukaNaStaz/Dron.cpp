//
// Created by mf on 9/6/26.
//

#include "Dron.h"

#include <future>
#include  <mutex>
#include <iostream>
std::mutex db_mutex;
Dron::Dron() {

    int dbStatus = sqlite3_open("odczytPOzycji.db", &db);
    if (dbStatus != SQLITE_OK) {
        std::cerr<<"[BLAD BAZY] nie mozna otworzyc bazdy, kod bledu: "<<sqlite3_errmsg(db)<<std::endl;
        exit(1);
    }

    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS LogiPozycji("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                             "Latitude REAL, "
                             "Longitude REAL, "
                             "Altitude REAL);";

    char * errorMsg = nullptr;
    dbStatus = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errorMsg);
    if (dbStatus != SQLITE_OK) {
        std::cerr<<"Nie mozna utworzyc tabeli: blad bazy: "<<errorMsg<<std::endl;
        sqlite3_free(errorMsg);
        exit(1);
    }


    ConnectionResult connect = mavsdk.add_any_connection("udp://:14540");
    if (connect != ConnectionResult::Success) {
        std::cerr<<"Connection failed\n";
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
    gimbal = std::make_unique<Gimbal>(system);
    camera = std::make_unique<Camera>(system);
    mission = std::make_unique<Mission>(system);


    telemetry ->set_rate_position(0.2); // w hz czyli raz na 5 sekund
    telemetry->subscribe_position([this](Telemetry::Position position) {
        std::string sql = "INSERT INTO LogiPozycji (Latitude, Longitude, Altitude) VALUES (" +
                            std::to_string(position.latitude_deg) + ", " +
                                std::to_string(position.longitude_deg) + ", " +
                                    std::to_string(position.relative_altitude_m) + ")";

        std::lock_guard<std::mutex>lcok(db_mutex);
        char *error = nullptr;
        int status = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
        if (status != SQLITE_OK) {
            std::cerr<<"Blad wpisania do bazy danych kod bledu: "<<error<<std::endl;
            sqlite3_free(error);
        }
    });

    //blokujemy watek glowny dopokoki nie zglosi nam ze wszystko dobrze
    while (!telemetry->health().is_armable || telemetry->gps_info().num_satellites == 0) {
        sleep_for(seconds(1));
    }

}

Dron::~Dron() {
    sqlite3_close(db);
}

void Dron::startAndArm() {
    Action::Result arm = action->arm();
    if (arm!= Action::Result::Success) {
        std::cerr<<"Nie udalo sie uzbroic silnikow\n";
        exit(1);
    }

    Action::Result takeoff = action->takeoff();
    if (takeoff!= Action::Result::Success) {
        std::cerr<<"Nie udalo sie wystartowac blad przy komendzie takeoff\n";
        exit(1);
    }
}

void Dron::misja() {

    Mission::MissionPlan plan{};
    Mission::MissionItem punkt1{}, punkt2{};

    punkt1.latitude_deg = 47.3983000;
    punkt1.longitude_deg = 8.5453000;
    punkt1.relative_altitude_m = 15.0f;
    punkt1.speed_m_s = 10.0f;
    punkt1.is_fly_through= true;

    punkt2.latitude_deg = 47.3985000;
    punkt2.longitude_deg = 8.5458000;
    punkt2.relative_altitude_m = 10.0f;
    punkt2.speed_m_s = 10.0f;
    punkt2.is_fly_through= false;

    plan.mission_items.push_back(punkt1);
    plan.mission_items.push_back(punkt2);

    Mission::Result wynik = mission->upload_mission(plan);
    if (wynik!= Mission::Result::Success) {
        std::cerr<<"Blad nie mozna wgrac poprawnie misji\n";
        exit(1);
    }

    std::promise<void> mission_promise;
    std::future<void> mission_future = mission_promise.get_future();

    mission->subscribe_mission_progress([&mission_promise](Mission::MissionProgress progress) {
        std::cout << "Postep misji: " << progress.current << " / " << progress.total << std::endl;
        if (progress.current == progress.total) {
            mission_promise.set_value();
        }
    });


    Mission::Result startMisji = mission->start_mission();
    if (startMisji!= Mission::Result::Success) {
        std::cerr<<"Blad startu misji\n";
        exit(1);
    }

    mission_future.wait();
    std::cout << "Misja osiągnęła ostatni punkt.\n";
}

void Dron::zrobZdjecie() {
    Gimbal::Result kontrola = gimbal->take_control(Gimbal::ControlMode::Primary);
    if (kontrola != Gimbal::Result::Success) {
        std::cerr<<"Nie mozna przejac kontorli nad gimbalem\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }

    Gimbal::Result mode = gimbal->set_mode(Gimbal::GimbalMode::YawLock);
    if (mode != Gimbal::Result::Success) {
        std::cerr<<"Nie mozna ustawic mode dla gimbal\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }

    Gimbal::Result ustawGimbal = gimbal->set_pitch_and_yaw(-90.0f,0.0f);
    if (ustawGimbal != Gimbal::Result::Success) {
        std::cerr<<"Nie mozna ustawic gimbal\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }

    //na wszelki wypadek
    sleep_for(seconds(2));
    Camera::Result cameraMode = camera->set_mode(Camera::Mode::Photo);
    if (cameraMode != Camera::Result::Success) {
        std::cerr<<"Nie mozna ustawic trybu kamery\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }

    Camera::Result takePhoto = camera->take_photo();
    if (takePhoto!= Camera::Result::Success) {
        std::cerr<<"Nie mozna zrobic zdjecia\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }
    sleep_for(seconds(1));

    ustawGimbal = gimbal->set_pitch_and_yaw(0.0f,0.0f);
    if (ustawGimbal != Gimbal::Result::Success) {
        std::cerr<<"Nie mozna ustawic gimbal\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }

    kontrola = gimbal->release_control();
    if (kontrola != Gimbal::Result::Success) {
        std::cerr<<"Nie mozna oddac kontorli nad gimbalem\n";
        std::cerr << "Ostrzezenie: Brak kamery\n";
    }
}

void Dron::koniecMisji() {
    Action::Result returnToLaunch = action->return_to_launch();
    if (returnToLaunch != Action::Result::Success) {
        std::cerr<<"Nie dziala return to launch \n";
        exit(1);
    }
    while (telemetry->in_air()) {
        sleep_for(seconds(1));
    }
}
