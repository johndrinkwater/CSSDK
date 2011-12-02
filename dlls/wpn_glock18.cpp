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

#define GLOCK18_WEIGHT        7
#define GLOCK18_DEFAULT_GIVE  20
#define GLOCK18_MAX_CARRY     AMMO_9MM_MAX_CARRY
#define GLOCK18_MAX_CLIP      20
#define GLOCK18_PLAYER_SPEED  250.0
#define GLOCK18_RELOAD_TIME   2.2

#define GLOCK18_ACCURACY_MIN_DELAY   0.325
#define GLOCK18_ACCURACY_OFFSET      0.275
#define GLOCK18_MIN_INACCURACY       0.60
#define GLOCK18_MAX_INACCURACY       0.90
#define GLOCK18_PENETRATION          1
#define GLOCK18_DAMAGE               25
#define GLOCK18_RANGE_MODIFIER       0.75
#define GLOCK18_IDLE_TIME            2.5
#define GLOCK18_IDLE_INTERVAL        20.0
#define GLOCK18_ADJUSTED_CYCLE_TIME  0.05

enum glock18_e
{
    GLOCK18_IDLE1 = 0,
    GLOCK18_IDLE2,
    GLOCK18_IDLE3,
    GLOCK18_SHOOT1,
    GLOCK18_SHOOT2,
    GLOCK18_SHOOT3,
    GLOCK18_SHOOT_EMPTY,
    GLOCK18_RELOAD,
    GLOCK18_DRAW,
    GLOCK18_HOLSTER,
    GLOCK18_ADD_SILENCER,
    GLOCK18_DRAW2,
    GLOCK18_RELOAD2
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

LINK_ENTITY_TO_CLASS( weapon_glock18, CGLOCK18 );

void CGLOCK18::Precache()
{
    PRECACHE_MODEL( "models/v_glock18.mdl" );
    PRECACHE_MODEL( "models/w_glock18.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_glock18.mdl" );

    PRECACHE_SOUND( "weapons/glock18-1.wav"     );
    PRECACHE_SOUND( "weapons/glock18-2.wav"     );
    PRECACHE_SOUND( "weapons/clipout1.wav"      );
    PRECACHE_SOUND( "weapons/clipin1.wav"       );
    PRECACHE_SOUND( "weapons/sliderelease1.wav" );
    PRECACHE_SOUND( "weapons/slideback1.wav"    );
    PRECACHE_SOUND( "weapons/357_cock1.wav"     );
    PRECACHE_SOUND( "weapons/de_clipin.wav"     );
    PRECACHE_SOUND( "weapons/de_clipout.wav"    );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usGlock18 = PRECACHE_EVENT( 1, "events/glock18.sc"  );
}

void CGLOCK18::Spawn()
{
    Precache();
    m_iId = WEAPON_GLOCK18;

    SET_MODEL( ENT( pev ), "models/w_glock18.mdl" );

    m_iGlock18ShotsFired = 0;
    m_flGlock18Shoot     = 0.0;
    m_flAccuracy         = GLOCK18_MAX_INACCURACY;
    m_iDefaultAmmo       = GLOCK18_DEFAULT_GIVE;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CGLOCK18::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = GLOCK18_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = GLOCK18_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_GLOCK18;
    p->iWeight   = GLOCK18_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CGLOCK18::iItemSlot()
{
    return 2;
}

BOOL CGLOCK18::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CGLOCK18::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CGLOCK18::IsPistol()
{
    return TRUE;
}

BOOL CGLOCK18::Deploy()
{
    m_iGlock18ShotsFired = 0;
    m_flGlock18Shoot     = 0.0;
    m_flAccuracy         = GLOCK18_MAX_INACCURACY;
    m_flWeaponSpeed      = GLOCK18_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
    {
        ClearBits( m_fWeaponState, WEAPONSTATE_GLOCK18_BURST_MODE );
        return DefaultDeploy( "models/shield/v_shield_glock18.mdl", "models/shield/p_shield_glock18.mdl", GUN_DRAW, "shieldgun", UseDecrement() );
    }
    else
    {
        int drawAnim = RANDOM_LONG( 0, 1 ) ? GLOCK18_DRAW : GLOCK18_DRAW2;
        return DefaultDeploy( "models/v_glock18.mdl", "models/p_glock18.mdl", drawAnim, "onehanded", UseDecrement() );
    }
}

void CGLOCK18::PrimaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_GLOCK18_BURST_MODE ) )
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 1.2, 0.5, TRUE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.185, 0.5, TRUE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.095, 0.5, TRUE );
        }
        else
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.3, 0.5, TRUE );
        }
    }
    else
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 1.0, 0.2, FALSE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.165, 0.2, FALSE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.075, 0.2, FALSE );
        }
        else
        {
            GLOCK18Fire( ( 1 - m_flAccuracy ) * 0.1, 0.2, FALSE );
        }
    }
}

void CGLOCK18::SecondaryAttack()
{
    if( ShieldSecondaryFire( GUN_SHIELD_UP, GUN_SHIELD_DOWN ) )
    {
        return;
    }

    if( FBitSet( m_fWeaponState, WEAPONSTATE_GLOCK18_BURST_MODE ) )
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_SemiAuto" );
        ClearBits( m_fWeaponState, WEAPONSTATE_GLOCK18_BURST_MODE );
    }
    else
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
        SetBits( m_fWeaponState, WEAPONSTATE_GLOCK18_BURST_MODE );
    }

    m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}

void CGLOCK18::Reload()
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    int reloadAnim;

    if( m_pPlayer->HasShield() )
    {
        reloadAnim = GUN_RELOAD;
    }
    else
    {
        reloadAnim = RANDOM_LONG( 0, 1 ) ? GLOCK18_RELOAD : GLOCK18_RELOAD2;
    }

    if( DefaultReload( GLOCK18_MAX_CLIP, reloadAnim, GLOCK18_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = GLOCK18_MAX_INACCURACY;
    }
}

void CGLOCK18::GLOCK18Fire( float flSpread, float flCycleTime, BOOL fFireBurst )
{
    if( fFireBurst  )
    {
        m_iGlock18ShotsFired = 0;
    }
    else
    {
        m_iShotsFired++;
        flCycleTime -= GLOCK18_ADJUSTED_CYCLE_TIME;

        if( m_iShotsFired > 1 )
        {
            return;
        }
    }

    if( !m_flLastFire )
    {
        m_flLastFire = gpGlobals->time;
    }
    else
    {
        m_flAccuracy -= ( GLOCK18_ACCURACY_MIN_DELAY - ( gpGlobals->time - m_flLastFire ) ) * GLOCK18_ACCURACY_OFFSET;

        if( m_flAccuracy > GLOCK18_MAX_INACCURACY )
        {
            m_flAccuracy = GLOCK18_MAX_INACCURACY;
        }
        else if( m_flAccuracy < GLOCK18_MIN_INACCURACY )
        {
            m_flAccuracy = GLOCK18_MIN_INACCURACY;
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

    SetPlayerShieldAnim();
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        GLOCK18_PENETRATION, BULLET_PLAYER_9MM, GLOCK18_DAMAGE, GLOCK18_RANGE_MODIFIER, m_pPlayer->pev, 1, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usGlock18, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, m_iClip ? FALSE : TRUE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GLOCK18_IDLE_TIME;

    if( fFireBurst )
    {
        m_iGlock18ShotsFired++;
        m_flGlock18Shoot = gpGlobals->time + 0.1;
    }

    ResetPlayerShieldAnim();
}

void CGLOCK18::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_pPlayer->HasShield() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GLOCK18_IDLE_INTERVAL;

        if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( GUN_SHIELD_IDLE, UseDecrement() );
        }
    }
    else if( m_iClip )
    {
        int   anim;
        float flRandom;
        float flTime;

        flRandom = RANDOM_FLOAT( 0, 1 );

        if( flRandom <= 0.3 )
        {
            anim   = GLOCK18_IDLE3;
            flTime = 3.065;
        }
        else if( flRandom <= 0.6 )
        {
            anim   = GLOCK18_IDLE2;
            flTime = 3.75;
        }
        else
        {
            anim   = GLOCK18_IDLE1;
            flTime = 2.5;
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime;
        SendWeaponAnim( anim, UseDecrement() );
    }
}

