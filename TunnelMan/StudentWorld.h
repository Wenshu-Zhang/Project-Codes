#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <vector>
#include <queue>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class TunnelMan;
class Earth;

class StudentWorld : public GameWorld
{
public:
    //constructor
    StudentWorld(std::string assetDir);
    //destructor
    virtual ~StudentWorld();
    //three methods
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    //accessors
    int getX() const{
        return m_x;
    }
    int getY() const{
        return m_y;
    }
    void setDisplayText();
    
    //Mutators
    void addActor(Actor* actor);
    bool digEarth(int x, int y);
    void decreaseBarrels();
    void increaseGold();
    void decreaseGold();
    void increaseSonar();
    void increaseWater();
    void dropAGold();
    void useSonar();
    void annoyPlayer(Actor* actor, int amount, double radius);
    bool annoyPlayerOrProtesters(Actor* actor, int amount, double radius);
    void decreaseProtester();
    Actor* getPlayer();
    bool revealObject(Actor* actor, double radius);
    bool canMove(Actor* actor, int x, int y);
    bool canMoveInDir(Actor* actor, GraphObject::Direction dir);
    
    bool finishLevel();
    bool aroundPlayer(Actor* actor, double radius);
    bool bribeProtester(Actor* actor, double radius);
    bool noEarth(int x, int y);
    bool addGoodie();
    bool isValidForPool(int x, int y);
    int addSonarOrPool();
    int getGoodieTime() const;
    double distance(int x1, int x2, int y1, int y2);
    bool facingPlayer(Actor* actor);
    void markMaze(int maze[][VIEW_HEIGHT-3], int sx, int sy);
    GraphObject::Direction findDirForward(int x, int y);
    GraphObject::Direction findExitDirection(int x, int y);
    GraphObject::Direction findDirectionToPlayer(int x, int y);
private:
    struct Location{
    public:
        Location(int x, int y):m_x(x),m_y(y){}
        int getx() const {return m_x;}
        int gety() const {return m_y;}
    private:
        int m_x;
        int m_y;
    };
    TunnelMan* m_player;
    Earth* m_earth[VIEW_WIDTH][VIEW_HEIGHT];
    std::vector<Actor*> actors;
    int maze[VIEW_WIDTH-3][VIEW_HEIGHT-3];
    std::queue<Location> mazeQ;
    int probabilityOfGoodie;
    int probabilityOfHardcore;
    int m_nTicks;
    int m_nTicksToAddProtester;
    int m_x;
    int m_y;
    int m_nBarrels;
    int m_nGoldNuggets;
    int goodieLast;
    int m_target_n_Protesters;
    int m_nProtesters;
    
    void pickALocation(int &x, int &y, int xLower, int xUpper, int yLower, int yUpper);
    bool validLocation(int x, int y);
    std::string formatDisplayText(int level, int lives, int health, int squirts, int gold, int barrelsLeft, int sonar, int score);
};

#endif // STUDENTWORLD_H_
