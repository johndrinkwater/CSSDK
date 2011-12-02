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
#include "gamerules.h"

#define SG550_WEIGHT             20
#define SG550_DEFAULT_GIVE       AMMO_556NATOCLIP_GIVE
#define SG550_MAX_CARRY          AMMO_556NATO_MAX_CARRY
#define SG550_MAX_CLIP           AMMO_556NATOCLIP_GIVE
#define SG550_PLAYER_SPEED       210.0
#define SG550_PLAYER_SPEED_ZOOM  150.0
#define SG550_RELOAD_TIME        3.35

#define SG550_ACCURACY_MIN_DELAY 0.650
#define SG550_ACCURACY_OFFSET    0.350
#define SG550_MAX_INACCURACY     0.98
#define SG550_PENETRATION        2
#define SG550_DAMAGE             70
#define SG550_RANGE_MODIFIER     0.98
#define SG550_IDLE_TIME          1.8
#define SG550_IDLE_INTERVAL      60.0
#define SG550_AJUSTED_SPREAD     0.025

enum sg550_e
{
    SG550_IDLE = 0,
    SG550_SHOOT1,
    SG550_SHOOT2,
    SG550_RELOAD,
    SG550_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_sg550, CSG550 );

void CSG550::Precache()
{
    PRECACHE_MODEL( "models/v_sg550.mdl" );
    PRECACHE_MODEL( "models/w_sg550.mdl" );

    PRECACHE_SOUND( "weapons/sg550-1.wav"        );
    PRECACHE_SOUND( "weapons/sg550_boltpull.wav" );
    PRECACHE_SOUND( "weapons/sg550_clipin.wav"   );
    PRECACHE_SOUND( "weapons/sg550_clipout.wav"  );
    PRECACHE_SOUND( "weapons/zoom.wav"           );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usSG550 = PRECACHE_EVENT( 1, "events/sg550.sc" );
}

void CSG550::Spawn()
{
    Precache();
    m_iId = WEAPON_SG550;

    SET_MODEL( ENT( pev ), "models/w_sg550.mdl" );

    m_iDefaultAmmo = SG550_DEFAULT_GIVE;
    m_flLastFire   = 0;

    FallInit();
}

int CSG550::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = SG550_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = SG550_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 16;
    p->iId       = m_iId = WEAPON_SG550;
    p->iWeight   = SG550_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CSG550::iItemSlot()
{
    return 1;
}

BOOL CSG550::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CSG550::GetMaxSpeed()
{
    return m_pPlayer->m_iFOV == 90 ? SG550_PLAYER_SPEED : SG550_PLAYER_SPEED_ZOOM;
}

BOOL CSG550::Deploy()
{
    return DefaultDeploy( "models/v_sg550.mdl", "models/p_sg550.mdl", SG550_DRAW, "rifle", UseDecrement() );
}

void CSG550::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        SG550Fire( ( 1 - m_flAccuracy ) * 0.45, 0.25 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        SG550Fire( 0.15, 0.25, FALSE );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        SG550Fire( ( 1 - m_flAccuracy ) * 0.04, 0.25 );
    }
    else
    {
        SG550Fire( ( 1 - m_flAccuracy ) * 0.05, 0.25 );
    }
}

void CSG550::SecondaryAttack()
{
    switch( m_pPlayer->m_iFOV )
    {
        case 90 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40;
        case 40 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
        case 15 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
    }

    /*! @todo: Implements this :
    if( TheBots )
        TheBots->OnEvent( EVENT_WEAPON_ZOOMED, m_pPlayer->pev, NULL ); */

    m_pPlayer->ResetMaxSpeed();
    EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/zoom.wav", VOL_NORM, 2.4 );

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CSG550::Reload()
{
    if( m_pPlayer->ammo_556Nato <= 0 )
    {
        return;
    }

    if( DefaultReload( SG550_MAX_CLIP, SG550_RELOAD, SG550_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        if( m_pPlayer->pev->fov != 90 )
        {
            m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
            SecondaryAttack();
        }
    }
}

void CSG550::SG550Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    if( m_pPlayer->pev->fov == 90 )
    {
        flSpread += SG550_AJUSTED_SPREAD;
    }

    if( !m_flLastFire )
    {
        m_flLastFire = gpGlobals->time;
    }
    else
    {
        m_flAccuracy = SG550_ACCURACY_MIN_DELAY + ( gpGlobals->time - m_flLastFire ) * SG550_ACCURACY_OFFSET;

        if( m_flAccuracy > SG550_MAX_INACCURACY )
        {
            m_flAccuracy = SG550_MAX_INACCURACY;
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

        /*! @todo Implements me :
        if( TheBots )
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL ); */

        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        SG550_PENETRATION, BULLET_PLAYER_556MM, SG550_DAMAGE, SG550_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usSG550, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 5, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SG550_IDLE_TIME;

    m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.75, 1.25 ) + ( m_pPlayer->pev->punchangle.x * 0.25 );
    m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat( m_pPlayer->random_seed, -0.75, 0.75 );
}

void CSG550::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SG550_IDLE_INTERVAL;
        SendWeaponAnim( SG550_IDLE, UseDecrement() );
    }
}