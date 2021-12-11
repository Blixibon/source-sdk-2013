//==============================================================================
//
// Purpose: Osprey helicopter created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbasehelicopter.h"
#include "ai_base_bm_npc.h"

#define OSPREY_MODEL "models/props_vehicles/osprey.mdl"
#define OSPREY_HOVER_SOUND	"npc_sounds_osprey.HoverLoop"
#define OSPREY_ROTOR_BLAST_SOUND	"NPC_AttackHelicopter.RotorBlast"

#define OSPREY_ACCEL_RATE				300

//-----------------------------------------------------------------------------
// A custom helicopter 
//-----------------------------------------------------------------------------
class CNPC_BM_Osprey : public CAI_Base_BM_NPC<CBaseHelicopter>
{
public:
	DECLARE_CLASS( CNPC_BM_Osprey, CAI_Base_BM_NPC<CBaseHelicopter> );
	DECLARE_DATADESC();

	CNPC_BM_Osprey();
	~CNPC_BM_Osprey();

	virtual void	Precache( void );
	virtual void	Spawn( void );
	Class_T Classify( void ) { return CLASS_MILITARY; }

	void	InitializeRotorSound( void );
	void	Flight( void );

	float GetAcceleration( void ) { return m_flAcceleration; }

	float			m_flAcceleration;

protected:

	int m_poseLeftRudder, m_poseMiddleRudder, m_poseRightRudder,
		m_poseLeftWingFlap, m_poseRightWingFlap,
		m_poseLeftNacelle, m_poseRightNacelle,
		m_poseSideDoor, m_poseBackDoor,
		m_poseGear;
	virtual void	PopulatePoseParameters( void );

private:

	QAngle	m_vecAngAcceleration;
};
