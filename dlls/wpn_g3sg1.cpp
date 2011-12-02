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

#define G3SG1_WEIGHT             30
#define G3SG1_DEFAULT_GIVE       20
#define G3SG1_MAX_CARRY          AMMO_762NATO_MAX_CARRY
#define G3SG1_MAX_CLIP           20
#define G3SG1_PLAYER_SPEED       210.0
#define G3SG1_PLAYER_SPEED_ZOOM  150.0
#define G3SG1_RELOAD_TIME        3.35

#define G3SG1_DEFAULT_ACCURACY    0.2
#define G3SG1_ACCURACY_MIN_DELAY  0.55
#define G3SG1_ACCURACY_OFFSET     0.30
#define G3SG1_MAX_INACCURACY      0.98
#define G3SG1_PENETRATION         3
#define G3SG1_DAMAGE              80
#define G3SG1_RANGE_MODIFIER      0.98
#define G3SG1_IDLE_TIME           1.8
#define G3SG1_IDLE_INTERVAL       60.0
#define G3SG1_ADJUSTED_SPREAD     0.025

enum g3sg1_e
{
    G3SG1_IDLE = 0,
    G3SG1_SHOOT1,
    G3SG1_SHOOT2,
    G3SG1_RELOAD,
    G3SG1_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CG3SG1 );

void CG3SG1::Precache()
{
    PRECACHE_MODEL( "models/v_g3sg1.mdl" );
    PRECACHE_MODEL( "models/w_g3sg1.mdl" );

    PRECACHE_SOUND( "weapons/g3sg1-1.wav"       );
    PRECACHE_SOUND( "weapons/g3sg1_slide.wav"   );
    PRECACHE_SOUND( "weapons/g3sg1_clipin.wav"  );
    PRECACHE_SOUND( "weapons/g3sg1_clipout.wav" );
    PRECACHE_SOUND( "weapons/zoom.wav"          );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usG3SG1 = PRECACHE_EVENT( 1, "events/g3sg1.sc" );
}

void CG3SG1::Spawn()
{
    Precache();
    m_iId = WEAPON_G3SG1;

    SET_MODEL( ENT( pev ), "models/w_g3sg1.mdl" );

    m_iDefaultAmmo = G3SG1_DEFAULT_GIVE;
    m_flLastFire   = 0.0;

    FallInit();
}

int CG3SG1::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "762Nato";
    p->iMaxAmmo1 = G3SG1_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = G3SG1_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 3;
    p->iId       = m_iId = WEAPON_G3SG1;
    p->iWeight   = G3SG1_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CG3SG1::iItemSlot()
{
    return 1;
}

BOOL CG3SG1::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CG3SG1::GetMaxSpeed()
{
    return m_pPlayer->m_iFOV == 90 ? G3SG1_PLAYER_SPEED : G3SG1_PLAYER_SPEED_ZOOM;
}

BOOL CG3SG1::Deploy()
{
    return DefaultDeploy( "models/v_g3sg1.mdl", "models/p_g3sg1.mdl", G3SG1_DRAW, "rifle", UseDecrement() );
}

void CG3SG1::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        G3SG1Fire( 0.45, 0.25 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        G3SG1Fire( 0.15, 0.25 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        G3SG1Fire( 0.035, 0.25 );
    }
    else
    {
        G3SG1Fire( 0.055, 0.25 );
    }
}

void CG3SG1::SecondaryAttack()
{
    int newFOV;

    if( m_pPlayer->m_iFOV == 90 )
        newFOV = 40;
    else if( m_pPlayer->m_iFOV == 40 )
        newFOV = 15;
    else if( m_pPlayer->m_iFOV == 15 )
        newFOV = 90;

    m_pPlayer->m_iFOV = m_pPlayer->pev->fov = newFOV;

    /*! @todo Implements this :
    if( TheBots )
        TheBots->OnEvents( EVENT_WEAPON_ZOOMED, m_pPlayer->pev, NULL );*/

    m_pPlayer->ResetMaxSpeed();
    EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/zoom.wav", VOL_NORM, 2.4 );

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CG3SG1::Reload()
{
    if( m_pPlayer->ammo_762nato <= 0 )
    {
        return;
    }

    if( DefaultReload( G3SG1_MAX_CLIP, G3SG1_RELOAD, G3SG1_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        if( m_pPlayer->pev->fov != 90 )
        {
            m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
            SecondaryAttack();
        }
    }
}

void CG3SG1::G3SG1Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    if( m_pPlayer->pev->fov == 90 )
    {
        flSpread += G3SG1_ADJUSTED_SPREAD;
    }

    if( !m_flLastFire )
    {
        m_flLastFire = gpGlobals->time;
    }
    else
    {
        m_flAccuracy = G3SG1_ACCURACY_MIN_DELAY + ( gpGlobals->time - m_flLastFire ) * G3SG1_ACCURACY_OFFSET;

        if( m_flAccuracy > G3SG1_MAX_INACCURACY )
        {
            m_flAccuracy = G3SG1_MAX_INACCURACY;
        }

        m_flLastFire = gpGlobals->time;
    }

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
        }

        /*! @todo Implements this :
        if( TheBots )
            TheBots->OnEvents( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );*/

        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3(
        m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        G3SG1_PENETRATION, BULLET_PLAYER_762MM, G3SG1_DAMAGE, G3SG1_RANGE_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usG3SG1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, TRUE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + G3SG1_IDLE_TIME;

    m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.75, 1.75 ) + ( m_pPlayer->pev->punchangle.x * 0.25 );
    m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat( m_pPlayer->random_seed, -0.75, 0.75 );
}

void CG3SG1::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + G3SG1_IDLE_INTERVAL;
        SendWeaponAnim( G3SG1_IDLE, UseDecrement() );
    }
}