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

#define MP5N_WEIGHT        25
#define MP5N_DEFAULT_GIVE  AMMO_9MMCLIP_GIVE
#define MP5N_MAX_CARRY     AMMO_9MM_MAX_CARRY
#define MP5N_MAX_CLIP      AMMO_9MMCLIP_GIVE
#define MP5N_PLAYER_SPEED  250.0
#define MP5N_RELOAD_TIME   2.63

#define MP5N_DEFAULT_ACCURACY  0.15
#define MP5N_ACCURACY_DIVISOR  220.1
#define MP5N_ACCURACY_OFFSET   0.45
#define MP5N_MAX_INACCURACY    0.75
#define MP5N_PENETRATION       1
#define MP5N_DAMAGE            26
#define MP5N_RANGE_MODIFIER    0.84
#define MP5N_IDLE_TIME         2.0
#define MP5N_IDLE_INTERVAL     20.0

enum mp5n_e
{
    MP5N_IDLE = 0,
    MP5N_RELOAD,
    MP5N_DRAW,
    MP5N_SHOOT1,
    MP5N_SHOOT2,
    MP5N_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5N );

void CMP5N::Precache()
{
    PRECACHE_MODEL( "models/v_mp5.mdl" );
    PRECACHE_MODEL( "models/w_mp5.mdl" );

    PRECACHE_SOUND( "weapons/mp5-1.wav" );
    PRECACHE_SOUND( "weapons/mp5-2.wav" );
    PRECACHE_SOUND( "weapons/mp5_clipout.wav" );
    PRECACHE_SOUND( "weapons/mp5_clipin.wav" );
    PRECACHE_SOUND( "weapons/mp5_slideback.wav" );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usMP5N  = PRECACHE_EVENT( 1, "events/mp5n.sc" );
}

void CMP5N::Spawn()
{
    Precache();
    m_iId = WEAPON_MP5;

    SET_MODEL( ENT( pev ), "models/w_mp5n.mdl" );

    m_iDefaultAmmo = MP5N_DEFAULT_GIVE;
    m_iShotsFired  = 0;
    m_fDelayFire   = TRUE;

    FallInit();
}

int CMP5N::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = MP5N_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = MP5N_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 7;
    p->iId       = m_iId = WEAPON_MP5;
    p->iWeight   = MP5N_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CMP5N::iItemSlot()
{
    return 1;
}

BOOL CMP5N::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CMP5N::GetMaxSpeed()
{
    return MP5N_PLAYER_SPEED;
}

BOOL CMP5N::Deploy()
{
    m_iShotsFired = 0;
    m_fDelayFire  = FALSE;

    return DefaultDeploy( "models/v_mp5.mdl", "models/p_mp5.mdl", MP5N_DRAW, "mp5", UseDecrement() );
}

void CMP5N::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        MP5NFire( m_flAccuracy * 0.2, 0.075, FALSE );
    }
    else
    {
        MP5NFire( m_flAccuracy * 0.04, 0.075, FALSE );
    }
}

void CMP5N::Reload()
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    if( DefaultReload( MP5N_MAX_CLIP, MP5N_RELOAD, MP5N_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = 0;
        m_iShotsFired = 0;
    }
}

void CMP5N::MP5NFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( m_iShotsFired  * m_iShotsFired ) / MP5N_ACCURACY_DIVISOR + MP5N_ACCURACY_OFFSET;

    if( m_flAccuracy > MP5N_MAX_INACCURACY )
    {
        m_flAccuracy = MP5N_MAX_INACCURACY;
    }

    if( m_iClip <= 0 )
    {
        if ( m_fFireOnEmpty )
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
        MP5N_PENETRATION, BULLET_PLAYER_9MM, MP5N_DAMAGE, MP5N_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usMP5N, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 0, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + MP5N_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.9, 0.475, 0.35, 0.0425, 5, 3, 6 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 0.5, 0.275, 0.2, 0.03, 3, 2, 10 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.225, 0.15, 0.1, 0.015, 2, 1, 10 );
    }
    else
    {
        KickBack( 0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10 );
    }
}

void CMP5N::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + MP5N_IDLE_INTERVAL;
    SendWeaponAnim( MP5N_IDLE, UseDecrement() );
}