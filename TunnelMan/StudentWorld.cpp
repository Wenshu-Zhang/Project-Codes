#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

class GraphObject;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

//constructor
StudentWorld::StudentWorld(std::string assetDir): GameWorld(assetDir){
    //create a player with nullptr
    m_player = nullptr;
    //make all squares nullptr
    for(int i = 0; i < VIEW_HEIGHT-4; i++){
        for(int k = 0; k < VIEW_WIDTH; k++)
            m_earth[k][i] = nullptr;
    }
}

//destructor
StudentWorld::~StudentWorld(){
    //remove the player
    delete m_player;
    //delete all earth
    for(int i = 0; i < VIEW_HEIGHT-4; i++){
        for(int k = 0; k < VIEW_WIDTH; k++)
            delete m_earth[k][i];
    }
    //delete all actors
    vector<Actor*>::iterator it = actors.begin();
    while(it != actors.end()){
        delete *it;
        it = actors.erase(it);
    }
}

int StudentWorld::init(){
    probabilityOfGoodie = (int)getLevel() * 25 + 300;
    goodieLast = max(100, 300-10*((int)getLevel()));
    m_nTicks = 0;
    m_nTicksToAddProtester = max(25, 200-(int)getLevel());
    m_nProtesters = 0;
    m_target_n_Protesters = min(15, 2+(int)((int)getLevel()*1.5));
    probabilityOfHardcore = min(90, (int)getLevel() * 10 + 30);
    
    //create a player
    m_player = new TunnelMan(this);
    
    //create earth
    for(int i = 0; i < VIEW_WIDTH; i++){
        for(int k = 0; k < VIEW_HEIGHT; k++){
            if(i >= 30 && i <= 33 && k >= 4  && k <= 59)
                m_earth[i][k] = nullptr;
            else if(k >59)
                m_earth[i][k] = nullptr;
            else
                m_earth[i][k] = new Earth(this, i, k);
        }
    }
    
    //create Boulders
    int B = min((int)getLevel() / 2 + 2, 9);
    for(int i = 0; i < B; i++){
        int x,y;
        do{
            pickALocation(x, y, 0, 60, 20, 56);
        }while(!validLocation(x, y));
        addActor(new Boulders(this, x, y));
    }
    
    //create oil barrels
    int L = min(2 + (int)getLevel(), 21);
    for(int i = 0; i < L; i++){
        int x,y;
        do{
            pickALocation(x, y, 0, 60, 0, 56);
        }while(!validLocation(x, y));
        addActor(new OilBarrels(this, x, y));
    }
    m_nBarrels = L;
    
    //create gold nuggets
    int G = max(5-(int)getLevel() / 2, 2);
    for(int i = 0; i < G; i++){
        int x,y;
        do{
            pickALocation(x, y, 0, 60, 0, 56);
        }while(!validLocation(x, y));
        addActor(new Gold(this, x, y, false));
    }
    //continue the game
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move(){
    //display the stats
    setDisplayText();
    
    //if time to add a protester and not enough proteseters, add a protester
    if(m_nTicks == 0 && m_nProtesters < m_target_n_Protesters){
        //determine which kind of protester to add with random integer
        int randInt = rand() % 100;
        if(randInt < probabilityOfHardcore)
            addActor(new HardcoreProtester(this));
        else
            addActor(new Protester(this, TID_PROTESTER, 5, 100));
        m_nProtesters++;
        m_nTicks = m_nTicksToAddProtester;
    }
    //if not time yet, decrease the wait ticks
    else if(m_nTicks > 0)
        m_nTicks--;
        
    
    //determine if a goodie needs to be added
    if(addGoodie()){
        if(addSonarOrPool() == 0)           //add a sonar
            addActor(new Sonar(this));
        else{                               //add a water pool
            int x,y;
            do{
            pickALocation(x, y, 0, VIEW_WIDTH-4, 0, VIEW_HEIGHT-4);
            }while(!isValidForPool(x, y));
            addActor(new WaterPool(this, x, y));
        }
    }
    //make the player do something
    m_player->doSomething();
    //if the player is dead
    if(!m_player->isAlive()){
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    //make all actors do something
    for(int i = 0; i < actors.size(); i++){
        if(actors[i]->isAlive()){
            actors[i]->doSomething();
            if(!m_player->isAlive()){
                return GWSTATUS_PLAYER_DIED;
            }
            //if the level is finished
            if(finishLevel()){
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    //if the player died
    if(!m_player->isAlive()){
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    //if the level is finished
    if(finishLevel()){
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    //remove dead actors
    for(vector<Actor*>::iterator it = actors.begin(); it != actors.end();){
        if (!(*it)->isAlive()){
            delete *it;
            it = actors.erase(it);
        }
        else
            it++;
    }
    //continue the game since the player is still alive
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp(){
    //remove the player
    delete m_player;
    //delete all earth
    for(int i = 0; i < VIEW_WIDTH; i++){
        for(int k = 0; k < VIEW_HEIGHT-4; k++)
            delete m_earth[i][k];
    }
    //delete all actors
    vector<Actor*>::iterator it = actors.begin();
    while(it != actors.end()){
        delete *it;
        it = actors.erase(it);
    }
}

bool StudentWorld::finishLevel(){
    if(m_nBarrels == 0)             //the level is finished if there's no more barrels to be picked up
        return true;
    else
        return false;
}

void StudentWorld::addActor(Actor* actor){
    actors.push_back(actor);        //store the new actor in the vector
}

void StudentWorld::setDisplayText(){
    //get the stats
    int level = getLevel();
    int lives = getLives();
    int health = m_player->getHealth();
    int squirts = m_player->getWater();
    int gold = m_player->getGold();
    int barrelsLeft = m_nBarrels;
    int sonar = m_player->getSonar();
    int score = getScore();
    
    //format the stats
    string s = formatDisplayText(level, lives, health, squirts, gold, barrelsLeft, sonar, score);
    //display the stats
    setGameStatText(s);
}

string StudentWorld::formatDisplayText(int level, int lives, int health, int squirts, int gold, int barrelsLeft, int sonar, int score){
    string scr = to_string(score);
    while(scr.size() != 6)
        scr = "0"+scr;
    return "Lvl: "+to_string(level)+"  Lives: "+to_string(lives)+"  Hlth: "+to_string(health*10)+"%"+"  Wtr: "+to_string(squirts)+"  Gld: "+to_string(gold)+"  Oil Left: "+to_string(barrelsLeft)+"  Sonar: "+to_string(sonar)+"  Scr: "+scr;
}

void StudentWorld::dropAGold(){
    if(m_player->getGold() > 0){        //if the player has gold
        int x = m_player->getX();       //display a gold at the location
        int y = m_player->getY();
        addActor(new Gold(this, x, y, true));
        m_player->loseGold();           //decrease the number of gold the player has
    }
    else
        return;
}

void StudentWorld::useSonar(){
    if(m_player->getSonar() > 0){       //if the player has sonar
        playSound(SOUND_SONAR);         //play the sound of using sonar
        //display all objects in the radius
        for(vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
            if(revealObject(*it, 12.0))
                (*it)->setVisible(true);
        m_player->loseSonar();          //decrease the number of sonars the player has
    }
    else
        return;
}


bool StudentWorld::noEarth(int x, int y){
    //check if the 4*4 square contains earth
    for(int i = x; i < x+4 && i <= VIEW_WIDTH; i++)
        for(int k = y; k < y+4 && k <= VIEW_HEIGHT-4; k++)
            if(m_earth[i][k] != nullptr)
                return false;
    return true;
}

bool StudentWorld::digEarth(int x, int y){
    bool removeEarth = false;
    //if where the player goes had earth, disEarth is true
    //delete the earth and set the pointer to nullptr
    for(int i = x; i < x+4 && i <= VIEW_WIDTH; i++){
        for(int k = y; k < y+4 && k <= VIEW_HEIGHT-4; k++){
            if(m_earth[i][k] != nullptr){
                delete m_earth[i][k];
                m_earth[i][k] = nullptr;
                removeEarth = true;
            }
        }
    }
    //return id any earth is removed
    return removeEarth;
}

bool StudentWorld::canMoveInDir(Actor* actor, GraphObject::Direction dir){
    GraphObject::Direction direction = dir;
    //determine if the next spot in the direction is viable
    switch(direction){
        case GraphObject::up:
            if(canMove(actor, actor->getX(), actor->getY()+1))
                return true;
            break;
        case GraphObject::down:
            if(canMove(actor, actor->getX(), actor->getY()-1))
                return true;
            break;
        case GraphObject::right:
            if(canMove(actor, actor->getX()+1, actor->getY()))
                return true;
            break;
        case GraphObject::left:
            if(canMove(actor, actor->getX()-1, actor->getY()))
                return true;
            break;
        case GraphObject::none:
            break;
    }
    return false;
}

void StudentWorld::decreaseBarrels(){       //decrease the number of barrels in the oil field
    m_nBarrels--;
}

void StudentWorld::increaseGold(){          //increase the number of gold the player has
    m_player->addGold();
}

void StudentWorld::decreaseGold(){          //decrease the number of gold in the oil field
    m_nGoldNuggets--;
}

void StudentWorld::increaseSonar(){         //increase the number of sonars the player has
    m_player->addSonar();
}

void StudentWorld::increaseWater(){         //increase the number of water squirts the player has
    m_player->addWater();
}

void StudentWorld::decreaseProtester(){     //decrease the number of protesters in the oil field
    m_nProtesters--;
}

bool StudentWorld::canMove(Actor* actor, int x, int y){
    //if the actor is not player and the spot has earth, it cannot move to the location
    if(!noEarth(x, y) && actor != m_player)
        return false;
    //all actors are blocked by the boulders
    for(vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
        if((x < 0 || x > VIEW_WIDTH - 4 || y < 0 || y > VIEW_HEIGHT - 4) || (*it != actor && (*it)->block() && distance(x, (*it)->getX(), y, (*it)->getY()) <= 3))
            return false;
    }
    return true;
}

bool StudentWorld::revealObject(Actor* actor, double radius){
    if(aroundPlayer(actor, radius))
        return true;
    else
        return false;
}

double StudentWorld::distance(int x1, int x2, int y1, int y2){
    //calculate the distance between actors
    int dx = x1-x2;
    int dy = y1-y2;
    return sqrt(dx*dx+dy*dy);
}

bool StudentWorld::aroundPlayer(Actor* actor, double radius){
    //check if the distance between the actors is less than the required radius
    double dist = distance(m_player->getX(), actor->getX(), m_player->getY(), actor->getY());
    return dist <= radius;
}

void StudentWorld::pickALocation(int &x, int &y, int xLower, int xUpper, int yLower, int yUpper){
    //pick a random location
    x = rand() % (xUpper - xLower + 1) + xLower;
    y = rand() % (yUpper - yLower + 1) + yLower;
}

bool StudentWorld::validLocation(int x, int y){
    //check if the the picked location is valid for the actors
    if(x >= 26 && x <= 34)
        return false;
    for(vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
        if(distance(x, (*it)->getX(), y, (*it)->getY()) <= 6)
            return false;
    }
    return true;
}

bool StudentWorld::isValidForPool(int x, int y){
    //check if the picked location is valid for water pools
    for(int i = x; i < x+4; i++)
        for(int k = y; k < y+4; k++)
            if(!noEarth(i,k))
                return false;
    for(vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
        if(distance(x, (*it)->getX(), y, (*it)->getY()) <= 6)
            return false;
    }
    return true;
}


bool StudentWorld::addGoodie(){
    //check if a goodie needs to be added
    int randInt = rand() % probabilityOfGoodie;
    if(randInt == 0)
        return true;
    else
        return false;
}

int StudentWorld::addSonarOrPool(){
    //determine which goodie needs to be added
    int randInt = rand() % 5;
    if(randInt == 0)
        return 0;
    else
        return 1;
}

int StudentWorld::getGoodieTime() const{
    return goodieLast;
}

bool StudentWorld::facingPlayer(Actor* actor){
    //check if the protester is facing the player
    GraphObject::Direction dir = actor->getDirection();
    //get the protester's direction and see if the player is in that direction
    switch(dir){
        case GraphObject::up:
            if(actor->getX() == m_player->getX() && actor->getY() < m_player->getY())
                return true;
            break;
        case GraphObject::down:
            if(actor->getX() == m_player->getX() && actor->getY() > m_player->getY())
                return true;
            break;
        case GraphObject::left:
            if(actor->getX() > m_player->getX() && actor->getY() == m_player->getY())
                return true;
            break;
        case GraphObject::right:
            if(actor->getX() < m_player->getX() && actor->getY() == m_player->getY())
                return true;
            break;
        case GraphObject::none:
            break;
    }
    //if the player is not in the direction that the protester is facing
    return false;
}

void StudentWorld::annoyPlayer(Actor* actor, int amount, double radius){
    //the player is annoyed by a protester
    //the player loses 2 hitpoints
    m_player->annoyed(2);
}

bool StudentWorld::annoyPlayerOrProtesters(Actor* actor, int amount, double radius){
    bool annoyed = false;
    //check if the player is annoyed
    if((distance(actor->getX(), m_player->getX(), actor->getY(), m_player->getY()) <= radius) && m_player->annoyed(amount))
            annoyed = true;
    //check if any protesters are annoyed
    for(vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
        if((distance((*it)->getX(), actor->getX(), (*it)->getY(), actor->getY()) <= radius) && (*it)->annoyed(amount))
            annoyed = true;
    }
    return annoyed;
}

Actor* StudentWorld::getPlayer(){
    return m_player;
}

bool StudentWorld::bribeProtester(Actor* actor, double radius){
    for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
        //determine if the protester is near the gold nugget
        bool withinDist;
        if(distance(actor->getX(), (*it)->getX(), actor->getY(), (*it)->getY()) <= radius)
            withinDist = true;
        else
            withinDist = false;
        //if the actor is a protester and it is near the gold
        //it is bribed
        if ((*it)->isProtester() && withinDist){
            Protester* protester = dynamic_cast<Protester*>(*it);
            //give the protester a gold
            protester->addGold();
            return true;
        }
    }
        return false;
}

void StudentWorld::markMaze(int maze[][VIEW_HEIGHT-3], int destinationX, int destinationY){
    //update locations with numbers
    for(int i = 0; i <= VIEW_WIDTH-4; i++){
        for(int k = 0; k <= VIEW_HEIGHT-4; k++){
            if(!noEarth(i, k) || !canMove(m_player, i, k))
                maze[i][k] = -1;                //cannot move to these locations
            else
                maze[i][k] = -2;                 //viable locations
        }
    }
    //start a breadth first search starting from the destination
    Location dest(destinationX ,destinationY);
    //push the starting point to the maze queue
    mazeQ.push(dest);
    //mark it with 0 because it is 0 steps away from the destination
    maze[destinationX][destinationY] = 0;
    //mark all positions in the maze and group them by the steps from the destination
    int stepsWalked = 1;
    int sameDepthCount = 1;
    while(!mazeQ.empty()){      //if the queue is empty, we are done
        int x = mazeQ.front().getx();
        int y = mazeQ.front().gety();
        mazeQ.pop();
        sameDepthCount--;
        //mark the locations that are visited
        if(maze[x+1][y] == -2 && x+1 <= VIEW_WIDTH-4){
            Location path(x+1, y);
            mazeQ.push(path);
            maze[x+1][y] = stepsWalked;
        }
        if(maze[x-1][y] == -2 && x-1 >= 0){
            Location path(x-1, y);
            mazeQ.push(path);
            maze[x-1][y] = stepsWalked;
        }
        if(maze[x][y+1] == -2 && y+1 <= VIEW_HEIGHT-4){
            Location path(x, y+1);
            mazeQ.push(path);
            maze[x][y+1] = stepsWalked;
        }
        if(maze[x][y-1] == -2 && y-1 >= 0){
            Location path(x, y-1);
            mazeQ.push(path);
            maze[x][y-1] = stepsWalked;
        }
        if(sameDepthCount == 0){            //if all positions with the same depth are visited
            //start marking the positions of the next depth
            sameDepthCount = mazeQ.size();
            stepsWalked++;
        }
    }
}

GraphObject::Direction StudentWorld::findExitDirection(int x, int y){
    //find the optimal direction to move for the leaving protesters
    markMaze(maze, 60, 60);
    return findDirForward(x, y);
}

GraphObject::Direction StudentWorld::findDirectionToPlayer(int x, int y){
    //find the optimal direction to move towards the player for the hardcore protesters
    markMaze(maze, m_player->getX(), m_player->getY());
    int currentStepsAway = maze[x][y];
    int maxStepsAway = 16 + (int)getLevel() * 2;
    if(currentStepsAway > maxStepsAway)
        return GraphObject::none;
    return findDirForward(x, y);
}

GraphObject::Direction StudentWorld::findDirForward(int x, int y){
    //find the optimal direction to move towards the destination
    int currentStepsAway = maze[x][y];
    //the best direction is the one with a smaller number
    if(x+1 <= VIEW_WIDTH-4){
        if(maze[x+1][y] < currentStepsAway && maze[x+1][y] >= 0)
            return GraphObject::right;
    }
    if(x-1 >= 0){
        if(maze[x-1][y] < currentStepsAway && maze[x-1][y] >= 0)
            return GraphObject::left;
    }
    if(y+1 <= VIEW_HEIGHT-4){
        if(maze[x][y+1] < currentStepsAway && maze[x][y+1] >= 0)
            return GraphObject::up;
    }
    if(y-1 >= 0){
        if(maze[x][y-1] < currentStepsAway && maze[x][y-1] >= 0)
            return GraphObject::down;
    }
    return GraphObject::none;
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp
