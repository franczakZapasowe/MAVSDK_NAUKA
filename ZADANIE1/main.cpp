#include <iostream>
#include <chrono>
#include <thread>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>

using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

int main() {

    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    ConnectionResult connect = mavsdk.add_any_connection("udp://:14540");
    if (connect != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connect<< std::endl;
        return 1;
    }

    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(seconds(1));
        systems = mavsdk.systems();
    }
    auto system = systems.at(0);

    auto telemetry = Telemetry{system};
    auto action = Action{system};

    telemetry.subscribe_position([](Telemetry::Position position) {
        std::cout<<"Wysokosc: "<<position.relative_altitude_m<<" m.\n";
    });

    Action::Result arm = action.arm();
    if (arm == Action::Result::Success) {
        std::cout<<"Silniki uzborojone zaraz starujemy\n";
        Action::Result takeoff = action.takeoff();
    }else {
        std::cerr << "Arm failed: " << arm << std::endl;
        return 1;
    }
    sleep_for(seconds(10));
    Action::Result land = action.land();
    sleep_for(seconds(10));

    std::cout<<"Koniec poragmu\n";
    return 0;
}
