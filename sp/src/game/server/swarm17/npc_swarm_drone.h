//=============================================================================
//
// Purpose: Swarm NPC drone
//
//=============================================================================

#ifndef NPC_SWARM_DRONE_H
#define NPC_SWARM_DRONE_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_swarm_base.h"

class CNPC_SwarmDrone : public CAI_BaseSwarmNPC
{
public:
	DECLARE_CLASS( CNPC_SwarmDrone, CAI_BaseSwarmNPC );
	DECLARE_DATADESC();

	CNPC_SwarmDrone();

	const char *GetDefaultModel() { return "models/aliens/drone/drone.mdl"; }

	void Spawn();
	void Precache();

	void PainSound( const CTakeDamageInfo &info );
	void AlertSound();
	void DeathSound( const CTakeDamageInfo &info );

private:
};



#endif // NPC_SWARM_DRONE_H
