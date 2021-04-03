#include "cbase.h"
#include "asw_missile_round_shared.h"
#ifndef SWARM17
#include "asw_gamerules.h"
#include "asw_melee_system.h"
#include "asw_trace_filter_shot.h"
#endif
#ifdef CLIENT_DLL
#include "particles_simple.h"
#else
#include "world.h"
#include "asw_util_shared.h"
#include "iasw_spawnable_npc.h"
#include "particle_parse.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "te_effect_dispatch.h"
#include "utlvector.h"
#ifdef SWARM17
#include "soundent.h"
#else
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "asw_player.h"
#endif
#include "inetchannelinfo.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Missile_Round, DT_ASW_Missile_Round );

BEGIN_NETWORK_TABLE( CASW_Missile_Round, DT_ASW_Missile_Round )
#ifdef GAME_DLL
SendPropInt( SENDINFO( m_nParticleTrail ) ),
SendPropBool( SENDINFO( m_bDetonated ) ),
SendPropVector( SENDINFO( m_vEndPosition ), 0, SPROP_NOSCALE ),
#else
RecvPropInt( RECVINFO( m_nParticleTrail ) ),
RecvPropBool( RECVINFO( m_bDetonated ) ),
RecvPropVector( RECVINFO( m_vEndPosition ) ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL

LINK_ENTITY_TO_CLASS( asw_missile_round, CASW_Missile_Round );
PRECACHE_REGISTER( asw_missile_round );

BEGIN_DATADESC( CASW_Missile_Round )	
#ifdef SWARM17
	DEFINE_FIELD( m_fDangerRadius, FIELD_FLOAT ),
#endif
END_DATADESC()

CUtlVector<CASW_Missile_Round*> g_vecMissileRounds;
extern ConVar sv_maxunlag;
ConVar sv_unlag_alien_projectiles( "sv_unlag_alien_projectiles", "1", FCVAR_NONE, "If enabled, server will rewind time based on a player's ping when doing ranger projectile collision vs marines." );

#ifdef SWARM17
static const char *s_pDangerSoundContext = "DangerSoundThink";
#endif

CASW_Missile_Round::CASW_Missile_Round()
{
	m_nParticleTrail = -1;
	m_vEndPosition.Init();
	m_bDetonated = false;
	m_bMarineFriendly = false;

#ifdef SWARM17
	m_fDangerRadius = 100;
#endif

	g_vecMissileRounds.AddToTail( this );
}

CASW_Missile_Round::~CASW_Missile_Round()
{
	g_vecMissileRounds.FindAndRemove( this );
}

void CASW_Missile_Round::Setup( const CASW_AlienShot &shot, const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner )
{
	m_ShotDef = shot;
	SetAbsAngles( angles );
	UTIL_SetOrigin( this, position );
	m_hOwner = pOwner;
	SetOwnerEntity( pOwner );

	SetAbsVelocity( velocity );
	QAngle vecAngVelocity( random->RandomFloat( -200, 200 ), random->RandomFloat( -200, 200 ), random->RandomFloat( -200, 200 ) );
	SetLocalAngularVelocity( vecAngVelocity );	

#ifndef SWARM17
	m_LagCompensation.Init(this);
#endif
}

void CASW_Missile_Round::Spawn( void )
{
	Precache();
	SetModel( m_ShotDef.m_strModel );
	
	//SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	SetMoveType( MOVETYPE_CUSTOM );

	m_takedamage	= m_ShotDef.m_bShootable ? DAMAGE_YES : DAMAGE_NO;
	m_iHealth		= 1;

	SetSize( -Vector(m_ShotDef.m_flSize,m_ShotDef.m_flSize,m_ShotDef.m_flSize), Vector(m_ShotDef.m_flSize,m_ShotDef.m_flSize,m_ShotDef.m_flSize) );
	SetSolid( SOLID_NONE );
	SetGravity( m_ShotDef.m_flGravity );
#ifdef SWARM17
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
#else
	SetCollisionGroup( ASW_COLLISION_GROUP_ALIEN_MISSILE );
#endif

	SetTouch( &CASW_Missile_Round::Touch );

	ResetSequence( LookupSequence( "MortarBugProjectile_Closed" ) );

	EmitSound( m_ShotDef.m_strSound_spawn );

	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + m_ShotDef.m_flFuse );

#ifdef SWARM17
	SetContextThink( &CASW_Missile_Round::DangerSoundThink, gpGlobals->curtime + 0.05f, s_pDangerSoundContext );
#endif

	m_vecOldPosition = GetAbsOrigin();
}

void CASW_Missile_Round::PerformCustomPhysics( Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity )
{
	if ( m_bDetonated )
	{
		*pNewPosition = GetAbsOrigin();
		*pNewVelocity = GetAbsVelocity();	
		return;
	}
	Vector vNorm = GetAbsVelocity();
	VectorNormalize( vNorm );
	QAngle angles;
	VectorAngles( vNorm, angles );
	SetAbsAngles( angles );

	Vector vNewPosition = GetAbsOrigin() + GetAbsVelocity() * gpGlobals->frametime;

	trace_t tr;
#ifdef SWARM17 // TODO
	CTraceFilterSimple filter( this, GetCollisionGroup() );
#else
	CASWTraceFilterShot filter( this, NULL, GetCollisionGroup() );

	if ( m_bMarineFriendly )
	{
		filter.SetSkipMarines( true );
		filter.SetSkipAliens( false );
	}
	else
	{
		filter.SetSkipAliens( true );
		filter.SetSkipMarines( false );
		filter.SetSkipRollingMarines( true );
	}
#endif
	UTIL_TraceHull( GetAbsOrigin(), vNewPosition, WorldAlignMins(), WorldAlignMaxs(), MASK_SHOT, &filter, &tr );
	vNewPosition = tr.endpos;

#ifndef SWARM17
	if ( tr.DidHit() && tr.m_pEnt ) 
	{
		if ( tr.m_pEnt->Classify() == CLASS_ASW_MARINE )
		{
			/*
			float flDesiredMoveDist = (GetAbsVelocity() * gpGlobals->frametime ).Length();
			float flMoveDist = ( tr.endpos - GetAbsOrigin() ).Length();
			if ( flDesiredMoveDist)
			float flMoveAmount = 
			filter.SetSkipMarinesReflectingProjectiles( true );
			*/
			CASW_Marine *pMarine = assert_cast<CASW_Marine*>( tr.m_pEnt );
			if ( pMarine->IsReflectingProjectiles() )
			{
				// reflect
				m_bMarineFriendly = true;
				SetCollisionGroup( ASW_COLLISION_GROUP_PLAYER_MISSILE );
				SetOwnerEntity( pMarine );
				SetAbsVelocity( -GetAbsVelocity() );

				*pNewPosition = GetAbsOrigin();
				*pNewVelocity = GetAbsVelocity();	
				return;
			}
		}		
	}
#endif

	SetAbsOrigin( vNewPosition );

	DoLagCompensatedMarineCollision();

	UpdatePhysicsShadowToCurrentPosition(gpGlobals->frametime);
	PhysicsTouchTriggers();

	*pNewPosition = GetAbsOrigin();
	*pNewVelocity = GetAbsVelocity();

	if ( tr.m_pEnt )
	{
		MissileHit( tr.m_pEnt, tr );
	}
}

void CASW_Missile_Round::Precache( )
{
	BaseClass::Precache();

	if ( !m_ShotDef.m_strModel.IsEmpty() )
		PrecacheModel( m_ShotDef.m_strModel );

	if ( !m_ShotDef.m_strSound_spawn.IsEmpty() )
		PrecacheScriptSound( m_ShotDef.m_strSound_spawn );

	if ( !m_ShotDef.m_strSound_hitNPC.IsEmpty() )
		PrecacheScriptSound( m_ShotDef.m_strSound_hitNPC );

	if ( !m_ShotDef.m_strSound_hitWorld.IsEmpty() )
		PrecacheScriptSound( m_ShotDef.m_strSound_hitWorld );

	if ( !m_ShotDef.m_strParticles_trail.IsEmpty() )
#ifdef SWARM17
	{
		PrecacheParticleSystem( m_ShotDef.m_strParticles_trail );
		m_nParticleTrail = GetParticleSystemIndex( m_ShotDef.m_strParticles_trail );
	}
#else
		m_nParticleTrail = PrecacheParticleSystem( m_ShotDef.m_strParticles_trail );
#endif

	if ( !m_ShotDef.m_strParticles_hit.IsEmpty() )
		PrecacheParticleSystem( m_ShotDef.m_strParticles_hit );
}

void CASW_Missile_Round::DoLagCompensatedMarineCollision()
{
#ifndef SWARM17
	if ( !ASWGameResource() )
		return;

	if ( gpGlobals->maxClients > 1 )
	{
		m_LagCompensation.StorePositionHistory();
	}

	Vector vecMovement = GetAbsOrigin() - m_vecOldPosition;

	int nMaxResources = ASWGameResource()->GetMaxMarineResources();
	for ( int i = 0; i < nMaxResources; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( !pMR )
			continue;

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine )
			continue;

		if ( pMarine->GetCurrentMeleeAttack() && pMarine->GetCurrentMeleeAttack()->m_nAttackID == CASW_Melee_System::s_nRollAttackID )
			continue;

		Vector vecPos = GetAbsOrigin();

		if ( pMarine->IsInhabited() && sv_unlag_alien_projectiles.GetBool() )
		{
			CASW_Player *pPlayer = pMarine->GetCommander();
			if ( !pPlayer )
				continue;

			float correct = 0.0f;

			INetChannelInfo *nci = engine->GetPlayerNetInfo( pPlayer->entindex() ); 
			if ( nci )
			{
				correct+= nci->GetLatency( FLOW_OUTGOING ) * 0.5f;					// add network latency
			}

			int lerpTicks = TIME_TO_TICKS( pPlayer->m_fLerpTime );			// calc number of view interpolation ticks - 1
			correct += TICKS_TO_TIME( lerpTicks );							// add view interpolation latency see C_BaseEntity::GetInterpolationAmount()
			correct = clamp( correct, 0.0f, sv_maxunlag.GetFloat() );		// check bounds [0,sv_maxunlag]

			// correct tick send by player 
			int targettick = gpGlobals->tickcount - TIME_TO_TICKS( correct );

			// check the player has enough lag to warrant doing the work of lag compensation
			const float fLaggedTime = TICKS_TO_TIME( targettick );
			if ( gpGlobals->curtime - fLaggedTime < ASW_MIN_LAG_TIME )
				continue;

			vecPos = m_LagCompensation.GetLaggedPosition( fLaggedTime );

			//NDebugOverlay::Box( vecPos, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), 0, 0, 255, 0, 0.1f );
		}
		
		trace_t tr;
		CTraceFilterOnlyHitThis traceFilter( pMarine );
		AI_TraceLine( vecPos - vecMovement, vecPos, MASK_SHOT, &traceFilter, &tr );

		if ( tr.DidHitNonWorldEntity() )
		{
			MissileHit( tr.m_pEnt, tr );
		}
	
		if ( m_bDetonated )
		{
			break;
		}
		
		m_vecOldPosition = GetAbsOrigin();
	}
#endif
}

void CASW_Missile_Round::Touch( CBaseEntity *pOther )
{
	if ( m_bDetonated )
		return;

	if ( !pOther || pOther == GetOwnerEntity() )
	{
		return;
	}

	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
	{
		return;
	}

	// make sure we don't die on things we shouldn't
	if ( !ASWGameRules() || !ASWGameRules()->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
	{
		return;
	}

	trace_t	tr;
	tr = BaseClass::GetTouchTrace();
	MissileHit( pOther, tr );
}

void CASW_Missile_Round::MissileHit( CBaseEntity *pEnt, trace_t &tr )
{
#ifndef SWARM17
	// don't collide with marines doing a roll
	if ( pEnt->Classify() == CLASS_ASW_MARINE )
	{
		CASW_Marine *pMarine = static_cast<CASW_Marine*>( pEnt );
		if ( pMarine->GetCurrentMeleeAttack() && pMarine->GetCurrentMeleeAttack()->m_nAttackID == CASW_Melee_System::s_nRollAttackID )
		{
			return;
		}
		if ( pMarine->IsReflectingProjectiles() )
		{
			// TODO: ignore this marine (don't collide with him again)
			// TODO: change direction
			return;
		}
	}
#endif

	if ( pEnt->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr2;
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

#ifdef SWARM17
		CTakeDamageInfo	dmgInfo( this, m_hOwner, m_ShotDef.m_flDamage_direct, m_ShotDef.m_iDamageType );
#else
		CTakeDamageInfo	dmgInfo( this, m_hOwner, m_ShotDef.m_flDamage_direct, DMG_GENERIC | DMG_NEVERGIB );
#endif
		CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
		dmgInfo.SetDamagePosition( tr.endpos );
		trace_t *pTrace = &tr;
		pEnt->DispatchTraceAttack( dmgInfo, vecNormalizedVel, pTrace );

		ApplyMultiDamage();

		// keep going through the glass.
		if ( pEnt->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			return;

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pEnt, COLLISION_GROUP_NONE, &tr2 );
	}
	else
	{
		if ( m_ShotDef.m_flBounce != 0.0f )
		{
			Vector	vecVel = GetAbsVelocity();
			float	flDot = vecVel.Dot( tr.plane.normal );
			vecVel -= ( m_ShotDef.m_flBounce + 1.0f ) * flDot * tr.plane.normal;
			SetAbsVelocity( vecVel );
			EmitSound( m_ShotDef.m_strSound_hitWorld );
			return;
		}

		// Put a mark unless we've hit the sky
		if ( ( tr.surface.flags & SURF_SKY ) == false )
		{
			UTIL_ImpactTrace( &tr, DMG_GENERIC );
		}
	}

	DispatchParticleEffect( m_ShotDef.m_strParticles_hit, GetAbsOrigin(), QAngle( 0, 0, 0 ) );

#ifdef SWARM17
	CBaseCombatCharacter *pMarine = pEnt->MyCombatCharacterPointer();
	if ( pMarine )
#else
	// play body "thwack" sound
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pEnt );
	if ( pMarine )
#endif
		EmitSound( m_ShotDef.m_strSound_hitNPC );
	else
		EmitSound( m_ShotDef.m_strSound_hitWorld );

	m_bDetonated = true;
	m_vEndPosition = GetAbsOrigin();

	SetTouch( NULL );
	SetAbsVelocity( Vector( 0, 0, 0 ) );
	SetSolid( SOLID_NONE );
	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

#ifdef SWARM17
void CASW_Missile_Round::DangerSoundThink()
{
	// Copied from grenade_ar2

	// The old way of making danger sounds would scare the crap out of EVERYONE between you and where the grenade
	// was going to hit. The radius of the danger sound now 'blossoms' over the grenade's lifetime, making it seem
	// dangerous to a larger area downrange than it does from where it was fired.
	if( m_fDangerRadius <= 300 )
	{
		m_fDangerRadius += ( 300 * 0.05 );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, m_fDangerRadius, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );

	SetNextThink( gpGlobals->curtime + 0.05f, s_pDangerSoundContext );
}
#endif

CASW_Missile_Round* CASW_Missile_Round::Missile_Round_Create( const CASW_AlienShot &shot, const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner )
{
	CASW_Missile_Round *pMissile = ( CASW_Missile_Round * )CreateEntityByName( "asw_missile_round" );
	pMissile->Setup( shot, position, angles, velocity, pOwner );	
	pMissile->Spawn();
	return pMissile;
}

#endif		// GAME_DLL

#ifdef CLIENT_DLL

CASW_Missile_Round::CASW_Missile_Round()
{
	m_bDetonated = false;
	m_vEndPosition.Init();
	m_pTrail = NULL;
}

void CASW_Missile_Round::ClientThink()
{
	// wait until our origin is the same as the server and then hide our model and particle trail
	Vector origin = GetAbsOrigin();
	if ( ( m_vEndPosition - origin ).LengthSqr() < 1.0f )
	{
		SetModelIndex( 0 );
		if ( m_pTrail )
		{
			ParticleProp()->StopEmission( m_pTrail );
			m_pTrail = NULL;
		}
	}
}

void CASW_Missile_Round::PostDataUpdate( DataUpdateType_t updateType )
{	
	BaseClass::PostDataUpdate(updateType);

	// If this entity was new, then latch in various values no matter what.
	if ( updateType == DATA_UPDATE_CREATED )
	{
		if ( m_nParticleTrail >= 0 )
		{
#ifndef SWARM17 // TODO
			m_pTrail = ParticleProp()->CreatePrecached( m_nParticleTrail, PATTACH_ABSORIGIN_FOLLOW );
#endif
			if (m_pTrail )
			{ 
				ParticleProp()->AddControlPoint( m_pTrail, 1, this, PATTACH_ABSORIGIN_FOLLOW );
				m_pTrail->SetControlPoint( 0, GetAbsOrigin() );
				m_pTrail->SetControlPoint( 1, GetAbsOrigin() );
				m_pTrail->SetControlPointEntity( 0, this );
				m_pTrail->SetControlPointEntity( 1, this );
			}
		}
	}
	else if ( updateType == DATA_UPDATE_DATATABLE_CHANGED )
	{
		if ( m_bDetonated )
		{
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}
}

#endif      // CLIENT_DLL
