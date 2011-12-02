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

#define TMP_WEIGHT        25
#define TMP_DEFAULT_GIVE  AMMO_9MMCLIP_GIVE
#define TMP_MAX_CARRY     AMMO_9MM_MAX_CARRY
#define TMP_MAX_CLIP      AMMO_9MMCLIP_GIVE
#define TMP_PLAYER_SPEED  250.0
#define TMP_RELOAD_TIME   2.2

#define TMP_DEFAULT_ACCURACY  0.2
#define TMP_ACCURACY_DIVISOR  200
#define TMP_ACCURACY_OFFSET   0.55
#define TMP_MAX_INACCURACY    1.4
#define TMP_PENETRATION       1
#define TMP_DAMAGE            20
#define TMP_RANGE_MODIFIER    0.85
#define TMP_IDLE_TIME         2.0
#define TMP_IDLE_INTERVAL     20.0

enum tmp_e
{
    TMP_IDLE = 0,
    TMP_RELOAD,
    TMP_DRAW,
    TMP_SHOOT1,
    TMP_SHOOT2,
    TMP_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_tmp, CTMP );

void CTMP::Precache()
{
    PRECACHE_MODEL( "models/v_tmp.mdl" );
    PRECACHE_MODEL( "models/w_tmp.mdl" );

    PRECACHE_SOUND( "weapons/tmp-1.wav" );
    PRECACHE_SOUND( "weapons/tmp-2.wav" );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usTMP  = PRECACHE_EVENT( 1, "events/tmp.sc" );
}

void CTMP::Spawn()
{
    Precache();
    m_iId = WEAPON_TMP;

    SET_MODEL( ENT( pev ), "models/w_tmp.mdl" );

    m_iDefaultAmmo = TMP_DEFAULT_GIVE;
    m_flAccuracy   = TMP_DEFAULT_ACCURACY;
    m_iShotsFired  = 0;
    m_fDelayFire   = FALSE;

    FallInit();
}

int CTMP::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = TMP_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = TMP_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 11;
    p->iId       = m_iId = WEAPON_TMP;
    p->iWeight   = TMP_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CTMP::iItemSlot()
{
    return 1;
}

BOOL CTMP::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CTMP::GetMaxSpeed()
{
    return TMP_PLAYER_SPEED;
}

BOOL CTMP::Deploy()
{
    m_flAccuracy  = TMP_DEFAULT_ACCURACY;
    m_iShotsFired = 0;
    m_fDelayFire  = FALSE;

    return DefaultDeploy( "models/v_tmp.mdl", "models/p_tmp.mdl", TMP_DRAW, "onehanded", UseDecrement() );
}

void CTMP::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        TMPFire( m_flAccuracy * 0.25, 0.07 );
    }
    else
    {
        TMPFire( m_flAccuracy * 0.03, 0.07 );
    }
}

void CTMP::Reload()
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    if( DefaultReload( TMP_MAX_CLIP, TMP_RELOAD, TMP_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = TMP_DEFAULT_ACCURACY;
        m_iShotsFired  = 0;
    }
}

void CTMP::TMPFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / TMP_ACCURACY_DIVISOR ) + TMP_ACCURACY_OFFSET;

    if( m_flAccuracy > TMP_ACCURACY_DIVISOR )
    {
        m_flAccuracy = TMP_ACCURACY_DIVISOR;
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
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        TMP_PENETRATION, BULLET_PLAYER_9MM, TMP_DAMAGE, TMP_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usTMP, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 5, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + TMP_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.8, 0.4, 0.2, 0.03, 3, 2.5, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.1, 0.5, 0.35, 0.045, 4.5, 3.5, 6 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.7, 0.35, 0.125, 0.025, 2.5, 2, 10 );
    }
    else
    {
        KickBack( 0.725, 0.375, 0.15, 0.025, 2.75, 2.75, 9 );
    }
}

void CTMP::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + TMP_IDLE_INTERVAL;
    SendWeaponAnim( TMP_IDLE, UseDecrement() );
}