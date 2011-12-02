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

#define DEAGLE_WEIGHT        7
#define DEAGLE_DEFAULT_GIVE  AMMO_50AECLIP_GIVE
#define DEAGLE_MAX_CARRY     AMMO_50AE_MAX_CARRY
#define DEAGLE_MAX_CLIP      AMMO_50AECLIP_GIVE
#define DEAGLE_PLAYER_SPEED  250.0
#define DEAGLE_RELOAD_TIME   2.2

#define DEAGLE_ACCURACY_MIN_DELAY	0.4
#define DEAGLE_ACCURACY_OFFSET      0.35
#define DEAGLE_MIN_INACCURACY       0.55
#define DEAGLE_MAX_INACCURACY       0.90
#define DEAGLE_PENETRATION          2
#define DEAGLE_DAMAGE               54
#define DEAGLE_RANGE_MODIFIER       0.81
#define DEAGLE_IDLE_TIME            1.8
#define DEAGLE_IDLE_INTERVAL        20.0
#define DEAGLE_ADJUSTED_CYCLE_TIME  0.075

enum deagle_e
{
    DEAGLE_IDLE1 = 0,
    DEAGLE_SHOOT1,
    DEAGLE_SHOOT2,
    DEAGLE_SHOOT_EMPTY,
    DEAGLE_RELOAD,
    DEAGLE_DRAW
};

enum shieldgun_e
{
    GUN_IDLE1 = 0,
    GUN_SHOOT1,
    GUN_SHOOT2,
    GUN_SHOOT_EMPTY,
    GUN_RELOAD,
    GUN_DRAW,
    GUN_SHIELD_IDLE,
    GUN_SHIELD_UP,
    GUN_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_deagle, CDEAGLE );

void CDEAGLE::Precache()
{
    PRECACHE_MODEL( "models/v_deagle.mdl" );
    PRECACHE_MODEL( "models/w_deagle.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_deagle.mdl" );

    PRECACHE_SOUND( "weapons/deagle-1.wav"   );
    PRECACHE_SOUND( "weapons/deagle-2.wav"   );
    PRECACHE_SOUND( "weapons/de_clipout.wav" );
    PRECACHE_SOUND( "weapons/de_clipin.wav"  );
    PRECACHE_SOUND( "weapons/de_deploy.wav"  );

    m_iShell   = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usDeagle = PRECACHE_EVENT( 1, "events/deagle.sc" );
}

void CDEAGLE::Spawn()
{
    Precache();
    m_iId = WEAPON_DEAGLE;

    SET_MODEL( ENT( pev ), "models/w_deagle.mdl" );

    m_flAccuracy   = DEAGLE_MAX_INACCURACY;
    m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CDEAGLE::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "50AE";
    p->iMaxAmmo1 = DEAGLE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = DEAGLE_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_DEAGLE;
    p->iWeight   = DEAGLE_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CDEAGLE::iItemSlot()
{
    return 2;
}

BOOL CDEAGLE::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CDEAGLE::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CDEAGLE::IsPistol()
{
    return TRUE;
}

BOOL CDEAGLE::Deploy()
{
    m_flAccuracy    = DEAGLE_MAX_INACCURACY;
    m_flWeaponSpeed = DEAGLE_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
	ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_deagle.mdl", "models/shield/p_shield_deagle.mdl", GUN_RELOAD, "shieldgun", UseDecrement() );
    else
        return DefaultDeploy( "models/v_deagle.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded", UseDecrement() );
}

void CDEAGLE::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        DEAGLEFire( ( 1 - m_flAccuracy ) * 1.5, 0.3, FALSE );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        DEAGLEFire( ( 1 - m_flAccuracy ) * 0.25, 0.3, FALSE );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        DEAGLEFire( ( 1 - m_flAccuracy ) * 0.115, 0.3, FALSE );
    }
    else
    {
        DEAGLEFire( ( 1 - m_flAccuracy ) * 0.13, 0.3, FALSE );
    }
}

void CDEAGLE::SecondaryAttack()
{
    ShieldSecondaryFire( GUN_SHIELD_UP, GUN_SHIELD_DOWN );
}

void CDEAGLE::Reload()
{
    if( m_pPlayer->ammo_50ae <= 0 )
    {
        return;
    }

    if ( DefaultReload( DEAGLE_MAX_CLIP, DEAGLE_RELOAD, DEAGLE_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = DEAGLE_MAX_INACCURACY;
    }
}

void CDEAGLE::DEAGLEFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    flCycleTime -= DEAGLE_ADJUSTED_CYCLE_TIME;

    if( m_iShotsFired > 1 )
    {
        return;
    }

    if( !m_flLastFire )
    {
        m_flLastFire = gpGlobals->time;
    }
    else
    {
        m_flAccuracy -= ( DEAGLE_ACCURACY_MIN_DELAY - ( gpGlobals->time - m_flLastFire ) ) * DEAGLE_ACCURACY_OFFSET;

        if ( m_flAccuracy > DEAGLE_MAX_INACCURACY )
        {
             m_flAccuracy = DEAGLE_MAX_INACCURACY;
        }
        else if ( m_flAccuracy < DEAGLE_MIN_INACCURACY )
        {
            m_flAccuracy = DEAGLE_MIN_INACCURACY;
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
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );
        */
        return;
    }

    m_iClip--;

    SetPlayerShieldAnim();
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3(
        m_pPlayer->GetGunPosition(), gpGlobals->v_forward,
        flSpread, 4096.0, DEAGLE_PENETRATION, BULLET_PLAYER_50AE, DEAGLE_DAMAGE, DEAGLE_RANGE_MODIFIER, m_pPlayer->pev, 1, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(),
        m_usDeagle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, m_iClip ? FALSE : TRUE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if ( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + DEAGLE_IDLE_TIME;

    ResetPlayerShieldAnim();
}

void CDEAGLE::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if ( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + DEAGLE_IDLE_INTERVAL;

    if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        SendWeaponAnim( DEAGLE_IDLE1, UseDecrement() );
    }
}


