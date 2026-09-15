#include <iostream>
#include <future>
#include <chrono>
#include <mavsdk.h>
#include <thread>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/action/action.h>
using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;



int main(){

    Mavsdk mavsdk {Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    ConnectionResult connection_result = mavsdk.add_udp_connection("udp://:14540");
    if (connection_result != ConnectionResult::Success) {
        std::cerr<<"Connection Failure: "<<connection_result<<std::endl;
        exit(1);
    }
    mavsdk.subscribe_on_new_system([&promise]() {
        promise.set_value();
    });
    future.wait();

    auto systems = mavsdk.systems();
    while (systems.empty()) {
        sleep_for(seconds(1));
        systems = mavsdk.systems();
    }

    auto system = systems.at(0);
    auto telemtry = Telemetry{system};
    auto action = Action{system};
    auto offboard = Offboard{system};


    // telemtry.set_rate_attitude_euler(10);
    // telemtry.set_rate_position(10);

    // telemtry.subscribe_attitude_euler([](Telemetry::EulerAngle euler) {
    //    std::cout<<"YWA: "<<euler.yaw_deg<<" PITCH: "
    //     <<euler.pitch_deg<<" ROLL: "<<euler.roll_deg<<"\n";
    // });
    //
    // telemtry.subscribe_position([](Telemetry::Position position) {
    //     std::cout<<"Absolute position: "<<position.absolute_altitude_m<<"\n";
    // });
    // sleep_for(seconds(13));

    Action::Result arm_result = action.arm();
    if (arm_result != Action::Result::Success) {
        std::cerr<<"FAIL TO ARM\n";
        exit(1);
    }

    Action::Result takeoffResult = action.takeoff();
    if (takeoffResult != Action::Result::Success) {
        std::cerr<<"FAIL TO TAKEOFF\n";
        exit(1);
    }

    sleep_for(seconds(3));
    Offboard::VelocityNedYaw setPoint{};
    setPoint.north_m_s = 0.0f;
    setPoint.down_m_s = 0.0f;
    setPoint.yaw_deg = 0.0f;
    setPoint.east_m_s= 0.0f;
    Offboard::Result setVeclocityResult = offboard.set_velocity_ned(setPoint);
    if (setVeclocityResult != Offboard::Result::Success) {
        std::cerr<<"Fail to set setPoints\n";
        exit(1);
    }

    Offboard::Result startOffboardResult = offboard.start();
    if (startOffboardResult != Offboard::Result::Success) {
        std::cerr<<"Fail to start offboard\n";
        exit(1);
    }

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (true) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - begin;
        elapsed_seconds = std::chrono::duration_cast<seconds>(elapsed_seconds);
        if (elapsed_seconds.count() >= 5.0) break;
        setPoint.north_m_s = 2.0f;
        setVeclocityResult = offboard.set_velocity_ned(setPoint);
        if (setVeclocityResult != Offboard::Result::Success) {
            std::cerr<<"Fail to set setPoints\n";
            exit(1);
        }
        sleep_for(std::chrono::milliseconds(50));
    }

    setPoint.north_m_s = 0.0f;
    setPoint.down_m_s = 0.0f;
    setPoint.yaw_deg = 0.0f;
    setPoint.east_m_s= 0.0f;
    setVeclocityResult = offboard.set_velocity_ned(setPoint);
    if (setVeclocityResult != Offboard::Result::Success) {
        std::cerr<<"Fail to set setPoints\n";
        exit(1);
    }
    sleep_for(seconds(3));

    Offboard::Result stopOffboardResult = offboard.stop();
    if (stopOffboardResult != Offboard::Result::Success) {
        std::cerr<<"Fail to stop offboard\n";
        exit(1);
    }
    sleep_for(seconds(2));
    Action::Result landResult = action.land();
    if (landResult != Action::Result::Success) {
        std::cerr<<"FAIL TO LAND\n";
        exit(1);
    }

    while (telemtry.in_air()) {
        sleep_for(seconds(1));
    }

    Action::Result disarm_result = action.disarm();
    if (disarm_result != Action::Result::Success) {
        std::cerr<<"FAIL TO DISARM\n";
        exit(1);
    }

    return 0;
}