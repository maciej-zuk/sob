#ifndef PHYSICS_H_
#define PHYSICS_H_
#include <Box2D/Box2D.h>

#define physics_collideBgCategory 0x100
#define physics_collideFgCategory 0x200

/** frequently used structure
 *
 */
class physics_RayCastClosestCallback: public b2RayCastCallback {
public:
    physics_RayCastClosestCallback() {
        m_fixture = NULL;
        hit = false;
    }
    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) {
        m_fixture = fixture;
        m_point = point;
        m_normal = normal;
        m_fraction = fraction;
        hit = true;
        return fraction;
    }
    b2Fixture* m_fixture;
    b2Vec2 m_point;
    b2Vec2 m_normal;
    float32 m_fraction;
    bool hit;
};

/** get global world instance
 *
 */
b2World *physics_getWorld();

/** setup physics to work, called automatialy
 *
 * @see engine.h
 *
 */
void physics_startUp();

/** cleans up physics structures, called automaticaly
 *
 */
void physics_cleanUp();

/** destroys whole physical representation of scene and recreates new one
 *
 */
void physics_cleanEnvironment();

/** redraw physics layer
 *
 */
void physics_redrawBodies();

/** physics step
 *  @params dt, vi, pi - forwarded to world::Step
 */
void physics_step(float dt, int vi, int pi);

/** check if physics is solving now
 *
 *  don't add bodies when true
 */
bool physics_isDuringSolve();
#endif /* PHYSICS_H_ */
