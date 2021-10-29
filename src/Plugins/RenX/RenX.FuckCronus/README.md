== Function ==

Sanitizes logs going to the Renegade X devbot (server list), and sanitizes commands coming from the Renegade X devbot.

When all sanitizations are disabled, this can also be used to change the listed IP address for a Renegade X server,
without relying on Cronus. This is done by hosting Jupiter on the IP address you wish to have listed, and routing
traffic from there.

== Rationale ==

*Short*: Thieves shouldn't be trusted with personally identifiable information. This prevents that.

*Long* (copied from RenX.FuckCronus.ini):

Certain server owners report great unease about a specific Totem Arts sysadmin who is entirely unrestrained, who has
entirely unrestrained access to player personal information such as IP addresses, Hardware IDs (MAC addresses),
SteamIDs, and usernames, particularly since it's known that this sysadmin logs all of that information every time any
player joins any Renegade X server. It's also known that this data is stored alongside C&C Renegade player information,
meaning that C&C Renegade players and Renegade X players can be tracked across each game.

By sanitizing personal information going to the DevBot, players can be feel safe knowing that their data is not being
tracked by a sysadmin who is extremely well known to hoard, steal, and misuse data.