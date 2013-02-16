#ifndef PLAYER_H_
#define PLAYER_H_

#include <Box2D/Box2D.h>
#include "entity.h"
#define PLAYER_MAXCHAINLENGTH 15

/** called automaticaly @see engine.h
 *
 */
void player_startUp();

/** called automaticaly @see engine.h
 *
 */
void player_cleanUp();

/** recreate player
 *
 *  if engine_centerOnPlayer() was used, it should be called again after this function
 */
void player_cleanEnvironment();

class Player{
    b2Body* body;
    float alpha;
    float alphaChangeSpeed;
    bool isAlphaChanged;
    int frameNumber;
    int animationId;
    float health;


    b2Body *chainBody[1 + PLAYER_MAXCHAINLENGTH / 2];
    b2Joint *chainJoints[1 + PLAYER_MAXCHAINLENGTH / 2];
    int chainBodyLen;
    int chainJointsLen;
    bool ifChainShoot;
    bool ifCanJump;
    int jumpCooldown;


    b2FixtureDef plankFixture;
    b2PolygonShape plankShape;

    Entity *chainEntity;

public:

    Player();
    ~Player();
    /** recreates player physical body
     *
     *  if engine_centerOnPlayer() was used, it should be called again after this function
     */
    void resetPhysics();

    /** probably you want resetPhysics()
     *
     *  player should not be used after calling this
     */
    void destroyPhysics();

    /**
     * @todo redrawing output layer of player
     */
    void redrawOutput();

    /** get player body and do whatever you want with it
     *
     */
    b2Body * getBody();

    /** get chain entity
     *
     * used internally, not much to do with it
     */
    Entity * getChainEntity();

    /** player moves left
     *
     */
    void moveLeft(float speed=5.0f);

    /** player moves right
     *
     */
    void moveRight(float speed=5.0f);

    /** true if player shoot the rope
     *
     */

    bool ropeShoot();


    /** true if player may jump now
     *
     */

    bool canJump();

    /** player jumps
     *  @return false if jump failed (not on ground)
     */
    bool jump();

    /** must be called every frame to update some internal player state
     *
     */
    void update();

    /** shoot rope
     *
     */
    void shootChain(b2Vec2 direction=b2Vec2(0, PLAYER_MAXCHAINLENGTH));

    /** release rope
     *
     */
    void destroyChain();

    /** @code
     *  if rope_active:
     *     destroyChain()
     *  else:
     *     if(toMouse)
     *         shootChain(toMouseDirection)
     *     else
     *         shootChain(upward)
     *  @endcode
     */
    void chainTrigger(bool toMouse=false);

};

/**
 * get global player instance
 */
Player* player_getPlayer();

/** set spawn point for player
 *
 *  you should call Player::resetPhysics() after this to respawn player
 */
void player_setSpawnPoint(float x, float y);

#endif /* PLAYER_H_ */
