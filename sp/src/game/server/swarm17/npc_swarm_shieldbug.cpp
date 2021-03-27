//=============================================================================
//
// Purpose: Swarm NPC shieldbug
//
//=============================================================================

#include "cbase.h"
#include "npc_swarm_shieldbug.h"

ConVar asw_shieldbug_health( "asw_shieldbug_health", "1000" );

BEGIN_DATADESC( CNPC_SwarmShieldbug )

END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_swarm_shieldbug, CNPC_SwarmShieldbug );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_SwarmShieldbug::CNPC_SwarmShieldbug()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SwarmShieldbug::Spawn()
{
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );	// | bits_CAP_MOVE_JUMP
	CapabilitiesRemove( bits_CAP_MOVE_JUMP );

	m_takedamage = DAMAGE_NO;	// alien is invulnerable until she finds her first enemy

	m_iHealth = m_iMaxHealth = asw_shieldbug_health.GetInt(); // ASWGameRules()->ModifyAlienHealthBySkillLevel

	SetHullType( HULL_WIDE_SHORT );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SwarmShieldbug::Precache()
{
	PrecacheScriptSound( "ASW_Drone.Alert" );
	PrecacheScriptSound( "ASW_Drone.Attack" );
	PrecacheScriptSound( "ASW_Parasite.Death" );
	PrecacheScriptSound( "ASW_Parasite.Idle" );
	PrecacheScriptSound( "ASW_Parasite.Attack" );

	PrecacheScriptSound( "ASW_ShieldBug.StepLight" );
	PrecacheScriptSound( "ASW_ShieldBug.Pain" );
	PrecacheScriptSound( "ASW_ShieldBug.Alert" );
	PrecacheScriptSound( "ASW_ShieldBug.Death" );
	PrecacheScriptSound( "ASW_ShieldBug.Attack" );
	PrecacheScriptSound( "ASW_ShieldBug.Circle" );
	PrecacheScriptSound( "ASW_ShieldBug.Idle" );

	// these are all his breakables
	PrecacheModel( "models/aliens/shieldbug/gib_back_leg.mdl" );
	PrecacheModel( "models/aliens/shieldbug/gib_leg_claw.mdl" );
	PrecacheModel( "models/aliens/shieldbug/gib_leg_middle.mdl" );
	PrecacheModel( "models/aliens/shieldbug/gib_leg_upper.mdl" );
	PrecacheModel( "models/aliens/shieldbug/gib_leg_l.mdl" );
	PrecacheModel( "models/aliens/shieldbug/gib_leg_r.mdl" );

	// particles
	PrecacheParticleSystem( "shieldbug_brain_explode" );
	PrecacheParticleSystem( "shieldbug_fountain" );
	PrecacheParticleSystem( "shieldbug_body_explode" );

	BaseClass::Precache();
}

void CNPC_SwarmShieldbug::PainSound( const CTakeDamageInfo &info )
{
	if (gpGlobals->curtime > m_fNextPainSound)
	{
		EmitSound("ASW_ShieldBug.Pain");
		m_fNextPainSound = gpGlobals->curtime + 0.5f;
	}
}

void CNPC_SwarmShieldbug::AlertSound()
{
	EmitSound( "ASW_ShieldBug.Alert" );
}

void CNPC_SwarmShieldbug::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "ASW_ShieldBug.Death" );
}
