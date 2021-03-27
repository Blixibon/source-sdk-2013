//=============================================================================
//
// Purpose: Swarm NPC shieldbug
//
//=============================================================================

#ifndef NPC_SWARM_SHELDBUG_H
#define NPC_SWARM_SHELDBUG_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_swarm_base.h"

class CNPC_SwarmShieldbug : public CAI_BaseSwarmNPC
{
public:
	DECLARE_CLASS( CNPC_SwarmShieldbug, CAI_BaseSwarmNPC );
	DECLARE_DATADESC();

	CNPC_SwarmShieldbug();

	const char *GetDefaultModel() { return "models/aliens/shieldbug/shieldbug.mdl"; }

	void Spawn();
	void Precache();

	void PainSound( const CTakeDamageInfo &info );
	void AlertSound();
	void DeathSound( const CTakeDamageInfo &info );

private:
};



#endif // NPC_SWARM_SHELDBUG_H
