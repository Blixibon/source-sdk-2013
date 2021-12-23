//==============================================================================
//
// Purpose: Black Ops assassins created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "c_npc_bm_human_assassin.h"
#include "particle_parse.h"
#include "functionproxy.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// forward declarations
void ToolFramework_RecordMaterialParams( IMaterial *pMaterial );

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BM_HumanFemaleAssassin, DT_NPC_BM_HumanFemaleAssassin, CNPC_BM_HumanFemaleAssassin )
	RecvPropInt( RECVINFO( m_iCharacterIndex ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_BM_HumanFemaleAssassin::C_NPC_BM_HumanFemaleAssassin()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_NPC_BM_HumanFemaleAssassin::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if (type == DATA_UPDATE_CREATED)
	{
		// Turn on eye glows
		DispatchParticleEffect( "npc_assassin_eyeglow", PATTACH_POINT_FOLLOW, this, LookupAttachment( "eye1" ) );
		DispatchParticleEffect( "npc_assassin_eyeglow", PATTACH_POINT_FOLLOW, this, LookupAttachment( "eye2" ) );
	}
	else if (m_lifeState == LIFE_DEAD)
	{
		StopParticleEffects( this );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT( C_NPC_BM_HumanMaleAssassin, DT_NPC_BM_HumanMaleAssassin, CNPC_BM_HumanMaleAssassin )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_BM_HumanMaleAssassin::C_NPC_BM_HumanMaleAssassin()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_NPC_BM_HumanMaleAssassin::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if (type == DATA_UPDATE_CREATED)
	{
		// If we have the headset, turn on eye glows
		if (GetBodygroup( FindBodygroupByName( "head" ) ) == 3)
		{
			DispatchParticleEffect( "npc_assassin_eyeglow", PATTACH_POINT_FOLLOW, this, LookupAttachment( "eye1" ) );
			DispatchParticleEffect( "npc_assassin_eyeglow", PATTACH_POINT_FOLLOW, this, LookupAttachment( "eye2" ) );
		}
	}
	else if (m_lifeState == LIFE_DEAD)
	{
		StopParticleEffects( this );
	}
}

//-----------------------------------------------------------------------------
// Controls assassin cloak
//-----------------------------------------------------------------------------
class CProxyAssassinCloak : public CResultProxy
{
public:
	bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	void OnBind( void *pC_BaseEntity );
};

bool CProxyAssassinCloak::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if (!CResultProxy::Init( pMaterial, pKeyValues ))
		return false;

	return true;
}

void CProxyAssassinCloak::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
	if ( pEntity && pEntity->IsNPC() )
	{
		C_NPC_BM_HumanFemaleAssassin *pAssassin = assert_cast<C_NPC_BM_HumanFemaleAssassin *>( pEntity );
		if ( pAssassin )
		{
			SetFloatResult( pAssassin->m_flCloakFactor );
		}
	}
	else
	{
		SetFloatResult( 0.0f );
	}
	if ( ToolsEnabled() )
	{
		ToolFramework_RecordMaterialParams( GetMaterial() );
	}
}

EXPOSE_INTERFACE( CProxyAssassinCloak, IMaterialProxy, "AssassinCloak" IMATERIAL_PROXY_INTERFACE_VERSION );
