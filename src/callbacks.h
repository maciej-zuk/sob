#ifndef CALLBACKS_H_
#define CALLBACKS_H_
#include "entity.h"

/**called for entity1 when entity1 hits entity2
 *
 * callback should be definied like this:
 * @code
 * function entity1.onHit(entity1, entity2, velocty)
 *    (...)
 * end
 * @endcode
 *
 * @param entity1 - entity whose callback is called
 * @param entity2 - this entity will be callback argument
 * @param velocity - velocity of objects hit
 *
 */
void callback_entity_onHit(Entity *entity1, Entity *entity2, float velocity);

/**called for entity when player touches it
 *
 * callback should be definied like this:
 * @code
 * function entity1.onTouch()
 *    (...)
 * end
 * @endcode
 *
 */
void callback_entity_onTouch(Entity *entity);

/**called for entity when player stop touching it
 *
 * callback should be definied like this:
 * @code
 * function entity1.onLeave()
 *    (...)
 * end
 * @endcode
 *
 */
void callback_entity_onLeave(Entity *entity);

/**called for player when it hits entity with given velocity
 *
 * callback should be definied like this:
 * @code
 * function player.onHit(entity, velocty)
 *    (...)
 * end
 * @endcode
 */
void callback_player_onHit(Entity *entity, float velocity);


/**called when rope hits entity
 *
 * callback should be definied like this:
 * @code
 * function player.onRopeHit(entity)
 *    (...)
 * end
 * @endcode
 *
 */
void callback_player_onRopeHit(Entity *entity);

/**called when player jumped
 *
 */
void callback_player_onJump();


/**process deferred callback calls
 *
 * should be called when environment is safe (eg. after solving physics )
 */
void callback_processDeferredCalls();

/**onTick callback
 *
 * callback should be definied like this:
 * @code
 * function entity.onTick(entity)
 *    (...)
 * end
 * @endcode
 */
void callback_entity_onTick(Entity *entity);

/**playedSoundStop callback
 *  no real callback is called here, just playedSound is destroyed after finishing playing
 */
void callback_playedSound_finished(int channel);

#endif /* CALLBACKS_H_ */
