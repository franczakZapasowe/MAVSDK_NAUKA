#pragma once
#include <thread>
#include <mavsdk/mavsdk.h>
#include <atomic>
#include <mavsdk.h>
#include <mavsdk.h>
#include <mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/mission/mission.h>
#include <sqlite3.h>
#include <mavsdk/plugins/camera/camera.h>
#include <mavsdk/plugins/gimbal/gimbal.h>

using namespace mavsdk;
using std::this_thread::sleep_for;
using std::chrono::seconds;

class Dron {
    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    std::shared_ptr<System> system;
    std::unique_ptr<Action> action;
    std::unique_ptr<Mission> mission;
    std::unique_ptr<Telemetry> telemetry;
    std::unique_ptr<Offboard> offboard;
    std::unique_ptr<Gimbal> gimbal;
    std::unique_ptr<Camera> camera;
    sqlite3 *db;

public:
    Dron();
    ~Dron();
    void startAndArm();
    void misja();
    void koniecMisji();
    void zrobZdjecie();

};


