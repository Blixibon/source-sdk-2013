//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "hl2/npc_turret_floor.h"
#include "ai_base_bm_npc.h"
#include "beam_shared.h"

template <class BASE_NPC>
class CAI_Base_BM_Sentry : public CAI_Base_BM_NPC<BASE_NPC>
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_Sentry, CAI_Base_BM_NPC<BASE_NPC> );

public:

	void	SentryPreThink( turretState_e state );

	bool	PreThink( turretState_e state );
	bool	UpdateFacing( void );
	void	DoMuzzleFlash();
	void	UpdateLaser();

	virtual CBeam *GetLaser() { return NULL; }
	virtual void RemoveLaser() {}
	virtual int GetLaserAttachment() { return 0; }

	virtual const char *GetMuzzleFlashParticle() { return NULL; }
	virtual int GetMuzzleAttachment() { return 0; }

	virtual const char *GetMoveStopSound() { return NULL; }
	virtual float GetFireRate() { return 0.05f; }

protected:

	bool m_bPlayingMotor;
	bool m_bMuzzleFlashActive;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Sentry<BASE_NPC>::SentryPreThink( turretState_e state )
{
	if (state == TURRET_DEAD)
	{
		if ( this->GetLaser() )
		{
			this->RemoveLaser();
		}
	}
	else if (this->GetLaser())
	{
		this->UpdateLaser();
	}

	// HACKHACK (removes muzzle flash)
	if (this->m_bMuzzleFlashActive && this->m_flShotTime <= gpGlobals->curtime + this->GetFireRate()) // GetActivity() != (Activity)ACT_CEILING_TURRET_FIRE
	{
		StopParticleEffects( this );
		this->m_bMuzzleFlashActive = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Sentry<BASE_NPC>::PreThink( turretState_e state )
{
	this->SentryPreThink( state );

	return BaseClass::PreThink(state);
}

//-----------------------------------------------------------------------------
// Purpose: Causes the turret to face its desired angles
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Sentry<BASE_NPC>::UpdateFacing( void )
{
	bool  bMoved = BaseClass::UpdateFacing();

	if (bMoved)
	{
		if (!this->m_bPlayingMotor)
		{
			this->EmitSound( this->GetMoveSound() );
			this->m_bPlayingMotor = true;
		}
	}
	else if (this->m_bPlayingMotor)
	{
		this->EmitSound( this->GetMoveStopSound() );
		this->m_bPlayingMotor = false;
	}

	return bMoved;
}

//-----------------------------------------------------------------------------
// Purpose: Overload our muzzle flash and send it to any actively held weapon
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Sentry<BASE_NPC>::DoMuzzleFlash()
{
	if (!this->m_bMuzzleFlashActive)
	{
		DispatchParticleEffect( this->GetMuzzleFlashParticle(), PATTACH_POINT_FOLLOW, this, this->GetMuzzleAttachment() );
		this->m_bMuzzleFlashActive = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Sentry<BASE_NPC>::UpdateLaser()
{
	// Update laser
	Vector vecLaserOrigin, vecLaserForward;
	GetAttachment( this->GetLaserAttachment(), vecLaserOrigin, &vecLaserForward );

	trace_t tr;
	UTIL_TraceLine( vecLaserOrigin, vecLaserOrigin + (vecLaserForward * 2048.0f), MASK_BLOCKLOS_AND_NPCS, this, COLLISION_GROUP_NONE, &tr );

	this->GetLaser()->PointEntInit( tr.endpos, this );
	this->GetLaser()->SetEndAttachment( this->GetLaserAttachment() );
}

#define DECLARE_BM_SENTRY_DATADESC() \
	DEFINE_FIELD( m_bPlayingMotor, FIELD_BOOLEAN ), \
	DEFINE_FIELD( m_bMuzzleFlashActive, FIELD_BOOLEAN ), \

class CNPC_BM_FloorSentry : public CAI_Base_BM_Sentry<CNPC_FloorTurret>
{
public:
	DECLARE_CLASS( CNPC_BM_FloorSentry, CAI_Base_BM_Sentry<CNPC_FloorTurret> );

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );

	bool	PreThink( turretState_e state );

	void	SetEyeState( eyeState_t state );
	void	Ping( void );
	
	const char *GetRetireSound()	{ return "npc_sentry_ground.Retract"; }
	const char *GetRetractSound()	{ return "npc_sentry_ground.Retract"; }
	const char *GetDeploySound()	{ return "npc_sentry_ground.Deploy"; }
	const char *GetMoveSound()		{ return "npc_sentry_ground.MotorLoop"; }
	const char *GetMoveStopSound()	{ return "npc_sentry_ground.MotorStop"; }
	const char *GetActivateSound()	{ return "npc_sentry_ground.Activate"; }
	const char *GetAlertSound()		{ return "npc_sentry_ground.Alert"; }
	const char *GetAlarmSound()		{ return "npc_sentry_ground.Alarm"; }
	const char *GetShootSound()		{ return "npc_sentry_ground.Shoot"; }
	const char *GetPingSound()		{ return "npc_sentry_ground.Ping"; }
	const char *GetDieSound()		{ return "npc_sentry_ground.Die"; }

	void		StopLoopingSounds( void ) { EmitSound( "npc_sentry_ground.MotorStop" ); }

	float		GetFireRate() { return 1.0f; } // Only used by CAI_Base_BM_Sentry

	virtual CBeam *GetLaser() { return m_hLaser; }
	virtual void RemoveLaser() { UTIL_Remove( m_hLaser.Get() ); m_hLaser = NULL; }
	virtual int GetLaserAttachment() { return m_iEyeAttachment; }

	virtual const char *GetMuzzleFlashParticle() { return "npc_sentry_ground_muzzleflash"; }
	virtual int GetMuzzleAttachment() { return m_iMuzzleAttachment; }

	DECLARE_DATADESC();

private:

	bool m_bPlayingMotor;
	bool m_bMuzzleFlashActive;
};
