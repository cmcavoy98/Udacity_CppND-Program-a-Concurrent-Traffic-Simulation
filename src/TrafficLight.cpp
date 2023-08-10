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
    std::unique_lock<std::mutex> lock(_mutexMsgQ);

    _condition.wait(lock, [this] { return !_queue.empty(); });

    T t = std::move(_queue.back());
    _queue.pop_back();
    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutexMsgQ);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        if (_messageQueue.receive() == TrafficLightPhase::green) {
            return;
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

    std::random_device dev;
    std::mt19937 rng(dev());  

    double cycleDuration = 4000; // 4 seconds minimum
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();

    while (true) {

        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (timeSinceLastUpdate > cycleDuration)
        {
            std::cout << "Traffic light interval was " << timeSinceLastUpdate << " Milliseconds.\n";
            if (_currentPhase == TrafficLightPhase::green) {
                _currentPhase = TrafficLightPhase::red;
                std::cout << "Red Light!\n";
            }
            else {
                _currentPhase = TrafficLightPhase::green;
                std::cout << "Green Light!\n";
            }
            TrafficLightPhase phase(_currentPhase);
            _messageQueue.send(std::move(phase));

            // Reset Stopwatch 
            lastUpdate = std::chrono::system_clock::now();

            // Generaste a random number between 4 and 6 and wait
            std::uniform_int_distribution<std::mt19937::result_type> dist(4, 6);
            std::this_thread::sleep_for(std::chrono::seconds(dist(rng)));            
        }
    }
}
