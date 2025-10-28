
#ifndef __BODYDAMAGETYPE_H_
#define __BODYDAMAGETYPE_H_

//-------------------------------------------------------------------------------------------------
/** Damage states for structures
 *
 * NOTE: the macros IS_CONDITION_WORSE and IS_CONDITION_BETTER depend on this enumeration being in
 * sequential order.
 */
//-------------------------------------------------------------------------------------------------
enum BodyDamageType
{
        BODY_PRISTINE,                          ///< unit should appear in pristine condition
        BODY_DAMAGED,                           ///< unit has been damaged
        BODY_REALLYDAMAGED,                     ///< unit is extremely damaged / nearly destroyed
        BODY_RUBBLE,                            ///< unit has been reduced to rubble/corpse/exploded-hulk, etc

        BODYDAMAGETYPE_COUNT,
};

#ifdef DEFINE_BODYDAMAGETYPE_NAMES
static const char* TheBodyDamageTypeNames[] =
{
        "PRISTINE",
        "DAMAGED",
        "REALLYDAMAGED",
        "RUBBLE",

        NULL
};
#endif

#endif // __BODYDAMAGETYPE_H_
