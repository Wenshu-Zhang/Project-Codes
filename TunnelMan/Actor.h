#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
#include <string>

class Actor: public GraphObject{
public:
    //constructor
    Actor(StudentWorld* world, int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, bool visible);
    //destructor
    virtual ~Actor();
    
    //accessors
    StudentWorld* getWorld(){
        return m_world;
    }
    
    //mutators
    void dead();
    bool isAlive() const;
    virtual bool isTunnelMan();
    virtual bool isProtester();
    
    virtual bool block();
    virtual bool annoyed(int amount);
    virtual void doSomething() = 0;
private:
    StudentWorld* m_world;
    bool m_live;
};

class MovingActor: public Actor{
public:
    //constructor
    MovingActor(StudentWorld* world, int imageID, int startX, int startY, Direction dir, int health);
    //destructor
    virtual ~MovingActor();
    //accessors
    int getHealth();
    
    virtual void addGold() = 0;
    virtual bool annoyed(int amount);
    
private:
    virtual void giveUp() = 0;
    int m_health;
};


//Moving Actors
class TunnelMan: public MovingActor{
public:
    //constructor
    TunnelMan(StudentWorld* world);
    //destructor
    virtual ~TunnelMan();
    virtual void doSomething();
    virtual bool isTunnelMan();
    virtual void addGold();
    void loseGold();
    void addSonar();
    void addWater();
    int getGold();
    int getSonar();
    int getWater();
    void loseSonar();
    void shootWater();
    
    
private:
    virtual void giveUp();
    int m_nWater;
    int m_nSonar;
    int m_nGold;
};

//protestors
class Protester: public MovingActor{
public:
    //constructor
    Protester(StudentWorld* world, int imageId, int health, int score);
    //destructor
    virtual ~Protester();
    virtual void doSomething();
    int numSquaresToMoveInCurrentDirection();
    Direction chooseRandDir();
    Direction chooseRandPerp();
    virtual bool annoyed(int amount);
    virtual void addGold();
    virtual bool isProtester();
    void resetRestTicks(int ticks);
    virtual bool isHardcore();
    void sensePlayer(int x, int y);
    Direction playerInSight();
private:
    virtual void giveUp();
    int m_health;
    int m_score;
    int ticksToWaitBetweenMoves;
    int m_stepsInDir;
    int m_restTicks;
    int m_stunTicks;
    int m_shoutTicks;
    int m_perpTicks;
    bool m_leaving;
};

class HardcoreProtester: public Protester{
public:
    HardcoreProtester(StudentWorld* world);
    virtual ~HardcoreProtester();
    virtual void addGold();
    virtual bool isHardcore();
private:
    int ticks_to_stare;
};

//earth
class Earth: public Actor{
public:
    //constructor
    Earth(StudentWorld* world, int startX, int startY);
    //destructor
    virtual ~Earth();
    
    virtual void doSomething();
};

//water squirt
class Water: public Actor{
public:
    //constructor
    Water(StudentWorld* world, int startX, int startY, Direction dir);
    //destructor
    virtual ~Water();
    virtual void doSomething();
private:
    int distance;
    Direction direction;
};

//boulders
class Boulders: public Actor{
public:
    //constructor
    Boulders(StudentWorld* world, int startX, int startY);
    //destructor
    virtual ~Boulders();
    virtual void doSomething();
    bool isStable(int x, int y);
    virtual bool block();
private:
    int m_ticksFalling;
};

//Goodies
class Goodies: public Actor{
public:
    //constructor
    Goodies(StudentWorld* world, int imageID, int startX, int startY, bool visible);
    //destructor
    virtual ~Goodies();
    virtual void doSomething();
private:
    virtual void pickedUp() = 0;
};

class OilBarrels: public Goodies{
public:
    //constructor
    OilBarrels(StudentWorld* world, int startX, int startY);
    //destructor
    virtual ~OilBarrels();
private:
    virtual void pickedUp();
};

class Gold: public Goodies{
public:
    //constructor
    Gold(StudentWorld* world, int startX, int startY, bool visible);
    //destructor
    virtual ~Gold();
    virtual void doSomething();
private:
    virtual void pickedUp();
    std::string m_state;
    int tempGoldTicks;
};

class Sonar: public Goodies{
public:
    //constructor
    Sonar(StudentWorld* world);
    //destructor
    virtual ~Sonar();
    virtual void doSomething();
private:
    virtual void pickedUp();
    int sonarTicks;
};

class WaterPool: public Goodies{
public:
    //constructor
    WaterPool(StudentWorld* world, int startX, int startY);
    //destructor
    virtual ~WaterPool();
    virtual void doSomething();
private:
    virtual void pickedUp();
    int waterPoolTicks;
};



// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

#endif // ACTOR_H_
