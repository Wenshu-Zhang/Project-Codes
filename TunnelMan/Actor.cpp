#include "Actor.h"
#include "StudentWorld.h"
#include <algorithm>
#include <string>
using namespace std;

//Actors
//constructor
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, bool visible):GraphObject(imageID, startX, startY, dir, size, depth){
    m_world = world;
    m_live = true;
    setVisible(visible);
}

//destructor
Actor::~Actor(){};

bool Actor::isAlive() const{
    return m_live;
}

void Actor::dead(){
    m_live = false;
}

bool Actor::isTunnelMan(){
    return false;
}

bool Actor::isProtester(){
    return false;
}

bool Actor::annoyed(int amount){
    return false;
}

bool Actor::block(){
    return false;
}

//Earth
//constructor
Earth::Earth(StudentWorld* world, int startX, int startY):Actor(world, TID_EARTH, startX, startY, right, 0.25, 3, true){}

//destructor
Earth::~Earth(){}

//Earth doesn't do anything
void Earth::doSomething(){}

//MovingActors: TunnelMan and Protesters
//constructor
MovingActor::MovingActor(StudentWorld* world, int imageID, int startX, int startY, Direction dir, int health):Actor(world, imageID, startX, startY, dir, 1.0, 0, true){
    m_health = health;
}

//destructor
MovingActor::~MovingActor(){}

int MovingActor::getHealth(){
    return m_health;
}


bool MovingActor::annoyed(int amount){
    if(!isAlive())                  //can't be annoyed if the actor is not alive
        return false;
    m_health -= amount;             //lose health if annoyed
    if(m_health <= 0)               //if health is 0 or below
        giveUp();                   //the actor gives up
    return true;
}

//TunnelMan
//constructor
TunnelMan::TunnelMan(StudentWorld* world):MovingActor(world, TID_PLAYER, 30, 60, right, 10){
    m_nGold = 0;
    m_nSonar = 1;
    m_nWater = 5;
}

//destructor
TunnelMan::~TunnelMan(){}

void TunnelMan::doSomething(){
    if(!isAlive())                  //can't do anything if the player is dead
        return;
    int key;
    if(getWorld()->getKey(key)){    //get the last key fromt he player
        switch(key){
            case KEY_PRESS_LEFT:    //if left if pressed
                if(getDirection() != left)
                    setDirection(left);     //set the direction to left
                else if(getWorld()->canMove(this, getX()-1, getY())){   //check if it is ok to move
                    setDirection(left);
                    moveTo(getX()-1, getY());       //move one spot to the left
                }
                break;
            case KEY_PRESS_RIGHT:   //if right if pressed
                if(getDirection() != right)
                    setDirection(right);    //set the direction to right
                else if(getWorld()->canMove(this, getX()+1, getY())){   //check if it is ok to move
                    setDirection(right);
                    moveTo(getX()+1, getY());       //move one spot to the right
                }
                break;
            case KEY_PRESS_DOWN:    //if down if pressed
                if(getDirection() != down)
                    setDirection(down);     //set the direction to down
                else if(getWorld()->canMove(this, getX(), getY()-1)){   //check if it is ok to move
                    setDirection(down);
                    moveTo(getX(), getY()-1);       //move to one spot below
                }
                break;
            case KEY_PRESS_UP:      //if up if pressed
                if(getDirection() != up)
                    setDirection(up);       //set the direction to up
                else if(getWorld()->canMove(this, getX(), getY()+1)){   //check if it is ok to move
                    setDirection(up);
                    moveTo(getX(), getY()+1);       //move to one spot up
                }
                break;
            case KEY_PRESS_ESCAPE:  //if escape if pressed
                dead();             //set the player dead
                break;
            case KEY_PRESS_SPACE:   //if space if pressed
                shootWater();       //use water squirt
                break;
            case KEY_PRESS_TAB:     //if tab if pressed
                getWorld()->dropAGold();    //drop a gold at the location
                break;
            case 'z':               //if z or Z is pressed
            case 'Z':
                getWorld()->useSonar();     //use sonar to find nearby objects
                break;
        }
    }
    if(getWorld()->digEarth(getX(), getY()) == true)    //if the player is digging earth
        getWorld()->playSound(SOUND_DIG);               //play the digging sound
}

bool TunnelMan::isTunnelMan(){
    return true;
}

void TunnelMan::addGold(){              //increase a gold
    m_nGold++;
}

void TunnelMan::loseGold(){             //decrease a gold
    m_nGold--;
}

void TunnelMan::addSonar(){             //add a sonar
    m_nSonar++;
}

void TunnelMan::loseSonar(){            //decrease a sonar
    m_nSonar--;
}

void TunnelMan::addWater(){             //add 5 squirts
    m_nWater += 5;
}

int TunnelMan::getGold(){               //return the number of gold nuggets
    return m_nGold;
}

int TunnelMan::getSonar(){              //return the number of sonars
    return m_nSonar;
}

int TunnelMan::getWater(){              //return the number of squirts
    return m_nWater;
}

void TunnelMan::shootWater(){
    if(getWater() > 0){             //if no more water is left
        Direction dir = getDirection();     //get the player's direction
        switch(dir){                //shoot in the direction that the player is facing
            case up:
                getWorld()->addActor(new Water(getWorld(), getX(), getY()+4, up));
                break;
            case down:
                getWorld()->addActor(new Water(getWorld(), getX(), getY()-4, down));
                break;
            case left:
                getWorld()->addActor(new Water(getWorld(), getX()-4, getY(), left));
                break;
            case right:
                getWorld()->addActor(new Water(getWorld(), getX()+4, getY(), right));
                break;
            case none:
                break;
        }
        //play the sound
        getWorld()->playSound(SOUND_PLAYER_SQUIRT);
        //decrease the number of squirts
        m_nWater--;
    }
    else
        return;             //otherwise do nothing
}

void TunnelMan::giveUp(){
    dead();                 //if the player gives up, set it to dead
    //and play the sound of player giving up
    getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
}

//Protester
//constructor
Protester::Protester(StudentWorld* world, int imageId, int health, int score):MovingActor(world, imageId, 60, 60, left, health){
    m_health = health;
    m_score = score;
    ticksToWaitBetweenMoves = max(0, 3 - (int)getWorld()->getLevel()/4);
    m_restTicks = 0;
    m_stepsInDir = numSquaresToMoveInCurrentDirection();
    m_shoutTicks = 0;
    m_stunTicks = max(50, 100 - (int)getWorld()->getLevel() * 10);
    m_perpTicks = 0;
    m_leaving = false;
}

//destructor
Protester::~Protester(){}

int Protester::numSquaresToMoveInCurrentDirection(){
    //determine how many squares to move in the current direction between 8 and 60
    int nSteps = rand() % (60-8+1) + 8;
    return nSteps;
}

void Protester::doSomething(){
    if(!isAlive())          //if the protester is dead, it can't do anything
        return;
    
    if(m_leaving == true){  //if the protester is in leaving state
        int x = getX();
        int y = getY();
        if(x == 60 && y == 60){     //if the protester gets to the destination
            getWorld()->decreaseProtester();    //decrease the number of protesters
            dead();                 //set it dead
        }
        else{//otherwise
             //get the optimal direction to exit
            Direction exit = getWorld()->findExitDirection(getX(), getY());
            //set the direction to the one determined
            setDirection(exit);
            switch(exit){   //move one step accordingly
                case up:
                    moveTo(getX(), getY()+1);
                    break;
                case down:
                    moveTo(getX(), getY()-1);
                    break;
                case right:
                    moveTo(getX()+1, getY());
                    break;
                case left:
                    moveTo(getX()-1, getY());
                    break;
                case none:
                    break;
            }
        }
        return;         //immediately return
    }
    
    if(m_restTicks > 0){    //if the protester is in resting state
        m_restTicks--;      //decrease the ticks for resting state
        return;             //immediately return
    }
    
    if(m_perpTicks < 200)   //if the protester had moved perpendicularly in the last 200 ticks
        m_perpTicks++;      //increase the tick
    
    //if it's the tick for protester to move, reset restTicks
    resetRestTicks(ticksToWaitBetweenMoves);
    
    //if the protester is near and facing the player
    //and has not shouted in the last 15 non-resting ticks
    if (getWorld()->aroundPlayer(this, 4.0) && getWorld()->facingPlayer(this) && m_shoutTicks == 0){
        m_shoutTicks = 15;                              //reset shoutTicks
        getWorld()->playSound(SOUND_PROTESTER_YELL);    //play the sound of protester shouting
        getWorld()->annoyPlayer(this, 2, 4.0);          //annoy the player
        return;
    }
    if(m_shoutTicks > 0){           //if protester had shouted in the last 15 non-resting ticks
        m_shoutTicks--;             //decrease shoutTicks and wait
        return;
    }
    //if the protester is not near the player
    //but is hardcore
    if(!getWorld()->aroundPlayer(this, 4.0) && isHardcore()){
        //check if it is within a certain steps to the player without blocks
        Direction toPlayer = getWorld()->findDirectionToPlayer(getX(), getY());
        //if the returned direction is not none
        //the hardcore protester senses the cell phone signal from the player
        if(toPlayer != none){
            //make the hardcore protester chase the player
            if(toPlayer == down){
                setDirection(down);
                moveTo(getX(), getY()-1);
            }
            if(toPlayer == up){
                    setDirection(up);
                    moveTo(getX(), getY()+1);
            }
            if(toPlayer == left){
                setDirection(left);
                moveTo(getX()-1, getY());
            }
            if(toPlayer == right){
                setDirection(right);
                moveTo(getX()+1, getY());
            }
            //reset the number of steps in the current direction
            m_stepsInDir = 0;
            return;
        }
    }
    
    //Otherwise, check if the player is in the line pf sight
    Direction canSeePlayer = playerInSight();
    //if the returned direction is not none
    if(canSeePlayer != none){
        //make the protester chase the player
        if(canSeePlayer == down){
            setDirection(down);
            moveTo(getX(), getY()-1);
        }
        if(canSeePlayer == up){
                setDirection(up);
                moveTo(getX(), getY()+1);
        }
        if(canSeePlayer == left){
            setDirection(left);
            moveTo(getX()-1, getY());
        }
        if(canSeePlayer == right){
            setDirection(right);
            moveTo(getX()+1, getY());
        }
        //reset the number of steps in the current direction
        m_stepsInDir = 0;
        return;
    }
    
    
    Direction randPerp = chooseRandPerp();
    //If the number of steps in the current direction is 0 or below 0
    if(m_stepsInDir <= 0){
        Direction randDir = none;
        //get a random viable direction to move
        while(!getWorld()->canMoveInDir(this, randDir))
            randDir = chooseRandDir();
        setDirection(randDir);
        m_stepsInDir = numSquaresToMoveInCurrentDirection();
    }
    //otherwise, if the player has not moved in perpendicular directions in the last 200 ticks
    //find a viable perpendicular direction and move in that direction
    else if(randPerp != none && m_perpTicks >= 200){
        m_perpTicks = 0;
        setDirection(randPerp);
        //determine the number of steps to move in the new direction
        m_stepsInDir = numSquaresToMoveInCurrentDirection();
    }
    
    //if the protester needs to continue moving in the current direction
    if(m_stepsInDir > 0){
        //move in the current direction
        //if the path is not viable, reset steps to move in the direction to 0
        //and change to a new direction in the next non-resting tick
        if(getDirection() == up ){
            if(getWorld()->canMove(this, getX(), getY()+1)){
                moveTo(getX(), getY()+1);
                m_stepsInDir--;
            }
            else{
                m_stepsInDir = 0;
                return;
            }
        }
        if(getDirection() == down){
            if(getWorld()->canMove(this, getX(), getY()-1)){
                moveTo(getX(), getY()-1);
                m_stepsInDir--;
            }
            else{
                m_stepsInDir = 0;
                return;
            }
        }
        if(getDirection() == right){
            if(getWorld()->canMove(this, getX()+1, getY())){
                moveTo(getX()+1, getY());
                m_stepsInDir--;
                m_perpTicks++;
            }
            else{
                m_stepsInDir = 0;
                return;
           }
        }
        if(getDirection() == left){
            if(getWorld()->canMove(this, getX()-1, getY())){
                moveTo(getX()-1, getY());
                m_stepsInDir--;
                m_perpTicks++;
            }
            else{
                m_stepsInDir = 0;
                return;
            }
        }
    }
}

//choose a random new direction with random integers
GraphObject::Direction Protester::chooseRandDir(){
    int randInt = rand() % 4;
    switch(randInt){
        case 0:
            return up;
            break;
        case 1:
            return down;
            break;
        case 2:
            return left;
            break;
        case 3:
            return right;
            break;
    }
    return none;
}

//choose a viable or random perpendicular direction with random integers
GraphObject::Direction Protester::chooseRandPerp(){
    if(getWorld()->canMoveInDir(this, up) && getWorld()->canMoveInDir(this, down)){
        int randInt = rand() % 2;
        switch(randInt){
            case 0:
                return up;
                break;
            case 1:
                return down;
                break;
        }
    }
    if(getWorld()->canMoveInDir(this, up))
        return up;
    if(getWorld()->canMoveInDir(this, down))
        return down;
    return none;
}

bool Protester::isProtester(){
    return true;
}

void Protester::giveUp(){
    m_leaving = true;       //if the protester gives up, it is in the leaving state
    m_restTicks = 0;        //don't need to rest anymore
    //play the sound of the protester giving up
    getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
}

bool Protester::annoyed(int amount){
    if(m_leaving == true)   //if the protester is leaving, it can't be annoyed
        return false;
    MovingActor::annoyed(amount);   //otherwise, be annoyed as a moving actor
    //play the sound of the protester being annoyed
    getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
    if(getHealth() > 0){            //if is still alive
        m_restTicks = m_stunTicks;  //be stunned
    }
    else{                           //otherwise, it gives up and increases the player's score
        if(amount == 100){          //annoyed by a boulder
            getWorld()->increaseScore(500);
        }
        if(amount == 2){            //annoyed by the player
            getWorld()->increaseScore(m_score);
        }
    }
    return true;
}

void Protester::addGold(){
    //play the sound of protester getting a gold
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    //increase the player's score for successfully bribing the protester
    getWorld()->increaseScore(25);
    m_leaving = true;       //it immediately leaves the oil field
}

void Protester::resetRestTicks(int ticks){
    m_restTicks = ticks;
}

//check if the player is in the line of sight of the protester
GraphObject::Direction Protester::playerInSight(){
    //get the protester's location
    int x = getX();
    int y = getY();
    //if they have the same y
    if(y == getWorld()->getPlayer()->getY()){
        //check if the player is on its right
        for(int i = x; i <= VIEW_WIDTH-4; i++){
            if(!getWorld()->canMove(this, i, y))
                break;
            if(i == getWorld()->getPlayer()->getX())
                return GraphObject::right;
        }
        //check if the player is on its left
        for(int i = x; i >= 0; i--){
            if(!getWorld()->canMove(this, i, y))
                break;
            if(i == getWorld()->getPlayer()->getX())
                return GraphObject::left;
        }
    }
    //if they have the same x
    if(x == getWorld()->getPlayer()->getX()){
        //check if the player is above
        for(int i = y; i <= VIEW_HEIGHT-4; i++){
            if(!getWorld()->canMove(this, x, i))
                break;
            if(i == getWorld()->getPlayer()->getY())
                return GraphObject::up;
        }
        //check if the player is below
        for(int i = y; i >= 0; i--){
            if(!getWorld()->canMove(this, x, i))
                break;
            if(i == getWorld()->getPlayer()->getY())
                return GraphObject::down;
        }
    }
    //being blocked, so return none
    return GraphObject::none;
}

bool Protester::isHardcore(){
    return false;
}

//HardcoreProtester
//constructor
HardcoreProtester::HardcoreProtester(StudentWorld* world):Protester(world, TID_HARD_CORE_PROTESTER, 20, 250){}

//destructor
HardcoreProtester::~HardcoreProtester(){}

void HardcoreProtester::addGold(){
    //play the sound of protester getting a gold
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    //increase the player's score for successfully bribing the protester
    getWorld()->increaseScore(50);
    //make the hardcore protester stare at the gold and be stunned
    int ticks_to_stare = max(50, 100-(int)getWorld()->getLevel() * 10);
    resetRestTicks(ticks_to_stare);
}

bool HardcoreProtester::isHardcore(){
    return true;
}

//Water
//constructor
Water::Water(StudentWorld* world, int startX, int startY, Direction dir):Actor(world, TID_WATER_SPURT, startX, startY, dir, 1.0, 1, true){
    distance = 0;
    direction = dir;
}

//destructor
Water::~Water(){}

void Water::doSomething(){
    if(distance == 4)       //if it has traveled 4 locations
        dead();             //set it to dead
    if(direction == up){
        //if blocked by earth or annoys protesters
        //set it to dead
        if(!getWorld()->noEarth(getX(), getY()+1) || getWorld()->annoyPlayerOrProtesters(this, 2, 3.0)){
           dead();
            return;
        }
        else
           moveTo(getX(), getY()+1);    //otherwise, move one step in the direction
    }
    else if(direction == down){
        //if blocked by earth or annoys protesters
        //set it to dead
        if(!getWorld()->noEarth(getX(), getY()+1) || getWorld()->annoyPlayerOrProtesters(this, 2, 3.0)){
           dead();
            return;
        }
        else
            moveTo(getX(), getY()-1);    //otherwise, move one step in the direction
    }
    else if(direction == left){
        //if blocked by earth or annoys protesters
        //set it to dead
        if(!getWorld()->noEarth(getX()-1, getY()) || getWorld()->annoyPlayerOrProtesters(this, 2, 3.0)){
           dead();
            return;
        }
        else
            moveTo(getX()-1, getY());    //otherwise, move one step in the direction
    }
    else if(direction == right){
        //if blocked by earth or annoys protesters
        //set it to dead
        if(!getWorld()->noEarth(getX()+1, getY()) || getWorld()->annoyPlayerOrProtesters(this, 2, 3.0)){
           dead();
            return;
        }
        else
            moveTo(getX()+1, getY());    //otherwise, move one step in the direction
    }
    //increase distance
    distance++;
}

//Boulders
//constructor
Boulders::Boulders(StudentWorld* world, int startX, int startY):Actor(world, TID_BOULDER, startX, startY, down, 1.0, 1, true){
    getWorld()->digEarth(startX, startY);
    m_ticksFalling = 0;
}

//destructor
Boulders::~Boulders(){}

bool Boulders::isStable(int x, int y){
    //if there's earth below, boulder is stable
    if(!getWorld()->noEarth(x, y-4))
        return true;
    //else, it's not stable
    return false;
}

void Boulders::doSomething(){
    //if it's falling
    if(m_ticksFalling >= 30){
        m_ticksFalling++;
        //move one step down if it's not blocked
        if(getWorld()->canMove(this, getX(),getY()-1)){
            moveTo(getX(), getY()-1);
            getWorld()->playSound(SOUND_FALLING_ROCK);
            getWorld()->annoyPlayerOrProtesters(this, 100, 3.0);
        }
        else
            dead();     //otherwise, set it to dead
    }
    //if it's not time to fall
    //increase ticksFalling
    else if(!isStable(getX(), getY())){
        m_ticksFalling++;
    }
}

//boulder blocks other actors
bool Boulders::block(){
    return true;
}

//Goodies
//constructor
Goodies::Goodies(StudentWorld* world, int imageID, int startX, int startY, bool visible):Actor(world, imageID, startX, startY, right, 1.0, 2, visible){}

//destructor
Goodies::~Goodies(){};

void Goodies::doSomething(){
    if(!isAlive())          //if the goodie is dead, do nothing
        return;
    //if the distance between this and the player is less than or equal to 4
    //reveal the goodie
    if(getWorld()->revealObject(this, 4.0))
        setVisible(true);
    //if the distance between this and the player is less than or equal to 3
    //the goodie is picked up
    if(getWorld()->aroundPlayer(this, 3.0)){
        pickedUp();
        dead();
    }
}

//OilBarrels
//constructor
OilBarrels::OilBarrels(StudentWorld* world, int startX, int startY):Goodies(world, TID_BARREL, startX, startY, false){}

//destructor
OilBarrels::~OilBarrels(){}

void OilBarrels::pickedUp(){
    //play the sound of oil barrels being picked up
    getWorld()->playSound(SOUND_FOUND_OIL);
    //increase the player's score
    getWorld()->increaseScore(1000);
    //decrease the number of barrels in the oil field
    getWorld()->decreaseBarrels();
}

//Gold
//constructor
Gold::Gold(StudentWorld* world, int startX, int startY, bool visible):Goodies(world, TID_GOLD, startX, startY, visible){
    if(visible == false)
        m_state = "permanent";
    else
        m_state = "temporary";
    tempGoldTicks = 0;
}

//destructor
Gold::~Gold(){}

void Gold::doSomething(){
    if(!isAlive())          //if the gold is dead, do nothing
        return;
    //if the gold nugget's state is permanent
    if(m_state == "permanent"){
        //if the distance between the gold and the player is less than or equal to 4
        //reveal the gold
        if(getWorld()->revealObject(this, 4.0))
            setVisible(true);
        //if the distance between the gold and the player is less than or equal to 3
        //the gold is picked up
        if(getWorld()->aroundPlayer(this, 3.0)){
            pickedUp();
            dead();
        }
    }
    //if the gold nugget's state is temporary
    else{
        //if the distance between the gold and the protester is less than or equal to 3
        //the gold is picked up, and the protester is bribed
        if(getWorld()->bribeProtester(this, 3.0))
            dead();
        //if not picked up after 100 ticks, it disappears
        if(tempGoldTicks == 100)
            dead();
        else
            tempGoldTicks++;    //if not dead yet, increase tick number
    }
}

void Gold::pickedUp(){
    //play the sound of a gold being picked up
    getWorld()->playSound(SOUND_GOT_GOODIE);
    //increase the player's score
    getWorld()->increaseScore(10);
    //give the player a piece of gold
    getWorld()->increaseGold();
    //decrease the number of gold in the oil field
    getWorld()->decreaseGold();
}

//Sonar
//constructor
Sonar::Sonar(StudentWorld* world):Goodies(world, TID_SONAR, 0, 60, true){
    sonarTicks = 0;
}

//destructor
Sonar::~Sonar(){}

void Sonar::doSomething(){
    if(!isAlive())              //if the sonar is dead, do nothing
        return;
    //the time limit is reached, set it to dead
    if(sonarTicks == getWorld()->getGoodieTime())
        dead();
    //if near player, it is picked up and dead
    if(getWorld()->aroundPlayer(this, 3.0)){
        pickedUp();
        dead();
    }
    else                        //otherwise, increase sonarTicks
        sonarTicks++;
}

void Sonar::pickedUp(){
    //play the sound
    getWorld()->playSound(SOUND_GOT_GOODIE);
    //increase the player's score
    getWorld()->increaseScore(75);
    //give the player a sonar
    getWorld()->increaseSonar();
}

//WaterPool
//constructor
WaterPool::WaterPool(StudentWorld* world, int startX, int startY):Goodies(world, TID_WATER_POOL, startX, startY, true){
    waterPoolTicks = 0;
}

//destructor
WaterPool::~WaterPool(){}

void WaterPool::doSomething(){
    if(!isAlive())              //if the water pool is dead, do nothing
        return;
    //the time limit is reached, set it to dead
    if(waterPoolTicks == getWorld()->getGoodieTime())
        dead();
    //if near player, it is picked up and dead
    if(getWorld()->aroundPlayer(this, 3.0)){
        pickedUp();
        dead();
    }
    else                        //otherwise, increase waterPoolTicks
        waterPoolTicks++;
}

void WaterPool::pickedUp(){
    //play the sound
    getWorld()->playSound(SOUND_GOT_GOODIE);
    //increase the player's score
    getWorld()->increaseScore(100);
    //give the player wate squirts
    getWorld()->increaseWater();
}
