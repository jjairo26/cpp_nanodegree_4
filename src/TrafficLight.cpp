#include <iostream>
#include <random>
#include "TrafficLight.h"

#include <thread>
#include <future>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_queue.empty(); });
    T message = _queue.back();
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lck(_mutex);
    _queue.clear();

    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red; //default red light
}

TrafficLight::~TrafficLight(){}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if (_queue.receive() == TrafficLightPhase::green)
        {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // Get random time between 4.0 and 6.0 seconds
    unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count();    
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> dist(4.0,6.0);
    double random_duration = dist(gen);
    // Set first time point
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto time_passed = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();

        if (time_passed >= random_duration){ // Between 4 and 6 seconds have passed

            //Toggle light
            (_currentPhase == TrafficLightPhase::red) ? _currentPhase = TrafficLightPhase::green : _currentPhase = TrafficLightPhase::red;
            
            //Reset the reference time point
            t1 = std::chrono::high_resolution_clock::now();

            // Send an update method to the message queue
            _queue.send(std::move(_currentPhase));
        }
        
    }

}

