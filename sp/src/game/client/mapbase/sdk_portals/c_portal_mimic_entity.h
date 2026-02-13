//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Portal mimic entity recreated using public Source SDK 2013 code only
//
// Author:	Blixibon
//
//===========================================================================//

#ifndef C_PORTAL_MIMIC_ENTITY_H
#define C_PORTAL_MIMIC_ENTITY_H

#include "c_baseanimating.h"
#include "c_base_portal.h"

class C_PortalMimicEntity : public C_BaseAnimating
{
	DECLARE_CLASS( C_PortalMimicEntity, C_BaseAnimating );
#ifndef CLIENTSIDE_PORTAL_MIMIC
	DECLARE_CLIENTCLASS();
#endif
public:
	C_PortalMimicEntity();
	~C_PortalMimicEntity();

#ifndef CLIENTSIDE_PORTAL_MIMIC
	void			OnDataChanged( DataUpdateType_t updateType );
#endif
	void			DelayedRemove();
	void			ClientThink();

	virtual void	Release();
	virtual bool	ShouldDraw();
	virtual int		DrawModel( int flags );

	virtual const Vector &GetRenderOrigin( void );
	virtual const QAngle &GetRenderAngles( void );

#ifdef CLIENTSIDE_PORTAL_MIMIC
	inline const VMatrix &GetMatrix() const { return m_matPortal; }
	VMatrix			m_matPortal;
	Vector			m_vecPortalOrigin;
	float			m_flScale;
#else
	inline const VMatrix &GetMatrix() const { return m_hSourcePortal->MatrixThisToLinked(); }
	CHandle<C_BasePortal>		m_hSourcePortal;
#endif
};

#endif
