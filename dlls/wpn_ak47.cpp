/**
 *  Copyright 2010,2011 Vincent Herbet, Reuben Morais, Carlos Sola.
 *
 *  This file is part of Counter-Strike SDK.
 *
 *  Counter-Strike SDK is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Counter-Strike SDK is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Counter-Strike SDK. If not, see <http://www.gnu.org/licenses/>.
 */

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#define AK47_WEIGHT        25
#define AK47_DEFAULT_GIVE  AMMO_762NATOCLIP_GIVE
#define AK47_MAX_CARRY     AMMO_762NATO_MAX_CARRY
#define AK47_MAX_CLIP      AMMO_762NATOCLIP_GIVE
#define AK47_PLAYER_SPEED  221.0
#define AK47_RELOAD_TIME   2.45

#define AK47_DEFAULT_ACCURACY  0.2
#define AK47_ACCURACY_DIVISOR  200
#define AK47_ACCURACY_OFFSET   0.35
#define AK47_MAX_INACCURACY	   1.25
#define AK47_PENETRATION	   2
#define AK47_DAMAGE			   36
#define AK47_RANGE_MODIFIER	   0.98
#define AK47_IDLE_TIME		   1.9
#define AK47_IDLE_INTERVAL	   20.0

enum ak47_e
{
    AK47_IDLE1 = 0,
    AK47_RELOAD,
    AK47_DRAW,
    AK47_SHOOT1,
    AK47_SHOOT2,
    AK47_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_ak47, CAK47 );

void CAK47::Precache()
{
    PRECACHE_MODEL( "models/v_ak47.mdl" );
    PRECACHE_MODEL( "models/w_ak47.mdl" );

    PRECACHE_SOUND( "weapons/ak47-1.wav" );
    PRECACHE_SOUND( "weapons/ak47-2.wav" );
    PRECACHE_SOUND( "weapons/ak47_clipout.wav"  );
    PRECACHE_SOUND( "weapons/ak47_clipin.wav"   );
    PRECACHE_SOUND( "weapons/ak47_boltpull.wav" );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usAk47 = PRECACHE_EVENT( 1, "events/ak47.sc" );
}

void CAK47::Spawn()
{
    Precache();
    m_iId = WEAPON_AK47;

    SET_MODEL( ENT( pev ), "models/w_ak47.mdl" );

    m_iDefaultAmmo = AK47_DEFAULT_GIVE;
    m_flAccuracy   = AK47_DEFAULT_ACCURACY;
    m_iShotsFired  = 0;

    FallInit();
}

int CAK47::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "762Nato";
    p->iMaxAmmo1 = AK47_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = AK47_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_AK47;
    p->iWeight   = AK47_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CAK47::iItemSlot()
{
    return 1;
}

BOOL CAK47::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CAK47::GetMaxSpeed()
{
	return AK47_PLAYER_SPEED;
}

BOOL CAK47::Deploy()
{
    m_flAccuracy  = AK47_DEFAULT_ACCURACY;
    m_iShotsFired = 0;

    return DefaultDeploy( "models/v_ak47.mdl", "models/p_ak47.mdl", AK47_DRAW, "ak47", UseDecrement() );
}

void CAK47::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        AK47Fire( m_flAccuracy * 0.4 + 0.4, 0.0955 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        AK47Fire( m_flAccuracy * 0.4 + 0.07, 0.0955 );
    }
    else
    {
        AK47Fire( m_flAccuracy * 0.0275, 0.0955 );
    }
}

void CAK47::SecondaryAttack()
{

}

void CAK47::Reload()
{
    if( m_pPlayer->ammo_762nato <= 0 )
    {
        return;
    }
    
    if( DefaultReload( AK47_MAX_CLIP, AK47_RELOAD, AK47_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = AK47_DEFAULT_ACCURACY;
        m_iShotsFired  = 0;
        m_fDelayFire   = FALSE;
    }
}

void CAK47::AK47Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired  ) / AK47_ACCURACY_DIVISOR ) + AK47_ACCURACY_OFFSET;

    if( m_flAccuracy > AK47_MAX_INACCURACY )
    {
        m_flAccuracy = AK47_MAX_INACCURACY;
    }

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
        }

// TODO: Adds support for bots.
// -
//      if ( TheBots )
//			TheBots->OnEvents( 2, m_pPlayer->pev, 0 );
//			
        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward,
        flSpread, 8192.0, AK47_PENETRATION, BULLET_PLAYER_762MM, AK47_DAMAGE, AK47_RANGE_MODIFIER, m_pPlayer->pev, 0, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usAk47, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_iClip, m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ], 0, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AK47_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 2, 1.0, 0.5, 0.35, 9, 6, 5 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9 );
    }
    else
    {
        KickBack( 1, 0.375, 0.175, 0.0375, 5.75, 1.75, 8 );
    }
}

void CAK47::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }
    
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AK47_IDLE_INTERVAL;
    SendWeaponAnim( AK47_IDLE1, UseDecrement() );
}