//==============================================================================
//
// Purpose: The base class for Black Mesa weapons.
// 
//==============================================================================

#include "cbase.h"
#include "weapon_bm_base.h"
#include "basehlcombatweapon.h"
#include "weapon_gauss.h"
#include "in_buttons.h"
#include "rumble_shared.h"
#include "soundenvelope.h"
#include "beam_shared.h"

#define GAUSS_MAX_CHARGE_AMMO		10 // TODO
#define GAUSS_OVERCHARGE_TIME		10

#define JEEP_GUN_SPIN				"gun_spin"
#define	JEEP_GUN_SPIN_RATE			15.0f

//-----------------------------------------------------------------------------
// CWeapon_BM_Gauss
//-----------------------------------------------------------------------------
class CWeapon_BM_Gauss : public CBase_BM_Weapon<CBaseHLCombatWeapon>
{
public:
	DECLARE_CLASS( CWeapon_BM_Gauss, CBase_BM_Weapon<CBaseHLCombatWeapon> );

	DECLARE_SERVERCLASS();

	CWeapon_BM_Gauss( void );
	
	void Spawn( void );
	void Activate( void );
	//void Precache( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void ItemPostFrame( void );
	void ItemHolsterFrame( void );
	
	void FireCannon( void );
	void ChargeCannon( void );
	void FireChargedCannon( void );
	
	const char		*GetTracerType( void ) { return "AR2Tracer"; }
	void			DoImpactEffect( trace_t &tr, int nDamageType );
	void DrawBeam( const Vector &startPos, const Vector &endPos, float width );

	void StopChargeSound( void );

	int CapabilitiesGet( void ) {	return bits_CAP_WEAPON_RANGE_ATTACK1;	}

	virtual float GetFireRate( void )
	{
		return 0.5f;
	}

	//DECLARE_ACTTABLE();
	DECLARE_DATADESC();

private:
	
	bool			m_bCannonCharging;
	bool			m_bPlayedChargeSound;
	float			m_flCannonTime;
	CNetworkVar( float, m_flCannonChargeStartTime );
	int				m_iCannonChargeStartAmmo;
	Vector			m_vecGunOrigin;
	CSoundPatch		*m_sndCannonCharge;
	CSoundPatch		*m_sndCannonOverCharge;
	int				m_nSpinPos;

	Activity		m_OverchargedActivity;
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_BM_Gauss, DT_Weapon_BM_Gauss )
	SendPropFloat( SENDINFO( m_flCannonChargeStartTime ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_bm_gauss, CWeapon_BM_Gauss );
PRECACHE_WEAPON_REGISTER( weapon_bm_gauss );

BEGIN_DATADESC( CWeapon_BM_Gauss )

	//DEFINE_FIELD( m_flCharge, FIELD_FLOAT ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeapon_BM_Gauss::CWeapon_BM_Gauss( void )
{
	m_bFiresUnderwater = false;
	m_flCannonChargeStartTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::Spawn()
{
	BaseClass::Spawn();

	m_nSpinPos = 0;

	Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::Activate()
{
	BaseClass::Activate();

	m_OverchargedActivity = (Activity)RegisterPrivateActivity( "ACT_VM_OVERCHARGED" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::PrimaryAttack()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::SecondaryAttack()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	if (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) > 0)
	{
		if ( pPlayer->m_nButtons & IN_ATTACK )
		{
			if ( m_bCannonCharging )
			{
				FireChargedCannon();
			}
			else
			{
				FireCannon();
			}
		}
		else if ( pPlayer->m_nButtons & IN_ATTACK2 )
		{
			ChargeCannon();
		}
	}

	// If we've released our secondary button, fire off our cannon
	if ( ( pPlayer->m_afButtonReleased & IN_ATTACK2 ) && ( m_bCannonCharging ) )
	{
		FireChargedCannon();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::ItemHolsterFrame()
{
	BaseClass::ItemHolsterFrame();

	if (m_bCannonCharging)
	{
		m_bCannonCharging = false;
		m_flCannonChargeStartTime = 0.0f;

		StopChargeSound();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::FireCannon( void )
{
	//Don't fire again if it's been too soon
	if ( m_flCannonTime > gpGlobals->curtime )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	CDisablePredictionFiltering disabler;

	m_flCannonTime = gpGlobals->curtime + 0.25f;
	m_bCannonCharging = false;
	m_flCannonChargeStartTime = 0.0f;

	FireBulletsInfo_t info( 1, pPlayer->Weapon_ShootPosition(), pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT ), VECTOR_CONE_1DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );

	info.m_nFlags = FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS;
	info.m_pAttacker = pPlayer;

	FireBullets( info );

	int iCurrentAmmo = pPlayer->GetAmmoCount( m_iPrimaryAmmoType );
	if (iCurrentAmmo >= 2)
		pPlayer->SetAmmoCount( iCurrentAmmo - 2, m_iPrimaryAmmoType );
	else
		pPlayer->SetAmmoCount( 0, m_iPrimaryAmmoType );

	//m_iClip1 -= 1;

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
	pPlayer->RumbleEffect( RUMBLE_PISTOL, 0, RUMBLE_FLAG_RESTART	);

	WeaponSound( WPN_DOUBLE );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	
	// make cylinders of gun spin a bit
	m_nSpinPos += JEEP_GUN_SPIN_RATE;

	CBaseViewModel *vm = pPlayer->GetViewModel();
	if ( vm != NULL )
	{
		vm->SetPoseParameter( JEEP_GUN_SPIN, m_nSpinPos );
	}

	Vector vecForward;
	pPlayer->GetVectors( &vecForward, NULL, NULL );

	if ( !g_pGameRules->IsMultiplayer() )
	{
		// in deathmatch, gauss can pop you up into the air. Not in single play.
		vecForward.z = 0.0f;
	}

	pPlayer->ApplyAbsVelocityImpulse( vecForward * -40.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::FireChargedCannon( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	CDisablePredictionFiltering disabler;

	bool penetrated = false;

	m_bCannonCharging	= false;
	m_flCannonTime		= gpGlobals->curtime + 1.0f;
	m_flCannonChargeStartTime = 0.0f;

	StopChargeSound();

	WeaponSound( SINGLE );

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	pPlayer->RumbleEffect( RUMBLE_357, 0, RUMBLE_FLAG_RESTART );

	Vector aimDir = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector endPos = pPlayer->Weapon_ShootPosition() + ( aimDir * MAX_TRACE_LENGTH );
	
	//Shoot a shot straight out
	trace_t	tr;
	UTIL_TraceLine( pPlayer->Weapon_ShootPosition(), endPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	
	ClearMultiDamage();

	//Find how much damage to do
	float flChargeAmount = ( gpGlobals->curtime - m_flCannonChargeStartTime ) / MAX_GAUSS_CHARGE_TIME;

	//Clamp this
	if ( flChargeAmount > 1.0f )
	{
		flChargeAmount = 1.0f;
	}

	//Determine the damage amount
	//FIXME: Use ConVars!
	float flDamage = 15 + ( ( 250 - 15 ) * flChargeAmount );

	CBaseEntity *pHit = tr.m_pEnt;
	
	//Look for wall penetration
	if ( tr.DidHitWorld() && !(tr.surface.flags & SURF_SKY) )
	{
		//Try wall penetration
		UTIL_ImpactTrace( &tr, m_iPrimaryAmmoType, "ImpactJeep" );
		UTIL_DecalTrace( &tr, "RedGlowFade" );

		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		
		Vector	testPos = tr.endpos + ( aimDir * 48.0f );

		UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, GetOwner(), COLLISION_GROUP_NONE, &tr );
			
		if ( tr.allsolid == false )
		{
			UTIL_DecalTrace( &tr, "RedGlowFade" );

			penetrated = true;
		}
	}
	else if ( pHit != NULL )
	{
		CTakeDamageInfo dmgInfo( this, pPlayer, flDamage, DMG_SHOCK );
		CalculateBulletDamageForce( &dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos, 1.0f + flChargeAmount * 4.0f );

		//Do direct damage to anything in our path
		pHit->DispatchTraceAttack( dmgInfo, aimDir, &tr );
	}

	ApplyMultiDamage();

	//Kick up an effect
	if ( !(tr.surface.flags & SURF_SKY) )
	{
  		UTIL_ImpactTrace( &tr, m_iPrimaryAmmoType, "ImpactJeep" );

		//Do a gauss explosion
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
	}

	//Show the effect
	DrawBeam( m_vecGunOrigin, tr.endpos, 9.6 );

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5f );

	//Do radius damage if we didn't penetrate the wall
	if ( penetrated == true )
	{
		RadiusDamage( CTakeDamageInfo( this, this, flDamage, DMG_SHOCK ), tr.endpos, 200.0f, CLASS_NONE, NULL );
	}

	// make cylinders of gun spin a bit
	m_nSpinPos += JEEP_GUN_SPIN_RATE;

	CBaseViewModel *vm = pPlayer->GetViewModel();
	if ( vm != NULL )
	{
		vm->SetPoseParameter( JEEP_GUN_SPIN, m_nSpinPos );
	}

	Vector vecForward;
	pPlayer->GetVectors( &vecForward, NULL, NULL );

	if ( !g_pGameRules->IsMultiplayer() )
	{
		// in deathmatch, gauss can pop you up into the air. Not in single play.
		vecForward.z = 0.0f;
	}

	pPlayer->ApplyAbsVelocityImpulse( vecForward * -200.0f * MAX( 0.2f, flChargeAmount ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::ChargeCannon( void )
{
	//Don't fire again if it's been too soon
	if ( m_flCannonTime > gpGlobals->curtime )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;

	//See if we're starting a charge
	if ( m_bCannonCharging == false )
	{
		m_flCannonChargeStartTime = gpGlobals->curtime;
		m_iCannonChargeStartAmmo = pPlayer->GetAmmoCount( m_iPrimaryAmmoType );
		m_bCannonCharging = true;
		m_bPlayedChargeSound = false;

		//Start charging sound
		CPASAttenuationFilter filter( this );
		m_sndCannonCharge = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, GetShootSound( SPECIAL1 ), ATTN_NORM );

		pPlayer->RumbleEffect( RUMBLE_FLAT_LEFT, (int)(0.1 * 100), RUMBLE_FLAG_RESTART | RUMBLE_FLAG_LOOP | RUMBLE_FLAG_INITIAL_SCALE );

		assert(m_sndCannonCharge!=NULL);
		if ( m_sndCannonCharge != NULL )
		{
			(CSoundEnvelopeController::GetController()).Play( m_sndCannonCharge, 1.0f, 100 ); // Play( m_sndCannonCharge, 1.0f, 50 )
			//(CSoundEnvelopeController::GetController()).SoundChangePitch( m_sndCannonCharge, 250, 3.0f );
		}

		SendWeaponAnim( ACT_GAUSS_SPINCYCLE );

		return;
	}
	else
	{
		float flChargeAmount = ( gpGlobals->curtime - m_flCannonChargeStartTime ) / MAX_GAUSS_CHARGE_TIME;
		if ( flChargeAmount > 1.0f )
		{
			flChargeAmount = 1.0f;
		}

		if (flChargeAmount == 1.0f && !m_bPlayedChargeSound)
		{
			WeaponSound( SPECIAL2 );

			//Start charging sound
			CPASAttenuationFilter filter( this );
			m_sndCannonOverCharge = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, GetShootSound( RELOAD ), ATTN_NORM );
			if (m_sndCannonOverCharge != NULL )
			{
				(CSoundEnvelopeController::GetController()).Play( m_sndCannonOverCharge, 1.0f, 100 );
			}

			m_bPlayedChargeSound = true;
		}

		float rumble = flChargeAmount * 0.5f;

		pPlayer->RumbleEffect( RUMBLE_FLAT_LEFT, (int)(rumble * 100), RUMBLE_FLAG_UPDATE_SCALE );

		pPlayer->SetAmmoCount( m_iCannonChargeStartAmmo - (int)( flChargeAmount * (float)GAUSS_MAX_CHARGE_AMMO ), m_iPrimaryAmmoType );
		//m_iClip1 = m_iCannonChargeStartAmmo - (int)( flChargeAmount * GAUSS_MAX_CHARGE_AMMO );

		m_nSpinPos += JEEP_GUN_SPIN_RATE * flChargeAmount;

		CBaseViewModel *vm = pPlayer->GetViewModel();
		if ( vm != NULL )
		{
			vm->SetPoseParameter( JEEP_GUN_SPIN, m_nSpinPos );
		}
	}

	//TODO: Add muzzle effect?

	if ( (gpGlobals->curtime - m_flCannonChargeStartTime) >= GAUSS_OVERCHARGE_TIME )
	{
		// Overcharge
		pPlayer->TakeDamage( CTakeDamageInfo( this, this, 50, DMG_SHOCK ) );

		StopChargeSound();

		WeaponSound( SPECIAL3 );

		SendWeaponAnim( m_OverchargedActivity );

		m_flCannonTime = gpGlobals->curtime + 2.0f;
		m_bCannonCharging = false;
		m_flCannonChargeStartTime = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw our beam
	DrawBeam( tr.startpos, tr.endpos, 2.4 );

	if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );

		UTIL_ImpactTrace( &tr, m_iPrimaryAmmoType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - 
//			&endPos - 
//			width - 
//			useMuzzle - 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{
	//Tracer down the middle
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );

	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( GAUSS_BEAM_SPRITE, 0.5 );
	
	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("fire02") );
	pBeam->SetWidth( width );
	pBeam->SetEndWidth( 0.05f );
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );

	//Draw electric bolts along shaft
	pBeam = CBeam::BeamCreate( GAUSS_BEAM_SPRITE, 3.0f );
	
	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("fire01") );

	pBeam->SetBrightness( random->RandomInt( 64, 255 ) );
	pBeam->SetColor( 255, 255, 150+random->RandomInt( 0, 64 ) );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );
	pBeam->SetNoise( 1.6f );
	pBeam->SetEndWidth( 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_BM_Gauss::StopChargeSound( void )
{
	if ( m_sndCannonCharge != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndCannonCharge, 0.1f );
	}
	
	if ( m_sndCannonOverCharge != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndCannonOverCharge, 0.1f );
	}

	if( GetOwner() && GetOwner()->IsPlayer() )
	{
		ToBasePlayer( GetOwner() )->RumbleEffect( RUMBLE_FLAT_LEFT, 0, RUMBLE_FLAG_STOP );
	}
}
