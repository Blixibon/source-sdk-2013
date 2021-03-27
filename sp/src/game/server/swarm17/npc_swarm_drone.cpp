//=============================================================================
//
// Purpose: Swarm NPC drone
//
//=============================================================================

#include "cbase.h"
#include "npc_swarm_drone.h"

ConVar asw_drone_health( "asw_drone_health", "45" );

BEGIN_DATADESC( CNPC_SwarmDrone )

END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_swarm_drone, CNPC_SwarmDrone );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_SwarmDrone::CNPC_SwarmDrone()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SwarmDrone::Spawn()
{
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_MOVE_JUMP | bits_CAP_INNATE_MELEE_ATTACK1 );
	CapabilitiesAdd( bits_CAP_SQUAD );

	m_iHealth = m_iMaxHealth = asw_drone_health.GetInt(); // ASWGameRules()->ModifyAlienHealthBySkillLevel

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SwarmDrone::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_Drone.Land" );
	PrecacheScriptSound( "ASW_Drone.Pain" );
	PrecacheScriptSound( "ASW_Drone.Alert" );
	PrecacheScriptSound( "ASW_Drone.Death" );
	PrecacheScriptSound( "ASW_Drone.Attack" );
	PrecacheScriptSound( "ASW_Drone.Swipe" );

	PrecacheModel( "models/swarm/DroneGibs/dronepart01.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart20.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart29.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart31.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart32.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart44.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart45.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart47.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart49.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart50.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart53.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart54.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart56.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart57.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart58.mdl" );
	PrecacheModel( "models/swarm/DroneGibs/dronepart59.mdl" );
}

void CNPC_SwarmDrone::PainSound( const CTakeDamageInfo &info )
{
	if (gpGlobals->curtime > m_fNextPainSound)
	{
		EmitSound("ASW_Drone.Pain");
		m_fNextPainSound = gpGlobals->curtime + 0.5f;
	}
}

void CNPC_SwarmDrone::AlertSound()
{
	EmitSound( "ASW_Drone.Alert" );
}

void CNPC_SwarmDrone::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "ASW_Drone.Death" );
}
