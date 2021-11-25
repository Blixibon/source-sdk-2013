#ifndef _INCLUDE_ASW_FX_SHARED_H
#define _INCLUDE_ASW_FX_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#ifndef SWARM_PORT
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#endif
#endif

void UTIL_ASW_BloodImpact( const Vector &pos, const Vector &dir, int color, int amount );
void UTIL_ASW_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount );
#ifndef SWARM_PORT
void UTIL_ASW_MarineTakeDamage( const Vector &origin, const Vector &direction, int color, int amount, CASW_Marine *pMarine, bool bFriendly = false );
#endif
void UTIL_ASW_DroneBleed( const Vector &pos, const Vector &dir, int amount );
void UTIL_ASW_EggGibs( const Vector &pos, int iFlags, int iEntIndex );
void UTIL_ASW_BuzzerDeath( const Vector &pos );
void UTIL_ASW_GrenadeExplosion( const Vector &vecPos, float flRadius );
void UTIL_ASW_EnvExplosionFX( const Vector &vecPos, float flRadius, bool bOnGround );

#if SWARM17 && CLIENT_DLL
class CDroneGibManager : public CAutoGameSystem
{
public:
	// Methods of IGameSystem
	virtual void Update( float frametime );
	virtual void LevelInitPreEntity( void );

	void	AddGib( C_BaseEntity *pEntity ); 
	void	RemoveGib( C_BaseEntity *pEntity );

private:
	typedef CHandle<C_BaseEntity> CGibHandle;
	CUtlLinkedList< CGibHandle > m_LRU; 
};

void FX_DroneBleed( const Vector &origin, const Vector &direction, float scale );
void FX_GibMeshEmitter( const char *szModel, const char *szTemplate, const Vector &origin, const Vector &direction, int skinm, float fScale=1.0f, bool bFrozen = false );
void FX_GrubGib( const Vector &origin, const Vector &direction, float scale, bool bOnFire );
void FX_DroneGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bOnFire );
void FX_HarvesterGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bOnFire );
void FX_ParasiteGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bUseGibImpactSounds, bool bOnFire );
void FX_EggGibs( const Vector &origin, int flags, int iEntIndex );
void FX_QueenSpitBurst( const Vector &origin, const Vector &direction, float scale, int skin );
#endif

#endif // _INCLUDE_ASW_FX_SHARED_H