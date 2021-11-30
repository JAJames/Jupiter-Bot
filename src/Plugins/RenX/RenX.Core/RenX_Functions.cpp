/**
 * Copyright (C) 2014-2017 Jessica James.
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

using namespace Jupiter::literals;
using namespace std::literals;

/** Team color translations */
Jupiter::ReferenceString NodColor = "04";
Jupiter::ReferenceString GDIColor = "08";
Jupiter::ReferenceString OtherColor = "14";

/** Short team name translations */
Jupiter::ReferenceString NodShortName = "Nod";
Jupiter::ReferenceString GDIShortName = "GDI";
Jupiter::ReferenceString OtherShortName = "N/A";

/** Long team name translations */
Jupiter::ReferenceString NodLongName = "Brotherhood of Nod";
Jupiter::ReferenceString GDILongName = "Global Defense Initiative";
Jupiter::ReferenceString OtherLongName = "Unknown";

/** RenegadeX RCON protocol message deliminator */
const char RenX::DelimC = '\x02';
const char RenX::DelimC3 = '\xA0';
const std::string_view RenX::DevBotName = "DevBot"sv;

/** WinType translations */

Jupiter::ReferenceString scoreWinTypeTranslation = "Domination (High Score)";
Jupiter::ReferenceString baseWinTypeTranslation = "Conquest (Base Destruction)";
Jupiter::ReferenceString beaconWinTypeTranslation = "Espionage (Beacon)";
Jupiter::ReferenceString tieWinTypeTranslation = "Draw (Tie)";
Jupiter::ReferenceString shutdownWinTypeTranslation = "Ceasefire (Shutdown)";
Jupiter::ReferenceString surrenderWinTypeTranslation = "Forfeit (Surrender)";
Jupiter::ReferenceString unknownWinTypeTranslation = "Aliens (Unknown)";

/** WinType plain translations */

Jupiter::ReferenceString scoreWinTypePlainTranslation = "High Score";
Jupiter::ReferenceString baseWinTypePlainTranslation = "Base Destruction";
Jupiter::ReferenceString beaconWinTypePlainTranslation = "Beacon";
Jupiter::ReferenceString tieWinTypePlainTranslation = "Tie";
Jupiter::ReferenceString shutdownWinTypePlainTranslation = "Shutdown";
Jupiter::ReferenceString surrenderWinTypePlainTranslation = "Surrender";
Jupiter::ReferenceString unknownWinTypePlainTranslation = "Unknown";

/** Vote translations */

Jupiter::ReferenceString translated_VoteMenuChoice_AddBots = STRING_LITERAL_AS_REFERENCE("Add Bots");
Jupiter::ReferenceString translated_VoteMenuChoice_ChangeMap = STRING_LITERAL_AS_REFERENCE("Change Map");
Jupiter::ReferenceString translated_VoteMenuChoice_Donate = STRING_LITERAL_AS_REFERENCE("Donate");
Jupiter::ReferenceString translated_VoteMenuChoice_Kick = STRING_LITERAL_AS_REFERENCE("Kick");
Jupiter::ReferenceString translated_VoteMenuChoice_RemoveBots = STRING_LITERAL_AS_REFERENCE("Remove Bots");
Jupiter::ReferenceString translated_VoteMenuChoice_RestartMatch = STRING_LITERAL_AS_REFERENCE("Restart Match");
Jupiter::ReferenceString translated_VoteMenuChoice_StartMatch = STRING_LITERAL_AS_REFERENCE("Start Match");
Jupiter::ReferenceString translated_VoteMenuChoice_Survey = STRING_LITERAL_AS_REFERENCE("Survey");

/** Object / damage type translations */

/** Nod Vehicles */
Jupiter::ReferenceString translated_Vehicle_Harvester_Nod = STRING_LITERAL_AS_REFERENCE("Nod Harvester");
Jupiter::ReferenceString translated_Vehicle_Buggy = STRING_LITERAL_AS_REFERENCE("Buggy");
Jupiter::ReferenceString translated_Vehicle_Artillery = STRING_LITERAL_AS_REFERENCE("Mobile Artillery");
Jupiter::ReferenceString translated_Vehicle_APC_Nod = STRING_LITERAL_AS_REFERENCE("Nod APC");
Jupiter::ReferenceString translated_Vehicle_LightTank = STRING_LITERAL_AS_REFERENCE("Light Tank");
Jupiter::ReferenceString translated_Vehicle_FlameTank = STRING_LITERAL_AS_REFERENCE("Flame Tank");
Jupiter::ReferenceString translated_Vehicle_StealthTank = STRING_LITERAL_AS_REFERENCE("Stealth Tank");
Jupiter::ReferenceString translated_Vehicle_Chinook_Nod = STRING_LITERAL_AS_REFERENCE("Nod Chinook");
Jupiter::ReferenceString translated_Vehicle_Apache = STRING_LITERAL_AS_REFERENCE("Apache");

Jupiter::ReferenceString translated_Vehicle_ReconBike = STRING_LITERAL_AS_REFERENCE("Recon Bike");
Jupiter::ReferenceString translated_Vehicle_TickTank = STRING_LITERAL_AS_REFERENCE("Tick Tank");

/** GDI Vehicles */
Jupiter::ReferenceString translated_Vehicle_Harvester_GDI = STRING_LITERAL_AS_REFERENCE("GDI Harvester");
Jupiter::ReferenceString translated_Vehicle_Humvee = STRING_LITERAL_AS_REFERENCE("Humvee");
Jupiter::ReferenceString translated_Vehicle_MRLS = STRING_LITERAL_AS_REFERENCE("Mobile Rocket Launcher System");
Jupiter::ReferenceString translated_Vehicle_APC_GDI = STRING_LITERAL_AS_REFERENCE("GDI APC");
Jupiter::ReferenceString translated_Vehicle_MediumTank = STRING_LITERAL_AS_REFERENCE("Medium Tank");
Jupiter::ReferenceString translated_Vehicle_MammothTank = STRING_LITERAL_AS_REFERENCE("Mammoth Tank");
Jupiter::ReferenceString translated_Vehicle_Chinook_GDI = STRING_LITERAL_AS_REFERENCE("GDI Chinook");
Jupiter::ReferenceString translated_Vehicle_Orca = STRING_LITERAL_AS_REFERENCE("Orca");

Jupiter::ReferenceString translated_Vehicle_HoverMRLS = STRING_LITERAL_AS_REFERENCE("Hover MRLS");
Jupiter::ReferenceString translated_Vehicle_Titan = STRING_LITERAL_AS_REFERENCE("Titan");
Jupiter::ReferenceString translated_Vehicle_Wolverine = STRING_LITERAL_AS_REFERENCE("Wolverine");

/** Other Vehicles */
Jupiter::ReferenceString translated_Vehicle_A10_DmgType_GattlingGun = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Gattling Gun");
Jupiter::ReferenceString translated_Vehicle_A10_DmgType_Bomb = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Bomb");
Jupiter::ReferenceString translated_Vehicle_AC130_DmgType_HeavyCannon = STRING_LITERAL_AS_REFERENCE("AC130 Heavy Cannon");
Jupiter::ReferenceString translated_Vehicle_AC130_DmgType_AutoCannon = STRING_LITERAL_AS_REFERENCE("AC130 Auto Cannon");

/** Weapons */
Jupiter::ReferenceString translated_Weapon_HeavyPistol = STRING_LITERAL_AS_REFERENCE("Heavy Pistol");
Jupiter::ReferenceString translated_Weapon_Carbine = STRING_LITERAL_AS_REFERENCE("Carbine");
Jupiter::ReferenceString translated_Weapon_Airstrike_GDI = STRING_LITERAL_AS_REFERENCE("GDI Airstrike");
Jupiter::ReferenceString translated_Weapon_Airstrike_Nod = STRING_LITERAL_AS_REFERENCE("Nod Airstrike");
Jupiter::ReferenceString translated_Weapon_TiberiumFlechetteRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Flechette Gun");
Jupiter::ReferenceString translated_Weapon_TiberiumAutoRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Automatic Rifle");
Jupiter::ReferenceString translated_Weapon_TiberiumAutoRifle_Blue = STRING_LITERAL_AS_REFERENCE("Blue Tiberium Automatic Rifle Burst");
Jupiter::ReferenceString translated_Weapon_EMPGrenade = STRING_LITERAL_AS_REFERENCE("EMP Grenade");
Jupiter::ReferenceString translated_Weapon_SmokeGrenade = STRING_LITERAL_AS_REFERENCE("Smoke Grenade");
Jupiter::ReferenceString translated_Weapon_TimedC4 = STRING_LITERAL_AS_REFERENCE("Timed C4");
Jupiter::ReferenceString translated_Weapon_RemoteC4 = STRING_LITERAL_AS_REFERENCE("Remote C4");
Jupiter::ReferenceString translated_Weapon_ProxyC4 = STRING_LITERAL_AS_REFERENCE("Proximity C4");
Jupiter::ReferenceString translated_Weapon_ATMine = STRING_LITERAL_AS_REFERENCE("Anti-Tank Mine");
Jupiter::ReferenceString translated_Weapon_IonCannonBeacon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Beacon");
Jupiter::ReferenceString translated_Weapon_NukeBeacon = STRING_LITERAL_AS_REFERENCE("Nuclear Missile Beacon");
Jupiter::ReferenceString translated_Weapon_DeployedC4 = STRING_LITERAL_AS_REFERENCE("Timed C4");
Jupiter::ReferenceString translated_Weapon_DeployedTimedC4 = STRING_LITERAL_AS_REFERENCE("Timed C4");
Jupiter::ReferenceString translated_Weapon_DeployedRemoteC4 = STRING_LITERAL_AS_REFERENCE("Remote C4");
Jupiter::ReferenceString translated_Weapon_DeployedProxyC4 = STRING_LITERAL_AS_REFERENCE("Proximity C4");
Jupiter::ReferenceString translated_Weapon_DeployedATMine = STRING_LITERAL_AS_REFERENCE("Anti-Tank Mine");
Jupiter::ReferenceString translated_Weapon_DeployedIonCannonBeacon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Beacon");
Jupiter::ReferenceString translated_Weapon_DeployedNukeBeacon = STRING_LITERAL_AS_REFERENCE("Nuclear Missile Beacon");
Jupiter::ReferenceString translated_Weapon_CrateNuke = STRING_LITERAL_AS_REFERENCE("Nuclear Crate Explosion");
Jupiter::ReferenceString translated_Weapon_DevNuke = STRING_LITERAL_AS_REFERENCE("Nuclear Dev Explosion");

/** Projectiles */
Jupiter::ReferenceString translated_Projectile_EMPGrenade = STRING_LITERAL_AS_REFERENCE("EMP Grenade");
Jupiter::ReferenceString translated_Projectile_SmokeGrenade = STRING_LITERAL_AS_REFERENCE("Smoke Grenade");

/** GDI Characters */
Jupiter::ReferenceString translated_GDI_Deadeye = STRING_LITERAL_AS_REFERENCE("Deadeye");
Jupiter::ReferenceString translated_GDI_Engineer = STRING_LITERAL_AS_REFERENCE("GDI Engineer");
Jupiter::ReferenceString translated_GDI_Grenadier = STRING_LITERAL_AS_REFERENCE("Grenadier");
Jupiter::ReferenceString translated_GDI_Gunner = STRING_LITERAL_AS_REFERENCE("Gunner");
Jupiter::ReferenceString translated_GDI_Havoc = STRING_LITERAL_AS_REFERENCE("Havoc");
Jupiter::ReferenceString translated_GDI_Hotwire = STRING_LITERAL_AS_REFERENCE("Hotwire");
Jupiter::ReferenceString translated_GDI_Marksman = STRING_LITERAL_AS_REFERENCE("GDI Marksman");
Jupiter::ReferenceString translated_GDI_McFarland = STRING_LITERAL_AS_REFERENCE("McFarland");
Jupiter::ReferenceString translated_GDI_Mobius = STRING_LITERAL_AS_REFERENCE("Mobius");
Jupiter::ReferenceString translated_GDI_Officer = STRING_LITERAL_AS_REFERENCE("GDI Officer");
Jupiter::ReferenceString translated_GDI_Patch = STRING_LITERAL_AS_REFERENCE("Patch");
Jupiter::ReferenceString translated_GDI_RocketSoldier = STRING_LITERAL_AS_REFERENCE("GDI Rocket Soldier");
Jupiter::ReferenceString translated_GDI_Shotgunner = STRING_LITERAL_AS_REFERENCE("GDI Shotgunner");
Jupiter::ReferenceString translated_GDI_Soldier = STRING_LITERAL_AS_REFERENCE("GDI Soldier");
Jupiter::ReferenceString translated_GDI_Sydney = STRING_LITERAL_AS_REFERENCE("Sydney");

/** Nod Characters */
Jupiter::ReferenceString translated_Nod_BlackHandSniper = STRING_LITERAL_AS_REFERENCE("Black Hand Sniper");
Jupiter::ReferenceString translated_Nod_ChemicalTrooper = STRING_LITERAL_AS_REFERENCE("Chemical Trooper");
Jupiter::ReferenceString translated_Nod_Engineer = STRING_LITERAL_AS_REFERENCE("Nod Engineer");
Jupiter::ReferenceString translated_Nod_FlameTrooper = STRING_LITERAL_AS_REFERENCE("Flame Trooper");
Jupiter::ReferenceString translated_Nod_LaserChainGunner = STRING_LITERAL_AS_REFERENCE("Laser Chain Gunner");
Jupiter::ReferenceString translated_Nod_Marksman = STRING_LITERAL_AS_REFERENCE("Nod Marksman");
Jupiter::ReferenceString translated_Nod_Mendoza = STRING_LITERAL_AS_REFERENCE("Mendoza");
Jupiter::ReferenceString translated_Nod_Officer = STRING_LITERAL_AS_REFERENCE("Nod Officer");
Jupiter::ReferenceString translated_Nod_Raveshaw = STRING_LITERAL_AS_REFERENCE("Raveshaw");
Jupiter::ReferenceString translated_Nod_RocketSoldier = STRING_LITERAL_AS_REFERENCE("Nod Rocket Soldier");
Jupiter::ReferenceString translated_Nod_Sakura = STRING_LITERAL_AS_REFERENCE("Sakura");
Jupiter::ReferenceString translated_Nod_Shotgunner = STRING_LITERAL_AS_REFERENCE("Nod Shotgunner");
Jupiter::ReferenceString translated_Nod_Soldier = STRING_LITERAL_AS_REFERENCE("Nod Soldier");
Jupiter::ReferenceString translated_Nod_StealthBlackHand = STRING_LITERAL_AS_REFERENCE("Stealth Black Hand");
Jupiter::ReferenceString translated_Nod_Technician = STRING_LITERAL_AS_REFERENCE("Technician");

/** Non-weapon damage types */
Jupiter::ReferenceString translated_DmgType_Suicided = STRING_LITERAL_AS_REFERENCE("Suicide");
Jupiter::ReferenceString translated_DmgType_Fell = STRING_LITERAL_AS_REFERENCE("Fall");
Jupiter::ReferenceString translated_DmgType_Tiberium = STRING_LITERAL_AS_REFERENCE("Tiberium");
Jupiter::ReferenceString translated_DmgType_TiberiumBleed = STRING_LITERAL_AS_REFERENCE("Tiberium Decay");
Jupiter::ReferenceString translated_DmgType_TiberiumBleed_Blue = STRING_LITERAL_AS_REFERENCE("Blue Tiberium Decay");
Jupiter::ReferenceString translated_DmgType_RanOver = STRING_LITERAL_AS_REFERENCE("Crushed");

/** Infantry weapons */
Jupiter::ReferenceString translated_DmgType_Pistol = STRING_LITERAL_AS_REFERENCE("Pistol");
Jupiter::ReferenceString translated_DmgType_SMG = STRING_LITERAL_AS_REFERENCE("Machine Pistol");
Jupiter::ReferenceString translated_DmgType_HeavyPistol = STRING_LITERAL_AS_REFERENCE("Heavy Pistol");
Jupiter::ReferenceString translated_DmgType_Carbine = STRING_LITERAL_AS_REFERENCE("Carbine");
Jupiter::ReferenceString translated_DmgType_TiberiumFlechetteRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Flechette Gun"); // Not a rifle.
Jupiter::ReferenceString translated_DmgType_TiberiumAutoRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Automatic Rifle");
Jupiter::ReferenceString translated_DmgType_TiberiumAutoRifle_Blue = STRING_LITERAL_AS_REFERENCE("Blue Tiberium Automatic Rifle Burst");
Jupiter::ReferenceString translated_DmgType_TiberiumAutoRifle_Flechette_Blue = STRING_LITERAL_AS_REFERENCE("Blue Tiberium Automatic Rifle");

Jupiter::ReferenceString translated_DmgType_Grenade = STRING_LITERAL_AS_REFERENCE("Grenade");
Jupiter::ReferenceString translated_DmgType_TimedC4 = STRING_LITERAL_AS_REFERENCE("Timed C4");
Jupiter::ReferenceString translated_DmgType_RemoteC4 = STRING_LITERAL_AS_REFERENCE("Remote C4");
Jupiter::ReferenceString translated_DmgType_ProxyC4 = STRING_LITERAL_AS_REFERENCE("Proximity C4");
Jupiter::ReferenceString translated_DmgType_ATMine = STRING_LITERAL_AS_REFERENCE("Anti-Tank Mine");
Jupiter::ReferenceString translated_DmgType_EMPGrenade = STRING_LITERAL_AS_REFERENCE("EMP Grenade");
Jupiter::ReferenceString translated_DmgType_BurnC4 = STRING_LITERAL_AS_REFERENCE("C4 Burn");
Jupiter::ReferenceString translated_DmgType_FireBleed = STRING_LITERAL_AS_REFERENCE("Fire Burn"); // Caused by C4 or flame weapons

Jupiter::ReferenceString translated_DmgType_AutoRifle = STRING_LITERAL_AS_REFERENCE("Automatic Rifle");
Jupiter::ReferenceString translated_DmgType_Shotgun = STRING_LITERAL_AS_REFERENCE("Shotgun");
Jupiter::ReferenceString translated_DmgType_FlameThrower = STRING_LITERAL_AS_REFERENCE("Flamethrower");
Jupiter::ReferenceString translated_DmgType_GrenadeLauncher = STRING_LITERAL_AS_REFERENCE("Grenade Launcher");
Jupiter::ReferenceString translated_DmgType_MarksmanRifle = STRING_LITERAL_AS_REFERENCE("Marksman's Rifle");

Jupiter::ReferenceString translated_DmgType_ChainGun = STRING_LITERAL_AS_REFERENCE("Chain Gun");
Jupiter::ReferenceString translated_DmgType_MissileLauncher = STRING_LITERAL_AS_REFERENCE("Missile Launcher");
Jupiter::ReferenceString translated_DmgType_MissileLauncher_Alt = STRING_LITERAL_AS_REFERENCE("Missile Launcher");
Jupiter::ReferenceString translated_DmgType_ChemicalThrower = STRING_LITERAL_AS_REFERENCE("Chemical Spray Gun");
Jupiter::ReferenceString translated_DmgType_LaserRifle = STRING_LITERAL_AS_REFERENCE("Laser Rifle");
Jupiter::ReferenceString translated_DmgType_TacticalRifle = STRING_LITERAL_AS_REFERENCE("Tactical Rifle");
Jupiter::ReferenceString translated_DmgType_RocketLauncher = STRING_LITERAL_AS_REFERENCE("Rocket Launcher");
Jupiter::ReferenceString translated_DmgType_LaserChainGun = STRING_LITERAL_AS_REFERENCE("Laser Chain Gun");
Jupiter::ReferenceString translated_DmgType_FlakCannon = STRING_LITERAL_AS_REFERENCE("Flak");
Jupiter::ReferenceString translated_DmgType_FlakCannon_Alt = STRING_LITERAL_AS_REFERENCE("Concentrated Flak");
Jupiter::ReferenceString translated_DmgType_SniperRifle = STRING_LITERAL_AS_REFERENCE("Sniper Rifle");
Jupiter::ReferenceString translated_DmgType_RamjetRifle = STRING_LITERAL_AS_REFERENCE("Ramjet");
Jupiter::ReferenceString translated_DmgType_Railgun = STRING_LITERAL_AS_REFERENCE("Railgun");
Jupiter::ReferenceString translated_DmgType_PersonalIonCannon = STRING_LITERAL_AS_REFERENCE("Personal Ion Cannon");
Jupiter::ReferenceString translated_DmgType_VoltRifle = STRING_LITERAL_AS_REFERENCE("Volt Rifle");
Jupiter::ReferenceString translated_DmgType_VoltRifle_Alt = STRING_LITERAL_AS_REFERENCE("Volt Rifle Burst");
Jupiter::ReferenceString translated_DmgType_VoltAutoRifle = STRING_LITERAL_AS_REFERENCE("Volt Automatic Rifle");
Jupiter::ReferenceString translated_DmgType_VoltAutoRifle_Alt = STRING_LITERAL_AS_REFERENCE("Volt Automatic Rifle Burst");

/** Vehicle weapons */
Jupiter::ReferenceString translated_DmgType_MammothTank_Missile = STRING_LITERAL_AS_REFERENCE("Mammoth Tank Missile");
Jupiter::ReferenceString translated_DmgType_MammothTank_Cannon = STRING_LITERAL_AS_REFERENCE("Mammoth Tank Cannon");
Jupiter::ReferenceString translated_DmgType_Orca_Missile = STRING_LITERAL_AS_REFERENCE("Orca Missile");
Jupiter::ReferenceString translated_DmgType_Orca_Gun = STRING_LITERAL_AS_REFERENCE("Orca Gun");
Jupiter::ReferenceString translated_DmgType_Orca_Passenger = STRING_LITERAL_AS_REFERENCE("Orca Passenger Missile");
Jupiter::ReferenceString translated_DmgType_Apache_Rocket = STRING_LITERAL_AS_REFERENCE("Apache Rocket");
Jupiter::ReferenceString translated_DmgType_Apache_Gun = STRING_LITERAL_AS_REFERENCE("Apache Gun");
Jupiter::ReferenceString translated_DmgType_Apache_Passenger = STRING_LITERAL_AS_REFERENCE("Apache Passenger Missile");

/** Other weapons */
Jupiter::ReferenceString translated_DmgType_AGT_MG = STRING_LITERAL_AS_REFERENCE("Machine Gun");
Jupiter::ReferenceString translated_DmgType_AGT_Rocket = STRING_LITERAL_AS_REFERENCE("Rocket");
Jupiter::ReferenceString translated_DmgType_Obelisk = STRING_LITERAL_AS_REFERENCE("Obelisk Laser");
Jupiter::ReferenceString translated_DmgType_GuardTower = STRING_LITERAL_AS_REFERENCE("Guard Tower");
Jupiter::ReferenceString translated_DmgType_Turret = STRING_LITERAL_AS_REFERENCE("Turret");
Jupiter::ReferenceString translated_DmgType_SAMSite = STRING_LITERAL_AS_REFERENCE("SAM Site");
Jupiter::ReferenceString translated_DmgType_AATower = STRING_LITERAL_AS_REFERENCE("Anti-Air Guard Tower");
Jupiter::ReferenceString translated_DmgType_GunEmpl = STRING_LITERAL_AS_REFERENCE("Gun Emplacement Gattling Gun");
Jupiter::ReferenceString translated_DmgType_GunEmpl_Alt = STRING_LITERAL_AS_REFERENCE("Gun Emplacement Automatic Cannon");
Jupiter::ReferenceString translated_DmgType_RocketEmpl_Swarm = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement Swarm Missile");
Jupiter::ReferenceString translated_DmgType_RocketEmpl_Missile = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement Hellfire Missile");
Jupiter::ReferenceString translated_DmgType_Nuke = STRING_LITERAL_AS_REFERENCE("Nuclear Missile Strike");
Jupiter::ReferenceString translated_DmgType_IonCannon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Strike");

/** Nod Vehicles */
Jupiter::ReferenceString translated_DmgType_Harvester_Nod = STRING_LITERAL_AS_REFERENCE("Nod Harvester");
Jupiter::ReferenceString translated_DmgType_Buggy = STRING_LITERAL_AS_REFERENCE("Buggy");
Jupiter::ReferenceString translated_DmgType_Artillery = STRING_LITERAL_AS_REFERENCE("Mobile Artillery");
Jupiter::ReferenceString translated_DmgType_APC_Nod = STRING_LITERAL_AS_REFERENCE("Nod APC");
Jupiter::ReferenceString translated_DmgType_LightTank = STRING_LITERAL_AS_REFERENCE("Light Tank");
Jupiter::ReferenceString translated_DmgType_FlameTank = STRING_LITERAL_AS_REFERENCE("Flame Tank");
Jupiter::ReferenceString translated_DmgType_StealthTank = STRING_LITERAL_AS_REFERENCE("Stealth Tank");
Jupiter::ReferenceString translated_DmgType_Chinook_Nod = STRING_LITERAL_AS_REFERENCE("Nod Chinook");
Jupiter::ReferenceString translated_DmgType_Apache = STRING_LITERAL_AS_REFERENCE("Apache");

Jupiter::ReferenceString translated_Vehicle_ReconBike_DmgType = STRING_LITERAL_AS_REFERENCE("Recon Bike");
Jupiter::ReferenceString translated_Vehicle_TickTank_DmgType = STRING_LITERAL_AS_REFERENCE("Tick Tank");

/** GDI Vehicles */
Jupiter::ReferenceString translated_DmgType_Harvester_GDI = STRING_LITERAL_AS_REFERENCE("GDI Harvester");
Jupiter::ReferenceString translated_DmgType_Humvee = STRING_LITERAL_AS_REFERENCE("Humvee");
Jupiter::ReferenceString translated_DmgType_MRLS = STRING_LITERAL_AS_REFERENCE("Mobile Rocket Launcher System");
Jupiter::ReferenceString translated_DmgType_APC_GDI = STRING_LITERAL_AS_REFERENCE("GDI APC");
Jupiter::ReferenceString translated_DmgType_MediumTank = STRING_LITERAL_AS_REFERENCE("Medium Tank");
Jupiter::ReferenceString translated_DmgType_MammothTank = STRING_LITERAL_AS_REFERENCE("Mammoth Tank");
Jupiter::ReferenceString translated_DmgType_Chinook_GDI = STRING_LITERAL_AS_REFERENCE("GDI Chinook");
Jupiter::ReferenceString translated_DmgType_Orca = STRING_LITERAL_AS_REFERENCE("Orca");

Jupiter::ReferenceString translated_Vehicle_HoverMRLS_DmgType = STRING_LITERAL_AS_REFERENCE("Hover MRLS");
Jupiter::ReferenceString translated_Vehicle_Titan_DmgType = STRING_LITERAL_AS_REFERENCE("Titan");
Jupiter::ReferenceString translated_Vehicle_Wolverine_DmgType = STRING_LITERAL_AS_REFERENCE("Wolverine");

/** Other Vehicles */
Jupiter::ReferenceString translated_DmgType_A10_Missile = STRING_LITERAL_AS_REFERENCE("A10 Missile");

/** Nod structures */
Jupiter::ReferenceString translated_Building_HandOfNod = STRING_LITERAL_AS_REFERENCE("Hand of Nod");
Jupiter::ReferenceString translated_Building_AirTower = STRING_LITERAL_AS_REFERENCE("Airstrip");
Jupiter::ReferenceString translated_Building_Airstrip = STRING_LITERAL_AS_REFERENCE("Airstrip");
Jupiter::ReferenceString translated_Building_Refinery_Nod = STRING_LITERAL_AS_REFERENCE("Nod Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_Nod = STRING_LITERAL_AS_REFERENCE("Nod Power Plant");
Jupiter::ReferenceString translated_Building_HandOfNod_Internals = STRING_LITERAL_AS_REFERENCE("Hand of Nod");
Jupiter::ReferenceString translated_Building_AirTower_Internals = STRING_LITERAL_AS_REFERENCE("Airstrip");
Jupiter::ReferenceString translated_Building_Airstrip_Internals = STRING_LITERAL_AS_REFERENCE("Airstrip");
Jupiter::ReferenceString translated_Building_Refinery_Nod_Internals = STRING_LITERAL_AS_REFERENCE("Nod Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_Nod_Internals = STRING_LITERAL_AS_REFERENCE("Nod Power Plant");

/** GDI structures */
Jupiter::ReferenceString translated_Building_Barracks = STRING_LITERAL_AS_REFERENCE("Barracks");
Jupiter::ReferenceString translated_Building_WeaponsFactory = STRING_LITERAL_AS_REFERENCE("Weapons Factory");
Jupiter::ReferenceString translated_Building_Refinery_GDI = STRING_LITERAL_AS_REFERENCE("GDI Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_GDI = STRING_LITERAL_AS_REFERENCE("GDI Power Plant");
Jupiter::ReferenceString translated_Building_Barracks_Internals = STRING_LITERAL_AS_REFERENCE("Barracks");
Jupiter::ReferenceString translated_Building_WeaponsFactory_Internals = STRING_LITERAL_AS_REFERENCE("Weapons Factory");
Jupiter::ReferenceString translated_Building_Refinery_GDI_Internals = STRING_LITERAL_AS_REFERENCE("GDI Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_GDI_Internals = STRING_LITERAL_AS_REFERENCE("GDI Power Plant");

/** Defense structures */
Jupiter::ReferenceString translated_Building_AdvancedGuardTower = STRING_LITERAL_AS_REFERENCE("Advanced Guard Tower");
Jupiter::ReferenceString translated_Building_Obelisk = STRING_LITERAL_AS_REFERENCE("Obelisk of Light");
Jupiter::ReferenceString translated_Building_AdvancedGuardTower_Internals = STRING_LITERAL_AS_REFERENCE("Advanced Guard Tower");
Jupiter::ReferenceString translated_Building_Obelisk_Internals = STRING_LITERAL_AS_REFERENCE("Obelisk of Light");

/** Other structures */
Jupiter::ReferenceString translated_Building_Silo = STRING_LITERAL_AS_REFERENCE("Tiberium Silo");
Jupiter::ReferenceString translated_Building_CommCentre = STRING_LITERAL_AS_REFERENCE("Communications Center");
Jupiter::ReferenceString translated_Building_Silo_Internals = STRING_LITERAL_AS_REFERENCE("Tiberium Silo");
Jupiter::ReferenceString translated_Building_CommCentre_Internals = STRING_LITERAL_AS_REFERENCE("Communications Center");

/** Fort structures */
Jupiter::ReferenceString translated_CapturableMCT_Fort = STRING_LITERAL_AS_REFERENCE("Fort");
Jupiter::ReferenceString translated_CapturableMCT_MC = STRING_LITERAL_AS_REFERENCE("Medical Center");
Jupiter::ReferenceString translated_CapturableMCT_Fort_Internals = STRING_LITERAL_AS_REFERENCE("Fort");
Jupiter::ReferenceString translated_CapturableMCT_MC_Internals = STRING_LITERAL_AS_REFERENCE("Medical Center");

/** Defences */
Jupiter::ReferenceString translated_Defence_GuardTower = STRING_LITERAL_AS_REFERENCE("Guard Tower");
Jupiter::ReferenceString translated_Defence_Turret = STRING_LITERAL_AS_REFERENCE("Turret");
Jupiter::ReferenceString translated_Defence_SAMSite = STRING_LITERAL_AS_REFERENCE("SAM Site");
Jupiter::ReferenceString translated_Defence_AATower = STRING_LITERAL_AS_REFERENCE("Anti-Air Guard Tower");
Jupiter::ReferenceString translated_Defence_GunEmplacement = STRING_LITERAL_AS_REFERENCE("Gun Emplacement");
Jupiter::ReferenceString translated_Defence_RocketEmplacement = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement");

/** Defences - Sentinels */
Jupiter::ReferenceString translated_Sentinel_AGT_MG_Base = STRING_LITERAL_AS_REFERENCE("Advanced Guard Tower");
Jupiter::ReferenceString translated_Sentinel_AGT_Rockets_Base = STRING_LITERAL_AS_REFERENCE("Advanced Guard Tower");
Jupiter::ReferenceString translated_Sentinel_Obelisk_Laser_Base = STRING_LITERAL_AS_REFERENCE("Obelisk of Light");

/** UT damage types */
Jupiter::ReferenceString translated_UTDmgType_VehicleExplosion = STRING_LITERAL_AS_REFERENCE("Vehicle Explosion");
Jupiter::ReferenceString translated_UTDmgType_Drowned = STRING_LITERAL_AS_REFERENCE("Drowned");

/** Beacons */
Jupiter::ReferenceString translated_IonCannonBeacon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Beacon");
Jupiter::ReferenceString translated_NukeBeacon = STRING_LITERAL_AS_REFERENCE("Nuclear Strike Beacon");
Jupiter::ReferenceString translated_KillZDamageType = STRING_LITERAL_AS_REFERENCE("Kill Zone");

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

const Jupiter::ReadableString &RenX::getTeamColor(TeamType team)
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

const Jupiter::ReadableString &RenX::getTeamName(TeamType team)
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

const Jupiter::ReadableString &RenX::getFullTeamName(TeamType team)
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

const Jupiter::ReadableString &RenX::getCharacter(const Jupiter::ReadableString &chr)
{
	static Jupiter::ReferenceString object;

	object = chr;
	if (object.find(STRING_LITERAL_AS_REFERENCE("Rx_")) == 0)
		object.remove_prefix(3);
	if (object.find(STRING_LITERAL_AS_REFERENCE("InventoryManager_")) == 0)
		object.remove_prefix(17);
	else if (object.find(STRING_LITERAL_AS_REFERENCE("FamilyInfo_")) == 0)
		object.remove_prefix(11);

	return object;
}

const Jupiter::ReferenceString &translateCharacter(Jupiter::ReferenceString &object)
{
	if (object.find(STRING_LITERAL_AS_REFERENCE("GDI_")) == 0)
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
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Nod_")) == 0)
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

std::string_view RenX::translateName(std::string_view obj)
{
	if (obj.empty())
		return ""_jrs;

	Jupiter::ReferenceString iniTranslation = RenX::getCore()->getConfig()["Name"_jrs].get(Jupiter::ReferenceString{obj});
	if (iniTranslation.isNotEmpty())
		return iniTranslation;

	Jupiter::ReferenceString object = obj;

	if (object.find(STRING_LITERAL_AS_REFERENCE("nBab_")) == 0)
		object.remove_prefix(5);
	
	if (object.find(STRING_LITERAL_AS_REFERENCE("Rx_")) == 0)
		object.remove_prefix(3);
	else if (object.find(STRING_LITERAL_AS_REFERENCE("TS_")) == 0)
		object.remove_prefix(3);

	if (object.find(STRING_LITERAL_AS_REFERENCE("Vehicle_")) == 0)
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
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Weapon_")) == 0)
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
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Projectile_")) == 0)
	{
		object.remove_prefix(11);
		if (object == "EMPGrenade"sv) return translated_Projectile_EMPGrenade;
		if (object == "SmokeGrenade"sv) return translated_Projectile_SmokeGrenade;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("InventoryManager_")) == 0)
	{
		object.remove_prefix(17);
		return translateCharacter(object);
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("FamilyInfo_")) == 0)
	{
		object.remove_prefix(11);
		return translateCharacter(object);
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("DmgType_")) == 0)
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
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Building_")) == 0)
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
	else if (object.find(STRING_LITERAL_AS_REFERENCE("CapturableMCT_")) == 0)
	{
		object.remove_prefix(14);
		if (object == "Fort"sv) return translated_CapturableMCT_Fort;
		if (object == "MC"sv) return translated_CapturableMCT_MC;
		if (object == "Fort_Internals"sv) return translated_CapturableMCT_Fort_Internals;
		if (object == "MC_Internals"sv) return translated_CapturableMCT_MC_Internals;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Defence_")) == 0)
	{
		object.remove_prefix(8);
		if (object == "GuardTower"sv) return translated_Defence_GuardTower;
		if (object == "Turret"sv) return translated_Defence_Turret;
		if (object == "SAMSite"sv) return translated_Defence_SAMSite;
		if (object == "AATower"sv) return translated_Defence_AATower;
		if (object == "GunEmplacement"sv) return translated_Defence_GunEmplacement;
		if (object == "RocketEmplacement"sv) return translated_Defence_RocketEmplacement;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Sentinel_")) == 0)
	{
		object.remove_prefix(9);
		if (object == "AGT_MG_Base"sv) return translated_Sentinel_AGT_MG_Base;
		if (object == "AGT_Rockets_Base"sv) return translated_Sentinel_AGT_Rockets_Base;
		if (object == "Obelisk_Laser_Base"sv) return translated_Sentinel_Obelisk_Laser_Base;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("UTDmgType_")) == 0)
	{
		object.remove_prefix(10);
		if (object == "VehicleExplosion"sv) return translated_UTDmgType_VehicleExplosion;
		if (object == "Drowned"sv) return translated_UTDmgType_Drowned;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("VoteMenuChoice_")) == 0)
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

const Jupiter::ReadableString &RenX::translateWinType(RenX::WinType winType)
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

const Jupiter::ReadableString &RenX::translateWinTypePlain(RenX::WinType winType)
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
	NodColor = translationsFile["TeamColor"_jrs].get("Nod"_jrs, "04"_jrs);
	GDIColor = translationsFile["TeamColor"_jrs].get("GDI"_jrs, "08"_jrs);
	OtherColor = translationsFile["TeamColor"_jrs].get("Other"_jrs, "14"_jrs);

	NodShortName = translationsFile["ShortTeamName"_jrs].get("Nod"_jrs, "Nod"_jrs);
	GDIShortName = translationsFile["ShortTeamName"_jrs].get("GDI"_jrs, "GDI"_jrs);
	OtherShortName = translationsFile["ShortTeamName"_jrs].get("Other"_jrs, "N/A"_jrs);
	NodLongName = translationsFile["LongTeamName"_jrs].get("Nod"_jrs, "Brotherhood of Nod"_jrs);
	GDILongName = translationsFile["LongTeamName"_jrs].get("GDI"_jrs, "Global Defense Initiative"_jrs);
	OtherLongName = translationsFile["LongTeamName"_jrs].get("Other"_jrs, "Unknown"_jrs);

	scoreWinTypeTranslation = translationsFile["WinType"_jrs].get("Score"_jrs, "Domination (High Score)"_jrs);
	baseWinTypeTranslation = translationsFile["WinType"_jrs].get("Base"_jrs, "Conquest (Base Destruction)"_jrs);
	beaconWinTypeTranslation = translationsFile["WinType"_jrs].get("Beacon"_jrs, "Espionage (Beacon)"_jrs);
	tieWinTypeTranslation = translationsFile["WinType"_jrs].get("Tie"_jrs, "Draw (Tie)"_jrs);
	shutdownWinTypeTranslation = translationsFile["WinType"_jrs].get("Shutdown"_jrs, "Ceasefire (Shutdown)"_jrs);
	surrenderWinTypeTranslation = translationsFile["WinType"_jrs].get("Surrender"_jrs, "Forfeit (Surrender)"_jrs);
	unknownWinTypeTranslation = translationsFile["WinType"_jrs].get("Unknown"_jrs, "Aliens (Unknown)"_jrs);

	scoreWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Score"_jrs, "High Score"_jrs);
	baseWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Base"_jrs, "Base Destruction"_jrs);
	beaconWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Beacon"_jrs, "Beacon"_jrs);
	tieWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Tie"_jrs, "Tie"_jrs);
	shutdownWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Shutdown"_jrs, "Shutdown"_jrs);
	surrenderWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Surrender"_jrs, "Surrender"_jrs);
	unknownWinTypePlainTranslation = translationsFile["WinTypePlain"_jrs].get("Unknown"_jrs, "Unknown"_jrs);
}

Jupiter::String RenX::getFormattedPlayerName(const RenX::PlayerInfo &player)
{
	Jupiter::String r = player.formatNamePrefix;
	r += IRCCOLOR;
	r += RenX::getTeamColor(player.team);
	r += player.name;
	return r;
}

Jupiter::StringS RenX::formatGUID(const RenX::Map &map)
{
	return Jupiter::StringS::Format("%.16llX%.16llX", map.guid[0], map.guid[1]);
}

std::chrono::milliseconds RenX::getServerTime(const RenX::PlayerInfo &player)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.joinTime);
}

Jupiter::StringS RenX::default_uuid_func(RenX::PlayerInfo &player)
{
	if (player.steamid != 0U)
		return Jupiter::StringS::Format("0x%.16llX", player.steamid);

	return Jupiter::StringS();
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
