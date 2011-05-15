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

#define AUG_WEIGHT        25
#define AUG_DEFAULT_GIVE  AMMO_556NATOCLIP_GIVE
#define AUG_MAX_CARRY     AMMO_556NATO_MAX_CARRY
#define AUG_MAX_CLIP      AMMO_556NATOCLIP_GIVE
#define AUG_PLAYER_SPEED  240.0
#define AUG_RELOAD_TIME   3.3

#define AUG_DEFAULT_ACCURACY	0.2
#define AUG_ACCURACY_DIVISOR	215
#define AUG_ACCURACY_OFFSET		0.3
#define AUG_MAX_INACCURACY		1.0
#define AUG_PENETRATION			2
#define AUG_DAMAGE				32
#define AUG_RANGE_MODIFIER		0.96
#define AUG_IDLE_TIME			1.9
#define AUG_IDLE_INTERVAL		20.0

enum aug_e
{
    AUG_IDLE = 0,
    AUG_RELOAD,
    AUG_DRAW,
    AUG_SHOOT1,
    AUG_SHOOT2,
    AUG_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_aug, CAUG );

void CAUG::Precache()
{
    PRECACHE_MODEL( "models/v_aug.mdl" );
    PRECACHE_MODEL( "models/w_aug.mdl" );

    PRECACHE_SOUND( "weapons/aug-1.wav" );
    PRECACHE_SOUND( "weapons/aug_clipout.wav"  );
    PRECACHE_SOUND( "weapons/aug_clipin.wav"   );
    PRECACHE_SOUND( "weapons/aug_boltpull.wav" );
    PRECACHE_SOUND( "weapons/aug_boltslap.wav" );
    PRECACHE_SOUND( "weapons/aug_forearm.wav"  );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usAug  = PRECACHE_EVENT( 1, "events/aug.sc" );
}

void CAUG::Spawn()
{
    Precache();
    m_iId = WEAPON_AUG;

    SET_MODEL( ENT( pev ), "models/w_aug.mdl" );

    m_iDefaultAmmo = AUG_DEFAULT_GIVE;
    m_flAccuracy   = AUG_DEFAULT_ACCURACY;
    m_iShotsFired  = 0;

    FallInit();
}

int CAUG::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = AUG_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = AUG_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 14;
    p->iId       = m_iId = WEAPON_AUG;
    p->iWeight   = AUG_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CAUG::iItemSlot()
{
    return 1;
}

BOOL CAUG::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CAUG::GetMaxSpeed()
{
    return AUG_PLAYER_SPEED;
}

BOOL CAUG::Deploy()
{
    m_flAccuracy  = AUG_DEFAULT_ACCURACY;
    m_iShotsFired = 0;

    return DefaultDeploy( "models/v_aug.mdl", "models/p_aug.mdl", AUG_DRAW, "carbine", UseDecrement() );
}

void CAUG::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        AUGFire( m_flAccuracy * 0.4 + 0.035, 0.0825 );
    }
    else if ( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        AUGFire( m_flAccuracy * 0.07 + 0.035, 0.0825 );
    }
    else if ( m_pPlayer->pev->fov == 90 )
    {
        AUGFire( m_flAccuracy * 0.02, 0.0825 );
    }
    else
    {
        AUGFire( m_flAccuracy * 0.02, 0.135 );
    }
}

void CAUG::SecondaryAttack()
{
    int newFOV;
    
    if( m_pPlayer->m_iFOV == 90 )
        newFOV = 55;
    else
        newFOV = 90;
 
    m_pPlayer->m_iFOV = m_pPlayer->pev->fov = newFOV;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAUG::Reload()
{
    if( m_pPlayer->ammo_556Nato <= 0 )
    {
        return;
    }
    
    if( DefaultReload( AUG_MAX_CLIP, AUG_RELOAD, AUG_RELOAD_TIME ) )
    {
        if( m_pPlayer->m_iFOV != 90 )
        {
            SecondaryAttack();
        }
        
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = 0;
        m_iShotsFired = 0;
        m_fDelayFire  = FALSE;
    }
}

void CAUG::AUGFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_fDelayFire = TRUE;
    m_iShotsFired++;

    m_flAccuracy = ( m_iShotsFired * m_iShotsFired * m_iShotsFired / AUG_ACCURACY_DIVISOR ) + AUG_ACCURACY_OFFSET;

    if( m_flAccuracy > AUG_MAX_INACCURACY )
    {
        m_flAccuracy = AUG_MAX_INACCURACY;
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

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 
        8192.0, AUG_PENETRATION, BULLET_PLAYER_556MM, AUG_DAMAGE, AUG_RANGE_MODIFIER, m_pPlayer->pev, 0, m_pPlayer->random_seed );
        
    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usAug, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_iClip, m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ], FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AUG_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1, 0.45, 0.275, 0.05, 4, 2.5, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.25, 0.45, 0.22, 0.18, 5.5, 4, 5 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.575, 0.325, 0.2, 0.011, 3.25, 2, 8 );
    }
    else
    {
        KickBack( 0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8 );
    }
}

void CAUG::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AUG_IDLE_INTERVAL;
        SendWeaponAnim( AUG_IDLE, UseDecrement() );
    }
}
