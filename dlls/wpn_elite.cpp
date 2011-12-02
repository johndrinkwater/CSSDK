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

#define ELITE_WEIGHT        5
#define ELITE_DEFAULT_GIVE  AMMO_9MMCLIP_GIVE
#define ELITE_MAX_CARRY     AMMO_9MM_MAX_CARRY
#define ELITE_MAX_CLIP      AMMO_9MMCLIP_GIVE
#define ELITE_PLAYER_SPEED  250.0
#define ELITE_RELOAD_TIME   4.5

#define DEAGLE_ACCURACY_MIN_DELAY	0.325
#define DEAGLE_ACCURACY_OFFSET      0.275
#define DEAGLE_MIN_INACCURACY       0.55
#define DEAGLE_MAX_INACCURACY       0.88
#define DEAGLE_PENETRATION          1
#define DEAGLE_DAMAGE               36
#define DEAGLE_RANGE_MODIFIER       0.76
#define DEAGLE_IDLE_TIME            2.0
#define DEAGLE_IDLE_INTERVAL        60.0
#define DEAGLE_ADJUSTED_CYCLE_TIME  0.125

enum elite_e
{
    ELITE_IDLE = 0,
    ELITE_IDLE_LEFT_EMPTY,
    ELITE_SHOOT_LEFT1,
    ELITE_SHOOT_LEFT2,
    ELITE_SHOOT_LEFT3,
    ELITE_SHOOT_LEFT4,
    ELITE_SHOOT_LEFT5,
    ELITE_SHOOT_LEFT_LAST,
    ELITE_SHOOT_RIGHT1,
    ELITE_SHOOT_RIGHT2,
    ELITE_SHOOT_RIGHT3,
    ELITE_SHOOT_RIGHT4,
    ELITE_SHOOT_RIGHT5,
    ELITE_SHOOT_RIGHT_LAST,
    ELITE_RELOAD,
    ELITE_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_elite, CELITE );

void CELITE::Precache()
{
    PRECACHE_MODEL( "models/v_elite.mdl" );
    PRECACHE_MODEL( "models/w_elite.mdl" );

    PRECACHE_SOUND( "weapons/elite_fire.wav"         );
    PRECACHE_SOUND( "weapons/elite_reloadstart.wav"  );
    PRECACHE_SOUND( "weapons/elite_leftclipin.wav"   );
    PRECACHE_SOUND( "weapons/elite_clipout.wav"      );
    PRECACHE_SOUND( "weapons/elite_sliderelease.wav" );
    PRECACHE_SOUND( "weapons/elite_rightclipin.wav"  );
    PRECACHE_SOUND( "weapons/elite_deploy.wav"       );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );

    m_usEliteLeft  = PRECACHE_EVENT( 1, "events/elite_left.sc"  );
    m_usEliteRight = PRECACHE_EVENT( 1, "events/elite_right.sc" );
}

void CELITE::Spawn()
{
    Precache();
    m_iId = WEAPON_ELITE;

    SET_MODEL( ENT( pev ), "models/w_elite.mdl" );

    m_iDefaultAmmo = ELITE_DEFAULT_GIVE;
    m_flAccuracy   = DEAGLE_MAX_INACCURACY;

    FallInit();
}

int CELITE::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = ELITE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = ELITE_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 4;
    p->iId       = m_iId = WEAPON_ELITE;
    p->iWeight   = ELITE_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CELITE::iItemSlot()
{
    return 2;
}

BOOL CELITE::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CELITE::GetMaxSpeed()
{
    return ELITE_PLAYER_SPEED;
}

BOOL CELITE::IsPistol()
{
    return TRUE;
}

BOOL CELITE::Deploy()
{
    m_flAccuracy = DEAGLE_MAX_INACCURACY;

    if( !m_iClip )
    {
        SetBits( m_fWeaponState, WEAPONSTATE_ELITE_LEFT );
    }

    return DefaultDeploy( "models/v_elite.mdl", "models/p_elite.mdl", ELITE_DRAW, "dualpistols", UseDecrement() );
}

void CELITE::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        ELITEFire( ( 1 - m_flAccuracy ) * 1.3, 0.2 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        ELITEFire( ( 1 - m_flAccuracy ) * 0.175, 0.2 );
    }
    else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        ELITEFire( ( 1 - m_flAccuracy ) * 0.08, 0.200 );
    }
    else
    {
        ELITEFire( ( 1 - m_flAccuracy ) * 0.1, 0.2 );
    }
}

void CELITE::Reload()
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    if( DefaultReload( ELITE_MAX_CLIP, ELITE_RELOAD, ELITE_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = DEAGLE_MAX_INACCURACY;
    }
}

void CELITE::ELITEFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    flCycleTime -= DEAGLE_ADJUSTED_CYCLE_TIME;
    m_iShotsFired++;

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

        if( m_flAccuracy > DEAGLE_MAX_INACCURACY )
        {
            m_flAccuracy = DEAGLE_MAX_INACCURACY;
        }
        else if( m_flAccuracy < DEAGLE_MIN_INACCURACY )
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
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    PLAYER_ANIM       playerAnim;
    unsigned short    usEvent;
    Vector            vecSource;

    if( FBitSet( m_fWeaponState, WEAPONSTATE_ELITE_LEFT ) )
    {
        ClearBits( m_fWeaponState, WEAPONSTATE_ELITE_LEFT );

        playerAnim    = PLAYER_ATTACK1;
        vecSource    = m_pPlayer->GetGunPosition() - gpGlobals->v_right * 5.0;
        usEvent        = m_usEliteLeft;
    }
    else
    {
        SetBits( m_fWeaponState, WEAPONSTATE_ELITE_LEFT );

        playerAnim = PLAYER_ATTACK1_RIGHT;
        vecSource  = m_pPlayer->GetGunPosition() + gpGlobals->v_right * 5.0;
        usEvent    = m_usEliteRight;
    }

    m_pPlayer->SetAnimation( playerAnim );

    Vector vecDir = m_pPlayer->FireBullets3(
        vecSource, gpGlobals->v_forward, flSpread, 8192.0,
        DEAGLE_PENETRATION, BULLET_PLAYER_9MM, DEAGLE_DAMAGE, DEAGLE_RANGE_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(),
        m_usEliteLeft, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.y * 100, m_iClip, FALSE, FALSE );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + DEAGLE_IDLE_TIME;
}

void CELITE::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + DEAGLE_IDLE_INTERVAL;
        SendWeaponAnim( m_iClip == 1 ? ELITE_IDLE : ELITE_IDLE_LEFT_EMPTY, UseDecrement() );
    }
}