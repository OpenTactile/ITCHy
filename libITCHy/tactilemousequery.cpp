#include "itchy/tactilemousequery.h"
#include "itchy/itchy.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>

struct TactileMouseQuery::impl
{
    bool detached = false;
    bool detachedRunning = false;
    std::thread* mouseThread = nullptr;
    std::mutex mutex;
    unsigned int timeout;
    ITCHy mouse;
    ITCHy::State stateA;
    ITCHy::State stateB;
};

TactileMouseQuery::TactileMouseQuery(bool detached, unsigned int timeout)
{
    implementation = new impl;
    implementation->timeout = timeout;
    implementation->detached = detached;
}

TactileMouseQuery::~TactileMouseQuery()
{
    implementation->detachedRunning = false;
    usleep(2.5e5);
    if(implementation->mouseThread)
        delete implementation->mouseThread;
    implementation->mouse.setColor({0,0,0});
    implementation->mouse.disconnect();
    delete implementation;
}

bool TactileMouseQuery::initialize()
{
    int cnt = 0;
    while(!implementation->mouse.tryConnect() &&  cnt++ < 100)
    {
        usleep(10000);
    }

    if(implementation->mouse.connected())
    {
        feedback(255,128,0);
        implementation->detachedRunning = true;
        if(implementation->detached)
        {
            implementation->mouseThread = new std::thread(
                        [&]()
            {
                while(implementation->detachedRunning)
                {
                    ITCHy::State tmp = implementation->mouse.currentState(500);
                    {
                        std::lock_guard<std::mutex> guard(implementation->mutex);
                        implementation->stateA = tmp;
                    }
                }
            });
        }
    }

    return implementation->mouse.connected();
}

void TactileMouseQuery::feedback(unsigned char r, unsigned char g, unsigned char b)
{
    implementation->mouse.setColor({r,g,b});
}

void TactileMouseQuery::update()
{
    if(implementation->detached)
    {
        std::lock_guard<std::mutex> guard(implementation->mutex);
        implementation->stateB = implementation->stateA;
    }
    else
    {
        implementation->stateB = implementation->mouse.currentState(implementation->timeout);
    }
}

QVector2D TactileMouseQuery::position() const
{
    return QVector2D(
                implementation->stateB.position[0],
                implementation->stateB.position[1]);
}

QVector2D TactileMouseQuery::velocity() const
{
    return QVector2D(
                implementation->stateB.velocity[0],
                implementation->stateB.velocity[1]);
}

float TactileMouseQuery::orientation() const
{
    return implementation->stateB.angle;
}

float TactileMouseQuery::angularVelocity() const
{
    return implementation->stateB.angularVelocity;
}

bool TactileMouseQuery::buttonPressed() const
{
   return (implementation->stateB.button == 1);
}
