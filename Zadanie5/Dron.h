//
// Created by mf on 8/14/26.
//

#ifndef ZADANIE5_DRON_H
#define ZADANIE5_DRON_H
#include <chrono>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <atomic>
#include <mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
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
    std::atomic<float> szerokosc =0.0f;
    std::atomic<float> wysokosc = 0.0f;
    std::atomic<float> dlguosc =0.0f;
    std::atomic<bool>awarja = false;
public:
    Dron();
    ~Dron();
    void fazaPierwsza();
    void fazaDruga();
    void koniecMisji();
};


#endif //ZADANIE5_DRON_H
