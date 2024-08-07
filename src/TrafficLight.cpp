#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uniqueLock(_mutexMsgQueue);
    _conditionMsgQueue.wait(uniqueLock, [this](){ return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_front();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lockGaurd(_mutexMsgQueue);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _conditionMsgQueue.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{

}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_messageQueue.receive() == TrafficLightPhase::green)
            return;
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
    std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> randomValue(4.0,6.0);
    double cycleDuration = randomValue(gen);

    auto updateStart = std::chrono::high_resolution_clock::now();

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto stop = std::chrono::high_resolution_clock::now();
        double cycleLoop = std::chrono::duration_cast<std::chrono::seconds>(stop-updateStart).count();
        if(cycleLoop > cycleDuration)
        {
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }
        
            auto updateStart = std::chrono::high_resolution_clock::now();
            cycleDuration = randomValue(gen);
        
            _messageQueue.send(std::move(_currentPhase));
        }
    }
}

