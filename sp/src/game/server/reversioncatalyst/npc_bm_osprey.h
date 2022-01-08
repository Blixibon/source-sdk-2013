//==============================================================================
//
// Purpose: Osprey helicopter created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbasehelicopter.h"
#include "ai_base_bm_npc.h"

#define OSPREY_MODEL "models/props_vehicles/osprey.mdl"
#define OSPREY_HOVER_SOUND_FAR	"npc_sounds_osprey.HoverFarLoop"
#define OSPREY_HOVER_SOUND	"npc_sounds_osprey.HoverLoop"
#define OSPREY_ROTOR_BLAST_SOUND	"NPC_AttackHelicopter.RotorBlast"

#define OSPREY_MAX_SPEED				(60.0f * 17.6f) // 120 miles per hour.
#define OSPREY_ACCEL_RATE				300

#define OSPREY_PLANE_DEFAULT_TRANSITION_TIME	7.5f
#define OSPREY_PLANE_MAX_SPEED				(120.0f * 17.6f) // 240 miles per hour.
#define OSPREY_PLANE_ACCEL_RATE				750

// Special actions
#define OSPREY_DEFAULT_SOLDIERS		4
#define OSPREY_MAX_SOLDIERS			6

// Bodygroups
#define OSPREY_BODY_PROPS_BLUR		"props_2d"
#define OSPREY_BODY_PROPS_NORMAL	"props_3d"

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
	void	UpdateRotorWashVolume();
	void	UpdateRotorSoundPitch( int iPitch );
	void	StopLoopingSounds();
	void	Flight( void );

	float	GetAcceleration( void ) { return m_bPlaneMode ? OSPREY_PLANE_ACCEL_RATE : OSPREY_ACCEL_RATE; }
	
	void	InputEnterPlaneMode( inputdata_t &inputdata );
	void	InputExitPlaneMode( inputdata_t &inputdata );

	//--------------------------------------------------------
	// Dropping Soldiers
	//--------------------------------------------------------
	void	SpawnTroop( void );

	void	InputBeginRappellingGrunts( inputdata_t& inputdata );
	void	InputRemoveGrunts( inputdata_t& inputdata );
	void	InputWaitHereTillReady( inputdata_t& inputdata );
	void	InputRappelToTarget( inputdata_t& inputdata );
	void	InputKillRappelingGrunts( inputdata_t& inputdata );

	COutputEvent	m_OnReadyToMoveDeployZone;
	COutputEvent	m_OnReadyToRetreat;
	COutputEHANDLE	m_OnSpawnNPC;

	// Templates for soldiers dropped off
	string_t	m_sNPCTemplate[ OSPREY_MAX_SOLDIERS ];
	string_t	m_sNPCTemplateData[ OSPREY_MAX_SOLDIERS ];	
	string_t	m_sDustoffPoints[ OSPREY_MAX_SOLDIERS ];	
	int			m_iCurrentTroopExiting;
	EHANDLE		m_hLastTroopToLeave;

	// Timers
	float	m_flTimeTakeOff;
	float	m_flNextTroopSpawnAttempt;
	float	m_flDropDelay;			// delta between each mine
	float	m_flTimeNextAttack;
	float	m_flLastTime;

	enum LandingState_t
	{
		LANDING_NO = 0,

		// Dropoff
		LANDING_LEVEL_OUT,		// Heading to a point above the dropoff point
		LANDING_DESCEND,		// Descending from to the dropoff point
		LANDING_TOUCHDOWN,
		LANDING_UNLOADING,
		LANDING_UNLOADED,
		LANDING_LIFTOFF,

		// Pickup
		LANDING_SWOOPING,		// Swooping down to the target

		// Hovering, which we're saying is a type of landing since there's so much landing code to leverage
		LANDING_START_HOVER,
		LANDING_HOVER_LEVEL_OUT,
		LANDING_HOVER_DESCEND,
		LANDING_HOVER_TOUCHDOWN,
		LANDING_END_HOVER,
	};

	void SetLandingState( LandingState_t landingState );
	LandingState_t GetLandingState() const { return (LandingState_t)m_iLandState; }

protected:

	int m_poseLeftRudder, m_poseMiddleRudder, m_poseRightRudder,
		m_poseLeftWingFlap, m_poseRightWingFlap,
		m_poseLeftNacelle, m_poseRightNacelle,
		m_poseSideDoor, m_poseBackDoor,
		m_poseGear;
	virtual void	PopulatePoseParameters( void );

private:

	QAngle	m_vecAngAcceleration;

	bool	m_bPlaneMode;
	float	m_flPlaneTransitionStartTime;
	float	m_flPlaneTransitionEndTime;

	int		m_soldiersToDrop;
	int		m_iDropState;
	int		m_iLandState;

	// Sounds
	CSoundPatch		*m_pNearRotorSound;
};
