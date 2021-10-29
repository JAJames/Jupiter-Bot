# Setup
1) Setup Jupiter on the same server as your game server
2) Add `RenX.Listen` and `RenX.FuckCronus` to your root `Config.ini`
3) Configure `RenX-ListenServer` (see RenX.Listen.ini) server block in `RenX.Core.ini`. These are the settings for any servers which connect to your bot.
4) In UDKRenegadeX.ini on the game server, replace `devbot.ren-x.com` with `127.0.0.1`
5) Startup Jupiter Bot
6) Startup game server
7) Verify your server is listed on the RenX server list

### Optional:
It's also possible to use this plugin as a general proxy for devbot traffic, so that you can change the listed IP address of your game server. To do this:
1) Setup Jupiter on the server (i.e: proxy node) you wish to have listed instead of your game server
2) Follow the same steps as above, except use your proxy node's IP address instead of `127.0.0.1`

# Function

Sanitizes logs going to the Renegade X devbot (server list), and sanitizes commands coming from the Renegade X devbot.

When all sanitizations are disabled, this can also be used to change the listed IP address for a Renegade X server,
without relying on Cronus. This is done by hosting Jupiter on the IP address you wish to have listed, and routing
traffic from there.

# Rationale

*Short*: Thieves shouldn't be trusted with personally identifiable information. This prevents that.

*Long* (copied from RenX.FuckCronus.ini):

Certain server owners report great unease about a specific Totem Arts sysadmin who is entirely unrestrained, who has
entirely unrestrained access to player personal information such as IP addresses, Hardware IDs (MAC addresses),
SteamIDs, and usernames, particularly since it's known that this sysadmin logs all of that information every time any
player joins any Renegade X server. It's also known that this data is stored alongside C&C Renegade player information,
meaning that C&C Renegade players and Renegade X players can be tracked across each game.

By sanitizing personal information going to the DevBot, players can be feel safe knowing that their data is not being
tracked by a sysadmin who is extremely well known to hoard, steal, and misuse data.