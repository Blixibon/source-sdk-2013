//=============================================================================
//
// Purpose: Swarm NPC base
//
//=============================================================================

#ifndef NPC_SWARM_BASE_H
#define NPC_SWARM_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "ai_baseactor.h"

typedef CAI_BlendingHost< CAI_BehaviorHost<CAI_BaseNPC> > CAI_BaseSwarmNPCBase;

class CAI_BaseSwarmNPC : public CAI_BaseSwarmNPCBase
{
public:
	DECLARE_CLASS( CAI_BaseSwarmNPC, CAI_BaseSwarmNPCBase );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	Class_T Classify( void ) { return CLASS_SWARM_DRONE; }

	virtual const char *GetDefaultModel() { return "models/aliens/drone/drone.mdl"; }

	void Spawn();
	void Precache();

	bool ShouldGib( const CTakeDamageInfo &info );
	bool CorpseGib( const CTakeDamageInfo &info );
	bool CanBecomeRagdoll();

	Activity NPC_TranslateActivity( Activity baseAct );

	float m_fNextPainSound;

private:
};



#endif // NPC_SWARM_BASE_H
