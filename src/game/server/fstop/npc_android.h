//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP C++ Header File
//
//======================================================================//

#ifndef NPC_ANDROID_H
#define NPC_ANDROID_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_baseactor.h"


class CNPC_Android : public CAI_BaseActor
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CNPC_Android, CAI_BaseActor );

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );

	// Larger hearing distance
	// TODO: Perhaps more advanced hearing modeling system for stealth
	float HearingSensitivity( void ) { return 4.0; }

	void GatherConditions();

	// This is a dummy field. In order to provide save/restore
	// code in this file, we must have at least one field
	// for the code to operate on. Delete this field when
	// you are ready to do your own save/restore for this
	// character.
	int		m_iDeleteThisField;

	DEFINE_CUSTOM_AI;

	//===== Schedules =====//
	enum
	{
		SCHED_ANDROID_CHARGE_ENEMY = LAST_SHARED_SCHEDULE,
		SCHED_ANDROID_SWATITEM,
		SCHED_ANDROID_WANDER_MEDIUM,	// medium range wandering behavior.
		SCHED_ANDROID_WANDER_ANGRILY,
		SCHED_ANDROID_BASH_DOOR,

		LAST_ANDROID_SCHEDULE,
	};

	//===== Tasks =====//
	enum
	{
		TASK_ANDROID_EXPRESS_ANGER = LAST_SHARED_TASK,
		TASK_ANDROID_ATTACK_DOOR,
		//TASK_ANDROID_SWAT_ITEM,
		TASK_ANDROID_DIE,

		LAST_ANDROID_TASK,
	};

	//===== Conditions =====//
	enum
	{
		COND_ANDROID_ANGRY = LAST_SHARED_CONDITION,
	};
	
};

#endif // NPC_ANDROID_H
