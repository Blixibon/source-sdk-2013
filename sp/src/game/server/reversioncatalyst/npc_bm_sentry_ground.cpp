//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_sentry_ground.h"
#include "Sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define SENTRY_FLOOR_MODEL "models/NPCs/sentry_ground.mdl"

LINK_ENTITY_TO_CLASS( npc_bm_sentry_floor, CNPC_BM_FloorSentry );
LINK_ENTITY_TO_CLASS( npc_sentry_ground, CNPC_BM_FloorSentry ); // For simplicity/ease of use/legacy support/etc.

BEGIN_DATADESC( CNPC_BM_FloorSentry )

	DECLARE_BM_SENTRY_DATADESC()
	DECLARE_BM_NPC_DATADESC()

END_DATADESC()

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_FloorSentry::Classify( void )
{
	return CLASS_MILITARY;
}

#define FLOOR_TURRET_GLOW_SPRITE	"sprites/glow1.vmt"
#define	LASER_BEAM_SPRITE			"effects/laser1.vmt"

short sSentryHaloSprite;

//=========================================================
// Spawn
//=========================================================
void CNPC_BM_FloorSentry::Spawn()
{
	BaseClass::Spawn();

	m_takedamage = DAMAGE_YES;
	SetHealth( 50 ); // test

	m_hLaser = CBeam::BeamCreate( "sprites/laserbeam.vmt", 1.0f );
	m_hLaser->SetColor( 255, 24, 24 );
	m_hLaser->SetBrightness( 0 ); // Start off

	m_hLaser->PointEntInit( GetAbsOrigin(), this );
	m_hLaser->SetEndAttachment( m_iEyeAttachment );
	m_hLaser->SetNoise( 0 );
	m_hLaser->SetWidth( 0.5f );
	m_hLaser->SetEndWidth( 0 );
	m_hLaser->SetScrollRate( 0 );
	m_hLaser->SetFadeLength( 0 );
	m_hLaser->SetHaloTexture( sSentryHaloSprite );
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_BM_FloorSentry::Precache()
{
	if (GetModelName() == NULL_STRING)
		SetModelName( AllocPooledString( SENTRY_FLOOR_MODEL ) );

	BaseClass::Precache();

	PrecacheScriptSound( "npc_sentry_ground.MotorStop" );

	PrecacheParticleSystem( "npc_sentry_ground_muzzleflash" );

	sSentryHaloSprite = PrecacheModel( "sprites/light_glow03.vmt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_BM_FloorSentry::PreThink( turretState_e state )
{
	return BaseClass::PreThink(state);
}

//-----------------------------------------------------------------------------
// Purpose: Sets the state of the glowing eye attached to the turret
// Input  : state - state the eye should be in
//-----------------------------------------------------------------------------
void CNPC_BM_FloorSentry::SetEyeState( eyeState_t state )
{
	// Must have a valid eye to affect
	if ( !m_hEyeGlow && !HasSpawnFlags(SF_FLOOR_TURRET_NO_SPRITE) )
	{
		// Create our eye sprite
		m_hEyeGlow = CSprite::SpriteCreate( FLOOR_TURRET_GLOW_SPRITE, GetLocalOrigin(), false );
		if ( !m_hEyeGlow )
			return;

		m_hEyeGlow->SetTransparency( kRenderWorldGlow, 255, 0, 0, 128, kRenderFxNoDissipation );
		m_hEyeGlow->SetAttachment( this, m_iEyeAttachment );
	}

	m_iEyeState = state;

	//Set the state
	switch( state )
	{
	default:
	case TURRET_EYE_SEE_TARGET: //Fade in and scale up
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetBrightness( 164, 0.1f );
		m_hEyeGlow->SetScale( 0.4f, 0.1f );

		if (m_hLaser)
			m_hLaser->SetBrightness( 224 );
		break;

	case TURRET_EYE_SEEKING_TARGET: //Ping-pongs
		
		//Toggle our state
		m_bBlinkState = !m_bBlinkState;
		m_hEyeGlow->SetColor( 255, 128, 0 );
		
		if (m_hLaser)
			m_hLaser->SetBrightness( 128 );

		if ( m_bBlinkState )
		{
			//Fade up and scale up
			m_hEyeGlow->SetScale( 0.25f, 0.1f );
			m_hEyeGlow->SetBrightness( 164, 0.1f );
		}
		else
		{
			//Fade down and scale down
			m_hEyeGlow->SetScale( 0.2f, 0.1f );
			m_hEyeGlow->SetBrightness( 64, 0.1f );
		}

		break;

	case TURRET_EYE_DORMANT: //Fade out and scale down
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 0.5f );
		m_hEyeGlow->SetBrightness( 64, 0.5f );

		if (m_hLaser)
			m_hLaser->SetBrightness( 128 );
		break;

	case TURRET_EYE_DEAD: //Fade out slowly
		m_hEyeGlow->SetColor( 255, 0, 0 );
		m_hEyeGlow->SetScale( 0.1f, 3.0f );
		m_hEyeGlow->SetBrightness( 0, 3.0f );

		if (m_hLaser)
			m_hLaser->SetBrightness( 0 );
		break;

	case TURRET_EYE_DISABLED:
		m_hEyeGlow->SetColor( 0, 255, 0 );
		m_hEyeGlow->SetScale( 0.1f, 1.0f );
		m_hEyeGlow->SetBrightness( 0, 1.0f );

		if (m_hLaser)
			m_hLaser->SetBrightness( 0 );
		break;
	
	case TURRET_EYE_ALARM:
		{
			//Toggle our state
			m_bBlinkState = !m_bBlinkState;
			m_hEyeGlow->SetColor( 255, 0, 0 );

			if (m_hLaser)
				m_hLaser->SetBrightness( 224 );

			if ( m_bBlinkState )
			{
				//Fade up and scale up
				m_hEyeGlow->SetScale( 0.75f, 0.05f );
				m_hEyeGlow->SetBrightness( 192, 0.05f );
			}
			else
			{
				//Fade down and scale down
				m_hEyeGlow->SetScale( 0.25f, 0.25f );
				m_hEyeGlow->SetBrightness( 64, 0.25f );
			}
		}
		break;
	}
}

extern int ACT_FLOOR_TURRET_OPEN_IDLE;

//-----------------------------------------------------------------------------
// Purpose: Make a pinging noise so the player knows where we are
//-----------------------------------------------------------------------------
void CNPC_BM_FloorSentry::Ping( void )
{
	if (m_flPingTime > gpGlobals->curtime)
		return;

	BaseClass::Ping();

	// Longer ping time
	m_flPingTime = gpGlobals->curtime + 2.0f;
}
