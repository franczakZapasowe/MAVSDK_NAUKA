#include <iostream>
#include <chrono>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>

using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

int main () {

    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connect = mavsdk.add_any_connection("udp://:14540");
    if (connect != ConnectionResult::Success) {
        std::cerr<<"Nie udalo sie polaczyc: "<<connect<<std::endl;
        return 1;
    }

    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(seconds(1));
        systems = mavsdk.systems();
    }

    auto system = systems.at(0);

    auto telemetry = Telemetry{system};
    telemetry.subscribe_battery([](Telemetry::Battery battery) {
        std::cout<<"Poziom bateri: "<<battery.remaining_percent * 100.0f<<" %\n";
    });

    auto action = Action{system};

    while (!telemetry.health().is_armable) {
        sleep_for(seconds(1));
    }

    Action::Result arm_result = action.arm();
    if (arm_result != Action::Result::Success) {
        std::cerr<<"Nie udalo sie uzbroic drona: "<<arm_result<<std::endl;
        return 1;
    }

    Action::Result start_result = action.takeoff();
    if (start_result != Action::Result::Success) {
        std::cerr<<"Nie udalo sie wystartowac: "<<start_result<<std::endl;
        return 1;
    }

    sleep_for(seconds(5));
    auto offboard = Offboard{system};
    Offboard::VelocityBodyYawspeed setPoint{};
    setPoint.forward_m_s = 0.0f;
    setPoint.down_m_s = 0.0f;
    setPoint.right_m_s = 0.0f;
    setPoint.yawspeed_deg_s = 0.0f;

    Offboard::Result setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;
        return 1;
    }

    Offboard::Result start = offboard.start();
    if (start != Offboard::Result::Success) {
        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }

    setPoint.forward_m_s = 5.0f;
    setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;

        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }
    sleep_for(seconds(5));

    setPoint.forward_m_s = 0.0f;
    setPoint.right_m_s = 2.0f;

    setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;

        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }
    sleep_for(seconds(3));

    setPoint.right_m_s = 0.0f;
    setPoint.yawspeed_deg_s = 90.0f;

    setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;

        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }
    sleep_for(seconds(3));

    setPoint.yawspeed_deg_s = 0.0f;
    setPoint.forward_m_s = 5.0f;
    setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;

        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }
    sleep_for(seconds(3));

    setPoint.forward_m_s = 0.0f;
    setResult = offboard.set_velocity_body(setPoint);
    if (setResult != Offboard::Result::Success) {
        std::cerr<<"Blad ustawienia punktow: "<<setResult<<std::endl;

        Action::Result landResult = action.land();
        if (landResult != Action::Result::Success) {
            std::cerr<<"Ladowanie nie udane!\n";
            return 1;
        }
        return 1;
    }

    sleep_for(seconds(2));

    offboard.stop();
    Action::Result land = action.land();
    if (land != Action::Result::Success) {
        std::cout<<"Blad ladowania\n";
        return 1;
    }

    while (telemetry.in_air()) {
        sleep_for(seconds(1));
    }

    Action::Result disarm = action.disarm();
    if (disarm != Action::Result::Success) {
        std::cerr<<"Blad disarm\n";
        return 1;
    }

    std::cout<<"Koniec programu\n";
    return 0;
}