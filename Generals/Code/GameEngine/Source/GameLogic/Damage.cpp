/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//                                                                                                                             /
/
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
/
//                                                                                                                             /
/
////////////////////////////////////////////////////////////////////////////////

// FILE: Damage.cpp ////////////////////////////////////////////////////////////////////////////
// Desc:   Shared damage/death name tables
/////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"     // This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Damage.h"

const char *const TheDamageNames[DAMAGE_NUM_TYPES + 1] =
{
        "EXPLOSION",
        "CRUSH",
        "ARMOR_PIERCING",
        "SMALL_ARMS",
        "GATTLING",
        "RADIATION",
        "FLAME",
        "LASER",
        "SNIPER",
        "POISON",
        "HEALING",
        "UNRESISTABLE",
        "WATER",
        "DEPLOY",
        "SURRENDER",
        "HACK",
        "KILL_PILOT",
        "PENALTY",
        "FALLING",
        "MELEE",
        "DISARM",
        "HAZARD_CLEANUP",
        "PARTICLE_BEAM",
        "TOPPLING",
        "INFANTRY_MISSILE",
        "AURORA_BOMB",
        "LAND_MINE",
        "JET_MISSILES",
        "STEALTHJET_MISSILES",
        "MOLOTOV_COCKTAIL",
        "COMANCHE_VULCAN",
        "FLESHY_SNIPER",

        NULL
};

const char *const TheDeathNames[DEATH_NUM_TYPES + 1] =
{
        "NORMAL",
        "NONE",
        "CRUSHED",
        "BURNED",
        "EXPLODED",
        "POISONED",
        "TOPPLED",
        "FLOODED",
        "SUICIDED",
        "LASERED",
        "DETONATED",
        "SPLATTED",
        "POISONED_BETA",

        "EXTRA_2",
        "EXTRA_3",
        "EXTRA_4",
        "EXTRA_5",
        "EXTRA_6",
        "EXTRA_7",
        "EXTRA_8",

        NULL
};

static_assert((sizeof(TheDamageNames) / sizeof(*TheDamageNames)) == DAMAGE_NUM_TYPES + 1,
        "TheDamageNames must match DAMAGE_NUM_TYPES plus terminator");
static_assert((sizeof(TheDeathNames) / sizeof(*TheDeathNames)) == DEATH_NUM_TYPES + 1,
        "TheDeathNames must match DEATH_NUM_TYPES plus terminator");
