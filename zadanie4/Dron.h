//
// Created by mf on 8/12/26.
//

#ifndef ZADANIE4_DRON_H
#define ZADANIE4_DRON_H
#include <thread>
#include <chrono>
#include <atomic>
#include <mavsdk.h>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/mission/mission.h>
#include <sqlite3.h>
using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

class Dron {
    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    std::shared_ptr<System> system;
    std::unique_ptr<Action> action;
    std::unique_ptr<Offboard> offboard;
    std::unique_ptr<Mission> mission;
    std::unique_ptr<Telemetry> telemetry;
    sqlite3 *db;
    std::atomic<float>szerokosc = 0.0;
    std::atomic<float>dlguosc = 0.0;
    std::atomic<float>wysokosc =0.0;
public:
    Dron();
    ~Dron();
    void uzbrojenie();
    void fazaPierwsza();
    void fazaDruga();
    void ladwanie();

};


#endif //ZADANIE4_DRON_H
