#include <SDL2/SDL.h>
#ifdef RPI
#else
#include <GL/gl.h>
#endif
#include "physics.h"
#include "debug.h"
#include "player.h"
#include "scene.h"
#include "entity.h"
#include "callbacks.h"
#include "engine.h"

static b2World *world = NULL;
static bool physics_Active = false;
static bool duringSolve = false;
static GLfloat *p=NULL;
static int psize=0;


class NullContactFilter: public b2ContactFilter {
public:
    bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {
        B2_NOT_USED(fixtureA);
        B2_NOT_USED(fixtureB);
        return true;
    }
};

class ContactListener: public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) {
        b2Fixture *f1, *f2;
        b2Body *playerBody = player_getPlayer()->getBody();
        f1 = contact->GetFixtureA();
        f2 = contact->GetFixtureB();
        bool playerFixture = false;
        if (f1->GetBody() == playerBody) {
            playerFixture = true;
        } else if (f2->GetBody() == playerBody) {
            f1 = contact->GetFixtureB();
            f2 = contact->GetFixtureA();
            playerFixture = true;
        }
        if (playerFixture) {
            Entity *e = (Entity*) f2->GetBody()->GetUserData();
            if (e && e->getId() >= 0) {
                callback_entity_onTouch(e);
            }
        }
    }
    void EndContact(b2Contact* contact) {
        b2Fixture *f1, *f2;
        b2Body *playerBody = player_getPlayer()->getBody();
        f1 = contact->GetFixtureA();
        f2 = contact->GetFixtureB();
        bool playerFixture = false;
        if (f1->GetBody() == playerBody) {
            playerFixture = true;
        } else if (f2->GetBody() == playerBody) {
            f1 = contact->GetFixtureB();
            f2 = contact->GetFixtureA();
            playerFixture = true;
        }
        if (playerFixture) {
            Entity *e = (Entity*) f2->GetBody()->GetUserData();
            if (e && e->getId() >= 0) {
                callback_entity_onLeave(e);
            }
        }
    }
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
        const b2Filter& filterA = contact->GetFixtureA()->GetFilterData();
        const b2Filter& filterB = contact->GetFixtureB()->GetFilterData();
        bool collide;
        if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0) {
            collide = filterA.groupIndex > 0;
        } else
            collide = (filterA.maskBits & filterB.categoryBits) != 0 && (filterA.categoryBits & filterB.maskBits) != 0;
        if (!collide) {
            contact->SetEnabled(false);
            return;
        }
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        b2PointState state1[2], state2[2];
        b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
        if (state2[0] == b2_addState) {
            const b2Body* bodyA = contact->GetFixtureA()->GetBody();
            const b2Body* bodyB = contact->GetFixtureB()->GetBody();
            b2Vec2 point = worldManifold.points[0];
            b2Vec2 vA = bodyA->GetLinearVelocityFromWorldPoint(point);
            b2Vec2 vB = bodyB->GetLinearVelocityFromWorldPoint(point);
            float32 approachVelocity = fabs(b2Dot(vB - vA, worldManifold.normal));
            if (approachVelocity > 1) {
                b2Body *playerBody = player_getPlayer()->getBody();
                if (bodyA == playerBody || bodyB == playerBody) {
                    void *ud;
                    Entity *e;
                    if (bodyA == playerBody) {
                        ud = bodyB->GetUserData();
                        if (ud) {
                            e = (Entity*) ud;
                            if (e->getId() < 0) {
                                //callback_player_onHitChain(approachVelocity);
                            } else{
                                callback_player_onHit(e, approachVelocity);
                            }
                        }
                    } else {
                        ud = bodyA->GetUserData();
                        if (ud) {
                            e = (Entity*) ud;
                            if (e->getId() < 0) {
                                //callback_player_onHitChain(approachVelocity);
                            } else{
                                callback_player_onHit(e, approachVelocity);
                            }
                        }
                    }
                } else {
                    void *ud1 = bodyA->GetUserData();
                    void *ud2 = bodyB->GetUserData();
                    if (ud1 && ud2) {
                        Entity *e1 = (Entity*) ud1;
                        Entity *e2 = (Entity*) ud2;
                        if (e1->getId() < 0 || e2->getId() < 0) {
                            //callback_entity_onHit(e1, e2, approachVelocity);
                            //entity -> rope collision unsupported
                        } else {
                            callback_entity_onHit(e1, e2, approachVelocity);
                            callback_entity_onHit(e2, e1, approachVelocity);
                        }
                    }
                }
            }
        }
    }
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
        B2_NOT_USED(contact);
        B2_NOT_USED(impulse);
    }
};

class DestructionListener: public b2DestructionListener {
    void SayGoodbye(b2Joint* joint) {
        int *udata = (int*) joint->GetUserData();
        if (!udata)
            return;
        int jointid = *udata;
        delete udata;
        joint->SetUserData(NULL);
        scene_getJointContainer().erase(jointid);
    }

    void SayGoodbye(b2Fixture *fixture) {
        B2_NOT_USED(fixture);
    }
};

static DestructionListener destructionListener;
static ContactListener contactListener;
static NullContactFilter contactFilter;

b2World *physics_getWorld() {
    return world;
}

void physics_startUp() {
    if (physics_Active)
        return;
    physics_cleanEnvironment();
    physics_Active = true;
    DTRACETIME("clean");
}

void physics_cleanUp() {
    if (!physics_Active)
        return;
    physics_Active = false;
    if (world)
        delete world;
    if (p)
        delete p;
    DTRACETIME("clean");
}

void physics_cleanEnvironment() {
    if (world) {
        player_getPlayer()->destroyPhysics();
        delete world;
    }
    if(p)
        delete p;
    psize=1000;
    p=new GLfloat[psize];
    world = new b2World(b2Vec2(0, -60));
    world->SetContactFilter(&contactFilter);
    world->SetContactListener(&contactListener);
    world->SetDestructionListener(&destructionListener);
    DTRACETIME("env is clean");
}

void physics_redrawBodies() {
    float colors[][3] = { { 1.0f, 0.5f, 1.0f }, { 0.5f, 1.0f, 1.0f }, { 0.5f, 0.5f, 1.0f } };

    b2Body *body = world->GetBodyList();
    while (body != 0) {

        glColor4f(colors[body->GetType()][0], colors[body->GetType()][1], colors[body->GetType()][2], 1);

        b2Fixture *fixture = body->GetFixtureList();
        if (fixture == 0) {
            p[0]=body->GetPosition().x;
            p[1]=body->GetPosition().y;
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, p);
            glDrawArrays(GL_POINTS,0,1);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
        while (fixture != 0) {
            b2Shape *shape = fixture->GetShape();
            if (shape->m_type == b2Shape::e_circle) {
                b2Vec2 center = body->GetPosition();
                float theta = 0.0f;
                for (int i = 0; i < 16; ++i) {
                    b2Vec2 v = center + shape->m_radius * b2Vec2(cosf(theta), sinf(theta));
                    p[2*i+0]=v.x;
                    p[2*i+1]=v.y;
                    theta += 2.0f * (float) M_PI / 16.0f;
                }
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, p);
                glDrawArrays(GL_TRIANGLE_FAN,0,16);
                glDisableClientState(GL_VERTEX_ARRAY);
                glColor4f(colors[body->GetType()][0] + 0.5f, colors[body->GetType()][1] + 0.5f, colors[body->GetType()][2] + 0.5f, 1);
                p[0]=center.x;
                p[1]=center.y;
                p[2]=center.x + shape->m_radius * body->GetTransform().q.c;
                p[3]=center.y + shape->m_radius * body->GetTransform().q.s;
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, p);
                glDrawArrays(GL_LINES,0,2);
                glDisableClientState(GL_VERTEX_ARRAY);
            } else if (shape->m_type == b2Shape::e_chain) {
                b2ChainShape *chain = (b2ChainShape*) fixture->GetShape();
                b2EdgeShape edge;
                if(psize<chain->GetChildCount()*4){
                    delete p;
                    psize=chain->GetChildCount()*4;
                    p=new GLfloat[psize];
                }
                b2Transform t = body->GetTransform();
                for (int i = 0; i < chain->GetChildCount(); i++) {
                    chain->GetChildEdge(&edge, i);
                    b2Vec2 vert = edge.m_vertex1;
                    p[4*i+0]=t.p.x + vert.x * t.q.c - vert.y * t.q.s;
                    p[4*i+1]=t.p.y + vert.y * t.q.c + vert.x * t.q.s;
                    vert = edge.m_vertex2;
                    p[4*i+2]=t.p.x + vert.x * t.q.c - vert.y * t.q.s;
                    p[4*i+3]=t.p.y + vert.y * t.q.c + vert.x * t.q.s;
                }
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, p);
                glDrawArrays(GL_LINE_LOOP,0,2*chain->GetChildCount());
                glDisableClientState(GL_VERTEX_ARRAY);
            } else if (shape->m_type == b2Shape::e_polygon) {
                b2PolygonShape *poly = (b2PolygonShape*) fixture->GetShape();
                if(psize<poly->GetVertexCount()*2){
                    delete p;
                    psize=poly->GetVertexCount()*2;
                    p=new GLfloat[psize];
                }
                b2Transform t = body->GetTransform();
                for (int i = 0; i < poly->GetVertexCount(); i++) {
                    b2Vec2 vert = poly->GetVertex(i);
                    p[2*i+0]=t.p.x + vert.x * t.q.c - vert.y * t.q.s;
                    p[2*i+1]=t.p.y + vert.y * t.q.c + vert.x * t.q.s;
                }
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, p);
                glDrawArrays(GL_TRIANGLE_FAN,0,poly->GetVertexCount());
                glDisableClientState(GL_VERTEX_ARRAY);
            }
            fixture = fixture->GetNext();
        }
        body = body->GetNext();
    }
    b2Joint *joint = world->GetJointList();
    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
    while (joint) {
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
        if (joint->GetType() == e_revoluteJoint) {
            b2Vec2 center = joint->GetAnchorA();
            float theta = 0.0f;
            for (int i = 0; i < 16; ++i) {
                b2Vec2 v = center + 0.25 * b2Vec2(cosf(theta), sinf(theta));
                p[2*i+0]=v.x;
                p[2*i+1]=v.y;
                theta += 2.0f * (float) M_PI / 16.0f;
            }
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, p);
            glDrawArrays(GL_TRIANGLE_FAN,0,16);
            glDisableClientState(GL_VERTEX_ARRAY);
        } else if (joint->GetType() == e_distanceJoint) {
            p[0]=joint->GetAnchorA().x;
            p[1]=joint->GetAnchorA().y;
            p[2]=joint->GetAnchorB().x;
            p[3]=joint->GetAnchorB().y;
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, p);
            glDrawArrays(GL_LINES,0,2);
            glDisableClientState(GL_VERTEX_ARRAY);
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            b2Vec2 center = joint->GetAnchorA();
            float theta = 0.0f;
            for (int i = 0; i < 16; ++i) {
                b2Vec2 v = center + 0.1 * b2Vec2(cosf(theta), sinf(theta));
                p[2*i+0]=v.x;
                p[2*i+1]=v.y;
                theta += 2.0f * (float) M_PI / 16.0f;
            }
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, p);
            glDrawArrays(GL_TRIANGLE_FAN,0,16);
            glDisableClientState(GL_VERTEX_ARRAY);
            center = joint->GetAnchorB();
            theta = 0.0f;
            for (int i = 0; i < 16; ++i) {
                b2Vec2 v = center + 0.1 * b2Vec2(cosf(theta), sinf(theta));
                p[2*i+0]=v.x;
                p[2*i+1]=v.y;
                theta += 2.0f * (float) M_PI / 16.0f;
            }
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, p);
            glDrawArrays(GL_TRIANGLE_FAN,0,16);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
        joint = joint->GetNext();
    }
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    b2Vec2 center = (1.0/15.0)*(b2Vec2(mx,ENGINE_SH-my)-b2Vec2(ENGINE_SW/2.0, ENGINE_SH/2.0))+engine_getScreenCenter();
    float theta = 0.0f;
    for (int i = 0; i < 8; ++i) {
        b2Vec2 v = center + 0.1 * b2Vec2(cosf(theta), sinf(theta));
        p[2*i+0]=v.x;
        p[2*i+1]=v.y;
        theta += 2.0f * (float) M_PI / 8.0f;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, p);
    glDrawArrays(GL_TRIANGLE_FAN,0,8);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void physics_step(float dt, int vi, int pi) {
    duringSolve = true;
    world->Step(dt, vi, pi);
    duringSolve = false;
}

bool physics_isDuringSolve() {
    return duringSolve;
}
