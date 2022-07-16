//==============================================================================
//
// Purpose: The base class for Black Mesa weapons.
// 
//==============================================================================

#include "cbase.h"
#include "weapon_bm_base.h"
#include "c_weapon__stubs.h"
#include "c_basehlcombatweapon.h"
#include "activitylist.h"
#include "functionproxy.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

// Actual MAX_GAUSS_CHARGE_TIME is 3 and GAUSS_OVERCHARGE_TIME is 10
#define	MAX_GAUSS_VISUAL_CHARGE_TIME		10

//-----------------------------------------------------------------------------
// C_Weapon_BM_Gauss
//-----------------------------------------------------------------------------
class C_Weapon_BM_Gauss : public CBase_BM_Weapon<C_BaseHLCombatWeapon>
{
public:
	DECLARE_CLASS( C_Weapon_BM_Gauss, CBase_BM_Weapon<C_BaseHLCombatWeapon> );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_Weapon_BM_Gauss( void );

	void Spawn();
	void Activate();

public:

	float m_flCannonChargeStartTime;

	Activity m_OverchargedActivity;
};

IMPLEMENT_CLIENTCLASS_DT( C_Weapon_BM_Gauss, DT_Weapon_BM_Gauss, CWeapon_BM_Gauss )
	RecvPropFloat( RECVINFO( m_flCannonChargeStartTime ) ),
END_RECV_TABLE()

STUB_WEAPON_CLASS_IMPLEMENT( weapon_bm_gauss, C_Weapon_BM_Gauss );
PRECACHE_WEAPON_REGISTER( weapon_bm_gauss );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_Weapon_BM_Gauss::C_Weapon_BM_Gauss( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Weapon_BM_Gauss::Spawn()
{
	BaseClass::Spawn();

	Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Weapon_BM_Gauss::Activate()
{
	BaseClass::Activate();

	m_OverchargedActivity = (Activity)ActivityList_RegisterPrivateActivity( "ACT_VM_OVERCHARGED" );
}

//-----------------------------------------------------------------------------
// Apply effects when the tau cannon is charging
//-----------------------------------------------------------------------------
class CTauChargeMaterialProxy : public CResultProxy
{
public:
	CTauChargeMaterialProxy() {}
	virtual ~CTauChargeMaterialProxy() {}
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
};

bool CTauChargeMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if (!CResultProxy::Init( pMaterial, pKeyValues ))
		return false;

	return true;
}

void CTauChargeMaterialProxy::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );

	C_Weapon_BM_Gauss *pGauss = NULL;
	if ( pEntity->IsBaseCombatWeapon() )
		pGauss = assert_cast<C_Weapon_BM_Gauss*>( pEntity );
	else
	{
		C_BaseViewModel *pVM = dynamic_cast<C_BaseViewModel*>( pEntity );
		if (pVM)
			pGauss = assert_cast<C_Weapon_BM_Gauss*>( pVM->GetOwningWeapon() );
	}

	if ( pGauss && pGauss->m_flCannonChargeStartTime > 0.0f )
	{
		float flChargeAmount = ( gpGlobals->curtime - pGauss->m_flCannonChargeStartTime ) / MAX_GAUSS_VISUAL_CHARGE_TIME;
		SetFloatResult( flChargeAmount );
	}
	else
	{
		SetFloatResult( 0.0f );
	}
}

EXPOSE_INTERFACE( CTauChargeMaterialProxy, IMaterialProxy, "TauCharge" IMATERIAL_PROXY_INTERFACE_VERSION );
