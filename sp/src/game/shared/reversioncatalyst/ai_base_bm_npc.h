//==============================================================================
//
// Purpose: The base class for Black Mesa NPCs, which are created from scratch based on HL2 NPCs.
// 
//==============================================================================

#ifndef AI_BASE_BM_NPC
#define AI_BASE_BM_NPC
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "reversioncatalyst/rc_bm_character_manifest.h"
#ifdef CLIENT_DLL
#include "c_ai_basenpc.h"
#else
#include "saverestore_utlvector.h"
#include "ai_basenpc.h"
#include "props.h"
#include "particle_parse.h"
#endif

template <class BASE_NPC>
class CAI_Base_BM_NPC : public BASE_NPC
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_NPC, BASE_NPC );

public:
	virtual bool	IsBlackMesa() { return true; }
};

template <class BASE_NPC>
class CAI_Base_BM_Gibbable : public CAI_Base_BM_NPC<BASE_NPC>
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_Gibbable, CAI_Base_BM_NPC<BASE_NPC> );

public:

#ifndef CLIENT_DLL
	void SelectAndApplyCharacter();

	void	Precache();

	bool	ShouldGib( const CTakeDamageInfo &info );
	bool	CorpseGib( const CTakeDamageInfo &info );

	const char *GetGibParticle();
#endif
};

template <class BASE_NPC>
class CAI_Base_BM_Human : public CAI_Base_BM_Gibbable<BASE_NPC>
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_Human, CAI_Base_BM_Gibbable<BASE_NPC> );

public:

	CAI_Base_BM_Human() { m_iCharacterIndex = -1; }

	virtual const char *GetCharacterClassname() { return GetClassname(); }

#ifdef CLIENT_DLL
	bool	SetupGlobalWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	void	ApplyCharacterFlexes();

	int		m_iCharacterIndex;
	CUtlVector<LocalFlexController_t>	m_iCharacterFlexes;
#else
	void SelectAndApplyCharacter();

	CNetworkVar( int, m_iCharacterIndex ); // CNetworkVarForDerived
#endif

#ifndef CLIENT_DLL
protected:
	bool m_bCustomBody;

	CUtlVector<EHANDLE> m_hBonemergeProps;
#endif
};

#define DECLARE_BM_NPC_DATADESC() // unused

#define DECLARE_BM_HUMAN_DATADESC() \
	DEFINE_KEYFIELD( m_bCustomBody, FIELD_BOOLEAN, "CustomBody" ),	\
	DEFINE_UTLVECTOR( m_hBonemergeProps, FIELD_EHANDLE ), \

//------------------------------------------------------------------------------

#ifdef CLIENT_DLL
template <class BASE_NPC>
bool CAI_Base_BM_Human<BASE_NPC>::SetupGlobalWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights )
{
	CStudioHdr *hdr = this->GetModelPtr();
	if ( !hdr )
		return false;

	this->ApplyCharacterFlexes();

	return BaseClass::SetupGlobalWeights( pBoneToWorld, nFlexWeightCount, pFlexWeights, pFlexDelayedWeights );
}

template <class BASE_NPC>
void CAI_Base_BM_Human<BASE_NPC>::ApplyCharacterFlexes()
{
	if (this->m_iCharacterIndex == -1)
		return;

	const Character_t &pCharacter = g_CharacterManifestSystem.GetCharacter( this->m_iCharacterIndex );
	if (this->m_iCharacterFlexes.Count() == 0)
	{
		// Parse the flex controller indices
		for (unsigned int i = 0; i < pCharacter.flexData.Count(); i++)
		{
			this->m_iCharacterFlexes.AddToTail( this->FindFlexController( pCharacter.flexData.GetElementName( i ) ) );
		}
	}

	for (unsigned int i = 0; i < pCharacter.flexData.Count(); i++)
	{
		this->SetFlexWeight( this->m_iCharacterFlexes[i], pCharacter.flexData[i] );
	}
}
#else
template <class BASE_NPC>
void CAI_Base_BM_Human<BASE_NPC>::SelectAndApplyCharacter()
{
	if (this->m_bCustomBody)
	{
		this->m_iCharacterIndex = -1;
		return;
	}

	this->m_iCharacterIndex = g_CharacterManifestSystem.SelectCharacterFromEntity( STRING(this->GetEntityName()), this->GetCharacterClassname() );

	if (this->m_iCharacterIndex != -1)
	{
		const Character_t &pCharacter = g_CharacterManifestSystem.GetCharacter( this->m_iCharacterIndex );

		if (pCharacter.pszModel[0])
		{
			this->SetModelName( AllocPooledString( pCharacter.pszModel ) );
			this->SetModel( pCharacter.pszModel );
		}

		this->m_nSkin = pCharacter.skins[RandomInt( 0, pCharacter.skins.Count()-1 )];

		// Bodygroups
		for (unsigned int i = 0; i < pCharacter.bodygroupData.Count(); i++)
		{
			int iBody = this->FindBodygroupByName( pCharacter.bodygroupData.GetElementName( i ) );
			if (iBody != -1)
			{
				// Select one of the random bodygroup indices
				this->SetBodygroup( iBody, pCharacter.bodygroupData[i][ RandomInt(0, pCharacter.bodygroupData[i].Count()-1 ) ] );
			}
		}

		// Bonemerge
		for (int i = 0; i < pCharacter.bonemergeData.Count(); i++)
		{
			CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "prop_dynamic_override", this->GetLocalOrigin(), this->GetLocalAngles(), this );
			if (pEnt)
			{
				pEnt->SetModelName( AllocPooledString( pCharacter.bonemergeData[i] ) );
				DispatchSpawn( pEnt );
				pEnt->FollowEntity( this, true );
			}

			this->m_hBonemergeProps.AddToTail( pEnt );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Gibbable<BASE_NPC>::Precache()
{
	BaseClass::Precache();
	
	const char *pszGibParticle = this->GetGibParticle();
	if ( pszGibParticle != NULL )
		PrecacheParticleSystem( pszGibParticle );

	this->PrecacheScriptSound( "BaseCombatCharacter.CorpseGib" );

	PropBreakablePrecacheAll( this->GetModelName() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Gibbable<BASE_NPC>::ShouldGib( const CTakeDamageInfo &info )
{
	if ( this->IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB) )
		return true;

	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_PREVENT_PHYSICS_FORCE|DMG_DISSOLVE) )
		return false;

	if (info.GetDamageType() & DMG_BLAST)
	{
		if (this->m_iHealth < -20)
			return true;
	}
	else if (info.GetDamageType() & DMG_BULLET)
	{
		if (this->m_iHealth < -80)
			return true;
	}
	else
	{
		if (this->m_iHealth < -50)
			return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Gibbable<BASE_NPC>::CorpseGib( const CTakeDamageInfo &info )
{
	const char *pszGibParticle = this->GetGibParticle();
	if ( pszGibParticle != NULL )
		DispatchParticleEffect( pszGibParticle, this->WorldSpaceCenter(), QAngle( 0, 0, 0 ) );

	EmitSound( "BaseCombatCharacter.CorpseGib" );

	Vector velocity = (info.GetDamageForce() * 0.01f) + this->GetAbsVelocity(); // info.GetDamageForce().Normalized()
	AngularImpulse	angVelocity = RandomAngularImpulse( -150, 150 );
	breakablepropparams_t params( this->EyePosition(), this->GetAbsAngles(), velocity, angVelocity );
	params.impactEnergyScale = 1.0f;
	params.defBurstScale = 500.0f;
	params.defCollisionGroup = COLLISION_GROUP_DEBRIS;
	PropBreakableCreateAll( this->GetModelIndex(), NULL, params, this, -1, true, true );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
const char *CAI_Base_BM_Gibbable<BASE_NPC>::GetGibParticle()
{
	switch (this->BloodColor())
	{
		case BLOOD_COLOR_RED:		return "gib_human_spurt";

		case BLOOD_COLOR_ZOMBIE:
		case BLOOD_COLOR_YELLOW:
		case BLOOD_COLOR_GREEN:		return "gib_alien_spurt";

		default:					return NULL;
	}
}
#endif

#endif // AI_BASE_BM_NPC
