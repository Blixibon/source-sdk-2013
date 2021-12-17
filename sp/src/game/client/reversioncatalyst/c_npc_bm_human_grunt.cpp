//==============================================================================
//
// Purpose: HECU marines created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "c_npc_bm_human_grunt.h"
#include "cl_animevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BM_HumanGrunt, DT_NPC_BM_HumanGrunt, CNPC_BM_HumanGrunt )
	//RecvPropBool( RECVINFO ( m_bHasCigar) ),

	RecvPropInt( RECVINFO( m_iCharacterIndex ) ),
END_RECV_TABLE()

// This is originally from c_baseanimating.cpp, but extern doesn't work for some reason
const float RUN_SPEED_ESTIMATE_SQR = 150.0f * 150.0f;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_BM_HumanGrunt::C_NPC_BM_HumanGrunt()
{
	//m_bHasCigar = false;
	//m_iCigarFlex = LocalFlexController_t(-1);
}

//-----------------------------------------------------------------------------
// Purpose: Turns soldier footsteps into marine footsteps
//-----------------------------------------------------------------------------
void C_NPC_BM_HumanGrunt::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
}

//-----------------------------------------------------------------------------
// Purpose: Turns soldier footsteps into marine footsteps
//-----------------------------------------------------------------------------
void C_NPC_BM_HumanGrunt::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	switch( event )
	{
	case CL_EVENT_FOOTSTEP_LEFT:
		{
#ifndef HL2MP
			char pSoundName[256];
			if ( !options || !options[0] || FStrEq(options, "NPC_CombineS") )
			{
				options = "NPC_HumanGrunt";
			}

			Vector vel;
			EstimateAbsVelocity( vel );

			// If he's moving fast enough, play the run sound
			if ( vel.Length2DSqr() > RUN_SPEED_ESTIMATE_SQR )
			{
				Q_snprintf( pSoundName, 256, "%s.RunFootstepLeft", options );
			}
			else
			{
				Q_snprintf( pSoundName, 256, "%s.FootstepLeft", options );
			}
			EmitSound( pSoundName );
#endif
		}
		break;

	case CL_EVENT_FOOTSTEP_RIGHT:
		{
#ifndef HL2MP
			char pSoundName[256];
			if ( !options || !options[0] || FStrEq(options, "NPC_CombineS") )
			{
				options = "NPC_HumanGrunt";
			}

			Vector vel;
			EstimateAbsVelocity( vel );
			// If he's moving fast enough, play the run sound
			if ( vel.Length2DSqr() > RUN_SPEED_ESTIMATE_SQR )
			{
				Q_snprintf( pSoundName, 256, "%s.RunFootstepRight", options );
			}
			else
			{
				Q_snprintf( pSoundName, 256, "%s.FootstepRight", options );
			}
			EmitSound( pSoundName );
#endif
		}
		break;

	default:
		BaseClass::FireEvent(origin, angles, event, options);
		break;
	}
}
