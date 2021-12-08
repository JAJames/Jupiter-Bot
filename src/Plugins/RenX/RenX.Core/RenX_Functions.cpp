/**
 * Copyright (C) 2014-2021 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include <ctime>
#include "jessilib/unicode.hpp"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "ServerManager.h"
#include "RenX_Functions.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

using namespace std::literals;

/** Team color translations */
static std::string_view NodColor = "04";
static std::string_view GDIColor = "08";
static std::string_view OtherColor = "14";

/** Short team name translations */
static std::string_view NodShortName = "Nod";
static std::string_view GDIShortName = "GDI";
static std::string_view OtherShortName = "N/A";

/** Long team name translations */
static std::string_view NodLongName = "Brotherhood of Nod";
static std::string_view GDILongName = "Global Defense Initiative";
static std::string_view OtherLongName = "Unknown";

/** RenegadeX RCON protocol message deliminator */
const char RenX::DelimC = '\x02';
const char RenX::DelimC3 = '\xA0';
const std::string_view RenX::DevBotName = "DevBot"sv;

/** WinType translations */

static std::string_view scoreWinTypeTranslation = "Domination (High Score)";
static std::string_view baseWinTypeTranslation = "Conquest (Base Destruction)";
static std::string_view beaconWinTypeTranslation = "Espionage (Beacon)";
static std::string_view tieWinTypeTranslation = "Draw (Tie)";
static std::string_view shutdownWinTypeTranslation = "Ceasefire (Shutdown)";
static std::string_view surrenderWinTypeTranslation = "Forfeit (Surrender)";
static std::string_view unknownWinTypeTranslation = "Aliens (Unknown)";

/** WinType plain translations */

static std::string_view scoreWinTypePlainTranslation = "High Score";
static std::string_view baseWinTypePlainTranslation = "Base Destruction";
static std::string_view beaconWinTypePlainTranslation = "Beacon";
static std::string_view tieWinTypePlainTranslation = "Tie";
static std::string_view shutdownWinTypePlainTranslation = "Shutdown";
static std::string_view surrenderWinTypePlainTranslation = "Surrender";
static std::string_view unknownWinTypePlainTranslation = "Unknown";

/** Vote translations */

static constexpr std::string_view translated_VoteMenuChoice_AddBots = "Add Bots"sv;
static constexpr std::string_view translated_VoteMenuChoice_ChangeMap = "Change Map"sv;
static constexpr std::string_view translated_VoteMenuChoice_Donate = "Donate"sv;
static constexpr std::string_view translated_VoteMenuChoice_Kick = "Kick"sv;
static constexpr std::string_view translated_VoteMenuChoice_RemoveBots = "Remove Bots"sv;
static constexpr std::string_view translated_VoteMenuChoice_RestartMatch = "Restart Match"sv;
static constexpr std::string_view translated_VoteMenuChoice_StartMatch = "Start Match"sv;
static constexpr std::string_view translated_VoteMenuChoice_Survey = "Survey"sv;

/** Object / damage type translations */

/** Nod Vehicles */
static constexpr std::string_view translated_Vehicle_Harvester_Nod = "Nod Harvester"sv;
static constexpr std::string_view translated_Vehicle_Buggy = "Buggy"sv;
static constexpr std::string_view translated_Vehicle_Artillery = "Mobile Artillery"sv;
static constexpr std::string_view translated_Vehicle_APC_Nod = "Nod APC"sv;
static constexpr std::string_view translated_Vehicle_LightTank = "Light Tank"sv;
static constexpr std::string_view translated_Vehicle_FlameTank = "Flame Tank"sv;
static constexpr std::string_view translated_Vehicle_StealthTank = "Stealth Tank"sv;
static constexpr std::string_view translated_Vehicle_Chinook_Nod = "Nod Chinook"sv;
static constexpr std::string_view translated_Vehicle_Apache = "Apache"sv;

static constexpr std::string_view translated_Vehicle_ReconBike = "Recon Bike"sv;
static constexpr std::string_view translated_Vehicle_TickTank = "Tick Tank"sv;

/** GDI Vehicles */
static constexpr std::string_view translated_Vehicle_Harvester_GDI = "GDI Harvester"sv;
static constexpr std::string_view translated_Vehicle_Humvee = "Humvee"sv;
static constexpr std::string_view translated_Vehicle_MRLS = "Mobile Rocket Launcher System"sv;
static constexpr std::string_view translated_Vehicle_APC_GDI = "GDI APC"sv;
static constexpr std::string_view translated_Vehicle_MediumTank = "Medium Tank"sv;
static constexpr std::string_view translated_Vehicle_MammothTank = "Mammoth Tank"sv;
static constexpr std::string_view translated_Vehicle_Chinook_GDI = "GDI Chinook"sv;
static constexpr std::string_view translated_Vehicle_Orca = "Orca"sv;

static constexpr std::string_view translated_Vehicle_HoverMRLS = "Hover MRLS"sv;
static constexpr std::string_view translated_Vehicle_Titan = "Titan"sv;
static constexpr std::string_view translated_Vehicle_Wolverine = "Wolverine"sv;

/** Other Vehicles */
static constexpr std::string_view translated_Vehicle_A10_DmgType_GattlingGun = "A10 Thunderbolt Gattling Gun"sv;
static constexpr std::string_view translated_Vehicle_A10_DmgType_Bomb = "A10 Thunderbolt Bomb"sv;
static constexpr std::string_view translated_Vehicle_AC130_DmgType_HeavyCannon = "AC130 Heavy Cannon"sv;
static constexpr std::string_view translated_Vehicle_AC130_DmgType_AutoCannon = "AC130 Auto Cannon"sv;

/** Weapons */
static constexpr std::string_view translated_Weapon_HeavyPistol = "Heavy Pistol"sv;
static constexpr std::string_view translated_Weapon_Carbine = "Carbine"sv;
static constexpr std::string_view translated_Weapon_Airstrike_GDI = "GDI Airstrike"sv;
static constexpr std::string_view translated_Weapon_Airstrike_Nod = "Nod Airstrike"sv;
static constexpr std::string_view translated_Weapon_TiberiumFlechetteRifle = "Tiberium Flechette Gun"sv;
static constexpr std::string_view translated_Weapon_TiberiumAutoRifle = "Tiberium Automatic Rifle"sv;
static constexpr std::string_view translated_Weapon_TiberiumAutoRifle_Blue = "Blue Tiberium Automatic Rifle Burst"sv;
static constexpr std::string_view translated_Weapon_EMPGrenade = "EMP Grenade"sv;
static constexpr std::string_view translated_Weapon_SmokeGrenade = "Smoke Grenade"sv;
static constexpr std::string_view translated_Weapon_TimedC4 = "Timed C4"sv;
static constexpr std::string_view translated_Weapon_RemoteC4 = "Remote C4"sv;
static constexpr std::string_view translated_Weapon_ProxyC4 = "Proximity C4"sv;
static constexpr std::string_view translated_Weapon_ATMine = "Anti-Tank Mine"sv;
static constexpr std::string_view translated_Weapon_IonCannonBeacon = "Ion Cannon Beacon"sv;
static constexpr std::string_view translated_Weapon_NukeBeacon = "Nuclear Missile Beacon"sv;
static constexpr std::string_view translated_Weapon_DeployedC4 = "Timed C4"sv;
static constexpr std::string_view translated_Weapon_DeployedTimedC4 = "Timed C4"sv;
static constexpr std::string_view translated_Weapon_DeployedRemoteC4 = "Remote C4"sv;
static constexpr std::string_view translated_Weapon_DeployedProxyC4 = "Proximity C4"sv;
static constexpr std::string_view translated_Weapon_DeployedATMine = "Anti-Tank Mine"sv;
static constexpr std::string_view translated_Weapon_DeployedIonCannonBeacon = "Ion Cannon Beacon"sv;
static constexpr std::string_view translated_Weapon_DeployedNukeBeacon = "Nuclear Missile Beacon"sv;
static constexpr std::string_view translated_Weapon_CrateNuke = "Nuclear Crate Explosion"sv;
static constexpr std::string_view translated_Weapon_DevNuke = "Nuclear Dev Explosion"sv;

/** Projectiles */
static constexpr std::string_view translated_Projectile_EMPGrenade = "EMP Grenade"sv;
static constexpr std::string_view translated_Projectile_SmokeGrenade = "Smoke Grenade"sv;

/** GDI Characters */
static constexpr std::string_view translated_GDI_Deadeye = "Deadeye"sv;
static constexpr std::string_view translated_GDI_Engineer = "GDI Engineer"sv;
static constexpr std::string_view translated_GDI_Grenadier = "Grenadier"sv;
static constexpr std::string_view translated_GDI_Gunner = "Gunner"sv;
static constexpr std::string_view translated_GDI_Havoc = "Havoc"sv;
static constexpr std::string_view translated_GDI_Hotwire = "Hotwire"sv;
static constexpr std::string_view translated_GDI_Marksman = "GDI Marksman"sv;
static constexpr std::string_view translated_GDI_McFarland = "McFarland"sv;
static constexpr std::string_view translated_GDI_Mobius = "Mobius"sv;
static constexpr std::string_view translated_GDI_Officer = "GDI Officer"sv;
static constexpr std::string_view translated_GDI_Patch = "Patch"sv;
static constexpr std::string_view translated_GDI_RocketSoldier = "GDI Rocket Soldier"sv;
static constexpr std::string_view translated_GDI_Shotgunner = "GDI Shotgunner"sv;
static constexpr std::string_view translated_GDI_Soldier = "GDI Soldier"sv;
static constexpr std::string_view translated_GDI_Sydney = "Sydney"sv;

/** Nod Characters */
static constexpr std::string_view translated_Nod_BlackHandSniper = "Black Hand Sniper"sv;
static constexpr std::string_view translated_Nod_ChemicalTrooper = "Chemical Trooper"sv;
static constexpr std::string_view translated_Nod_Engineer = "Nod Engineer"sv;
static constexpr std::string_view translated_Nod_FlameTrooper = "Flame Trooper"sv;
static constexpr std::string_view translated_Nod_LaserChainGunner = "Laser Chain Gunner"sv;
static constexpr std::string_view translated_Nod_Marksman = "Nod Marksman"sv;
static constexpr std::string_view translated_Nod_Mendoza = "Mendoza"sv;
static constexpr std::string_view translated_Nod_Officer = "Nod Officer"sv;
static constexpr std::string_view translated_Nod_Raveshaw = "Raveshaw"sv;
static constexpr std::string_view translated_Nod_RocketSoldier = "Nod Rocket Soldier"sv;
static constexpr std::string_view translated_Nod_Sakura = "Sakura"sv;
static constexpr std::string_view translated_Nod_Shotgunner = "Nod Shotgunner"sv;
static constexpr std::string_view translated_Nod_Soldier = "Nod Soldier"sv;
static constexpr std::string_view translated_Nod_StealthBlackHand = "Stealth Black Hand"sv;
static constexpr std::string_view translated_Nod_Technician = "Technician"sv;

/** Non-weapon damage types */
static constexpr std::string_view translated_DmgType_Suicided = "Suicide"sv;
static constexpr std::string_view translated_DmgType_Fell = "Fall"sv;
static constexpr std::string_view translated_DmgType_Tiberium = "Tiberium"sv;
static constexpr std::string_view translated_DmgType_TiberiumBleed = "Tiberium Decay"sv;
static constexpr std::string_view translated_DmgType_TiberiumBleed_Blue = "Blue Tiberium Decay"sv;
static constexpr std::string_view translated_DmgType_RanOver = "Crushed"sv;

/** Infantry weapons */
static constexpr std::string_view translated_DmgType_Pistol = "Pistol"sv;
static constexpr std::string_view translated_DmgType_SMG = "Machine Pistol"sv;
static constexpr std::string_view translated_DmgType_HeavyPistol = "Heavy Pistol"sv;
static constexpr std::string_view translated_DmgType_Carbine = "Carbine"sv;
static constexpr std::string_view translated_DmgType_TiberiumFlechetteRifle = "Tiberium Flechette Gun"sv; // Not a rifle.
static constexpr std::string_view translated_DmgType_TiberiumAutoRifle = "Tiberium Automatic Rifle"sv;
static constexpr std::string_view translated_DmgType_TiberiumAutoRifle_Blue = "Blue Tiberium Automatic Rifle Burst"sv;
static constexpr std::string_view translated_DmgType_TiberiumAutoRifle_Flechette_Blue = "Blue Tiberium Automatic Rifle"sv;

static constexpr std::string_view translated_DmgType_Grenade = "Grenade"sv;
static constexpr std::string_view translated_DmgType_TimedC4 = "Timed C4"sv;
static constexpr std::string_view translated_DmgType_RemoteC4 = "Remote C4"sv;
static constexpr std::string_view translated_DmgType_ProxyC4 = "Proximity C4"sv;
static constexpr std::string_view translated_DmgType_ATMine = "Anti-Tank Mine"sv;
static constexpr std::string_view translated_DmgType_EMPGrenade = "EMP Grenade"sv;
static constexpr std::string_view translated_DmgType_BurnC4 = "C4 Burn"sv;
static constexpr std::string_view translated_DmgType_FireBleed = "Fire Burn"sv; // Caused by C4 or flame weapons

static constexpr std::string_view translated_DmgType_AutoRifle = "Automatic Rifle"sv;
static constexpr std::string_view translated_DmgType_Shotgun = "Shotgun"sv;
static constexpr std::string_view translated_DmgType_FlameThrower = "Flamethrower"sv;
static constexpr std::string_view translated_DmgType_GrenadeLauncher = "Grenade Launcher"sv;
static constexpr std::string_view translated_DmgType_MarksmanRifle = "Marksman's Rifle"sv;

static constexpr std::string_view translated_DmgType_ChainGun = "Chain Gun"sv;
static constexpr std::string_view translated_DmgType_MissileLauncher = "Missile Launcher"sv;
static constexpr std::string_view translated_DmgType_MissileLauncher_Alt = "Missile Launcher"sv;
static constexpr std::string_view translated_DmgType_ChemicalThrower = "Chemical Spray Gun"sv;
static constexpr std::string_view translated_DmgType_LaserRifle = "Laser Rifle"sv;
static constexpr std::string_view translated_DmgType_TacticalRifle = "Tactical Rifle"sv;
static constexpr std::string_view translated_DmgType_RocketLauncher = "Rocket Launcher"sv;
static constexpr std::string_view translated_DmgType_LaserChainGun = "Laser Chain Gun"sv;
static constexpr std::string_view translated_DmgType_FlakCannon = "Flak"sv;
static constexpr std::string_view translated_DmgType_FlakCannon_Alt = "Concentrated Flak"sv;
static constexpr std::string_view translated_DmgType_SniperRifle = "Sniper Rifle"sv;
static constexpr std::string_view translated_DmgType_RamjetRifle = "Ramjet"sv;
static constexpr std::string_view translated_DmgType_Railgun = "Railgun"sv;
static constexpr std::string_view translated_DmgType_PersonalIonCannon = "Personal Ion Cannon"sv;
static constexpr std::string_view translated_DmgType_VoltRifle = "Volt Rifle"sv;
static constexpr std::string_view translated_DmgType_VoltRifle_Alt = "Volt Rifle Burst"sv;
static constexpr std::string_view translated_DmgType_VoltAutoRifle = "Volt Automatic Rifle"sv;
static constexpr std::string_view translated_DmgType_VoltAutoRifle_Alt = "Volt Automatic Rifle Burst"sv;

/** Vehicle weapons */
static constexpr std::string_view translated_DmgType_MammothTank_Missile = "Mammoth Tank Missile"sv;
static constexpr std::string_view translated_DmgType_MammothTank_Cannon = "Mammoth Tank Cannon"sv;
static constexpr std::string_view translated_DmgType_Orca_Missile = "Orca Missile"sv;
static constexpr std::string_view translated_DmgType_Orca_Gun = "Orca Gun"sv;
static constexpr std::string_view translated_DmgType_Orca_Passenger = "Orca Passenger Missile"sv;
static constexpr std::string_view translated_DmgType_Apache_Rocket = "Apache Rocket"sv;
static constexpr std::string_view translated_DmgType_Apache_Gun = "Apache Gun"sv;
static constexpr std::string_view translated_DmgType_Apache_Passenger = "Apache Passenger Missile"sv;

/** Other weapons */
static constexpr std::string_view translated_DmgType_AGT_MG = "Machine Gun"sv;
static constexpr std::string_view translated_DmgType_AGT_Rocket = "Rocket"sv;
static constexpr std::string_view translated_DmgType_Obelisk = "Obelisk Laser"sv;
static constexpr std::string_view translated_DmgType_GuardTower = "Guard Tower"sv;
static constexpr std::string_view translated_DmgType_Turret = "Turret"sv;
static constexpr std::string_view translated_DmgType_SAMSite = "SAM Site"sv;
static constexpr std::string_view translated_DmgType_AATower = "Anti-Air Guard Tower"sv;
static constexpr std::string_view translated_DmgType_GunEmpl = "Gun Emplacement Gattling Gun"sv;
static constexpr std::string_view translated_DmgType_GunEmpl_Alt = "Gun Emplacement Automatic Cannon"sv;
static constexpr std::string_view translated_DmgType_RocketEmpl_Swarm = "Rocket Emplacement Swarm Missile"sv;
static constexpr std::string_view translated_DmgType_RocketEmpl_Missile = "Rocket Emplacement Hellfire Missile"sv;
static constexpr std::string_view translated_DmgType_Nuke = "Nuclear Missile Strike"sv;
static constexpr std::string_view translated_DmgType_IonCannon = "Ion Cannon Strike"sv;

/** Nod Vehicles */
static constexpr std::string_view translated_DmgType_Harvester_Nod = "Nod Harvester"sv;
static constexpr std::string_view translated_DmgType_Buggy = "Buggy"sv;
static constexpr std::string_view translated_DmgType_Artillery = "Mobile Artillery"sv;
static constexpr std::string_view translated_DmgType_APC_Nod = "Nod APC"sv;
static constexpr std::string_view translated_DmgType_LightTank = "Light Tank"sv;
static constexpr std::string_view translated_DmgType_FlameTank = "Flame Tank"sv;
static constexpr std::string_view translated_DmgType_StealthTank = "Stealth Tank"sv;
static constexpr std::string_view translated_DmgType_Chinook_Nod = "Nod Chinook"sv;
static constexpr std::string_view translated_DmgType_Apache = "Apache"sv;

static constexpr std::string_view translated_Vehicle_ReconBike_DmgType = "Recon Bike"sv;
static constexpr std::string_view translated_Vehicle_TickTank_DmgType = "Tick Tank"sv;

/** GDI Vehicles */
static constexpr std::string_view translated_DmgType_Harvester_GDI = "GDI Harvester"sv;
static constexpr std::string_view translated_DmgType_Humvee = "Humvee"sv;
static constexpr std::string_view translated_DmgType_MRLS = "Mobile Rocket Launcher System"sv;
static constexpr std::string_view translated_DmgType_APC_GDI = "GDI APC"sv;
static constexpr std::string_view translated_DmgType_MediumTank = "Medium Tank"sv;
static constexpr std::string_view translated_DmgType_MammothTank = "Mammoth Tank"sv;
static constexpr std::string_view translated_DmgType_Chinook_GDI = "GDI Chinook"sv;
static constexpr std::string_view translated_DmgType_Orca = "Orca"sv;

static constexpr std::string_view translated_Vehicle_HoverMRLS_DmgType = "Hover MRLS"sv;
static constexpr std::string_view translated_Vehicle_Titan_DmgType = "Titan"sv;
static constexpr std::string_view translated_Vehicle_Wolverine_DmgType = "Wolverine"sv;

/** Other Vehicles */
static constexpr std::string_view translated_DmgType_A10_Missile = "A10 Missile"sv;

/** Nod structures */
static constexpr std::string_view translated_Building_HandOfNod = "Hand of Nod"sv;
static constexpr std::string_view translated_Building_AirTower = "Airstrip"sv;
static constexpr std::string_view translated_Building_Airstrip = "Airstrip"sv;
static constexpr std::string_view translated_Building_Refinery_Nod = "Nod Refinery"sv;
static constexpr std::string_view translated_Building_PowerPlant_Nod = "Nod Power Plant"sv;
static constexpr std::string_view translated_Building_HandOfNod_Internals = "Hand of Nod"sv;
static constexpr std::string_view translated_Building_AirTower_Internals = "Airstrip"sv;
static constexpr std::string_view translated_Building_Airstrip_Internals = "Airstrip"sv;
static constexpr std::string_view translated_Building_Refinery_Nod_Internals = "Nod Refinery"sv;
static constexpr std::string_view translated_Building_PowerPlant_Nod_Internals = "Nod Power Plant"sv;

/** GDI structures */
static constexpr std::string_view translated_Building_Barracks = "Barracks"sv;
static constexpr std::string_view translated_Building_WeaponsFactory = "Weapons Factory"sv;
static constexpr std::string_view translated_Building_Refinery_GDI = "GDI Refinery"sv;
static constexpr std::string_view translated_Building_PowerPlant_GDI = "GDI Power Plant"sv;
static constexpr std::string_view translated_Building_Barracks_Internals = "Barracks"sv;
static constexpr std::string_view translated_Building_WeaponsFactory_Internals = "Weapons Factory"sv;
static constexpr std::string_view translated_Building_Refinery_GDI_Internals = "GDI Refinery"sv;
static constexpr std::string_view translated_Building_PowerPlant_GDI_Internals = "GDI Power Plant"sv;

/** Defense structures */
static constexpr std::string_view translated_Building_AdvancedGuardTower = "Advanced Guard Tower"sv;
static constexpr std::string_view translated_Building_Obelisk = "Obelisk of Light"sv;
static constexpr std::string_view translated_Building_AdvancedGuardTower_Internals = "Advanced Guard Tower"sv;
static constexpr std::string_view translated_Building_Obelisk_Internals = "Obelisk of Light"sv;

/** Other structures */
static constexpr std::string_view translated_Building_Silo = "Tiberium Silo"sv;
static constexpr std::string_view translated_Building_CommCentre = "Communications Center"sv;
static constexpr std::string_view translated_Building_Silo_Internals = "Tiberium Silo"sv;
static constexpr std::string_view translated_Building_CommCentre_Internals = "Communications Center"sv;

/** Fort structures */
static constexpr std::string_view translated_CapturableMCT_Fort = "Fort"sv;
static constexpr std::string_view translated_CapturableMCT_MC = "Medical Center"sv;
static constexpr std::string_view translated_CapturableMCT_Fort_Internals = "Fort"sv;
static constexpr std::string_view translated_CapturableMCT_MC_Internals = "Medical Center"sv;

/** Defences */
static constexpr std::string_view translated_Defence_GuardTower = "Guard Tower"sv;
static constexpr std::string_view translated_Defence_Turret = "Turret"sv;
static constexpr std::string_view translated_Defence_SAMSite = "SAM Site"sv;
static constexpr std::string_view translated_Defence_AATower = "Anti-Air Guard Tower"sv;
static constexpr std::string_view translated_Defence_GunEmplacement = "Gun Emplacement"sv;
static constexpr std::string_view translated_Defence_RocketEmplacement = "Rocket Emplacement"sv;

/** Defences - Sentinels */
static constexpr std::string_view translated_Sentinel_AGT_MG_Base = "Advanced Guard Tower"sv;
static constexpr std::string_view translated_Sentinel_AGT_Rockets_Base = "Advanced Guard Tower"sv;
static constexpr std::string_view translated_Sentinel_Obelisk_Laser_Base = "Obelisk of Light"sv;

/** UT damage types */
static constexpr std::string_view translated_UTDmgType_VehicleExplosion = "Vehicle Explosion"sv;
static constexpr std::string_view translated_UTDmgType_Drowned = "Drowned"sv;

/** Beacons */
static constexpr std::string_view translated_IonCannonBeacon = "Ion Cannon Beacon"sv;
static constexpr std::string_view translated_NukeBeacon = "Nuclear Strike Beacon"sv;
static constexpr std::string_view translated_KillZDamageType = "Kill Zone"sv;

RenX::TeamType RenX::getTeam(int teamNum)
{
	switch (teamNum)
	{
	case 0:
		return RenX::TeamType::GDI;
	case 1:
		return RenX::TeamType::Nod;
	case 255:
		return RenX::TeamType::None;
	default:
		return RenX::TeamType::Other;
	}
}

RenX::TeamType RenX::getTeam(std::string_view team) {
	if (jessilib::equalsi(team, "GDI"sv)) {
		return RenX::TeamType::GDI;
	}

	if (jessilib::equalsi(team, "Nod"sv)) {
		return RenX::TeamType::Nod;
	}

	if (team.empty() || jessilib::equalsi(team, "Neutral"sv) || jessilib::equalsi(team, "Civilians"sv)) {
		return RenX::TeamType::None;
	}

	return RenX::TeamType::Other;
}

std::string_view RenX::getTeamColor(TeamType team)
{
	switch (team)
	{
	case RenX::TeamType::GDI:
		return GDIColor;
	case RenX::TeamType::Nod:
		return NodColor;
	default:
		return OtherColor;
	}
}

std::string_view RenX::getTeamName(TeamType team)
{
	switch (team)
	{
	case RenX::TeamType::GDI:
		return GDIShortName;
	case RenX::TeamType::Nod:
		return NodShortName;
	default:
		return OtherShortName;
	}
}

std::string_view RenX::getFullTeamName(TeamType team)
{
	switch (team)
	{
	case RenX::TeamType::GDI:
		return GDILongName;
	case RenX::TeamType::Nod:
		return NodLongName;
	default:
		return OtherLongName;
	}
}

RenX::TeamType RenX::getEnemy(TeamType team)
{
	switch (team)
	{
	case RenX::TeamType::GDI:
		return RenX::TeamType::Nod;
	case RenX::TeamType::Nod:
		return RenX::TeamType::GDI;
	default:
		return RenX::TeamType::Other;
	}
}

std::string_view RenX::getCharacter(std::string_view object) {
	if (object.find("Rx_"sv) == 0)
		object.remove_prefix(3);
	if (object.find("InventoryManager_"sv) == 0)
		object.remove_prefix(17);
	else if (object.find("FamilyInfo_"sv) == 0)
		object.remove_prefix(11);

	return object;
}

std::string_view translateCharacter(std::string_view object)
{
	if (object.find("GDI_"sv) == 0)
	{
		object.remove_prefix(4);
		if (object == "Deadeye"sv) return translated_GDI_Deadeye;
		if (object == "Engineer"sv) return translated_GDI_Engineer;
		if (object == "Grenadier"sv) return translated_GDI_Grenadier;
		if (object == "Gunner"sv) return translated_GDI_Gunner;
		if (object == "Havoc"sv) return translated_GDI_Havoc;
		if (object == "Hotwire"sv) return translated_GDI_Hotwire;
		if (object == "Marksman"sv) return translated_GDI_Marksman;
		if (object == "McFarland"sv) return translated_GDI_McFarland;
		if (object == "Mobius"sv) return translated_GDI_Mobius;
		if (object == "Officer"sv) return translated_GDI_Officer;
		if (object == "Patch"sv) return translated_GDI_Patch;
		if (object == "RocketSoldier"sv) return translated_GDI_RocketSoldier;
		if (object == "Shotgunner"sv) return translated_GDI_Shotgunner;
		if (object == "Soldier"sv) return translated_GDI_Soldier;
		if (object == "Sydney"sv) return translated_GDI_Sydney;
	}
	else if (object.find("Nod_"sv) == 0)
	{
		object.remove_prefix(4);
		if (object == "BlackHandSniper"sv) return translated_Nod_BlackHandSniper;
		if (object == "ChemicalTrooper"sv) return translated_Nod_ChemicalTrooper;
		if (object == "Engineer"sv) return translated_Nod_Engineer;
		if (object == "FlameTrooper"sv) return translated_Nod_FlameTrooper;
		if (object == "LaserChainGunner"sv) return translated_Nod_LaserChainGunner;
		if (object == "Marksman"sv) return translated_Nod_Marksman;
		if (object == "Mendoza"sv) return translated_Nod_Mendoza;
		if (object == "Officer"sv) return translated_Nod_Officer;
		if (object == "Raveshaw"sv) return translated_Nod_Raveshaw;
		if (object == "RocketSoldier"sv) return translated_Nod_RocketSoldier;
		if (object == "Sakura"sv) return translated_Nod_Sakura;
		if (object == "Shotgunner"sv) return translated_Nod_Shotgunner;
		if (object == "Soldier"sv) return translated_Nod_Soldier;
		if (object == "StealthBlackHand"sv) return translated_Nod_StealthBlackHand;
		if (object == "Technician"sv) return translated_Nod_Technician;
	}

	return object;
}

// TODO: Use a map...
std::string_view RenX::translateName(std::string_view obj) {
	if (obj.empty())
		return ""sv;

	std::string_view iniTranslation = RenX::getCore()->getConfig()["Name"sv].get(obj);
	if (!iniTranslation.empty())
		return iniTranslation;

	std::string_view object = obj;

	if (object.find("nBab_"sv) == 0)
		object.remove_prefix(5);
	
	if (object.find("Rx_"sv) == 0)
		object.remove_prefix(3);
	else if (object.find("TS_"sv) == 0)
		object.remove_prefix(3);

	if (object.find("Vehicle_"sv) == 0)
	{
		object.remove_prefix(8);

		/** Nod Vehicles */
		if (object == "Harvester_Nod"sv) return translated_Vehicle_Harvester_Nod;
		if (object == "Buggy"sv) return translated_Vehicle_Buggy;
		if (object == "Artillery"sv) return translated_Vehicle_Artillery;
		if (object == "APC_Nod"sv) return translated_Vehicle_APC_Nod;
		if (object == "LightTank"sv) return translated_Vehicle_LightTank;
		if (object == "FlameTank"sv) return translated_Vehicle_FlameTank;
		if (object == "StealthTank"sv) return translated_Vehicle_StealthTank;
		if (object == "Chinook_Nod"sv) return translated_Vehicle_Chinook_Nod;
		if (object == "Apache"sv) return translated_Vehicle_Apache;

		if (object == "ReconBike"sv) return translated_Vehicle_ReconBike;
		if (object == "TickTank"sv) return translated_Vehicle_TickTank;

		/** GDI Vehicles */
		if (object == "Harvester_GDI"sv) return translated_Vehicle_Harvester_GDI;
		if (object == "Humvee"sv) return translated_Vehicle_Humvee;
		if (object == "MRLS"sv) return translated_Vehicle_MRLS;
		if (object == "APC_GDI"sv) return translated_Vehicle_APC_GDI;
		if (object == "MediumTank"sv) return translated_Vehicle_MediumTank;
		if (object == "MammothTank"sv) return translated_Vehicle_MammothTank;
		if (object == "Chinook_GDI"sv) return translated_Vehicle_Chinook_GDI;
		if (object == "Orca"sv) return translated_Vehicle_Orca;

		if (object == "HoverMRLS"sv) return translated_Vehicle_HoverMRLS;
		if (object == "Titan"sv) return translated_Vehicle_Titan;
		if (object == "Wolverine"sv) return translated_Vehicle_Wolverine;

		/** Other Vehicles */
		if (object == "A10_DmgType_GattlingGun"sv) return translated_Vehicle_A10_DmgType_GattlingGun;
		if (object == "A10_DmgType_Bomb"sv) return translated_Vehicle_A10_DmgType_Bomb;
		if (object == "AC130_DmgType_HeavyCannon"sv) return translated_Vehicle_AC130_DmgType_HeavyCannon;
		if (object == "AC130_DmgType_AutoCannon"sv) return translated_Vehicle_AC130_DmgType_AutoCannon;
	}
	else if (object.find("Weapon_"sv) == 0)
	{
		object.remove_prefix(7);
		if (object == "HeavyPistol"sv) return translated_Weapon_HeavyPistol;
		if (object == "Carbine"sv) return translated_Weapon_Carbine;
		if (object == "Airstrike_GDI"sv) return translated_Weapon_Airstrike_GDI;
		if (object == "Airstrike_Nod"sv) return translated_Weapon_Airstrike_Nod;
		if (object == "TiberiumFlechetteRifle"sv) return translated_Weapon_TiberiumFlechetteRifle;
		if (object == "TiberiumAutoRifle"sv) return translated_Weapon_TiberiumAutoRifle;
		if (object == "TiberiumAutoRifle_Blue"sv) return translated_Weapon_TiberiumAutoRifle_Blue;
		if (object == "EMPGrenade"sv) return translated_Weapon_EMPGrenade;
		if (object == "SmokeGrenade"sv) return translated_Weapon_SmokeGrenade;
		if (object == "TimedC4"sv) return translated_Weapon_TimedC4;
		if (object == "RemoteC4"sv) return translated_Weapon_RemoteC4;
		if (object == "ProxyC4"sv) return translated_Weapon_ProxyC4;
		if (object == "ATMine"sv) return translated_Weapon_ATMine;
		if (object == "IonCannonBeacon"sv) return translated_Weapon_IonCannonBeacon;
		if (object == "NukeBeacon"sv) return translated_Weapon_NukeBeacon;
		if (object == "DeployedC4"sv) return translated_Weapon_DeployedC4;
		if (object == "DeployedTimedC4"sv) return translated_Weapon_DeployedTimedC4;
		if (object == "DeployedRemoteC4"sv) return translated_Weapon_DeployedRemoteC4;
		if (object == "DeployedProxyC4"sv) return translated_Weapon_DeployedProxyC4;
		if (object == "DeployedATMine"sv) return translated_Weapon_DeployedATMine;
		if (object == "DeployedIonCannonBeacon"sv) return translated_Weapon_DeployedIonCannonBeacon;
		if (object == "DeployedNukeBeacon"sv) return translated_Weapon_DeployedNukeBeacon;
		if (object == "CrateNuke"sv) return translated_Weapon_CrateNuke;
	}
	else if (object.find("Projectile_"sv) == 0)
	{
		object.remove_prefix(11);
		if (object == "EMPGrenade"sv) return translated_Projectile_EMPGrenade;
		if (object == "SmokeGrenade"sv) return translated_Projectile_SmokeGrenade;
	}
	else if (object.find("InventoryManager_"sv) == 0)
	{
		object.remove_prefix(17);
		return translateCharacter(object);
	}
	else if (object.find("FamilyInfo_"sv) == 0)
	{
		object.remove_prefix(11);
		return translateCharacter(object);
	}
	else if (object.find("DmgType_"sv) == 0)
	{
		object.remove_prefix(8);

		/** Non-weapon damage types */
		if (object == "Suicided"sv) return translated_DmgType_Suicided;
		if (object == "Fell"sv) return translated_DmgType_Fell;
		if (object == "Tiberium"sv) return translated_DmgType_Tiberium;
		if (object == "TiberiumBleed"sv) return translated_DmgType_TiberiumBleed;
		if (object == "TiberiumBleed_Blue"sv) return translated_DmgType_TiberiumBleed_Blue;
		if (object == "RanOver"sv) return translated_DmgType_RanOver;

		/** Infantry weapons */
		if (object == "Pistol"sv) return translated_DmgType_Pistol;
		if (object == "SMG"sv) return translated_DmgType_SMG;
		if (object == "HeavyPistol"sv) return translated_DmgType_HeavyPistol;
		if (object == "Carbine"sv) return translated_DmgType_Carbine;
		if (object == "TiberiumFlechetteRifle"sv) return translated_DmgType_TiberiumFlechetteRifle; // Not a rifle.
		if (object == "TiberiumAutoRifle"sv) return translated_DmgType_TiberiumAutoRifle;
		if (object == "TiberiumAutoRifle_Blue"sv) return translated_DmgType_TiberiumAutoRifle_Blue;
		if (object == "TiberiumAutoRifle_Flechette_Blue"sv) return translated_DmgType_TiberiumAutoRifle_Flechette_Blue;

		if (object == "Grenade"sv) return translated_DmgType_Grenade;
		if (object == "TimedC4"sv) return translated_DmgType_TimedC4;
		if (object == "RemoteC4"sv) return translated_DmgType_RemoteC4;
		if (object == "ProxyC4"sv) return translated_DmgType_ProxyC4;
		if (object == "ATMine"sv) return translated_DmgType_ATMine;
		if (object == "EMPGrenade"sv) return translated_DmgType_EMPGrenade;
		if (object == "BurnC4"sv) return translated_DmgType_BurnC4;
		if (object == "FireBleed"sv) return translated_DmgType_FireBleed; // Caused by C4 or flame weapons

		if (object == "AutoRifle"sv) return translated_DmgType_AutoRifle;
		if (object == "Shotgun"sv) return translated_DmgType_Shotgun;
		if (object == "FlameThrower"sv) return translated_DmgType_FlameThrower;
		if (object == "GrenadeLauncher"sv) return translated_DmgType_GrenadeLauncher;
		if (object == "MarksmanRifle"sv) return translated_DmgType_MarksmanRifle;

		if (object == "ChainGun"sv) return translated_DmgType_ChainGun;
		if (object == "MissileLauncher"sv) return translated_DmgType_MissileLauncher;
		if (object == "MissileLauncher_Alt"sv) return translated_DmgType_MissileLauncher_Alt;
		if (object == "ChemicalThrower"sv) return translated_DmgType_ChemicalThrower;
		if (object == "LaserRifle"sv) return translated_DmgType_LaserRifle;
		if (object == "TacticalRifle"sv) return translated_DmgType_TacticalRifle;
		if (object == "RocketLauncher"sv) return translated_DmgType_RocketLauncher;
		if (object == "LaserChainGun"sv) return translated_DmgType_LaserChainGun;
		if (object == "FlakCannon"sv) return translated_DmgType_FlakCannon;
		if (object == "FlakCannon_Alt"sv) return translated_DmgType_FlakCannon_Alt;
		if (object == "SniperRifle"sv) return translated_DmgType_SniperRifle;
		if (object == "RamjetRifle"sv) return translated_DmgType_RamjetRifle;
		if (object == "Railgun"sv) return translated_DmgType_Railgun;
		if (object == "PersonalIonCannon"sv) return translated_DmgType_PersonalIonCannon;
		if (object == "VoltRifle"sv) return translated_DmgType_VoltRifle;
		if (object == "VoltRifle_Alt"sv) return translated_DmgType_VoltRifle_Alt;
		if (object == "VoltAutoRifle"sv) return translated_DmgType_VoltAutoRifle;
		if (object == "VoltAutoRifle_Alt"sv) return translated_DmgType_VoltAutoRifle_Alt;

		/** Vehicle weapons */
		if (object == "MammothTank_Missile"sv) return translated_DmgType_MammothTank_Missile;
		if (object == "MammothTank_Cannon"sv) return translated_DmgType_MammothTank_Cannon;
		if (object == "Orca_Missile"sv) return translated_DmgType_Orca_Missile;
		if (object == "Orca_Gun"sv) return translated_DmgType_Orca_Gun;
		if (object == "Orca_Passenger"sv) return translated_DmgType_Orca_Passenger;
		if (object == "Apache_Rocket"sv) return translated_DmgType_Apache_Rocket;
		if (object == "Apache_Gun"sv) return translated_DmgType_Apache_Gun;
		if (object == "Apache_Passenger"sv) return translated_DmgType_Apache_Passenger;

		/** Base Defence Weapons */
		if (object == "AGT_MG"sv) return translated_DmgType_AGT_MG;
		if (object == "AGT_Rocket"sv) return translated_DmgType_AGT_Rocket;
		if (object == "Obelisk"sv) return translated_DmgType_Obelisk;

		/** Defence Structure Weapons */
		if (object == "AGT_MG"sv) return translated_DmgType_AGT_MG;
		if (object == "AGT_Rocket"sv) return translated_DmgType_AGT_Rocket;
		if (object == "Obelisk"sv) return translated_DmgType_Obelisk;
		if (object == "GuardTower"sv) return translated_DmgType_GuardTower;
		if (object == "Turret"sv) return translated_DmgType_Turret;
		if (object == "SAMSite"sv) return translated_DmgType_SAMSite;
		if (object == "AATower"sv) return translated_DmgType_AATower;
		if (object == "GunEmpl"sv) return translated_DmgType_GunEmpl;
		if (object == "GunEmpl_Alt"sv) return translated_DmgType_GunEmpl_Alt;
		if (object == "RocketEmpl_Swarm"sv) return translated_DmgType_RocketEmpl_Swarm;
		if (object == "RocketEmpl_Missile"sv) return translated_DmgType_RocketEmpl_Missile;

		/** Other Weapons */
		if (object == "Nuke"sv) return translated_DmgType_Nuke;
		if (object == "IonCannon"sv) return translated_DmgType_IonCannon;

		/** Nod Vehicles */
		if (object == "Harvester_Nod"sv) return translated_DmgType_Harvester_Nod;
		if (object == "Buggy"sv) return translated_DmgType_Buggy;
		if (object == "Artillery"sv) return translated_DmgType_Artillery;
		if (object == "APC_Nod"sv) return translated_DmgType_APC_Nod;
		if (object == "LightTank"sv) return translated_DmgType_LightTank;
		if (object == "FlameTank"sv) return translated_DmgType_FlameTank;
		if (object == "StealthTank"sv) return translated_DmgType_StealthTank;
		if (object == "Chinook_Nod"sv) return translated_DmgType_Chinook_Nod;
		if (object == "Apache"sv) return translated_DmgType_Apache;

		/** GDI Vehicles */
		if (object == "Harvester_GDI"sv) return translated_DmgType_Harvester_GDI;
		if (object == "Humvee"sv) return translated_DmgType_Humvee;
		if (object == "MRLS"sv) return translated_DmgType_MRLS;
		if (object == "APC_GDI"sv) return translated_DmgType_APC_GDI;
		if (object == "MediumTank"sv) return translated_DmgType_MediumTank;
		if (object == "MammothTank"sv) return translated_DmgType_MammothTank;
		if (object == "Chinook_GDI"sv) return translated_DmgType_Chinook_GDI;
		if (object == "Orca"sv) return translated_DmgType_Orca;

		/** Other Vehicles */
		if (object == "A10_Missile"sv) return translated_DmgType_A10_Missile;
	}
	else if (object.find("Building_"sv) == 0)
	{
		object.remove_prefix(9);
		/** Nod structures */
		if (object == "HandOfNod"sv) return translated_Building_HandOfNod;
		if (object == "AirTower"sv) return translated_Building_AirTower;
		if (object == "Airstrip"sv) return translated_Building_Airstrip;
		if (object == "Refinery_Nod"sv) return translated_Building_Refinery_Nod;
		if (object == "PowerPlant_Nod"sv) return translated_Building_PowerPlant_Nod;
		if (object == "HandOfNod_Internals"sv) return translated_Building_HandOfNod_Internals;
		if (object == "AirTower_Internals"sv) return translated_Building_AirTower_Internals;
		if (object == "Airstrip_Internals"sv) return translated_Building_Airstrip_Internals;
		if (object == "Refinery_Nod_Internals"sv) return translated_Building_Refinery_Nod_Internals;
		if (object == "PowerPlant_Nod_Internals"sv) return translated_Building_PowerPlant_Nod_Internals;
		
		/** GDI structures */
		if (object == "Barracks"sv) return translated_Building_Barracks;
		if (object == "WeaponsFactory"sv) return translated_Building_WeaponsFactory;
		if (object == "Refinery_GDI"sv) return translated_Building_Refinery_GDI;
		if (object == "PowerPlant_GDI"sv) return translated_Building_PowerPlant_GDI;
		if (object == "Barracks_Internals"sv) return translated_Building_Barracks_Internals;
		if (object == "WeaponsFactory_Internals"sv) return translated_Building_WeaponsFactory_Internals;
		if (object == "Refinery_GDI_Internals"sv) return translated_Building_Refinery_GDI_Internals;
		if (object == "PowerPlant_GDI_Internals"sv) return translated_Building_PowerPlant_GDI_Internals;

		/** Defense structures */
		if (object == "AdvancedGuardTower"sv) return translated_Building_AdvancedGuardTower;
		if (object == "Obelisk"sv) return translated_Building_Obelisk;
		if (object == "AdvancedGuardTower_Internals"sv) return translated_Building_AdvancedGuardTower_Internals;
		if (object == "Obelisk_Internals"sv) return translated_Building_Obelisk_Internals;

		/** Other structures */
		if (object == "Silo"sv) return translated_Building_Silo;
		if (object == "CommCentre"sv) return translated_Building_CommCentre;
		if (object == "Silo_Internals"sv) return translated_Building_Silo_Internals;
		if (object == "CommCentre_Internals"sv) return translated_Building_CommCentre_Internals;
	}
	else if (object.find("CapturableMCT_"sv) == 0)
	{
		object.remove_prefix(14);
		if (object == "Fort"sv) return translated_CapturableMCT_Fort;
		if (object == "MC"sv) return translated_CapturableMCT_MC;
		if (object == "Fort_Internals"sv) return translated_CapturableMCT_Fort_Internals;
		if (object == "MC_Internals"sv) return translated_CapturableMCT_MC_Internals;
	}
	else if (object.find("Defence_"sv) == 0)
	{
		object.remove_prefix(8);
		if (object == "GuardTower"sv) return translated_Defence_GuardTower;
		if (object == "Turret"sv) return translated_Defence_Turret;
		if (object == "SAMSite"sv) return translated_Defence_SAMSite;
		if (object == "AATower"sv) return translated_Defence_AATower;
		if (object == "GunEmplacement"sv) return translated_Defence_GunEmplacement;
		if (object == "RocketEmplacement"sv) return translated_Defence_RocketEmplacement;
	}
	else if (object.find("Sentinel_"sv) == 0)
	{
		object.remove_prefix(9);
		if (object == "AGT_MG_Base"sv) return translated_Sentinel_AGT_MG_Base;
		if (object == "AGT_Rockets_Base"sv) return translated_Sentinel_AGT_Rockets_Base;
		if (object == "Obelisk_Laser_Base"sv) return translated_Sentinel_Obelisk_Laser_Base;
	}
	else if (object.find("UTDmgType_"sv) == 0)
	{
		object.remove_prefix(10);
		if (object == "VehicleExplosion"sv) return translated_UTDmgType_VehicleExplosion;
		if (object == "Drowned"sv) return translated_UTDmgType_Drowned;
	}
	else if (object.find("VoteMenuChoice_"sv) == 0)
	{
		object.remove_prefix(15);
		if (object == "AddBots"sv) return translated_VoteMenuChoice_AddBots;
		if (object == "ChangeMap"sv) return translated_VoteMenuChoice_ChangeMap;
		if (object == "Donate"sv) return translated_VoteMenuChoice_Donate;
		if (object == "Kick"sv) return translated_VoteMenuChoice_Kick;
		if (object == "RemoveBots"sv) return translated_VoteMenuChoice_RemoveBots;
		if (object == "RestartMatch"sv) return translated_VoteMenuChoice_RestartMatch;
		if (object == "StartMatch"sv) return translated_VoteMenuChoice_StartMatch;
		if (object == "Survey"sv) return translated_VoteMenuChoice_Survey;
	}
	else if (object == "IonCannonBeacon"sv) return translated_IonCannonBeacon;
	else if (object == "NukeBeacon"sv) return translated_NukeBeacon;
	else if (object == "KillZDamageType"sv) return translated_KillZDamageType;
	return translateCharacter(object);
}

std::string_view RenX::translateWinType(RenX::WinType winType)
{
	switch (winType)
	{
	case RenX::WinType::Score:
		return scoreWinTypeTranslation;
	case RenX::WinType::Base:
		return baseWinTypeTranslation;
	case RenX::WinType::Beacon:
		return beaconWinTypeTranslation;
	case RenX::WinType::Tie:
		return tieWinTypeTranslation;
	case RenX::WinType::Shutdown:
		return shutdownWinTypeTranslation;
	case RenX::WinType::Surrender:
		return surrenderWinTypeTranslation;
	case RenX::WinType::Unknown:
	default:
		return unknownWinTypeTranslation;
	}
}

std::string_view RenX::translateWinTypePlain(RenX::WinType winType)
{
	switch (winType)
	{
	case RenX::WinType::Score:
		return scoreWinTypePlainTranslation;
	case RenX::WinType::Base:
		return baseWinTypePlainTranslation;
	case RenX::WinType::Beacon:
		return beaconWinTypePlainTranslation;
	case RenX::WinType::Tie:
		return tieWinTypePlainTranslation;
	case RenX::WinType::Shutdown:
		return shutdownWinTypePlainTranslation;
	case RenX::WinType::Surrender:
		return surrenderWinTypePlainTranslation;
	case RenX::WinType::Unknown:
	default:
		return unknownWinTypePlainTranslation;
	}
}

void RenX::initTranslations(Jupiter::Config &translationsFile)
{
	NodColor = translationsFile["TeamColor"sv].get("Nod"sv, "04"sv);
	GDIColor = translationsFile["TeamColor"sv].get("GDI"sv, "08"sv);
	OtherColor = translationsFile["TeamColor"sv].get("Other"sv, "14"sv);

	NodShortName = translationsFile["ShortTeamName"sv].get("Nod"sv, "Nod"sv);
	GDIShortName = translationsFile["ShortTeamName"sv].get("GDI"sv, "GDI"sv);
	OtherShortName = translationsFile["ShortTeamName"sv].get("Other"sv, "N/A"sv);
	NodLongName = translationsFile["LongTeamName"sv].get("Nod"sv, "Brotherhood of Nod"sv);
	GDILongName = translationsFile["LongTeamName"sv].get("GDI"sv, "Global Defense Initiative"sv);
	OtherLongName = translationsFile["LongTeamName"sv].get("Other"sv, "Unknown"sv);

	scoreWinTypeTranslation = translationsFile["WinType"sv].get("Score"sv, "Domination (High Score)"sv);
	baseWinTypeTranslation = translationsFile["WinType"sv].get("Base"sv, "Conquest (Base Destruction)"sv);
	beaconWinTypeTranslation = translationsFile["WinType"sv].get("Beacon"sv, "Espionage (Beacon)"sv);
	tieWinTypeTranslation = translationsFile["WinType"sv].get("Tie"sv, "Draw (Tie)"sv);
	shutdownWinTypeTranslation = translationsFile["WinType"sv].get("Shutdown"sv, "Ceasefire (Shutdown)"sv);
	surrenderWinTypeTranslation = translationsFile["WinType"sv].get("Surrender"sv, "Forfeit (Surrender)"sv);
	unknownWinTypeTranslation = translationsFile["WinType"sv].get("Unknown"sv, "Aliens (Unknown)"sv);

	scoreWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Score"sv, "High Score"sv);
	baseWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Base"sv, "Base Destruction"sv);
	beaconWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Beacon"sv, "Beacon"sv);
	tieWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Tie"sv, "Tie"sv);
	shutdownWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Shutdown"sv, "Shutdown"sv);
	surrenderWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Surrender"sv, "Surrender"sv);
	unknownWinTypePlainTranslation = translationsFile["WinTypePlain"sv].get("Unknown"sv, "Unknown"sv);
}

std::string RenX::getFormattedPlayerName(const RenX::PlayerInfo &player)
{
	std::string r = player.formatNamePrefix;
	r += IRCCOLOR;
	r += RenX::getTeamColor(player.team);
	r += player.name;
	return r;
}

std::string RenX::formatGUID(const RenX::Map &map)
{
	return string_printf("%.16llX%.16llX", map.guid[0], map.guid[1]);
}

std::chrono::milliseconds RenX::getServerTime(const RenX::PlayerInfo &player)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.joinTime);
}

std::string RenX::default_uuid_func(RenX::PlayerInfo &player)
{
	if (player.steamid != 0U)
		return string_printf("0x%.16llX", player.steamid);

	return std::string();
}

double RenX::getKillDeathRatio(const RenX::PlayerInfo &player, bool includeSuicides)
{
	double deaths = player.deaths;

	if (includeSuicides == false)
		deaths -= player.suicides;

	if (deaths == 0)
		deaths = 1;

	return static_cast<double>(player.kills) / deaths;
}

double RenX::getHeadshotKillRatio(const RenX::PlayerInfo &player)
{
	if (player.kills == 0)
		return 0;

	return static_cast<double>(player.headshots) / static_cast<double>(player.kills);
}

std::string RenX::escapifyRCON(std::string_view str) {
	const char *ptr = str.data();
	size_t length = str.size();
	std::string result;
	result.reserve(str.size() + 32);
	uint16_t value;

	while (length != 0)
	{
		if ((*ptr & 0x80) != 0) // UTF-8 sequence
		{
			if (length < 2)
				break;

			if ((*ptr & 0x40) != 0) // validity check
			{
				// get codepoint value
				if ((*ptr & 0x20) != 0)
				{
					if (length < 3)
						break;

					if ((*ptr & 0x10) != 0) // This is a 4 byte sequence, which we can not fit into a 16-bit codepoint. ignore it.
					{
						if (length < 4)
							break;

						ptr += 4;
						length -= 4;
						continue;
					}
					else
					{
						// this is a 3 byte sequence
						value = (*ptr & 0x0F) << 12;
						value += (*++ptr & 0x3F) << 6;
						value += *++ptr & 0x3F;

						length -= 3;
					}
				}
				else
				{
					// This is a 2 byte sequence
					value = (*ptr & 0x1F) << 6;
					value += *++ptr & 0x3F;

					length -= 2;
				}

				// write escape sequence
				result += '\\';
				result += 'u';
				result += Jupiter_asHex_upper(value >> 8);
				result += Jupiter_asHex_upper(value & 0x00FF);
			}
			// else // This is an invalid 1 byte sequence
		}
		else if (*ptr == '\\') // backslash, which is used for escape sequencing
		{
			result += '\\';
			result += '\\';
			--length;
		}
		else // an ordinary character
		{
			result += *ptr;
			--length;
		}

		++ptr;
	}
	return result;
}
