#ifndef ENTITY_H_
#define ENTITY_H_
#include "Box2D/Box2D.h"
#include <string>

using std::string;

class Entity{
    b2Body *body;
    string name;
    int id;
    int bodyId;

public:
    Entity(b2Body *body, const string& name, int id, int bodyId);
    ~Entity();
    int getId();
    string &getName();
    b2Body *getBody();
};

#endif /* ENTITY_H_ */
