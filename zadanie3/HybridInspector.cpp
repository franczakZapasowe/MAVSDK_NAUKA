//
// Created by mf on 8/11/26.
//

#include "HybridInspector.h"

HybridInspector::HybridInspector() {
    std::cout<<"Sprawdzam polaczanie...\n";
    ConnectionResult connect = mavsdk.add_any_connection("udp://:14540");
    if (connect != ConnectionResult::Success) {
        std::cout<<"[BLAD POLACZENIA]: "<<connect<<"\n";
        exit(1);
    }
    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(seconds(1));
        systems = mavsdk.systems();
    }
    system = systems.at(0);
    telemetry = std::make_unique<Telemetry>(system);
    offboard = std::make_unique<Offboard>(system);
    mission = std::make_unique<Mission>(system);
    action = std::make_unique<Action>(system);

    std::cout << "Czekam na fixa GPS...\n";
    while (!telemetry->health().is_armable) {
        sleep_for(seconds(1));
    }

    telemetry->subscribe_battery([this](Telemetry::Battery battery) {
        if (battery.remaining_percent<0.10f) {
            std::cout<<"Awaryjne ladowanie bateria < 10 %\n";
            Action::Result awaryjne = action->return_to_launch();
            if (awaryjne != Action::Result::Success) {
                exit(1);
            }
        }
    });

}

void HybridInspector::flyToHangar() {
    Action::Result uzbroj = action->arm();
    if (uzbroj != Action::Result::Success) {
        std::cout<<"Nie mozna uzbroic\n";
        exit(1);
    }

    Mission::MissionPlan missionPlan;

    Mission::MissionItem item1{};
    item1.latitude_deg = 47.3980398;
    item1.longitude_deg = 8.5450150;
    item1.relative_altitude_m = 15.0f;
    item1.speed_m_s = 8.0f;
    item1.is_fly_through =true;

    Mission::MissionItem item2{};
    item2.latitude_deg = 47.3982000;
    item2.longitude_deg = 8.5456000;
    item2.relative_altitude_m = 5.0f;
    item2.speed_m_s = 3.0f;
    item2.is_fly_through =false;

    missionPlan.mission_items.push_back(item1);
    missionPlan.mission_items.push_back(item2);

    Mission::Result uploadResult = mission->upload_mission(missionPlan);
    if (uploadResult!=Mission::Result::Success) {
        std::cout<<"Upload misji nie udal sie\n";
        exit(1);
    }

    Mission::Result startReslut = mission->start_mission();
    if (startReslut != Mission::Result::Success) {
        std::cout<<"Nie udalo sie wystartowac\n";
        exit(1);
    }

    while (!mission->is_mission_finished().second) {
        sleep_for(seconds(1));
    }

    std::cout<<"Misja osiagnieta\n";
}

void HybridInspector::performIndoorInspection() {
    Offboard::VelocityBodyYawspeed setPoint;
    setPoint.down_m_s = 0.0f;
    setPoint.forward_m_s = 0.0f;
    setPoint.right_m_s = 0.0f;
    setPoint.yawspeed_deg_s = 0.0f;

    Offboard::Result setResult =offboard->set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cout<<"NIe mozna ustawic punktow\n";
        exit(1);
    }

    Offboard::Result startResult = offboard->start();
    if (startResult != Offboard::Result::Success) {
        exit(1);
    }

    setPoint.forward_m_s = 4.0f;
    setResult = offboard->set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cout<<"NIe mozna ustawic punktow\n";
        exit(1);
    }
    sleep_for(seconds(4));

    setPoint.forward_m_s = 0.0f;
    setResult = offboard->set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cout<<"NIe mozna ustawic punktow\n";
        exit(1);
    }
    sleep_for(seconds(2));
    setPoint.right_m_s = -2.0f;
    setPoint.yawspeed_deg_s = 45.0f;
    setResult = offboard->set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cout<<"NIe mozna ustawic punktow\n";
        exit(1);
    }
    sleep_for(seconds(3));

    setPoint.right_m_s = 0.0f;
    setPoint.forward_m_s = 0.0f;
    setResult = offboard->set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cout<<"NIe mozna ustawic punktow\n";
        exit(1);
    }
    sleep_for(seconds(3));

    Offboard::Result stopResult = offboard->stop();
    if (stopResult!=Offboard::Result::Success) {
        exit(1);
    }
}

void HybridInspector::secureLanding() {
    Action::Result ladowanieResult = action->land();
    if (ladowanieResult!=Action::Result::Success) {
        exit(1);
    }
    while (telemetry->in_air()) {
        sleep_for(seconds(1));
    }
    Action::Result disarmResult = action->disarm();
    if (disarmResult!=Action::Result::Success) {
        exit(1);
    }
}
