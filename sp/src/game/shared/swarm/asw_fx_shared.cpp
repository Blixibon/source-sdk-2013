#include "cbase.h"
#include "mathlib/mathlib.h"
#include "util_shared.h"
//#include "model_types.h"
#include "convar.h"
#include "IEffects.h"
//#include "vphysics/object_hash.h"
//#include "IceKey.H"
//#include "checksum_crc.h"
#include "asw_fx_shared.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
#ifdef SWARM17
	#include "c_gib.h"
	#include "c_asw_egg.h"
	#include "c_user_message_register.h"
#endif
#else
	#include "te_effect_dispatch.h"
	#include "Sprite.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
// link clientside sprite to CSprite - can remove this if we fixup all the maps to not have _clientside ones (do when we're sure we don't need clientside sprites)
LINK_ENTITY_TO_CLASS( env_sprite_clientside, CSprite );
#endif

void UTIL_ASW_EggGibs( const Vector &pos, int iFlags, int iEntIndex )
{
#ifdef GAME_DLL
	Vector vecOrigin = pos;
	CPASFilter filter( vecOrigin );
	UserMessageBegin( filter, "ASWEggEffects" );
	WRITE_FLOAT( vecOrigin.x );
	WRITE_FLOAT( vecOrigin.y );
	WRITE_FLOAT( vecOrigin.z );
	WRITE_SHORT( iFlags );
	WRITE_SHORT( iEntIndex );
	MessageEnd();
#endif
}

void UTIL_ASW_BuzzerDeath( const Vector &pos )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = pos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWBuzzerDeath" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	MessageEnd();
#endif
}

void UTIL_ASW_DroneBleed( const Vector &pos, const Vector &dir, int amount )
{
	CEffectData	data;

	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	//data.m_flScale = (float)amount;

	// todo: use filter?
	QAngle	vecAngles;
	VectorAngles( data.m_vNormal, vecAngles );
	DispatchParticleEffect( "drone_shot", data.m_vOrigin, vecAngles );

	//DispatchEffect( "DroneBleed", data );
}

void UTIL_ASW_BloodImpact( const Vector &pos, const Vector &dir, int color, int amount )
{
	CEffectData	data;

	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	data.m_flScale = (float)amount;
	data.m_nColor = (unsigned char)color;

	DispatchEffect( "ASWBloodImpact", data );
}

void UTIL_ASW_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( g_Language.GetInt() == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	if ( amount > 255 )
		amount = 255;

	if (color == BLOOD_COLOR_MECH)
	{
		g_pEffects->Sparks(origin);
		if (random->RandomFloat(0, 2) >= 1)
		{
			UTIL_Smoke(origin, random->RandomInt(10, 15), 10);
		}
	}
	else
	{
		// Normal blood impact
		//UTIL_ASW_BloodImpact( origin, direction, color, amount );
		QAngle	vecAngles;
		VectorAngles( direction, vecAngles );
		if ( amount < 4 )
			DispatchParticleEffect( "marine_bloodsplat_light", origin, vecAngles );
		else
			DispatchParticleEffect( "marine_bloodsplat_heavy", origin, vecAngles );
	}
}

#ifndef SWARM17
void UTIL_ASW_MarineTakeDamage( const Vector &origin, const Vector &direction, int color, int amount, CASW_Marine *pMarine, bool bFriendly )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( g_Language.GetInt() == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	if ( amount > 255 )
		amount = 255;

	// TODO: use amount to determine large versus small attacks taken?
	QAngle	vecAngles;
	VectorAngles( -direction, vecAngles );
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

#ifdef CLIENT_DLL
	const char *pchEffectName = NULL;
	if ( bFriendly )
		pchEffectName = "marine_hit_blood_ff";
	else
		pchEffectName = "marine_hit_blood";

	CUtlReference< CNewParticleEffect > pEffect;
	pEffect = pMarine->ParticleProp()->Create( pchEffectName, PATTACH_CUSTOMORIGIN );

	if ( pEffect )
	{
		pMarine->ParticleProp()->AddControlPoint( pEffect, 2, pMarine, PATTACH_ABSORIGIN_FOLLOW );
		pEffect->SetControlPoint( 0, origin );//origin - pMarine->GetAbsOrigin()
		pEffect->SetControlPointOrientation( 0, vecForward, vecRight, vecUp );
	}
	else
	{
		Warning( "Could not create effect for marine hurt: %s", pchEffectName );
	}
#endif
}
#endif

void UTIL_ASW_GrenadeExplosion( const Vector &vecPos, float flRadius )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = vecPos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWGrenadeExplosion" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	WRITE_FLOAT( flRadius );
	MessageEnd();
#endif
}

void UTIL_ASW_EnvExplosionFX( const Vector &vecPos, float flRadius, bool bOnGround )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = vecPos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWEnvExplosionFX" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	WRITE_FLOAT( flRadius );
	WRITE_BOOL( bOnGround );
	//damage.bFriendlyFire = msg.ReadOneBit() ? true : false;
	MessageEnd();
#endif
}

#if SWARM17 && CLIENT_DLL
#define ASW_BLOOD_BRIGHTNESS 0.25f
//#define ASW_DO_BLOOD_LIGHT_CALCS		// define to make blood particle systems scale color/alpha by light at that point

//extern void GetBloodColor( int bloodtype, colorentry_t &color );
extern PMaterialHandle g_Material_Spark;
PMaterialHandle g_Material_Blue_nocull;

// ==========
// Drone Gibs
// ==========

#define	NUM_DRONE_GIBS_UNIQUE	16
const char *pszDroneGibs_Unique[NUM_DRONE_GIBS_UNIQUE] = {
"models/swarm/DroneGibs/dronepart01.mdl",
//"models/swarm\DroneGibs\DronePart02.mdl",	// mouth
"models/swarm/DroneGibs/dronepart20.mdl",
//"models/swarm\DroneGibs\DronePart21.mdl",	// eyeball
"models/swarm/DroneGibs/dronepart29.mdl",
"models/swarm/DroneGibs/dronepart31.mdl",
"models/swarm/DroneGibs/dronepart32.mdl",
//"models/swarm\DroneGibs\DronePart36.mdl",	// wing
//"models/swarm\DroneGibs\DronePart37.mdl",	// wing
//"models/swarm\DroneGibs\DronePart38.mdl",		// wings
//"models/swarm\DroneGibs\DronePart39.mdl",		// wings
"models/swarm/DroneGibs/dronepart44.mdl",
"models/swarm/DroneGibs/dronepart45.mdl",
"models/swarm/DroneGibs/dronepart47.mdl",
"models/swarm/DroneGibs/dronepart49.mdl",
"models/swarm/DroneGibs/dronepart50.mdl",
"models/swarm/DroneGibs/dronepart53.mdl",
"models/swarm/DroneGibs/dronepart54.mdl",
"models/swarm/DroneGibs/dronepart56.mdl",
"models/swarm/DroneGibs/dronepart57.mdl",
"models/swarm/DroneGibs/dronepart58.mdl",
"models/swarm/DroneGibs/dronepart59.mdl"
};

ConVar g_drone_maxgibs( "g_drone_maxgibs", "16", FCVAR_ARCHIVE );

void CDroneGibManager::LevelInitPreEntity( void )
{
	m_LRU.Purge();
}

CDroneGibManager s_DroneGibManager;

void CDroneGibManager::AddGib( C_BaseEntity *pEntity )
{
	m_LRU.AddToTail( pEntity );
}

void CDroneGibManager::RemoveGib( C_BaseEntity *pEntity )
{
	m_LRU.FindAndRemove( pEntity );
}
	

//-----------------------------------------------------------------------------
// Methods of IGameSystem
//-----------------------------------------------------------------------------
void CDroneGibManager::Update( float frametime )
{
	if ( m_LRU.Count() < g_drone_maxgibs.GetInt() )
		 return;
	
	int i = 0;
	i = m_LRU.Head();

	if ( m_LRU[ i ].Get() )
	{
		 m_LRU[ i ].Get()->SetNextClientThink( gpGlobals->curtime );
	}

	m_LRU.Remove(i);
}

// Drone gib - marks surfaces when it bounces

class C_DroneGib : public C_Gib
{
	typedef C_Gib BaseClass;
public:
	
	static C_DroneGib *C_DroneGib::CreateClientsideGib( const char *pszModelName,
		Vector vecOrigin, Vector vecForceDir, AngularImpulse vecAngularImp,
		float m_flLifetime = DEFAULT_GIB_LIFETIME, int skin=0 )
	{
		C_DroneGib *pGib = new C_DroneGib;

		if ( pGib == NULL )
			return NULL;

		if ( pGib->InitializeGib( pszModelName, vecOrigin, vecForceDir, vecAngularImp, m_flLifetime ) == false )
			return NULL;

		pGib->SetSkin( skin );

		s_DroneGibManager.AddGib( pGib );

		// attach an emitter ot it

#ifndef SWARM17
		C_ASW_Emitter *pEmitter = new C_ASW_Emitter;
		if (pEmitter)
		{
			if (pEmitter->InitializeAsClientEntity( NULL, false ))
			{
				// randomly pick a jet, a drip or a burst
				float f = random->RandomFloat();
				if (f < 0.33f)
					Q_snprintf(pEmitter->m_szTemplateName, sizeof(pEmitter->m_szTemplateName), "dronebloodjet");
				else if (f < 0.66f)
					Q_snprintf(pEmitter->m_szTemplateName, sizeof(pEmitter->m_szTemplateName), "dronebloodburst");
				else
					Q_snprintf(pEmitter->m_szTemplateName, sizeof(pEmitter->m_szTemplateName), "droneblooddroplets");
				pEmitter->m_fScale = 1.0f;
				pEmitter->m_bEmit = true;
				pEmitter->SetAbsOrigin(vecOrigin);
				pEmitter->CreateEmitter();
				pEmitter->SetAbsOrigin(vecOrigin + Vector(0,0,30));
				pEmitter->SetAbsAngles(pGib->GetAbsAngles());

				// randomly pick an attach point
				pEmitter->ClientAttach(pGib, "bleed");
				pEmitter->SetDieTime(gpGlobals->curtime + asw_gib_bleed_time.GetFloat());	// stop emitting once the ragdoll gibs
			}
			else
			{
				UTIL_Remove( pEmitter );
			}
		}
#endif

		return pGib;
	}

	// Decal the surface
	virtual	void HitSurface( C_BaseEntity *pOther )
	{

	}
};


void FX_DroneBleed( const Vector &origin, const Vector &direction, float scale )
{
	Vector	offset;

#ifdef ASW_DO_BLOOD_LIGHT_CALCS
	Vector color = engine->GetLightForPointFast(origin, true);
	color.x = 0;
	color.y = LinearToTexture( color.y ) / 255.0f;
	color.z = 0;

	// only use half of lighting	
	color.y += (1.0f - color.y) * ASW_BLOOD_BRIGHTNESS;	
	color.y *= 255.0f;
#else
	Vector color(0, 255, 0);
#endif

	// Throw some blood
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "FX_DroneGib" );
	pSimple->SetSortOrigin( origin );

	PMaterialHandle	hMaterial = pSimple->GetPMaterial( "effects/blood" );

	Vector	vDir;

	vDir.Random( -1.0f, 1.0f );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
			return;

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= 255.0f * ASW_BLOOD_BRIGHTNESS;
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 8, 16 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}

	hMaterial = pSimple->GetPMaterial( "effects/blood2" );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
		{
			return;
		}

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= random->RandomInt( 64, 128 );
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 8, 16 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}
}

void DroneBleedCallback( const CEffectData &data )
{
	FX_DroneBleed( data.m_vOrigin, data.m_vNormal, data.m_flScale );
}

DECLARE_CLIENT_EFFECT( "DroneBleed", DroneBleedCallback );

void FX_GibMeshEmitter( const char *szModel, const char *szTemplate, const Vector &origin, const Vector &direction, int skin, float fScale, bool bFrozen )
{
#ifdef SWARM17
	C_Gib *pGib = C_Gib::CreateClientsideGib( szModel, origin, direction, Vector( RandomFloat( 20.0f, 20.0f ), RandomFloat( 20.0f, 20.0f ), RandomFloat( 20.0f, 20.0f ) ) );
	pGib->SetSkin( skin );
#else
	C_ASW_Mesh_Emitter *pEmitter = new C_ASW_Mesh_Emitter;
	if (pEmitter)
	{
		if (pEmitter->InitializeAsClientEntity( szModel, false ))
		{
			Q_snprintf(pEmitter->m_szTemplateName, sizeof(pEmitter->m_szTemplateName), szTemplate);
			pEmitter->SetSkin( skin );
			pEmitter->m_fScale = fScale;
			pEmitter->m_bEmit = true;
			pEmitter->SetAbsOrigin(origin);			
			pEmitter->CreateEmitter(direction);
			pEmitter->SetAbsOrigin(origin);
			pEmitter->SetDieTime(gpGlobals->curtime + 15.0f);
			pEmitter->SetFrozen( bFrozen );
		}
		else
		{
			UTIL_Remove( pEmitter );
		}
	}
#endif
}

void FX_DroneGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bOnFire )
{
	Vector	offset;
	// Throw some blood
#ifdef ASW_DO_BLOOD_LIGHT_CALCS
	Vector color = engine->GetLightForPointFast(origin, true);
	color.x = 0;
	color.y = LinearToTexture( color.y ) / 255.0f;
	color.z = 0;

	// only use half of lighting
	color.y += (1.0f - color.y) * ASW_BLOOD_BRIGHTNESS;
	color.y *= 255.0f;
#else
	Vector color(0, 255, 0);
#endif

	QAngle	vecAngles;
	VectorAngles( -direction, vecAngles );
	DispatchParticleEffect( "drone_death", origin, vecAngles );

	// make our gib emitters
	Vector vecForce = direction * 100.0f;
	if (bOnFire)
	{		
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart01.mdl", "dronegibfire1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegibfire1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart29.mdl", "dronegibfire1", origin, vecForce, skin);
	}
	else
	{
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart01.mdl", "dronegib1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegib2", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart29.mdl", "dronegib3", origin, vecForce, skin);
	}
}

void DroneGibCallback( const CEffectData &data )
{
	FX_DroneGib( data.m_vOrigin, data.m_vNormal, data.m_flScale, data.m_nColor, (data.m_fFlags & ASW_GIBFLAG_ON_FIRE) );
}

DECLARE_CLIENT_EFFECT( "DroneGib", DroneGibCallback );



void FX_HarvesterGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bOnFire )
{
	Vector	offset;
#ifdef ASW_DO_BLOOD_LIGHT_CALCS
	Vector color = engine->GetLightForPointFast(origin, true);
	color.x = 0;
	color.y = LinearToTexture( color.y ) / 255.0f;
	color.z = 0;

	// only use half of lighting
	color.y += (1.0f - color.y) * ASW_BLOOD_BRIGHTNESS;
	color.y *= 255.0f;
#else
	Vector color(0, 255, 0);
#endif

	// Throw some blood
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "FX_HarvesterGib" );
	pSimple->SetSortOrigin( origin );

	PMaterialHandle	hMaterial = pSimple->GetPMaterial( "effects/blood" );
	Vector	vDir;
	vDir.Random( -1.0f, 1.0f );
	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
			return;

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= 255.0f * ASW_BLOOD_BRIGHTNESS;
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}

	hMaterial = pSimple->GetPMaterial( "effects/blood2" );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
		{
			return;
		}

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= random->RandomInt( 64, 128 );
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}

	// make our gib emitters
	Vector vecForce = direction * 100.0f;
	if (bOnFire)
	{		
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegibfire1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegibfire1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart59.mdl", "dronegibfire1", origin, vecForce, skin);
	}
	else
	{
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegib1", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart58.mdl", "dronegib2", origin, vecForce, skin);
		FX_GibMeshEmitter("models/swarm/DroneGibs/dronepart59.mdl", "dronegib3", origin, vecForce, skin);
	}

	CLocalPlayerFilter filter;
	CSoundParameters params;

	// make a gib sound
	if ( C_BaseEntity::GetParametersForSound( "ASW_Drone.GibSplat", params, NULL ) )
	{
		EmitSound_t ep( params );
		
		ep.m_flVolume = 1.0f;
		ep.m_nChannel = CHAN_AUTO;
		ep.m_pOrigin = &origin;

		C_BaseEntity::EmitSound( filter, 0, ep );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void HarvesterGibCallback( const CEffectData &data )
{
	FX_HarvesterGib( data.m_vOrigin, data.m_vNormal, data.m_flScale, data.m_nColor, (data.m_fFlags & ASW_GIBFLAG_ON_FIRE) );
}

DECLARE_CLIENT_EFFECT( "HarvesterGib", HarvesterGibCallback );

// Grub Gibs

#define	NUM_GRUB_GIBS_UNIQUE	6
const char *pszGrubGibs_Unique[NUM_GRUB_GIBS_UNIQUE] = {
"models/Swarm/Grubs/GrubGib1.mdl",
"models/Swarm/Grubs/GrubGib2.mdl",
"models/Swarm/Grubs/GrubGib3.mdl",
"models/Swarm/Grubs/GrubGib4.mdl",
"models/Swarm/Grubs/GrubGib5.mdl",
"models/Swarm/Grubs/GrubGib6.mdl"
};

void FX_GrubGib( const Vector &origin, const Vector &direction, float scale, bool bOnFire )
{
	Vector offset = origin + Vector(0,0,8);
	if (bOnFire)
		DispatchParticleEffect( "grub_death_fire", offset, QAngle( 0, 0, 0 ) );
	else
		DispatchParticleEffect( "grub_death", offset, QAngle( 0, 0, 0 ) );
	
	CLocalPlayerFilter filter;						
	CSoundParameters params;

	// make a gib sound
	if ( C_BaseEntity::GetParametersForSound( "ASW_Drone.GibSplatQuiet", params, NULL ) )
	{
		EmitSound_t ep( params );
		ep.m_pOrigin = &origin;

		C_BaseEntity::EmitSound( filter, 0, ep );
	}
}

void GrubGibCallback( const CEffectData &data )
{
	FX_GrubGib( data.m_vOrigin, data.m_vNormal, data.m_flScale, (data.m_fFlags & ASW_GIBFLAG_ON_FIRE) );
}

DECLARE_CLIENT_EFFECT( "GrubGib", GrubGibCallback );

// ==========
// Parasite Gibs
// ==========

#define	NUM_PARASITE_GIBS_UNIQUE	8
const char *pszParasiteGibs_Unique[NUM_PARASITE_GIBS_UNIQUE] = {
"models/swarm/Parasite/ParasiteGibAbdomen.mdl",
"models/swarm/Parasite/ParasiteGibHead.mdl",
"models/swarm/Parasite/ParasiteGibFrontLeg.mdl",
"models/swarm/Parasite/ParasiteGibMidLeg.mdl",
"models/swarm/Parasite/ParasiteGibBackLeg.mdl",
"models/swarm/Parasite/ParasiteGibFrontLeg.mdl",
"models/swarm/Parasite/ParasiteGibMidLeg.mdl",
"models/swarm/Parasite/ParasiteGibBackLeg.mdl"
};

void FX_ParasiteGib( const Vector &origin, const Vector &direction, float scale, int skin, bool bUseGibImpactSounds, bool bOnFire )
{
	Vector	offset;
	// make our gib emitters.
	Vector vecForce = direction;
	vecForce.z = 1.0f;
	vecForce *= 100.0f;
	Vector gibspot = origin + Vector(0,0,5);
	if (bUseGibImpactSounds)
	{
		if (bOnFire)
		{
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibMidLeg.mdl", "parasitegibfire1", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibHead.mdl", "parasitegibfire1", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibAbdomen.mdl", "parasitegibfire1", gibspot, vecForce, 0);
		}
		else
		{
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibMidLeg.mdl", "parasitegib2", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibHead.mdl", "parasitegib1", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibAbdomen.mdl", "parasitegib1", gibspot, vecForce, 0);
		}
	}
	else
	{
		if (bOnFire)
		{
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibMidLeg.mdl", "parasitegibfire1quiet", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibHead.mdl", "parasitegibfire1quiet", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibAbdomen.mdl", "parasitegibfire1quiet", gibspot, vecForce, 0);
		}
		else
		{
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibMidLeg.mdl", "parasitegib2quiet", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibHead.mdl", "parasitegib1quiet", gibspot, vecForce, 0);
			FX_GibMeshEmitter("Models/Swarm/Parasite/ParasiteGibAbdomen.mdl", "parasitegib1quiet", gibspot, vecForce, 0);
		}
	}
	
	CLocalPlayerFilter filter;						
	CSoundParameters params;

	// make a gib sound
	if ( C_BaseEntity::GetParametersForSound( "ASW_Drone.GibSplatQuiet", params, NULL ) )
	{
		EmitSound_t ep( params );
		ep.m_pOrigin = &origin;

		C_BaseEntity::EmitSound( filter, 0, ep );
	}

	// Throw some blood

	QAngle	vecAngles;
	VectorAngles( direction, vecAngles );
	DispatchParticleEffect( "drone_shot", origin, vecAngles );

	/*
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "FX_DroneGib" );
	pSimple->SetSortOrigin( origin );

	PMaterialHandle	hMaterial = pSimple->GetPMaterial( "effects/blood" );

	Vector	vDir;

#ifdef ASW_DO_BLOOD_LIGHT_CALCS
	Vector color = engine->GetLightForPointFast(origin, true);
	color.x = 0;
	color.y = LinearToTexture( color.y ) / 255.0f;
	color.z = 0;

	// only use half of lighting
	color.y += (1.0f - color.y) * ASW_BLOOD_BRIGHTNESS;
	color.y *= 255.0f;
#else
	Vector color(0, 255, 0);
#endif

	vDir.Random( -1.0f, 1.0f );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
			return;

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= 255.0f * ASW_BLOOD_BRIGHTNESS;
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}

	hMaterial = pSimple->GetPMaterial( "effects/blood2" );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );
			
		if ( sParticle == NULL )
		{
			return;
		}

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 0.5f, 0.75f );
			
		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 0;
		sParticle->m_uchColor[1]	= color.y;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= random->RandomInt( 64, 128 );
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void ParasiteGibCallback( const CEffectData &data )
{
	FX_ParasiteGib( data.m_vOrigin, data.m_vNormal, data.m_flScale, data.m_nColor, true, (data.m_fFlags & ASW_GIBFLAG_ON_FIRE) );
}

DECLARE_CLIENT_EFFECT( "ParasiteGib", ParasiteGibCallback );

void HarvesiteGibCallback( const CEffectData &data )
{
	FX_ParasiteGib( data.m_vOrigin, data.m_vNormal, data.m_flScale, data.m_nColor, false, (data.m_fFlags & ASW_GIBFLAG_ON_FIRE) );
}

DECLARE_CLIENT_EFFECT( "HarvesiteGib", HarvesiteGibCallback );

// egg gibs
void FX_EggGibs( const Vector &origin, int flags, int iEntIndex )
{
	C_ASW_Egg *pEgg = dynamic_cast<C_ASW_Egg*>(ClientEntityList().GetEnt(iEntIndex));
	MDLCACHE_CRITICAL_SECTION();
	C_BaseAnimating::PushAllowBoneAccess( true, false, "FX_EggGibs" );

	if (flags & EGG_FLAG_OPEN && pEgg)
	{
		DispatchParticleEffect( "egg_open", PATTACH_POINT_FOLLOW, pEgg, "attach_death" );
	}

	if (flags & EGG_FLAG_HATCH && pEgg)
	{
		DispatchParticleEffect( "egg_hatch", PATTACH_POINT_FOLLOW, pEgg, "attach_death" );
	}

	if (flags & EGG_FLAG_DIE)
	{
		DispatchParticleEffect( "egg_death", origin, QAngle( 0, 0, 0 ) );
	}

	if (flags & EGG_FLAG_GRUBSACK_DIE)
	{
		DispatchParticleEffect( "grubsack_death", origin, QAngle( 0, 0, 0 ) );
	}

	CLocalPlayerFilter filter;						
	CSoundParameters params;

	// make a gib sound
	if ( C_BaseEntity::GetParametersForSound( "ASW_Drone.GibSplatQuiet", params, NULL ) )
	{
		EmitSound_t ep( params );
		ep.m_pOrigin = &origin;

		C_BaseEntity::EmitSound( filter, 0, ep );
	}

	C_BaseAnimating::PopBoneAccess( "FX_EggGibs" );
}


void __MsgFunc_ASWEggEffects( bf_read &msg )
{
	Vector vecOrigin;
	vecOrigin.x = msg.ReadFloat();
	vecOrigin.y = msg.ReadFloat();
	vecOrigin.z = msg.ReadFloat();

	int iFlags = msg.ReadShort();	

	int iEggIndex = msg.ReadShort();		

	FX_EggGibs( vecOrigin, iFlags, iEggIndex );
}
USER_MESSAGE_REGISTER( ASWEggEffects );

/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void EggGibsCallback( const CEffectData &data )
{

	FX_EggGibs( data.m_vOrigin, data.m_fFlags, data.m_nOtherEntIndex );
}

DECLARE_CLIENT_EFFECT( EggGibs, EggGibsCallback );
*/

void __MsgFunc_ASWBuzzerDeath( bf_read &msg )
{
	Vector vecPos;
	vecPos.x = msg.ReadFloat();
	vecPos.y = msg.ReadFloat();
	vecPos.z = msg.ReadFloat();

	DispatchParticleEffect( "buzzer_death", vecPos, Vector( 0, 0, 0 ), QAngle( 0, 0, 0 ) );
}
USER_MESSAGE_REGISTER( ASWBuzzerDeath );
#endif
