#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _cv.template wait_for(uniqueLock, std::chrono::minutes(10));
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lockGv(_mutex);
    _queue.clear();
    _queue.emplace_back(msg);
    _cv.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::RED;
}

TrafficLight::~TrafficLight() {
    // is this a wright way to construct a destructor??))
    delete _currentPhase;
    delete _mutex;
    delete _condition;
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the recive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.

    while(_queue.receive() == TrafficLightPhase::RED) {
        continue;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    TrafficObject::threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    std::chrono::time_point<std::chrono::system_clock> last_update;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distribution(4000, 6000);
    int cycle_duration;

    //init last update for start
    lastUpdate = std::chrono::system_clock::now();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate
        = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_update).count();

        cycle_duration = distribution(eng);

        if (timeSinceLastUpdate >= cycle_duration) {
            TrafficLightPhase oldPhase = _currentPhase;
            if (oldPhase == TrafficLightPhase::RED) {
                _currentPhase = TrafficLightPhase::GREEN;
            } else {
                _currentPhase = TrafficLightPhase::RED;
            }
            _queue.send(std::move(_currentPhase));
            last_update = std::chrono::system_clock::now();
        }


    }
}
