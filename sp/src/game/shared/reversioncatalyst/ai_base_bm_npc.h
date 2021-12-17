//==============================================================================
//
// Purpose: The base class for Black Mesa NPCs, which are created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "reversioncatalyst/rc_bm_character_manifest.h"
#ifdef CLIENT_DLL
#include "c_ai_basenpc.h"
#else
#include "saverestore_utlvector.h"
#include "ai_basenpc.h"
#endif

template <class BASE_NPC>
class CAI_Base_BM_NPC : public BASE_NPC
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_NPC, BASE_NPC );

public:
	virtual bool	IsBlackMesa() { return true; }
};

template <class BASE_NPC>
class CAI_Base_BM_Human : public CAI_Base_BM_NPC<BASE_NPC>
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_Human, CAI_Base_BM_NPC<BASE_NPC> );

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
#endif
