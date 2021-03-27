//=============================================================================
//
// Purpose: Swarm NPC base
//
//=============================================================================

#include "cbase.h"
#include "npc_swarm_base.h"

#include "gib.h"
#include "props.h"
#include "particle_parse.h"

BEGIN_DATADESC( CAI_BaseSwarmNPC )

	DEFINE_FIELD( m_fNextPainSound, FIELD_TIME ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_swarm_base, CAI_BaseSwarmNPC );

int ACT_ASW_ALIEN_JUMP_START;
int ACT_ASW_ALIEN_LAND;

int ACT_DIE_FANCY;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseSwarmNPC::Spawn()
{
	if (GetModelName() == NULL_STRING)
	{
		SetModelName( AllocPooledString( GetDefaultModel() ) );
	}

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	SetHullSizeNormal( true );
	SetDefaultEyeOffset();
	SetActivity( ACT_IDLE );

	// TODO: Custom blood type?
	SetBloodColor( BLOOD_COLOR_ANTLION );

	BaseClass::Spawn();

	//Precache();

	SetModel( STRING(GetModelName()) );

	SetupVPhysicsHull();

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_BaseSwarmNPC::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( "AntlionGib" );
	PrecacheParticleSystem( "blood_impact_antlion_01" );

	PrecacheModel( STRING(GetModelName()) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_BaseSwarmNPC::ShouldGib( const CTakeDamageInfo &info )
{
	// If we're being hoisted, we only want to gib when the barnacle hurts us with his bite!
	if ( IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
	{
		if ( info.GetAttacker() && info.GetAttacker()->Classify() != CLASS_BARNACLE )
			return false;

		return true;
	}

	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB|DMG_BLAST) )
		return true;

	if ( m_iHealth < -20 )
		return true;
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_BaseSwarmNPC::CorpseGib( const CTakeDamageInfo &info )
{
	{
		// Use the bone position to handle being moved by an animation (like a dynamic scripted sequence)
		static int s_nBodyBone = -1;
		if ( s_nBodyBone == -1 )
		{
			s_nBodyBone = LookupBone( "root" );
		}

		Vector vecOrigin;
		QAngle angBone;
		GetBonePosition( s_nBodyBone, vecOrigin, angBone );

		DispatchParticleEffect( "AntlionGib", vecOrigin, QAngle( 0, 0, 0 ) );
	}

	Vector velocity = vec3_origin;
	AngularImpulse	angVelocity = RandomAngularImpulse( -150, 150 );
	breakablepropparams_t params( EyePosition(), GetAbsAngles(), velocity, angVelocity );
	params.impactEnergyScale = 1.0f;
	params.defBurstScale = 150.0f;
	params.defCollisionGroup = COLLISION_GROUP_DEBRIS;
	PropBreakableCreateAll( GetModelIndex(), NULL, params, this, -1, true, true );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_BaseSwarmNPC::CanBecomeRagdoll()
{
	// This prevents us from dying in the regular way. It forces a schedule selection
	// that will select SCHED_DIE, where we can do our poison burst thing.
	return ( m_NPCState == NPC_STATE_SCRIPT || IsCurSchedule( SCHED_DIE ) /*|| m_bDontExplode*/ ) && BaseClass::CanBecomeRagdoll();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Activity CAI_BaseSwarmNPC::NPC_TranslateActivity( Activity baseAct )
{
	// Explode if gibbing.
	if ( baseAct == ACT_DIESIMPLE )
	{
		return (Activity)ACT_DIE_FANCY;
	}

	if ( baseAct == ACT_JUMP )
	{
		return (Activity)ACT_ASW_ALIEN_JUMP_START;
	}

	if ( baseAct == ACT_LAND )
	{
		return (Activity)ACT_ASW_ALIEN_LAND;
	}

	return BaseClass::NPC_TranslateActivity( baseAct );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_swarm_base, CAI_BaseSwarmNPC )

	//DECLARE_ANIMEVENT( AE_COMPANION_PRODUCE_FLARE )

	DECLARE_ACTIVITY( ACT_ASW_ALIEN_JUMP_START )
	DECLARE_ACTIVITY( ACT_ASW_ALIEN_LAND )

	DECLARE_ACTIVITY( ACT_DIE_FANCY )

AI_END_CUSTOM_NPC()
