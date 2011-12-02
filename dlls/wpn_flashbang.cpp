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

#define FLASHBANG_WEIGHT        1
#define FLASHBANG_MAX_CARRY     2
#define FLASHBANG_DEFAULT_GIVE  1
#define FLASHBANG_PLAYER_SPEED  250.0

enum flashbang_e
{
    FLASHBANG_IDLE = 0,
    FLASHBANG_PULLPIN,
    FLASHBANG_THROW,
    FLASHBANG_DEPLOY
};

enum shieldgrenade_e
{
    GRENADE_IDLE = 0,
    GRENADE_PULLPIN,
    GRENADE_THROW,
    GRENADE_DRAW,
    GRENADE_SHIELD_IDLE,
    GRENADE_SHIELD_UP,
    GRENADE_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_flashbang, CFlashbang );

void CFlashbang::Precache()
{
    PRECACHE_MODEL( "models/v_flashbang.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_flashbang.mdl" );

    PRECACHE_SOUND( "weapons/flashbang-1.wav" );
    PRECACHE_SOUND( "weapons/flashbang-2.wav" );
    PRECACHE_SOUND( "weapons/pinpull.wav"     );
}

void CFlashbang::Spawn()
{
    Precache();
    m_iId = WEAPON_FLASHBANG;

    SET_MODEL( ENT( pev ), "models/w_flashbang.mdl" );

    pev->dmg = 4.0;

    m_iDefaultAmmo   = FLASHBANG_DEFAULT_GIVE;
    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CFlashbang::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "Flashbang";
    p->iMaxAmmo1 = FLASHBANG_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_FLASHBANG;
    p->iWeight   = FLASHBANG_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}

int CFlashbang::iItemSlot()
{
    return 4;
}

BOOL CFlashbang::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CFlashbang::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CFlashbang::CanDrop()
{
    return FALSE;
}

BOOL CFlashbang::CanDeploy()
{
    return m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ];
}

BOOL CFlashbang::Deploy()
{
    m_flReleaseThrow = -1.0;
    m_flWeaponSpeed  = FLASHBANG_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_flashbang.mdl", "models/shield/p_shield_flashbang.mdl", GRENADE_DRAW, "shieldgren", UseDecrement() );
    else
        return DefaultDeploy( "models/v_flashbang.mdl", "models/p_flashbang.mdl", FLASHBANG_DEPLOY, "grenade", UseDecrement() );
}

void CFlashbang::Holster()
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_FLASHBANG );
        DestroyItem();
    }

    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;
}

void CFlashbang::PrimaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        return;
    }

    if( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( FLASHBANG_PULLPIN );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CFlashbang::SecondaryAttack()
{
    ShieldSecondaryFire( GRENADE_SHIELD_UP, GRENADE_SHIELD_DOWN );
}

void CFlashbang::ShieldSecondaryFire( int animUp, int animDown )
{
    if( !m_pPlayer->HasShield() || m_flStartThrow )
    {
        return;
    }

    if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
        ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

        SendWeaponAnim( animDown, UseDecrement() );
        strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );

        m_flWeaponSpeed = 250.0;
    }
    else
    {
        SetBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
        SetBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

        SendWeaponAnim( animUp, UseDecrement() );
        strcpy( m_pPlayer->m_szAnimExtention, "shielded" );

        m_flWeaponSpeed = 180.0;
    }

    m_pPlayer->UpdateShieldCrosshair( ~m_fWeaponState & WEAPONSTATE_SHIELD_DRAWN );

    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.4;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.6;
}

void CFlashbang::ResetPlayerShieldAnim()
{
    if( m_pPlayer->HasShield() && FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
    }
}

void CFlashbang::SetPlayerShieldAnim()
{
    if( m_pPlayer->HasShield() )
    {
        if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shield" );
        }
        else
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
        }
    }
}

void CFlashbang::WeaponIdle()
{
    if( !m_flReleaseThrow && m_flStartThrow )
    {
        m_flReleaseThrow = gpGlobals->time;
    }

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_flStartThrow )
    {
        m_pPlayer->Radio( "%!MRAD_FIREINHOLE", "#Fire_in_the_hole", PITCH_NORM, TRUE );

        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if( angThrow.x < 0 )
            angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

        float flVel = ( 90 - angThrow.x ) * 6;

        if( flVel > 750 )
        {
            flVel = 750;
        }

        UTIL_MakeVectors( angThrow );

        Vector vecSrc   = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16;
        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        CGrenade::ShootTimed( m_pPlayer->pev, vecSrc, vecThrow, 1.5 );

        SendWeaponAnim( FLASHBANG_THROW, UseDecrement() );
        SetPlayerShieldAnim();

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        m_flStartThrow = 0;

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
        m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.75;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

        if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
           m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.5;
           m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
           m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.5;
        }

        ResetPlayerShieldAnim();
        return;
    }
    else if( m_flReleaseThrow )
    {
        m_flStartThrow = 0;

        if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            SendWeaponAnim( FLASHBANG_DEPLOY, UseDecrement() );

            m_flReleaseThrow   = -1.0;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
        }
        else
        {
            RetireWeapon();
        }

        return;
    }

    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        if( m_pPlayer->HasShield() )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

            if( !FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
            {
                SendWeaponAnim( GRENADE_SHIELD_IDLE, UseDecrement() );
            }
        }
        else
        {
            SendWeaponAnim( FLASHBANG_IDLE, UseDecrement() );
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ( m_flReleaseThrow > 0.75 ) ? 2.5 : RANDOM_FLOAT( 10.0, 15.0 );
        }
    }
}