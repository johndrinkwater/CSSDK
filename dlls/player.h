/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef _PLAYER_H_
#define _PLAYER_H_


#include "pm_materials.h"


#define PLAYER_FATAL_FALL_SPEED			1024	// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED		580		// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED			100.0 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED			200
#define PLAYER_FALL_PUNCH_THRESHHOLD	350.0	// won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define	PFLAG_ONLADDER	  ( 1<<0 )
#define	PFLAG_ONSWING	  ( 1<<0 )
#define	PFLAG_ONTRAIN	  ( 1<<1 )
#define	PFLAG_ONBARNACLE  ( 1<<2 )
#define	PFLAG_DUCKING	  ( 1<<3 )	// In the process of ducking, but totally squatted yet
#define	PFLAG_USING		  ( 1<<4 )	// Using a continuous entity.
#define	PFLAG_OBSERVER	  ( 1<<5 )	// Player is locked in stationary cam mode. Spectators can move, observers can't.

//
// m_fUserPrefs
//
#define USERPREFS_USE_VGUI_MENUS  ( 1 << 0 )
#define USERPREFS_USE_AUTO_HELP   ( 1 << 8 ) 
#define USERPREFS_SHIELD_DRAWN    ( 1 << 16 )
#define USERPREFS_HAS_SHIELD      ( 1 << 24 )

//
// m_iTeam
//
#define TEAM_UNASSIGNED  0
#define TEAM_TERRORIST   1
#define TEAM_CT          2
#define TEAM_SPECTATOR   3

//
// m_fMapZone, m_fClientMapZone
//
#define MAPZONE_BUY        ( 1 << 0 )
#define MAPZONE_BOMBTARGET ( 1 << 1 ) // (if C4 Carrier)
#define MAPZONE_HOSTAGE    ( 1 << 2 )
#define MAPZONE_ESCAPE     ( 1 << 3 )
#define MAPZONE_VIP        ( 1 << 4 )

// 
// Item type
//
#define ITEMSLOT_PRIMARY	1
#define ITEMSLOT_SECONDARY	2

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

//
// Player animation.
//
typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	
	// CSSDK
	PLAYER_ATTACK1_RIGHT,
	PLAYER_SMALL_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB
} PLAYER_ANIM;


#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

#define CHAT_INTERVAL 1.0f

class CBasePlayer : public CBaseMonster
{
public:
	int					random_seed;								// 96 /101 - See that is shared between client & server for shared weapons code.

	// Spectator
																	// 97 /102 -
	EHANDLE				m_hObserverTarget;							// 98 /103 -
																	// 99 /104 -
	float				m_flNextObserverInput;						// 100/105 - 
	int					m_iSpectatedPlayerWeaponId;					// 101/106 - 
	int					m_iSpectatedPlayerHasBomb;					// 102/107 - 
	int					m_iSpectatedPlayerHasDefuser;				// 103/108 - 
	int					m_iSpecView;								// 104/109 -

	// Animation
	//float				m_flNext_;									// 105/110 - 
																	// 106/111 -
	BOOL				m_fKilledByHeadshot;						// 107/112 -
	float				m_flPainShock;								// 108/113 - 

	int					m_iLastZoom;								// 109/114 -  
	BOOL				m_fResumeZoom;								// 110/115 - 
	float				m_flEjectBrass;								// 111/116 -

	int					m_iKevlar;									// 112/117 -
	BOOL				m_fNotKilled;								// 113/118 - 
	int					m_iTeam;									// 114/119 -
	int					m_iAccount;									// 115/120 - 

	BOOL				m_fHasPrimaryWeapon;						// 116/121 - 

	// Bomb
	BOOL				m_fCanPlantBomb;							// 193/198 - (1<<8)
	BOOL				m_fHasDefuseKit;							// 193/198 - (1<<16)

	BOOL				m_fIsVIP;									// 209/214 - (1<<8)
	BOOL				m_fBombDefusing;							// 232/237 - (1<<8)
	float				m_flNextMapZoneTime;						// 233/238 - Checked each 0.5 second.
	int					m_fMapZone;									// 234/239 - See MAPZONE_* constants.
	int					m_fClientMapZone;							// 235/240 - See MAPZONE_* constants.
	CBaseEntity*		m_pentBombTarget;							// 236/241 - 

	int					m_iPlayerSound;								// 237/242 - The index of the sound list slot reserved for this player.
	int					m_iTargetVolume;							// 238/243 - Ideal sound volume. 
	int					m_iWeaponVolume;							// 239/244 - How loud the player's weapon is right now.
	int					m_iExtraSoundTypes;							// 240/245 - Additional classification for this weapon's sound.
	int					m_iWeaponFlash;								// 241/246 - Brightness of the weapon flash.
	float				m_flStopExtraSoundTime;						// 242/247 -

	float				m_flFlashLightTime;							// 243/248 - Time until next battery draw/Recharge..
	int					m_iFlashBattery;							// 244/249 - Flashlight Battery Draw.

	int					m_afButtonLast;								// 245/250 - 
	int					m_afButtonPressed;							// 246/251 - 
	int					m_afButtonReleased;							// 247/252 - 

	edict_t*			m_pentSndLast;								// 248/253 - Last sound entity to modify player room type.
	float				m_flSndRoomtype;							// 249/254 - Last roomtype set by sound entity.
	float				m_flSndRange;								// 250/255 - Distance from player to sound entity.

	float				m_flFallVelocity;							// 251/256 -

	int					m_rgItems[MAX_ITEMS];						// 252->256/257->262 - (MAX_ITEMS = 4)

	unsigned int		m_afPhysicsFlags;							// 257/262 - Physics flags - set when 'normal' physics should be revisited or overriden.
	float				m_fNextSuicideTime;							// 258/263 - The time after which the player can next use the suicide command.

	// These are time-sensitive things that we keep track of
	float				m_flTimeStepSound;	// when the last stepping sound was made
	float				m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float				m_flSwimTime;		// how long player has been underwater
	float				m_flDuckTime;		// how long we've been ducking
	float				m_flWallJumpTime;	// how long until next walljump

	float				m_flSuitUpdate;					// when to play next suit update
	int					m_rgSuitPlayList[CSUITPLAYLIST];// next sentencenum to play for suit update
	int					m_iSuitPlayNext;				// next sentence slot for queue storage;
	int					m_rgiSuitNoRepeat[CSUITNOREPEAT];		// suit sentence no repeat list
	float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	// how long to wait before allowing repeat
	int					m_lastDamageAmount;		// Last damage taken
	float				m_tbdPrev;				// Time-based damage timer

	float				m_flgeigerRange;		// range to nearest radiation source
	float				m_flgeigerDelay;		// delay per update of range msg to client
	int					m_igeigerRangePrev;
	int					m_iStepLeft;			// alternate left/right foot stepping sound
	char				m_szTextureName[CBTEXTURENAMEMAX];	// current texture name we're standing on
	char				m_chTextureType;		// current texture type

	int					m_idrowndmg;			// track drowning damage taken
	int					m_idrownrestored;		// track drowning damage restored

	int					m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
												// the hude via the DAMAGE message
	BOOL				m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	BOOL				m_fGameHUDInitialized;
	int					m_iTrain;				// Train control position
	BOOL				m_fWeapon;				// Set this to FALSE to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	float				m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	BOOL				m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is TRUE. 
	BOOL				m_fLongJump; // does this player have the longjump module?

	float				m_tSneaking;
	int					m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	int					m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int					m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new
	int					m_iHideHUD;		// the players hud weapon info is to be hidden
	int					m_iClientHideHUD;
	int					m_iFOV;			// field of view
	int					m_iClientFOV;	// client's known FOV
	// usable player items 
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];

	// CSSDK
	CBasePlayerItem *m_pLastPrimaryItem;							// 368/373 -
	CBasePlayerItem *m_pLastSecondaryItem;							// 369/374 -

	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;  // client version of the active item
	CBasePlayerItem *m_pLastItem;
	// shared ammo slots
	int	m_rgAmmo[MAX_AMMO_SLOTS];
	int	m_rgAmmoLast[MAX_AMMO_SLOTS];

	Vector				m_vecAutoAim;
	BOOL				m_fOnTarget;
	int					m_iDeaths;
	float				m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx, m_lasty;  // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;// Custom clan logo frames for this player
	float	m_flNextDecalTime;// next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	// CSSDK

	// Flashbang
	float	m_flFlashedUntil;			// 514/519 - 
	float	m_flFlashedAt;				// 515/520 - 
	float	m_flFlashHoldTime;			// 516/521 - 
	float	m_flFlashDuration;			// 517/522 - 
	float	m_iFlashAlpha;				// 518/523 - 

	float	m_flNextAutoFollowTime;		// 519/524 -

	virtual void Spawn( void );
	void Pain( void );

//	virtual void Think( void );
	virtual void Jump( void );
	virtual void Duck( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual Vector GetGunPosition( void );
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ) + pev->view_ofs * RANDOM_FLOAT( 0.5, 1.1 ); };		// position to shoot at
	virtual void StartSneaking( void ) { m_tSneaking = gpGlobals->time - 1; }
	virtual void StopSneaking( void ) { m_tSneaking = gpGlobals->time + 30; }
	virtual BOOL IsSneaking( void ) { return m_tSneaking <= gpGlobals->time; }
	virtual BOOL IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual	BOOL IsPlayer( void ) { return TRUE; }			// Spectators should return FALSE for this, they aren't "players" as far as game logic is concerned

	virtual BOOL IsNetClient( void ) { return TRUE; }		// Bots should return FALSE for this, they can't receive NET messages
															// Spectators should return TRUE for this
	virtual const char *TeamID( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	// CSSDK
	virtual void ResetMaxSpeed();

	void RenewItems(void);
	void PackDeadPlayerItems( void );
	void RemoveAllItems( BOOL removeSuit );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon );

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData( void );
	
	static	TYPEDESCRIPTION m_playerSaveData[];

	// Player is moved across the transition by other means
	virtual int		ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual void	Precache( void );
	BOOL			IsOnLadder( void );
	BOOL			FlashlightIsOn( void );
	void			FlashlightTurnOn( void );
	void			FlashlightTurnOff( void );
	
	void UpdatePlayerSound ( void );
	void DeathSound ( void );

	int Classify ( void );
	void SetAnimation( PLAYER_ANIM playerAnim );
	void SetWeaponAnimType( const char *szExtention );
	char m_szAnimExtention[32];

	// custom player functions
	virtual void ImpulseCommands( void );
	void CheatImpulseCommands( int iImpulse );

	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	// CSSDK
	void AddAccount( int amount, bool trackChange );
	BOOL AddPlayerItem( CBasePlayerItem *pItem );
	void AllowAutoFollow( void );
	void Blind( float flTime, float flHoldtime, float flDuration, int iAlpha );
	BOOL CanAffordArmor( void );
	BOOL CanAffordDefuseKit( void );
	BOOL CanAffordGrenade( void );
	BOOL CanAffordPrimary( void );
	BOOL CanAffordPrimaryAmmo( void );
	BOOL CanAffordSecondaryAmmo( void );
	BOOL CanPlayerBuy( bool bDisplayMessage );
	BOOL CanSwitchObserverModes( void );
	void CheckPowerups( entvars_s* pPlayer );
	BOOL HasShield( void );
	BOOL IsProtectedByShield( void );
	BOOL IsHittingShield( Vector const &vecDir, TraceResult* ptr );
	void SetBombIcon( int status );
	void SetProgressBarTime( int time );
	void SetScoreboardAttributes( CBasePlayer* pPlayer );
	void UpdateShieldCrosshair( bool shouldRemove );

	void AddPoints( int score, BOOL bAllowNegativeScore );
	void AddPointsToTeam( int score, BOOL bAllowNegativeScore );
	BOOL RemovePlayerItem( CBasePlayerItem *pItem );
	void DropPlayerItem ( char *pszItemName );
	BOOL HasPlayerItem( CBasePlayerItem *pCheckItem );
	BOOL HasNamedPlayerItem( const char *pszItemName );
	BOOL HasWeapons( void );// do I have ANY weapons?
	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem(void);
	void SelectItem(const char *pstr);
	void ItemPreFrame( void );
	void ItemPostFrame( void );
	void GiveNamedItem( const char *szName );
	void EnableControl(BOOL fControl);

	int  GiveAmmo( int iAmount, char *szName, int iMax );
	void SendAmmoUpdate(void);

	void WaterMove( void );
	void EXPORT PlayerDeathThink( void );
	void PlayerUse( void );

	void CheckSuitUpdate();
	void SetSuitUpdate(char *name, int fgroup, int iNoRepeat);
	void UpdateGeigerCounter( void );
	void CheckTimeBasedDamage( void );

	BOOL FBecomeProne ( void );
	void BarnacleVictimBitten ( entvars_t *pevBarnacle );
	void BarnacleVictimReleased ( void );
	static int GetAmmoIndex(const char *psz);
	int AmmoInventory( int iAmmoIndex );
	int Illumination( void );

	void ResetAutoaim( void );
	Vector GetAutoaimVector( float flDelta  );
	Vector AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  );

	void ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	void DeathMessage( entvars_t *pevKiller );

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames( void );

	void CBasePlayer::TabulateAmmo( void );

	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;// while charging, when to absorb another unit of player's ammo?
	
	//Player ID
	void InitStatusBar( void );
	void UpdateStatusBar( void );
	

	int m_izSBarState[ SBAR_END ];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[ SBAR_STRING_SIZE ];
	char m_SbarString1[ SBAR_STRING_SIZE ];
	
	float m_flNextChatTime;

	// CSSDK
	int		m_iUserPrefs;				// 510/515 - 	
	float	m_flProgressBarStartTime;	// 605/610 -
	float	m_flProgressBarEndTime;		// 606/611 -

	int		m_fObserverAutoDirector;	// 607/612 - 1<<8
};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669


extern int	gmsgHudText;
extern BOOL gInitHUD;

#endif // _PLAYER_H_
