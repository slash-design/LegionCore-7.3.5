/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GuardAI.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "World.h"
#include "CreatureAIImpl.h"
#include "ScriptedCreature.h"

int GuardAI::Permissible(Creature const* creature)
{
    if (creature->isGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature* creature) : ScriptedAI(creature)
{
}

bool GuardAI::CanSeeAlways(WorldObject const* obj)
{
    if (!obj->IsUnit())
        return false;

    auto threatList = me->getThreatManager().getThreatList();
    for (std::list<HostileReference*>::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
        if ((*itr)->getUnitGuid() == obj->GetGUID())
            return true;

    return false;
}

void GuardAI::EnterEvadeMode()
{
    if (!me->isAlive())
    {
        me->GetMotionMaster()->MoveIdle();
        me->CombatStop(true);
        me->DeleteThreatList();
        me->ClearSaveThreatTarget();
        return;
    }

    TC_LOG_DEBUG("entities.unit", "Guard entry: %u enters evade mode.", me->GetEntry());

    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->ClearSaveThreatTarget();
    me->CombatStop(true);

    // Remove ChaseMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
        me->GetMotionMaster()->MoveTargetedHome();
}

void GuardAI::JustDied(Unit* killer)
{
    if (Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        me->SendZoneUnderAttackMessage(player);
}
