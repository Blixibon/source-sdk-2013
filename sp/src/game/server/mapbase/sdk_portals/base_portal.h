//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Base portal recreated using public Source SDK 2013 code only
// 
//			Based directly on SDK Stencil Portals by Flopgop
//			https://github.com/Flopgop/source-sdk-portals
//
// Author:	Blixibon with code from Flopgop as a basis
//
//===========================================================================//

#ifndef BASE_PORTAL_H
#define BASE_PORTAL_H

#include "baseentity.h"
#include "positionwatcher.h"

class CEnvMicrophone;
class CPortalMimicEntity;

class CBasePortal : public CBaseEntity, public IPositionWatcher
{
public:
	DECLARE_CLASS( CBasePortal, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CBasePortal();
	~CBasePortal();

	int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_PVSCHECK ); // FL_EDICT_ALWAYS
	}

	virtual void Activate( void );
	virtual void Spawn();
	virtual void UpdateOnRemove();

	virtual void Open( CBaseEntity *pActivator );
	virtual void Close( CBaseEntity *pActivator );

	inline bool HasPartner() const { return (m_hLinkedPortal != NULL); }
	inline CBasePortal *GetPartner() const { return m_hLinkedPortal; }
	void		SetPartner( CBasePortal *pPortalPartner );
	
	inline float GetWidth() const { return m_fHalfWidth; }
	inline float GetHeight() const { return m_fHalfHeight; }
	inline bool IsActive() const { return m_bActivated; }

	bool		FInViewCone( CBaseEntity *pEntity ) { return FInViewCone( pEntity->EyePosition() ); }
	bool		FInViewCone( const Vector &vecSpot );

	void		InputSetPartner( inputdata_t &inputdata );
	void		InputSetWidth( inputdata_t &inputdata ) { m_fHalfWidth = inputdata.value.Float(); }
	void		InputSetHeight( inputdata_t &inputdata ) { m_fHalfHeight = inputdata.value.Float(); }
	void		InputOpen( inputdata_t &inputdata ) { Open( inputdata.pActivator ); }
	void		InputClose( inputdata_t &inputdata ) { Close( inputdata.pActivator ); }
	void		InputEnablePhysics( inputdata_t &inputdata ) { m_bDisablePhysics = false; }
	void		InputEnableVisuals( inputdata_t &inputdata ) { m_bDisableVisuals = false; }
	void		InputDisablePhysics( inputdata_t &inputdata ) { m_bDisablePhysics = true; }
	void		InputDisableVisuals( inputdata_t &inputdata ) { m_bDisableVisuals = true; }
	void		InputSetPortalFilter( inputdata_t &inputdata );
	void		InputSetScale( inputdata_t &inputdata ) { m_flScale = inputdata.value.Float(); }
	void		InputEnableScaleEntities( inputdata_t &inputdata ) { m_bScaleEntities = true; }
	void		InputDisableScaleEntities( inputdata_t &inputdata ) { m_bScaleEntities = false; }
	void		InputTeleportEntity( inputdata_t &inputdata );
	void		InputTeleportEntityForce( inputdata_t &inputdata );

	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual bool GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen );

	// Portal stubs
	const VMatrix &MatrixThisToLinked() const;
	bool		IsActivedAndLinked() { return IsActive() && HasPartner(); }

	void			ApplyScale( Vector &vecOut );

	virtual bool	TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr );

	void			EntitiesTouchingThink();
	virtual void	StartTouch( CBaseEntity *pOther );
	virtual void	EndTouch( CBaseEntity *pOther );
	bool			IsEntityTouching( CBaseEntity *pEntity );

	void			PushEntitiesNearLinked( CBaseEntity *pEntity );

	virtual bool	PassesTriggerFilters( CBaseEntity *pOther );

	bool			IsBehindPortal( const Vector &vecOrigin, const Vector &vecForward );
	virtual bool	ShouldTeleportEntity( CBaseEntity *pOther, const Vector &vecForward );
	virtual bool	VerifyOutPosition( CBaseEntity *pOther, Vector &vecOrigin );
	virtual void	TeleportEntity( CBaseEntity *pOther, bool bForce = false );
	virtual void	OnTeleportFrom( CBaseEntity *pOther );
	virtual void	OnTeleportTo( CBaseEntity *pOther );

	void		 CleanupMimicEntities();

	// IPositionWatcher
	virtual void NotifyPositionChanged( CBaseEntity *pEntity );
public:

	string_t m_sPartnerName;
	CNetworkHandle( CBasePortal, m_hLinkedPortal );
	CNetworkVar( float, m_fHalfWidth );
	CNetworkVar( float, m_fHalfHeight );
	CNetworkVar( bool, m_bActivated );
	CNetworkVar( bool, m_bDisablePhysics );
	CNetworkVar( bool, m_bDisableVisuals );

	CNetworkVar( float, m_flScale );
	bool	m_bScaleEntities;

	float						m_flMicRange;
	CHandle<CEnvMicrophone>		m_hMicrophone;

	string_t		m_iszPortalFilter;
	EHANDLE			m_hPortalFilter;

	COutputEvent	m_OnOpen;
	COutputEvent	m_OnClose;
	COutputEvent	m_OnEntityTeleportFromMe;
	COutputEvent	m_OnPlayerTeleportFromMe;
	COutputEvent	m_OnEntityTeleportToMe;
	COutputEvent	m_OnPlayerTeleportToMe;

	CUtlVector< EHANDLE >						m_hTouchingEntities;
#ifdef SERVERSIDE_PORTAL_MIMIC
	CUtlVector< CHandle<CPortalMimicEntity> >	m_hMimicEntities;		// Entities on the other portal that reflect the ones touching this one
#endif

	// Referenced by code in SDK
	cplane_t		m_plane_Origin;
	Vector			m_vPortalCorners[4];

	// Caching the position of the linked portal
	bool			m_bCachedLinkedPortal;
	VMatrix			m_matCachedThisToLinked;

	static CUtlVector<CBasePortal*>	AllPortals;
};

#endif
