//
// Created by mf on 8/11/26.
//

#ifndef ZADANIE3_HYBRIDINSPECTOR_H
#define ZADANIE3_HYBRIDINSPECTOR_H
#include <iostream>
#include <chrono>
#include <mavsdk.h>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/mission/mission.h>

using namespace mavsdk;
using std::this_thread::sleep_for;
using std::chrono::seconds;

class HybridInspector {
    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    std::shared_ptr<System> system;
    std::unique_ptr<Telemetry> telemetry;
    std::unique_ptr<Offboard> offboard;
    std::unique_ptr<Mission> mission;
    std::unique_ptr<Action> action;

public:
    HybridInspector();
    void flyToHangar();
    void performIndoorInspection();
    void secureLanding();

};


#endif //ZADANIE3_HYBRIDINSPECTOR_H
