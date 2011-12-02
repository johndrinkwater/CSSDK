/***
 *  Copyright 2010,2011 Vincent Herbet.
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
#include "hltv.h"
#include "gamerules.h"

#define MAC10_WEIGHT        25
#define MAC10_DEFAULT_GIVE  30
#define MAC10_MAX_CARRY     AMMO_45ACP_MAX_CARRY
#define MAC10_MAX_CLIP      30
#define MAC10_PLAYER_SPEED  250.0
#define MAC10_RELOAD_TIME   3.15

#define MAC10_DEFAULT_ACCURACY  0.15
#define MAC10_ACCURACY_DIVISOR  200
#define MAC10_ACCURACY_OFFSET   0.6
#define MAC10_MAX_INACCURACY    1.65
#define MAC10_PENETRATION       1
#define MAC10_DAMAGE            29
#define MAC10_RANGE_MODIFIER    0.82
#define MAC10_IDLE_TIME         2.0
#define MAC10_IDLE_INTERVAL     20.0

enum mac10_e
{
    MAC10_IDLE = 0,
    MAC10_RELOAD,
    MAC10_DRAW,
    MAC10_SHOOT1,
    MAC10_SHOOT2,
    MAC10_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_mac10, CMAC10 );

void CMAC10::Precache()
{
    PRECACHE_MODEL( "models/v_mac10.mdl" );
    PRECACHE_MODEL( "models/w_mac10.mdl" );

    PRECACHE_SOUND( "weapons/mac10-1.wav"        );
    PRECACHE_SOUND( "weapons/mac10-2.wav"        );
    PRECACHE_SOUND( "weapons/mac10_clipout.wav"  );
    PRECACHE_SOUND( "weapons/mac10_clipin.wav"   );
    PRECACHE_SOUND( "weapons/mac10_boltpull.wav" );

    m_iShell  = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usMAC10 = PRECACHE_EVENT( 1, "events/mac10.sc" );
}

void CMAC10::Spawn()
{
    Precache();
    m_iId = WEAPON_MAC10;

    SET_MODEL( ENT( pev ), "models/w_mac10.mdl" );

    m_iDefaultAmmo = MAC10_DEFAULT_GIVE;
    m_flAccuracy   = MAC10_DEFAULT_ACCURACY;
    m_fDelayFire   = FALSE;

    FallInit();
}

int CMAC10::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "45acp";
    p->iMaxAmmo1 = MAC10_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = MAC10_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 7;
    p->iId       = m_iId = WEAPON_MAC10;
    p->iWeight   = MAC10_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CMAC10::iItemSlot()
{
    return 1;
}

BOOL CMAC10::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CMAC10::GetMaxSpeed()
{
    return MAC10_PLAYER_SPEED;
}

BOOL CMAC10::Deploy()
{
    m_flAccuracy = MAC10_DEFAULT_ACCURACY;
    m_fDelayFire = FALSE;

    return DefaultDeploy( "models/v_mac10.mdl", "models/p_mac10.mdl", MAC10_DRAW, "onehanded", UseDecrement() );
}

void CMAC10::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        MAC10Fire( m_flAccuracy * 0.375, 0.07 );
    }
    else
    {
        MAC10Fire( m_flAccuracy * 0.03, 0.07 );
    }
}

void CMAC10::Reload()
{
    if( m_pPlayer->ammo_45acp <= 0 )
    {
        return;
    }

    if( DefaultReload( MAC10_MAX_CLIP, MAC10_RELOAD, MAC10_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = 0;
        m_iShotsFired  = 0;
    }
}

void CMAC10::MAC10Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / MAC10_ACCURACY_DIVISOR ) + MAC10_ACCURACY_OFFSET;

    if( m_flAccuracy > MAC10_MAX_INACCURACY )
    {
        m_flAccuracy = MAC10_MAX_INACCURACY;
    }

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
        }

        /*! @todo Implements me :
        if( TheBots )
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );  */

        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        MAC10_PENETRATION, BULLET_PLAYER_45ACP, MAC10_DAMAGE, MAC10_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usMAC10, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 5, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + MAC10_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.9, 0.45, 0.25, 0.035, 3.5, 2.75, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.3, 0.55, 0.40, 0.05, 4.75, 3.75, 5 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.75, 0.40, 0.175, 0.03, 2.75, 2.5, 10 );
    }
    else
    {
        KickBack( 0.775, 0.425, 0.2, 0.03, 3, 2.75, 9 );
    }
}

void CMAC10::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + MAC10_IDLE_INTERVAL;
    SendWeaponAnim( MAC10_IDLE, UseDecrement() );
}