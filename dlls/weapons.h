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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "effects.h"

class CBasePlayer;
extern int gmsgWeapPickup;

void DeactivateSatchels( CBasePlayer *pOwner );

class CGrenade : public CBaseMonster
{
public:
	void Spawn( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	virtual int	ObjectCaps( void ) { return FCAP_CONTINUOUS_USE; }

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	static CGrenade* ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade* ShootTimed2( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int team, unsigned short usEvent );
	static CGrenade* ShootSmokeGrenade( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, unsigned short usEvent );
	static CGrenade* ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade* ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );

	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void Explode2( TraceResult *pTrace, int bitsDamageType );
	void Explode3( TraceResult *pTrace, int bitsDamageType );
	void SG_Explode( TraceResult *pTrace, int bitsDamageType );

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT C4Touch( CBaseEntity *pOther );
	void EXPORT C4Think( void );
	void EXPORT DangerSoundThink( void );
	void EXPORT Detonate( void );
	void EXPORT Detonate2( void );
	void EXPORT Detonate3( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PreDetonate( void );
	void EXPORT SG_Detonate( void );
	void EXPORT SG_Smoke( void );
	void EXPORT SG_TumbleThink( void );
	void EXPORT Smoke( void );
	void EXPORT Smoke2( void );
	void EXPORT Smoke3_A( void );
	void EXPORT Smoke3_B( void );
	void EXPORT Smoke3_C( void );
	void EXPORT TumbleThink( void );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual int	BloodColor( void ) { return DONT_BLEED; };
	virtual void BounceSound( void );

	static TYPEDESCRIPTION m_SaveData[];

	// CSSDK
	BOOL			m_fStartDefuse;			//  96/101 - 1<<0
	BOOL			m_fPlantedC4;			//  96/101 - 1<<8
	EHANDLE			m_hDefuser;				//  97/102 - 
	float			m_flDefuseCountDown;	//  99/104 - 
	float			m_flC4Blow;				// 100/105 - 
	float			m_flNextFreqInterval;	// 101/106 -
	float			m_flNextBeep;			// 102/107 -
	float			m_flNextFreq;			// 103/108 -
	char*			m_sBeepName;			// 104/109 -
	int				m_flAttenu;				// 105/110 -
	float			m_flNextBlink;			// 106/111 -
	float			m_flNextDefuseTime;  	// 107/112 -
	BOOL			m_fJustBlew;			// 108/113 - 1<<0
	int				m_iTeam;				// 109/114 -
	int				m_iC4Beep;				// 110/115 -
	CBaseEntity*	m_pentCurBombTarget;	// 111/116 -
	int				m_SGSmoke;				// 112/117 -
	BOOL			m_fLightSmoke;			// 113/118 -
	unsigned short	m_usEvent;				// 114/119 - & 1<<0 ?
	bool			m_bSGMulti;				// 114/119 - 1<<16
	bool			m_bSGDetonated;			// 114/119 - 1<<24
	Vector			m_SGExplosionPos;		// 115->117/120->122 -
	int				m_iNumBounce;			// 118/123 -
	BOOL			m_fRegisteredSound;		// 119/124 - whether or not this grenade has issued its DANGER sound to the world sound list yet.			
};


// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

#define WEAPON_NONE				0
#define WEAPON_CROWBAR			1
#define	WEAPON_GLOCK			2
#define WEAPON_PYTHON			3
#define WEAPON_MP5				4
#define WEAPON_CHAINGUN			5
#define WEAPON_CROSSBOW			6
#define WEAPON_SHOTGUN			7
#define WEAPON_RPG				8
#define WEAPON_GAUSS			9
#define WEAPON_EGON				10
#define WEAPON_HORNETGUN		11
#define WEAPON_HANDGRENADE		12
#define WEAPON_TRIPMINE			13
#define	WEAPON_SATCHEL			14
#define	WEAPON_SNARK			15

// CSSDK
#define WEAPON_P228          1
#define WEAPON_SCOUT         3
#define WEAPON_HEGRENADE     4
#define WEAPON_XM1014        5
#define WEAPON_C4            6
#define WEAPON_MAC10         7
#define WEAPON_AUG           8
#define WEAPON_SMOKEGRENADE  9
#define WEAPON_ELITE         10
#define WEAPON_FIVESEVEN     11
#define WEAPON_UMP45         12
#define WEAPON_SG550         13
#define WEAPON_GALIL         14
#define WEAPON_FAMAS         15
#define WEAPON_USP           16
#define WEAPON_GLOCK18       17
#define WEAPON_AWP           18
#define WEAPON_MP5NAVY       19
#define WEAPON_M249          20
#define WEAPON_M3            21
#define WEAPON_M4A1          22
#define WEAPON_TMP           23
#define WEAPON_G3SG1         24
#define WEAPON_FLASHBANG     25
#define WEAPON_DEAGLE        26
#define WEAPON_SG552         27
#define WEAPON_AK47          28
#define WEAPON_KNIFE         29
#define WEAPON_P90           30
#define WEAPON_VEST          31
#define WEAPON_VESTHELM      32
#define WEAPON_SHIELD        99

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			32


#define MAX_NORMAL_BATTERY	100


// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define CROWBAR_WEIGHT		0
#define GLOCK_WEIGHT		10
#define PYTHON_WEIGHT		15
#define MP5_WEIGHT			15
#define SHOTGUN_WEIGHT		15
#define CROSSBOW_WEIGHT		10
#define RPG_WEIGHT			20
#define GAUSS_WEIGHT		20
#define EGON_WEIGHT			20
#define HORNETGUN_WEIGHT	10
#define HANDGRENADE_WEIGHT	5
#define SNARK_WEIGHT		5
#define SATCHEL_WEIGHT		-10
#define TRIPMINE_WEIGHT		-10


// weapon clip/carry ammo capacities
#define URANIUM_MAX_CARRY		100
#define	_9MM_MAX_CARRY			250
#define _357_MAX_CARRY			36
#define BUCKSHOT_MAX_CARRY		125
#define BOLT_MAX_CARRY			50
#define ROCKET_MAX_CARRY		5
#define HANDGRENADE_MAX_CARRY	10
#define SATCHEL_MAX_CARRY		5
#define TRIPMINE_MAX_CARRY		5
#define SNARK_MAX_CARRY			15
#define HORNET_MAX_CARRY		8
#define M203_GRENADE_MAX_CARRY	10

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

//#define CROWBAR_MAX_CLIP		WEAPON_NOCLIP
#define GLOCK_MAX_CLIP			17
#define PYTHON_MAX_CLIP			6
#define MP5_MAX_CLIP			50
#define MP5_DEFAULT_AMMO		25
#define SHOTGUN_MAX_CLIP		8
#define CROSSBOW_MAX_CLIP		5
#define RPG_MAX_CLIP			1
#define GAUSS_MAX_CLIP			WEAPON_NOCLIP
#define EGON_MAX_CLIP			WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP		WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP	WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP		WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP		WEAPON_NOCLIP
#define SNARK_MAX_CLIP			WEAPON_NOCLIP


// the default amount of ammo that comes with each gun when it spawns
#define GLOCK_DEFAULT_GIVE			17
#define PYTHON_DEFAULT_GIVE			6
#define MP5_DEFAULT_GIVE			25
#define MP5_DEFAULT_AMMO			25
#define MP5_M203_DEFAULT_GIVE		0
#define SHOTGUN_DEFAULT_GIVE		12
#define CROSSBOW_DEFAULT_GIVE		5
#define RPG_DEFAULT_GIVE			1
#define GAUSS_DEFAULT_GIVE			20
#define EGON_DEFAULT_GIVE			20
#define HANDGRENADE_DEFAULT_GIVE	5
#define SATCHEL_DEFAULT_GIVE		1
#define TRIPMINE_DEFAULT_GIVE		1
#define SNARK_DEFAULT_GIVE			5
#define HIVEHAND_DEFAULT_GIVE		8

// The amount of ammo given to a player by an ammo item.
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_GLOCKCLIP_GIVE		GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE		PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE		MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE		200
#define AMMO_M203BOX_GIVE		2
#define AMMO_BUCKSHOTBOX_GIVE	12
#define AMMO_CROSSBOWCLIP_GIVE	CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE		RPG_MAX_CLIP
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_SNARKBOX_GIVE		5

//
// The amount of ammo given to a player by an ammo item.
//
#define AMMO_338MAGNUMCLIP_GIVE		10
#define AMMO_357SIGCLIP_GIVE		13
#define AMMO_45ACPCLIP_GIVE			12
#define AMMO_50AECLIP_GIVE			7
#define AMMO_556NATOCLIP_GIVE		30
#define AMMO_556NATOBOXCLIP_GIVE	30
#define AMMO_57MMCLIP_GIVE			50
#define AMMO_762NATOCLIP_GIVE		30
#define AMMO_9MMCLIP_GIVE			30

//
// CSSDK
// Weapon clip/carry ammo capacities
//
#define AMMO_338MAGNUM_MAX_CARRY	30
#define AMMO_357SIG_MAX_CARRY		52
#define AMMO_45ACP_MAX_CARRY		100
#define AMMO_50AE_MAX_CARRY			35
#define AMMO_556NATO_MAX_CARRY		90
#define AMMO_556NATOBOX_MAX_CARRY	200
#define AMMO_57MM_MAX_CARRY			100
#define AMMO_762NATO_MAX_CARRY		90
#define AMMO_9MM_MAX_CARRY			120

//
// Bullets types.
// Used in CBaseEntity::FireBullets*().
//
typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,      // Glock
	BULLET_PLAYER_MP5,      // Mp5
	BULLET_PLAYER_357,      // Python
	BULLET_PLAYER_BUCKSHOT, // Shotgun
	BULLET_PLAYER_CROWBAR,  // Crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	// CSSDK
	BULLET_PLAYER_45ACP,
	BULLET_PLAYER_338MAG,
	BULLET_PLAYER_762MM,
	BULLET_PLAYER_556MM,
	BULLET_PLAYER_50AE,
	BULLET_PLAYER_57MM,
	BULLET_PLAYER_357SIG
} Bullet;


#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_SLOT_RIFLE       0
#define WEAPON_SLOT_PISTOL      1
#define WEAPON_SLOT_KNIFE       2
#define WEAPON_SLOT_GRENADES    3
#define WEAPON_SLOT_C4          4

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;


// CSSDK
typedef struct 
{
	int	id;
	int cost;
	int teams;
	int slot;
	int acost;
} weapon_struct_t;

extern weapon_struct_t g_weaponStruct[ MAX_WEAPONS ];


// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void EXPORT FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize( void );// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn ( void );// copy a weapon
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( )								// returns is deploy was successful
		 { return TRUE; };

	virtual BOOL CanHolster( void ) { return TRUE; };// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; };

	// CSSDK
	virtual float GetMaxSpeed( void ) { return 0.0; };
	virtual BOOL CanDrop( void ) { return TRUE; };

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; };

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;												// WEAPON_???

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }
	
	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};


// m_fWeaponState
#define WEAPONSTATE_USP_SILENCED       ( 1 << 0 )
#define WEAPONSTATE_GLOCK18_BURST_MODE ( 1 << 1 )
#define WEAPONSTATE_M4A1_SILENCED      ( 1 << 2 )
#define WEAPONSTATE_ELITE_LEFT         ( 1 << 3 )
#define WEAPONSTATE_FAMAS_BURST_MODE   ( 1 << 4 )
#define WEAPONSTATE_SHIELD_DRAWN       ( 1 << 5 )

// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {};	// updates HUD state

	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };
	
	void SetPlayerShieldAnim( void );
	void ResetPlayerShieldAnim( void );
	BOOL ShieldSecondaryFire( int animUp, int animDown );

	int	PrimaryAmmoIndex(); 
	int	SecondaryAmmoIndex(); 

	void PrintState( void );

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; };

	//CSSDK
	void KickBack( float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change );


	float	m_flPumpTime;
	
	// CSSDK
	int		m_iPlayEmptySound;					// 44/48  - 
	int		m_fFireOnEmpty;						// 45/49  - True when the gun is empty and the player is still holding down the key(s).

	float	m_flNextPrimaryAttack;				// 46/50  - Soonest time ItemPostFrame will call PrimaryAttack.
	float	m_flNextSecondaryAttack;			// 47/51  - Soonest time ItemPostFrame will call SecondaryAttack.
	float	m_flTimeWeaponIdle;					// 48/52  - Soonest time ItemPostFrame will call WeaponIdle.
	int		m_iPrimaryAmmoType;					// 49/53  - "primary" ammo index into players m_rgAmmo[].
	int		m_iSecondaryAmmoType;				// 50/54  - "secondary" ammo index into players m_rgAmmo[].
	int		m_iClip;							// 51/55  - Number of shots left in the primary weapon clip, -1 it's not used.
	int		m_iClientClip;						// 52/56  - The last version of m_iClip sent to hud dll.
	int		m_iClientWeaponState;				// 53/57  - The last version of the weapon state sent to hud dll (is current weapon, is on target).
	int		m_fInReload;						// 54/58  - Are we in the middle of a reload ?
	int		m_fInSpecialReload;					// 55/59  - Are we in the middle of a reload for the shotguns ?
	int		m_iDefaultAmmo;						// 56/60  - How much ammo you get when you pick up this weapon as placed by a level designer.

	// CSSDK
	int		m_iShellLate;						// 57/61  -
	float	m_flWeaponSpeed;					// 58/62  -
	BOOL	m_fDelayFire;						// 59/63  -
	int		m_iDirection;						// 60/64  - The current lateral kicking direction ; 1 = right, 0 = left.
												// 61/65  -
	float	m_flAccuracy;						// 62/66  -
	float	m_flLastFire;						// 63/67  -
	int		m_iShotsFired;						// 64/68  -
												// 65/69  -
												// 66/70  -
												// 67/71  -
	int		m_iVewModel;						// 68/72  -
	float	m_flGlock18Shoot;					// 69/73  -
	int		m_iGlock18ShotsFired;				// 70/74  -
	float	m_flFamasShoot;						// 71/75  -
	int		m_iFamasShotsFired;					// 72/76  -
	BOOL	m_fBurstSpread;						// 73/77  -
	int		m_fWeaponState;						// 74/78  -
	float	m_flNextReload;						// 75/79  -
	float	m_flDecreaseShotsFired;				// 76/80  -
};


class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
};


extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

//CSSDK
extern DLL_GLOBAL	short	g_sModelIndexSmokePuff;
extern DLL_GLOBAL	short	g_sModelIndexFireball2;
extern DLL_GLOBAL	short	g_sModelIndexFireball3;
extern DLL_GLOBAL	short	g_sModelIndexFireball4;
extern DLL_GLOBAL	short	g_sModelIndexRadio;
extern DLL_GLOBAL	short	g_sModelIndexCTGhost;
extern DLL_GLOBAL	short	g_sModelIndexTGhost;
extern DLL_GLOBAL	short	g_sModelIndexC4Glow;

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;


#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	int  GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );

public:
	void EXPORT Kill ( void );
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );
	
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	int m_rgiszAmmo[MAX_AMMO_SLOTS];// ammo names
	int	m_rgAmmo[MAX_AMMO_SLOTS];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
BOOL bIsMultiplayer ( void );
void LoadVModel ( char *szViewModel, CBasePlayer *m_pPlayer );
#endif


class CAUG : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int  iItemSlot( void );
	int  GetItemInfo( ItemInfo *p );
	float GetMaxSpeed( void );
	virtual BOOL UseDecrement( void );

	BOOL Deploy( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Reload ( void );
	void WeaponIdle( void );
	void AUGFire( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

	int m_iShell;
	//BOOL  m_bUnknown79; (Deploy)

private:
	unsigned short m_usAug;
};

class CAK47 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int  iItemSlot( void );
	int  GetItemInfo( ItemInfo *p );
	float GetMaxSpeed( void );
	virtual BOOL UseDecrement( void );

	BOOL Deploy( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Reload ( void );
	void WeaponIdle( void );
	void AK47Fire( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

	int m_iShell;
	//BOOL  m_bUnknown79; (Deploy)

private:
	unsigned short m_usAk47;
};

class CAWP : public CBasePlayerWeapon
{
public:
	void		 Spawn			( void );
	void		 Precache		( void );
	int			 iItemSlot		( void );
	int			 GetItemInfo	( ItemInfo *p );
	float		 GetMaxSpeed	( void );
	virtual BOOL UseDecrement	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void AWPFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				 m_iShell;
	unsigned short	 m_usAwp;
};

class CC4 : public CBasePlayerWeapon
{
public:
	void			KeyValue	( KeyValueData* pkvd );
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void WeaponIdle		( void );

	void Holster();
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	BOOL	m_fStartedArming;		// 78/83 - m_fBombState & C4_STARTED_ARMING   (1<<0)
	BOOL	m_fBombPlacedAnimation;	// 78/83 - m_fBombState & C4_PLACED_ANIMATION (1<<8)
	float	m_flArmedTime;			// 79/84 -
	BOOL	m_fUnknown80;			// 80/85 - 
};

class CDEAGLE : public CBasePlayerWeapon
{
public:
	void			Spawn( void );
	void			Precache( void );
	int				iItemSlot( void );
	BOOL			IsPistol( void );
	int				GetItemInfo( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void DEAGLEFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usDeagle;
};

class CELITE : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	BOOL			IsPistol	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void ELITEFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usEliteLeft;
	unsigned short	m_usEliteRight;
};

class CFamas : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void FamasFire		( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL fUseBurst );

private:
	int				m_iShell;
	//BOOL			m_bUnknown79; // Deploy, not used
	unsigned short	m_usFamas;
};

class CFiveSeven : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	BOOL			IsPistol	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void FiveSevenFire	( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usFiveSeven;
};

class CFlashbang : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	BOOL			CanDrop		( void );
	BOOL			CanDeploy	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	void ShieldSecondaryFire	( int animUp, int animDown );
	void ResetPlayerShieldAnim	( void );
	void SetPlayerShieldAnim	( void );

	BOOL Deploy			( void );
	void Holster		( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void WeaponIdle		( void );
};

class CG3SG1 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void G3SG1Fire		( float flSpread, float flCycleTime, BOOL fUseBurst = FALSE );

private:
	int				m_iShell;
	// BOOL		 m_bUnknown79;
	unsigned short	m_usG3SG1;
};

class CGalil : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void GalilFire		( float flSpread, float flCycleTime, BOOL fUseBurst = FALSE );


private:
	int				m_iShell;
	unsigned short	m_usGalil;
};

class CGLOCK18 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	BOOL			IsPistol	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void GLOCK18Fire	( float flSpread, float flCycleTime, BOOL fFireBurst = FALSE );

private:
	int				m_iShell;
	//BOOL			m_bUnknown79;
	unsigned short	m_usGlock18;
};

class CHEGrenade : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	BOOL			CanDrop		( void );
	BOOL			CanDeploy	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	void ShieldSecondaryFire	( int animUp, int animDown );
	void ResetPlayerShieldAnim	( void );
	void SetPlayerShieldAnim	( void );

	BOOL Deploy			( void );
	void Holster		( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void WeaponIdle		( void );

	unsigned short m_usExplo;
private:
	//unsigned short m_usExplo;
};

class CKnife : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	BOOL			CanDrop		( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	void SetPlayerShieldAnim( void );

	BOOL Deploy			( void );
	void Holster		( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void WeaponIdle		( void );
	void WeaponAnimation( int anim );

	void EXPORT SwingAgain	( void );
	void EXPORT Smack		( void );

	BOOL Swing	( BOOL fFirst );
	BOOL Stab	( BOOL fFirst );

	TraceResult m_trHit;

private:
	unsigned short m_usKnife; // 92/96 ?
};

class CM3 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );

private:
	int				m_iShell;
	unsigned short	m_usM3;
};

class CM4A1 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void M4A1Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;				// 78
	//BOOL			m_bUnknown79;			// 79
	unsigned short	m_usM4A1;
};

class CM249 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void M249Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;		// 78
	//BOOL			m_bUnknown79;	// 79
	unsigned short	 m_usM249;		// 80
};

class CMAC10 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void MAC10Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;		// 78
	//BOOL			m_bUnknown79;	// 79
	unsigned short	m_usMAC10;		// 80
};

class CMP5N : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void MP5NFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:

	int				m_iShell;		// 78
	//BOOL			m_bUnknown79;	// 79
	unsigned short	m_usMP5N;		// 80
};

class CP90 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void P90Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;			// 78
	//BOOL			m_bUnknown79;		// 79
	unsigned short	m_usP90;			// 80
};

class CP228 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	BOOL			IsPistol	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void P228Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usP228;
};

class CSCOUT : public CBasePlayerWeapon
{
public:
	void		 Spawn			( void );
	void		 Precache		( void );
	int			 iItemSlot		( void );
	int			 GetItemInfo	( ItemInfo *p );
	float		 GetMaxSpeed	( void );
	virtual BOOL UseDecrement	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void SCOUTFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usSCOUT;
};

class CSG550 : public CBasePlayerWeapon
{
public:
	void		 Spawn			( void );
	void		 Precache		( void );
	int			 iItemSlot		( void );
	int			 GetItemInfo	( ItemInfo *p );
	float		 GetMaxSpeed	( void );
	virtual BOOL UseDecrement	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void SG550Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usSG550;
};

class CSG552 : public CBasePlayerWeapon
{
public:
	void		 Spawn			( void );
	void		 Precache		( void );
	int			 iItemSlot		( void );
	int			 GetItemInfo	( ItemInfo *p );
	float		 GetMaxSpeed	( void );
	virtual BOOL UseDecrement	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void SG552Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usSG552;
};

class CSmokeGrenade : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	BOOL			CanDrop		( void );
	BOOL			CanDeploy	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	void ShieldSecondaryFire	( int animUp, int animDown );
	void ResetPlayerShieldAnim	( void );
	void SetPlayerShieldAnim	( void );

	BOOL Deploy			( void );
	void Holster		( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void WeaponIdle		( void );

	unsigned short m_usSmoke;
private:
	//unsigned short m_usSmoke;
};

class CTMP : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void TMPFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;		// 78
	//BOOL			m_bUnknown79;	// 79
	unsigned short	m_usTMP;	// 80
};

class CUMP45 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	float			GetMaxSpeed	( void );
	virtual BOOL	UseDecrement( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void UMP45Fire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;		// 78
	//BOOL			m_bUnknown79;	// 79
	unsigned short	m_usUMP45;		// 80
};

class CUSP : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	BOOL			IsPistol	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void SecondaryAttack( void );
	void Reload			( void );
	void WeaponIdle		( void );
	void USPFire		( float flSpread, float flCycleTime, BOOL fUseSemi = FALSE );

private:
	int				m_iShell;
	unsigned short	m_usUSP;
};

class CXM1014 : public CBasePlayerWeapon
{
public:
	void			Spawn		( void );
	void			Precache	( void );
	int				iItemSlot	( void );
	int				GetItemInfo	( ItemInfo *p );
	virtual BOOL	UseDecrement( void );
	float			GetMaxSpeed	( void );

	BOOL Deploy			( void );
	void PrimaryAttack	( void );
	void Reload			( void );
	void WeaponIdle		( void );

private:
	int				m_iShell;
	unsigned short	m_usXM1014;
};




class CGlock : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void GlockFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	int m_iShell;
	

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;
};


class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usCrowbar;
};

class CPython : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 2; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flSoundDelay;

	BOOL m_fInZoom;// don't save this. 

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFirePython;
};

class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;
};

class CCrossbow : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 3; }
	int GetItemInfo(ItemInfo *p);

	void FireBolt( void );
	void FireSniperBolt( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );

	int m_fInZoom; // don't save this

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usCrossbow;
	unsigned short m_usCrossbow2;
};

class CShotgun : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn( void );
	void Precache( void );
	int iItemSlot( ) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( );
	void Reload( void );
	void WeaponIdle( void );
	int m_fInReload;
	float m_flNextReload;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;
};

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );
	
	static CLaserSpot *CreateSpot( void );
};

class CRpg : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	void Reload( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );

	void UpdateSpot( void );
	BOOL ShouldWeaponIdle( void ) { return TRUE; };

	CLaserSpot *m_pSpot;
	int m_fSpotActive;
	int m_cActiveRockets;// how many missiles in flight from this launcher right now?

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usRpg;

};

class CRpgRocket : public CGrenade
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn( void );
	void Precache( void );
	void EXPORT FollowThink( void );
	void EXPORT IgniteThink( void );
	void EXPORT RocketTouch( CBaseEntity *pOther );
	static CRpgRocket *CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher );

	int m_iTrail;
	float m_flIgniteTime;
	CRpg *m_pLauncher;// pointer back to the launcher that fired me. 
};

class CGauss : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0  );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );
	
	void StartFire( void );
	void Fire( Vector vecOrigSrc, Vector vecDirShooting, float flDamage );
	float GetFullChargeTime( void );
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	BOOL m_fPrimaryFire;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usGaussFire;
	unsigned short m_usGaussSpin;
};

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend );

	void CreateEffect ( void );
	void DestroyEffect ( void );

	void EndAttack( void );
	void Attack( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval( void );
	float GetDischargeInterval( void );

	void Fire( const Vector &vecOrigSrc, const Vector &vecDir );

	BOOL HasAmmo( void );

	void UseAmmo( int count );
	
	enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};

	CBeam				*m_pBeam;
	CBeam				*m_pNoise;
	CSprite				*m_pSprite;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

	unsigned short m_usEgonStop;

private:
	float				m_shootTime;
	EGON_FIREMODE		m_fireMode;
	float				m_shakeTime;
	BOOL				m_deployed;

	unsigned short m_usEgonFire;
};

class CHgun : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	BOOL IsUseable( void );
	void Holster( int skiplocal = 0 );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;

	float m_flRechargeTime;
	
	int m_iFirePhase;// don't save me.

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usHornetFire;
};



class CHandGrenade : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	
	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CSatchel : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	BOOL CanDeploy( void );
	BOOL Deploy( void );
	BOOL IsUseable( void );
	
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	void Throw( void );
	
	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};


class CTripmine : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	void SetObjectCollisionBox( void )
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28); 
	}

	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usTripFire;

};

class CSqueak : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	int m_fJustThrown;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usSnarkFire;
};


#endif // WEAPONS_H
