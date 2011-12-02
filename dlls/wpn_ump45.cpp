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

#define UMP45_WEIGHT        25
#define UMP45_DEFAULT_GIVE  25
#define UMP45_MAX_CARRY     AMMO_45ACP_MAX_CARRY
#define UMP45_MAX_CLIP      25
#define UMP45_PLAYER_SPEED  250.0
#define UMP45_RELOAD_TIME   2.2

#define UMP45_DEFAULT_ACCURACY  0
#define UMP45_ACCURACY_DIVISOR  210
#define UMP45_ACCURACY_OFFSET   0.5
#define UMP45_MAX_INACCURACY    1.0
#define UMP45_PENETRATION       1
#define UMP45_DAMAGE            30
#define UMP45_RANGE_MODIFIER    0.82
#define UMP45_IDLE_TIME         2.0
#define UMP45_IDLE_INTERVAL     20.0

enum ump45_e
{
    UMP45_IDLE = 0,
    UMP45_RELOAD,
    UMP45_DRAW,
    UMP45_SHOOT1,
    UMP45_SHOOT2,
    UMP45_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_ump45, CUMP45 );

void CUMP45::Precache()
{
    PRECACHE_MODEL( "models/v_ump45.mdl" );
    PRECACHE_MODEL( "models/w_ump45.mdl" );

    PRECACHE_SOUND( "weapons/ump45-1.wav"        );
    PRECACHE_SOUND( "weapons/ump45_clipout.wav"  );
    PRECACHE_SOUND( "weapons/ump45_clipin.wav"   );
    PRECACHE_SOUND( "weapons/ump45_boltslap.wav" );

    m_iShell  = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usUMP45 = PRECACHE_EVENT( 1, "events/ump45.sc" );
}

void CUMP45::Spawn()
{
    Precache();
    m_iId = WEAPON_UMP45;

    SET_MODEL( ENT( pev ), "models/w_ump45.mdl" );

    m_iDefaultAmmo = UMP45_DEFAULT_GIVE;
    m_flAccuracy   = UMP45_DEFAULT_ACCURACY;
    m_fDelayFire   = FALSE;

    FallInit();
}

int CUMP45::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "45acp";
    p->iMaxAmmo1 = UMP45_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = UMP45_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 15;
    p->iId       = m_iId = WEAPON_UMP45;
    p->iWeight   = UMP45_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CUMP45::iItemSlot()
{
    return 1;
}

BOOL CUMP45::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CUMP45::GetMaxSpeed()
{
    return UMP45_PLAYER_SPEED;
}

BOOL CUMP45::Deploy()
{
    m_iShotsFired = 0;
    m_fDelayFire  = FALSE;

    return DefaultDeploy( "models/v_ump45.mdl", "models/p_ump45.mdl", UMP45_DRAW, "carbine", UseDecrement() );
}

void CUMP45::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        UMP45Fire( m_flAccuracy * 0.24, 0.1 );
    }
    else
    {
        UMP45Fire( m_flAccuracy * 0.04, 0.1 );
    }
}

void CUMP45::Reload()
{
    if( m_pPlayer->ammo_45acp <= 0 )
    {
        return;
    }

    if( DefaultReload( UMP45_MAX_CLIP, UMP45_RELOAD, 3.5 ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = UMP45_DEFAULT_ACCURACY;
        m_iShotsFired  = 0;
    }
}

void CUMP45::UMP45Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired  ) / UMP45_ACCURACY_DIVISOR ) + UMP45_ACCURACY_OFFSET;

    if( m_flAccuracy > UMP45_MAX_INACCURACY )
    {
        m_flAccuracy = UMP45_MAX_INACCURACY;
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
        UMP45_PENETRATION, BULLET_PLAYER_45ACP, UMP45_DAMAGE, UMP45_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usUMP45, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 0, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UMP45_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.55, 0.3, 0.225, 0.03, 3.5, 2.5, 10 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 0.125, 0.65, 0.55, 0.0475, 5.5, 4, 10 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10 );
    }
    else
    {
        KickBack( 0.275, 0.2, 0.15, 0.0225, 2.5, 1.5, 10 );
    }
}

void CUMP45::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UMP45_IDLE_INTERVAL;
    SendWeaponAnim( UMP45_IDLE, UseDecrement() );
}