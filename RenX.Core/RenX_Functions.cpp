/**
 * Copyright (C) 2014 Justin James.
 *
 * This license must be preserved.
 * Any applications, libraries, or code which make any use of any
 * component of this program must not be commercial, unless explicit
 * permission is granted from the original author. The use of this
 * program for non-profit purposes is permitted.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In the event that this license restricts you from making desired use of this program, contact the original author.
 * Written by Justin James <justin.aj@hotmail.com>
 */

#include <ctime>
#include "Jupiter/Functions.h"
#include "Jupiter/INIFile.h"
#include "IRC_Bot.h"
#include "ServerManager.h"
#include "RenX_Functions.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

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
const char *RenX::DelimS = "\xA0";
const char RenX::DelimC = *RenX::DelimS;

/** WinType translations */

Jupiter::ReferenceString scoreWinTypeTranslation = "Domination (High Score)";
Jupiter::ReferenceString baseWinTypeTranslation = "Conquest (Base Destruction)";
Jupiter::ReferenceString beaconWinTypeTranslation = "Espionage (Beacon)";
Jupiter::ReferenceString tieWinTypeTranslation = "Draw (Tie)";
Jupiter::ReferenceString shutdownWinTypeTranslation = "Ceasefire (Shutdown)";
Jupiter::ReferenceString unknownWinTypeTranslation = "Aliens (Unknown)";

/** WinType plain translations */

Jupiter::ReferenceString scoreWinTypePlainTranslation = "High Score";
Jupiter::ReferenceString baseWinTypePlainTranslation = "Base Destruction";
Jupiter::ReferenceString beaconWinTypePlainTranslation = "Beacon";
Jupiter::ReferenceString tieWinTypePlainTranslation = "Tie";
Jupiter::ReferenceString shutdownWinTypePlainTranslation = "Shutdown";
Jupiter::ReferenceString unknownWinTypePlainTranslation = "Unknown";

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

/** GDI Vehicles */
Jupiter::ReferenceString translated_Vehicle_Harvester_GDI = STRING_LITERAL_AS_REFERENCE("GDI Harvester");
Jupiter::ReferenceString translated_Vehicle_Humvee = STRING_LITERAL_AS_REFERENCE("Humvee");
Jupiter::ReferenceString translated_Vehicle_MRLS = STRING_LITERAL_AS_REFERENCE("Mobile Rocket Launcher System");
Jupiter::ReferenceString translated_Vehicle_APC_GDI = STRING_LITERAL_AS_REFERENCE("GDI APC");
Jupiter::ReferenceString translated_Vehicle_MediumTank = STRING_LITERAL_AS_REFERENCE("Medium Tank");
Jupiter::ReferenceString translated_Vehicle_MammothTank = STRING_LITERAL_AS_REFERENCE("Mammoth Tank");
Jupiter::ReferenceString translated_Vehicle_Chinook_GDI = STRING_LITERAL_AS_REFERENCE("GDI Chinook");
Jupiter::ReferenceString translated_Vehicle_Orca = STRING_LITERAL_AS_REFERENCE("Orca");

/** Other Vehicles */
Jupiter::ReferenceString translated_Vehicle_A10_DmgType_GattlingGun = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Gattling Gun");
Jupiter::ReferenceString translated_Vehicle_A10_DmgType_Bomb = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Bomb");
Jupiter::ReferenceString translated_Vehicle_AC130_DmgType_HeavyCannon = STRING_LITERAL_AS_REFERENCE("AC130 Heavy Cannon");
Jupiter::ReferenceString translated_Vehicle_AC130_DmgType_AutoCannon = STRING_LITERAL_AS_REFERENCE("AC130 Auto Cannon");

/** Non-weapon damage types */
Jupiter::ReferenceString translated_DmgType_Suicided = STRING_LITERAL_AS_REFERENCE("Suicide");
Jupiter::ReferenceString translated_DmgType_Fell = STRING_LITERAL_AS_REFERENCE("Fall");
Jupiter::ReferenceString translated_DmgType_Tiberium = STRING_LITERAL_AS_REFERENCE("Tiberium");
Jupiter::ReferenceString translated_DmgType_TiberiumBleed = STRING_LITERAL_AS_REFERENCE("Tiberium Decay");
Jupiter::ReferenceString translated_DmgType_RanOver = STRING_LITERAL_AS_REFERENCE("Crushed");

/** Infantry weapons */
Jupiter::ReferenceString translated_DmgType_Pistol = STRING_LITERAL_AS_REFERENCE("Pistol");
Jupiter::ReferenceString translated_DmgType_SMG = STRING_LITERAL_AS_REFERENCE("Machine Pistol");
Jupiter::ReferenceString translated_DmgType_HeavyPistol = STRING_LITERAL_AS_REFERENCE("Heavy Pistol");
Jupiter::ReferenceString translated_DmgType_Carbine = STRING_LITERAL_AS_REFERENCE("Carbine");
Jupiter::ReferenceString translated_DmgType_TiberiumFlechetteRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Flechette Gun"); // Not a rifle.
Jupiter::ReferenceString translated_DmgType_TiberiumAutoRifle = STRING_LITERAL_AS_REFERENCE("Tiberium Automatic Rifle");

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
Jupiter::ReferenceString translated_DmgType_Orca_Missile = STRING_LITERAL_AS_REFERENCE("Orca Missle");
Jupiter::ReferenceString translated_DmgType_Orca_Gun = STRING_LITERAL_AS_REFERENCE("Orca Gun");
Jupiter::ReferenceString translated_DmgType_Apache_Rocket = STRING_LITERAL_AS_REFERENCE("Apache Rocket");
Jupiter::ReferenceString translated_DmgType_Apache_Gun = STRING_LITERAL_AS_REFERENCE("Apache Gun");

/** Other weapons */
Jupiter::ReferenceString translated_DmgType_Nuke = STRING_LITERAL_AS_REFERENCE("Nuclear Missile Strike");
Jupiter::ReferenceString translated_DmgType_IonCannon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Strike");
Jupiter::ReferenceString translated_DmgType_GunEmpl = STRING_LITERAL_AS_REFERENCE("Gun Emplacement Gattling Gun");
Jupiter::ReferenceString translated_DmgType_GunEmpl_Alt = STRING_LITERAL_AS_REFERENCE("Gun Emplacement Automatic Cannon");
Jupiter::ReferenceString translated_DmgType_RocketEmpl_Swarm = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement Swarm Missile");
Jupiter::ReferenceString translated_DmgType_RocketEmpl_Missile = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement Hellfire Missile");

// Vehicles copied from above.

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

/** GDI Vehicles */
Jupiter::ReferenceString translated_DmgType_Harvester_GDI = STRING_LITERAL_AS_REFERENCE("GDI Harvester");
Jupiter::ReferenceString translated_DmgType_Humvee = STRING_LITERAL_AS_REFERENCE("Humvee");
Jupiter::ReferenceString translated_DmgType_MRLS = STRING_LITERAL_AS_REFERENCE("Mobile Rocket Launcher System");
Jupiter::ReferenceString translated_DmgType_APC_GDI = STRING_LITERAL_AS_REFERENCE("GDI APC");
Jupiter::ReferenceString translated_DmgType_MediumTank = STRING_LITERAL_AS_REFERENCE("Medium Tank");
Jupiter::ReferenceString translated_DmgType_MammothTank = STRING_LITERAL_AS_REFERENCE("Mammoth Tank");
Jupiter::ReferenceString translated_DmgType_Chinook_GDI = STRING_LITERAL_AS_REFERENCE("GDI Chinook");
Jupiter::ReferenceString translated_DmgType_Orca = STRING_LITERAL_AS_REFERENCE("Orca");

/** Other Vehicles */
Jupiter::ReferenceString translated_DmgType_A10_DmgType_GattlingGun = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Gattling Gun");
Jupiter::ReferenceString translated_DmgType_A10_DmgType_Bomb = STRING_LITERAL_AS_REFERENCE("A10 Thunderbolt Bomb");
Jupiter::ReferenceString translated_DmgType_AC130_DmgType_HeavyCannon = STRING_LITERAL_AS_REFERENCE("AC130 Heavy Cannon");
Jupiter::ReferenceString translated_DmgType_AC130_DmgType_AutoCannon = STRING_LITERAL_AS_REFERENCE("AC130 Auto Cannon");

/** Nod structures */
Jupiter::ReferenceString translated_Building_HandOfNod_Internals = STRING_LITERAL_AS_REFERENCE("Hand of Nod");
Jupiter::ReferenceString translated_Building_AirTower_Internals = STRING_LITERAL_AS_REFERENCE("Airstrip");
Jupiter::ReferenceString translated_Building_Refinery_Nod_Internals = STRING_LITERAL_AS_REFERENCE("Nod Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_Nod_Internals = STRING_LITERAL_AS_REFERENCE("Nod Power Plant");

/** GDI structures */
Jupiter::ReferenceString translated_Building_Barracks_Internals = STRING_LITERAL_AS_REFERENCE("Barracks");
Jupiter::ReferenceString translated_Building_WeaponsFactory_Internals = STRING_LITERAL_AS_REFERENCE("Weapons Factory");
Jupiter::ReferenceString translated_Building_Refinery_GDI_Internals = STRING_LITERAL_AS_REFERENCE("GDI Refinery");
Jupiter::ReferenceString translated_Building_PowerPlant_GDI_Internals = STRING_LITERAL_AS_REFERENCE("GDI Power Plant");

/** Defense structures */
Jupiter::ReferenceString translated_Building_AdvancedGuardTower_Internals = STRING_LITERAL_AS_REFERENCE("Advanced Guard Tower");
Jupiter::ReferenceString translated_Building_Obelisk_Internals = STRING_LITERAL_AS_REFERENCE("Obelisk of Light");

/** Defences */
Jupiter::ReferenceString translated_Defence_GuardTower = STRING_LITERAL_AS_REFERENCE("Guard Tower");
Jupiter::ReferenceString translated_Defence_Turret = STRING_LITERAL_AS_REFERENCE("Turret");
Jupiter::ReferenceString translated_Defence_SAMSite = STRING_LITERAL_AS_REFERENCE("SAM Site");
Jupiter::ReferenceString translated_Defence_AATower = STRING_LITERAL_AS_REFERENCE("Anti-Air Guard Tower");
Jupiter::ReferenceString translated_Defence_GunEmplacement = STRING_LITERAL_AS_REFERENCE("Gun Emplacement");
Jupiter::ReferenceString translated_Defence_RocketEmplacement = STRING_LITERAL_AS_REFERENCE("Rocket Emplacement");

/** UT damage types */
Jupiter::ReferenceString translated_UTDmgType_VehicleExplosion = STRING_LITERAL_AS_REFERENCE("Vehicle Explosion");
Jupiter::ReferenceString translated_UTDmgType_Drowned = STRING_LITERAL_AS_REFERENCE("Drowned");

/** Beacons */
Jupiter::ReferenceString translated_IonCannonBeacon = STRING_LITERAL_AS_REFERENCE("Ion Cannon Beacon");
Jupiter::ReferenceString translated_NukeBeacon = STRING_LITERAL_AS_REFERENCE("Nuclear Strike Beacon");
Jupiter::ReferenceString translated_KillZDamageType = STRING_LITERAL_AS_REFERENCE("Kill Zone");

RenX::TeamType RenX::getTeam(char team)
{
	switch (team)
	{
	case 'g':
	case 'G':
		return GDI;
	case 'n':
	case 'N':
		return Nod;
	default:
		return Other;
	}
}

const Jupiter::ReadableString &RenX::getTeamColor(TeamType team)
{
	switch (team)
	{
	case GDI:
		return GDIColor;
	case Nod:
		return NodColor;
	default:
		return OtherColor;
	}
}

const Jupiter::ReadableString &RenX::getTeamName(TeamType team)
{
	switch (team)
	{
	case GDI:
		return GDIShortName;
	case Nod:
		return NodShortName;
	default:
		return OtherShortName;
	}
}

const Jupiter::ReadableString &RenX::getFullTeamName(TeamType team)
{
	switch (team)
	{
	case GDI:
		return GDILongName;
	case Nod:
		return NodLongName;
	default:
		return OtherLongName;
	}
}

RenX::TeamType RenX::getEnemy(TeamType team)
{
	switch (team)
	{
	case GDI:
		return Nod;
	case Nod:
		return GDI;
	default:
		return Other;
	}
}

inline const Jupiter::ReadableString &checkINIForValue(const Jupiter::ReadableString &section, const Jupiter::ReadableString &key)
{
	return RenX::getCore()->getTranslationsFile().get(section, key);
}

const Jupiter::ReadableString &RenX::translateName(const Jupiter::ReadableString &obj)
{
	if (obj.isEmpty())
		return Jupiter::ReferenceString::empty;

	Jupiter::ReferenceString iniTranslation = checkINIForValue(STRING_LITERAL_AS_REFERENCE("Name"), obj);
	if (iniTranslation.isEmpty() == false)
		return iniTranslation;

	static Jupiter::ReferenceString object;
	object = obj;

	if (object.find(STRING_LITERAL_AS_REFERENCE("Rx_")) == 0)
		object.shiftRight(3);

	if (object.find(STRING_LITERAL_AS_REFERENCE("Vehicle_")) == 0)
	{
		object.shiftRight(8);

		/** Nod Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Harvester_Nod"))) return translated_Vehicle_Harvester_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Buggy"))) return translated_Vehicle_Buggy;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Artillery"))) return translated_Vehicle_Artillery;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("APC_Nod"))) return translated_Vehicle_APC_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("LightTank"))) return translated_Vehicle_LightTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FlameTank"))) return translated_Vehicle_FlameTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("StealthTank"))) return translated_Vehicle_StealthTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Chinook_Nod"))) return translated_Vehicle_Chinook_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Apache"))) return translated_Vehicle_Apache;

		/** GDI Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Harvester_GDI"))) return translated_Vehicle_Harvester_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Humvee"))) return translated_Vehicle_Humvee;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MRLS"))) return translated_Vehicle_MRLS;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("APC_GDI"))) return translated_Vehicle_APC_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MediumTank"))) return translated_Vehicle_MediumTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MammothTank"))) return translated_Vehicle_MammothTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Chinook_GDI"))) return translated_Vehicle_Chinook_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Orca"))) return translated_Vehicle_Orca;

		/** Other Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("A10_DmgType_GattlingGun"))) return translated_Vehicle_A10_DmgType_GattlingGun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("A10_DmgType_Bomb"))) return translated_Vehicle_A10_DmgType_Bomb;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AC130_DmgType_HeavyCannon"))) return translated_Vehicle_AC130_DmgType_HeavyCannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AC130_DmgType_AutoCannon"))) return translated_Vehicle_AC130_DmgType_AutoCannon;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("DmgType_")) == 0)
	{
		object.shiftRight(8);

		/** Non-weapon damage types */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Suicided"))) return translated_DmgType_Suicided;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Fell"))) return translated_DmgType_Fell;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Tiberium"))) return translated_DmgType_Tiberium;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("TiberiumBleed"))) return translated_DmgType_TiberiumBleed;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RanOver"))) return translated_DmgType_RanOver;

		/** Infantry weapons */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Pistol"))) return translated_DmgType_Pistol;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("SMG"))) return translated_DmgType_SMG;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("HeavyPistol"))) return translated_DmgType_HeavyPistol;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Carbine"))) return translated_DmgType_Carbine;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("TiberiumFlechetteRifle"))) return translated_DmgType_TiberiumFlechetteRifle; // Not a rifle.
		if (object.equals(STRING_LITERAL_AS_REFERENCE("TiberiumAutoRifle"))) return translated_DmgType_TiberiumAutoRifle;

		if (object.equals(STRING_LITERAL_AS_REFERENCE("Grenade"))) return translated_DmgType_Grenade;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("TimedC4"))) return translated_DmgType_TimedC4;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RemoteC4"))) return translated_DmgType_RemoteC4;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("ProxyC4"))) return translated_DmgType_ProxyC4;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("ATMine"))) return translated_DmgType_ATMine;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("EMPGrenade"))) return translated_DmgType_EMPGrenade;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("BurnC4"))) return translated_DmgType_BurnC4;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FireBleed"))) return translated_DmgType_FireBleed; // Caused by C4 or flame weapons

		if (object.equals(STRING_LITERAL_AS_REFERENCE("AutoRifle"))) return translated_DmgType_AutoRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Shotgun"))) return translated_DmgType_Shotgun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FlameThrower"))) return translated_DmgType_FlameThrower;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("GrenadeLauncher"))) return translated_DmgType_GrenadeLauncher;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MarksmanRifle"))) return translated_DmgType_MarksmanRifle;

		if (object.equals(STRING_LITERAL_AS_REFERENCE("ChainGun"))) return translated_DmgType_ChainGun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MissileLauncher"))) return translated_DmgType_MissileLauncher;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MissileLauncher_Alt"))) return translated_DmgType_MissileLauncher_Alt;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("ChemicalThrower"))) return translated_DmgType_ChemicalThrower;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("LaserRifle"))) return translated_DmgType_LaserRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("TacticalRifle"))) return translated_DmgType_TacticalRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RocketLauncher"))) return translated_DmgType_RocketLauncher;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("LaserChainGun"))) return translated_DmgType_LaserChainGun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FlakCannon"))) return translated_DmgType_FlakCannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FlakCannon_Alt"))) return translated_DmgType_FlakCannon_Alt;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("SniperRifle"))) return translated_DmgType_SniperRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RamjetRifle"))) return translated_DmgType_RamjetRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Railgun"))) return translated_DmgType_Railgun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("PersonalIonCannon"))) return translated_DmgType_PersonalIonCannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("VoltRifle"))) return translated_DmgType_VoltRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("VoltRifle_Alt"))) return translated_DmgType_VoltRifle_Alt;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("VoltAutoRifle"))) return translated_DmgType_VoltAutoRifle;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("VoltAutoRifle_Alt"))) return translated_DmgType_VoltAutoRifle_Alt;

		/** Vehicle weapons */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MammothTank_Missile"))) return translated_DmgType_MammothTank_Missile;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MammothTank_Cannon"))) return translated_DmgType_MammothTank_Cannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Orca_Missile"))) return translated_DmgType_Orca_Missile;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Orca_Gun"))) return translated_DmgType_Orca_Gun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Apache_Rocket"))) return translated_DmgType_Apache_Rocket;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Apache_Gun"))) return translated_DmgType_Apache_Gun;

		/** Other weapons */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Nuke"))) return translated_DmgType_Nuke;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("IonCannon"))) return translated_DmgType_IonCannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("GunEmpl"))) return translated_DmgType_GunEmpl;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("GunEmpl_Alt"))) return translated_DmgType_GunEmpl_Alt;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RocketEmpl_Swarm"))) return translated_DmgType_RocketEmpl_Swarm;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RocketEmpl_Missile"))) return translated_DmgType_RocketEmpl_Missile;

		// Vehicles copied from above.

		/** Nod Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Harvester_Nod"))) return translated_DmgType_Harvester_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Buggy"))) return translated_DmgType_Buggy;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Artillery"))) return translated_DmgType_Artillery;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("APC_Nod"))) return translated_DmgType_APC_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("LightTank"))) return translated_DmgType_LightTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("FlameTank"))) return translated_DmgType_FlameTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("StealthTank"))) return translated_DmgType_StealthTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Chinook_Nod"))) return translated_DmgType_Chinook_Nod;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Apache"))) return translated_DmgType_Apache;

		/** GDI Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Harvester_GDI"))) return translated_DmgType_Harvester_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Humvee"))) return translated_DmgType_Humvee;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MRLS"))) return translated_DmgType_MRLS;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("APC_GDI"))) return translated_DmgType_APC_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MediumTank"))) return translated_DmgType_MediumTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("MammothTank"))) return translated_DmgType_MammothTank;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Chinook_GDI"))) return translated_DmgType_Chinook_GDI;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Orca"))) return translated_DmgType_Orca;

		/** Other Vehicles */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("A10_DmgType_GattlingGun"))) return translated_DmgType_A10_DmgType_GattlingGun;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("A10_DmgType_Bomb"))) return translated_DmgType_A10_DmgType_Bomb;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AC130_DmgType_HeavyCannon"))) return translated_DmgType_AC130_DmgType_HeavyCannon;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AC130_DmgType_AutoCannon"))) return translated_DmgType_AC130_DmgType_AutoCannon;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Building_")) == 0)
	{
		object.shiftRight(9);
		/** Nod structures */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("HandOfNod_Internals"))) return translated_Building_HandOfNod_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AirTower_Internals"))) return translated_Building_AirTower_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Refinery_Nod_Internals"))) return translated_Building_Refinery_Nod_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("PowerPlant_Nod_Internals"))) return translated_Building_PowerPlant_Nod_Internals;
		
		/** GDI structures */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Barracks_Internals"))) return translated_Building_Barracks_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("WeaponsFactory_Internals"))) return translated_Building_WeaponsFactory_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Refinery_GDI_Internals"))) return translated_Building_Refinery_GDI_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("PowerPlant_GDI_Internals"))) return translated_Building_PowerPlant_GDI_Internals;

		/** Defense structures */
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AdvancedGuardTower_Internals"))) return translated_Building_AdvancedGuardTower_Internals;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Obelisk_Internals"))) return translated_Building_Obelisk_Internals;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("Defence_")) == 0)
	{
		object.shiftRight(8);
		if (object.equals(STRING_LITERAL_AS_REFERENCE("GuardTower"))) return translated_Defence_GuardTower;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Turret"))) return translated_Defence_Turret;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("SAMSite"))) return translated_Defence_SAMSite;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("AATower"))) return translated_Defence_AATower;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("GunEmplacement"))) return translated_Defence_GunEmplacement;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("RocketEmplacement"))) return translated_Defence_RocketEmplacement;
	}
	else if (object.find(STRING_LITERAL_AS_REFERENCE("UTDmgType_")) == 0)
	{
		object.shiftRight(10);
		if (object.equals(STRING_LITERAL_AS_REFERENCE("VehicleExplosion"))) return translated_UTDmgType_VehicleExplosion;
		if (object.equals(STRING_LITERAL_AS_REFERENCE("Drowned"))) return translated_UTDmgType_Drowned;
	}
	else if (object.equals(STRING_LITERAL_AS_REFERENCE("IonCannonBeacon"))) return translated_IonCannonBeacon;
	else if (object.equals(STRING_LITERAL_AS_REFERENCE("NukeBeacon"))) return translated_NukeBeacon;
	else if (object.equals(STRING_LITERAL_AS_REFERENCE("KillZDamageType"))) return translated_KillZDamageType;
	return object;
}

const Jupiter::ReadableString &RenX::translateWinType(RenX::WinType winType)
{
	switch (winType)
	{
	case Score:
		return scoreWinTypeTranslation;
	case Base:
		return baseWinTypeTranslation;
	case Beacon:
		return beaconWinTypeTranslation;
	case Tie:
		return tieWinTypeTranslation;
	case Shutdown:
		return shutdownWinTypeTranslation;
	case Unknown:
	default:
		return unknownWinTypeTranslation;
	}
}

const Jupiter::ReadableString &RenX::translateWinTypePlain(RenX::WinType winType)
{
	switch (winType)
	{
	case Score:
		return scoreWinTypePlainTranslation;
	case Base:
		return baseWinTypePlainTranslation;
	case Beacon:
		return beaconWinTypePlainTranslation;
	case Tie:
		return tieWinTypePlainTranslation;
	case Shutdown:
		return shutdownWinTypePlainTranslation;
	case Unknown:
	default:
		return unknownWinTypePlainTranslation;
	}
}

void RenX::initTranslations(Jupiter::INIFile &translationsFile)
{
	NodColor = translationsFile.get(STRING_LITERAL_AS_REFERENCE("TeamColor"), STRING_LITERAL_AS_REFERENCE("Nod"), STRING_LITERAL_AS_REFERENCE("04"));
	GDIColor = translationsFile.get(STRING_LITERAL_AS_REFERENCE("TeamColor"), STRING_LITERAL_AS_REFERENCE("GDI"), STRING_LITERAL_AS_REFERENCE("08"));
	OtherColor = translationsFile.get(STRING_LITERAL_AS_REFERENCE("TeamColor"), STRING_LITERAL_AS_REFERENCE("Other"), STRING_LITERAL_AS_REFERENCE("14"));

	NodShortName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("ShortTeamName"), STRING_LITERAL_AS_REFERENCE("Nod"), STRING_LITERAL_AS_REFERENCE("Nod"));
	GDIShortName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("ShortTeamName"), STRING_LITERAL_AS_REFERENCE("GDI"), STRING_LITERAL_AS_REFERENCE("GDI"));
	OtherShortName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("ShortTeamName"), STRING_LITERAL_AS_REFERENCE("Other"), STRING_LITERAL_AS_REFERENCE("N/A"));
	NodLongName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("LongTeamName"), STRING_LITERAL_AS_REFERENCE("Nod"), STRING_LITERAL_AS_REFERENCE("Brotherhood of Nod"));
	GDILongName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("LongTeamName"), STRING_LITERAL_AS_REFERENCE("GDI"), STRING_LITERAL_AS_REFERENCE("Global Defense Initiative"));
	OtherLongName = translationsFile.get(STRING_LITERAL_AS_REFERENCE("LongTeamName"), STRING_LITERAL_AS_REFERENCE("Other"), STRING_LITERAL_AS_REFERENCE("Unknown"));

	scoreWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Score"), STRING_LITERAL_AS_REFERENCE("Domination (High Score)"));
	baseWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Base"), STRING_LITERAL_AS_REFERENCE("Conquest (Base Destruction)"));
	beaconWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Beacon"), STRING_LITERAL_AS_REFERENCE("Espionage (Beacon)"));
	tieWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Tie"), STRING_LITERAL_AS_REFERENCE("Draw (Tie)"));
	shutdownWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Shutdown"), STRING_LITERAL_AS_REFERENCE("Ceasefire (Shutdown)"));
	unknownWinTypeTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinType"), STRING_LITERAL_AS_REFERENCE("Unknown"), STRING_LITERAL_AS_REFERENCE("Aliens (Unknown)"));

	scoreWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Score"), STRING_LITERAL_AS_REFERENCE("High Score"));
	baseWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Base"), STRING_LITERAL_AS_REFERENCE("Base Destruction"));
	beaconWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Beacon"), STRING_LITERAL_AS_REFERENCE("Beacon"));
	tieWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Tie"), STRING_LITERAL_AS_REFERENCE("Tie"));
	shutdownWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Shutdown"), STRING_LITERAL_AS_REFERENCE("Shutdown"));
	unknownWinTypePlainTranslation = translationsFile.get(STRING_LITERAL_AS_REFERENCE("WinTypePlain"), STRING_LITERAL_AS_REFERENCE("Unknown"), STRING_LITERAL_AS_REFERENCE("Unknown"));
}

Jupiter::String RenX::getFormattedPlayerName(const RenX::PlayerInfo *player)
{
	Jupiter::String r;
	if (player->isBot) r += IRCCOLOR "05[B]";
	r += IRCCOLOR;
	r += RenX::getTeamColor(player->team);
	r += player->name;
	return r;
}

void RenX::sanitizeString(char *str)
{
	while (*str != 0)
	{
		switch (*str)
		{
		case '|':
			*str = '/';
			break;
		case '\\':
			if (*(str + 1) == 0) *str = '/'; // So users get something at least.
			break;
		default:
			break;
		}
		str++;
	}
}

time_t RenX::getGameTime(const RenX::PlayerInfo *player)
{
	time_t currentTime = time(0);
	if (player->joinTime == currentTime) currentTime++;
	return currentTime - player->joinTime;
}

double RenX::getKillDeathRatio(const RenX::PlayerInfo *player, bool includeSuicides)
{
	double deaths = player->deaths;
	if (includeSuicides == false) deaths -= player->suicides;
	if (deaths == 0) deaths = 1;
	return ((double)player->kills) / deaths;
}

double RenX::getHeadshotKillRatio(const RenX::PlayerInfo *player)
{
	if (player->kills == 0) return 0;
	return ((double)player->headshots) / ((double)player->kills);
}

double RenX::getKillsPerSecond(const RenX::PlayerInfo *player)
{
	return ((double)player->kills) / ((double)RenX::getGameTime(player));
}
