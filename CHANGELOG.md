
# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project does not adhere to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
<!--
If you are reading this in a text editor, simply ignore this section
-->

## [v2018.06.03] `Jun 3 2018`
### Added
- Added/updated packets support for clients from 2018-05-09 to 2018-05-23. (#2043)
- Added client/version-specific `enum clif_messages` values for msgstringtable message IDs. All the related functions have been updated. (#2038)
- Added the script commands `setpcblock()` and `checkpcblock()`, to prevent various character actions - see the script command docs for details. (#842)
- Implemented the Stylist UI, available in clients starting from 2015. Configurable in `stylist_db.conf`, accessible to scripts through `openstylist()`. (#2004)

### Changed
- Extended the script command `getunits()` with support to look up units globally, making the map argument optional. (#1851)
- Updated copyright headers to year 2018. (#2054)
- Converted `exp.txt` (now `exp_group_db.conf`) to the libconfig format, now better integrated with `job_db.conf`. (#2036, originally #1944)

### Fixed
- Fixed an issue in the mob skill db parser that limited the mob skills to a maximum of 5 (#2042, issue #2044)
- Fixed some incorrect msgstringtable IDs. (part of #2038)
- Fixed inheritance in the mob DB, no longer iverwriting the Range field with a default value. (#2055)
- Fixed the skill element getter for levels above `MAX_SKILL_LEVEL`. (#2059)
- Fixed interaction between the `pvp_nocalcrank` mapflag and the script/atcommands to toggle PvP. (#2057, issue #2056)

### Deprecated
- While not officially deprecated yet, use of `maprespawnguildid()` and `playbgmall()` has been superseded by `getunits()`. (part of #1851)
- Deprecated the `pcblockmove()` script command. Use the more flexible `setpcblock()` instead. (part pf #842)

## [v2018.05.06] `May 6 2018`
### Added
- Added a configurable PIN code blacklist, to prevent use of certain codes. (#2007 and #2029, issue #769)
- Added/updated packets support for clients from 2018-04-11 to 2018-05-02. (#2021, #2030)
- Implemented option to allow guild skill cooldowns to continue when the leader is logged out. Enabled by default and controlled by the `guild_skill_relog_delay` flag in guild.conf. (#2005, issue #1774)
- Implemented the Private Airship system, currently using the list of maps from the Zero server. (#1998)
- Added the constants `DEFAULT_MOB_JNAME` and `DEFAULT_MOB_NAME` (source only) to replace hardcoded use of `"--ja--"` and `"--en--"` respectively. (part of #2027)

### Changed
- Replaced custom messages related to the PIN code system with the official ones. (part of #2007)
- Updated the minimum client version that enables certain features: new drop packet now in `PACKETVER >= 20180418`, attendance system now in `PACKETVER_ZERO_NUM >= 20180411`. (#2020)
- Introduced a friendly error message when the `delwall()` script command fails due to a non-existent wall. (#2017)
- Refactored some code to move `MAPID_*` related code into separate functions. (#2022)
- Changed the plugins Makefile to honor the `$MYPLUGIN` variable passed through the environment, to make it easier to compile specific plugins without editing files. (#2025)
- Converted the Mob Skill DB to the libconfig format. A converter script (`mobskilldbconverter.py`) has been provided for convenience. (#2019)

### Fixed
- Fixed interaction between Curse and Blessing. When under Curse or Stone Curse, Blessing will only remove the negative statuses and will need to be cast again to obtain the buff. (#1706, issue #680)
- Added support for `time_t` as return type in the HPMHookGen. (bb0e228bd29dd689ca76f64578de8759415a763b)
- Fixed some possible buffer overflows. (#2028)
- Fixed the return value of `BUILDIN(getunitdata)`. (d6785d389cbee4f34078f6762626ca61b2d6cc25)
- Improved support for clients version 2018-02-07 and Zero 2018-01-31. (part of #2030)
- Fixed the clan names in some clan-related NPC dialogs. (#2032)
- Fixed the display name of monster summoned through the `SA_SUMMONMONSTER` skill. (#2027)

### Removed
- Removed all the code related to the anonymous-stat-reporting system. (#2023)

## [v2018.04.08] `April 8 2018`
### Added
- Added/updated packets support for clients from 2018-03-14 to 2018-04-04. (#1994 and #2014)
- Introduced macros `PACKETVER_RE_NUM`, `PACKETVER_ZERO_NUM` and `PACKETVER_MAIN_NUM` to simplify client type-specific version checks.
  These macros are defined to `PACKETVER` only if, respectively, `PACKETVER_RE`, `PACKETVER_ZERO` or neither are defined. (part of #1994)
- Implemented Hat Effects, available in clients starting from 2015-04-22. (#1965)
  - The `hateffect()` script command has been implemented.
  - The related constants (with prefix `HAT_EF_*`) have been added and made available to the script engine.
- Added the 2015 variant of the quest-related packets. (#1111)
- Added login date information for guild members, on clients starting from 2016-10-26. The message format can be customized on the client side, by editing line 3012 of msgstringtable.txt. (#1986)
- Added support for the `ZC_FORMATSTRING_MSG` and `ZC_MSG_COLOR` packets, handling msgstringtable messages. (#2012)
- Added a setting (`storage_use_item` in `items.conf`) to control the use of items (usable/consumable/boxes) when the storage is open. (#1868, issue #1806)
- Implemented the Attendance System, requiring client 2018-03-07bRagexeRE or newer. Configuration is available in `feature.conf` and `db/attendance_db.conf`. (#1990)
- Added a configurable delay to the MVP Tombstone. The delay can be configured through the `mvp_tomb_spawn_delay` setting in `monster.conf`. (#2001, issue #1980)

### Changed
- Updated the functions handling quest-related packets to use the struct-based form. (part of #1111)
- Converted the Pet DB to the libconfig format. A converter script (`petdbconverter.py`) has been provided for convenience. (#2000)
- The `noteleport` mapflag has been added to the Archer Village (`pay_arche`), to match official servers. (part of #2006)
- The `script->sprintf()` function has been renamed to `script->sprintf_helper()`. (part of #2009)

### Fixed
- Removed a duplicated line in the login server VS project that would prevent Visual Studio from loading it. (#1992)
- Prevented a console warning when a nonexistent map is passed to the `getmapinfo()` script command. (584e8de35)
- Fixed a RODEX loading data problem when a message's expiration date was manually edited. (#1995)
- Corrected the error messages displayed when using various restricted items to match the official servers. (#2006)
- Added a missing status refresh for the Homunculus Autofeed system when changing maps. (#2002)
- Fixed a NULL pointer check failure when `TK_JUMPKICK` is used by a non-player. (#2015, issue #1875)
- Fixed compilation of the HPMHooking plugin on systems where `sprintf()` is a macro. (#2009, issue #2003)

## [v2018.03.11] `March 11 2018`
### Added
- Added a new `mapcache` plugin to convert, update or recreate mapcache files in the new format. (part of #1552)
- Added appveyor configuration to the repository. (part of #1552)
- Exposed `script->sprintf()` to plugins. (#1976)
- Added/updated packets support for clients from 2018-02-21 to 2018-03-09. (#1983)

### Changed
- Updated the mapcache to a new, git-friendly, format having one file per map. (#1552, #1981)
  - For info on how to convert or recreate mapcache entries, see the mapcache plugin (`./map-server --load-plugin mapcache --help`)
- Removed the display of PIN codes and passwords from the `@accinfo` GM command. Old code is kept commented out for those that may wish to re-enable it. (#1975)
- Updated README.md with some clarifications and corrections. (#1985)

### Fixed
- Updated the VS project files with the recently added .h files, for better intellisense/search. (#1970)
- Fixed a NULL pointer in `login->accounts`, only affecting plugins. (part of #1979)
- Fixed a case of use after free in the `@reloadatcommand` GM command. (part of #1979)
- Added several missing checks in various `clif_parse_*` functions. (part of #1979)
- Fixed various PIN code related exploits. (part of #1979)
- Fixed a case of use after free when the option `delay_battle_damage` is set to false. (part of #1979)
- Fixed a segmentation fault in clan-related code when using the `db2sql` plugin. (#1989, issue #1984)
- Fixed an incorrect behavior in RODEX returned messages. (part of #1987)
- Fixed an error that made RODEX mails impossible to delete in some cases. (part of #1987)
- Fixed a NULL pointer in RODEX when the user tried to perform actions on unloaded mails. (part of #1987, issue #1977)
- Fixed an incorrect interaction between RODEX and NPCs. (#1936)
- Fixed an incorrect Kafra Points / Cash Points calculation. (#1541, issue #1540)

### Removed
- Removed the old `mapcache` executable, superseded by the new plugin. (part of #1552)

## [v2018.02.11+1] `February 13 2018` `PATCH 1`
### Fixed
- Fixed a possible crash in `@cvcon` (and possibly other functions) when a referenced map zone doesn't exist. (#1972, issue #1971)
- Fixed the messages displayed when enabling or disabling CvC. (part of #1972)
- Extended the `bg_message` string termination fix to all the clients. (#1973)

## [v2018.02.11] `February 11 2018`
### Added
- Added/updated packets support for clients from 2017-12-13 to 2018-01-24. (part of #1957)
- Implemented the official Clan System, including the possibility of customization and a Clan vs Clan versus mode. (#1718, #1964, #1968, related to issue #241)
  - New GM commands: `@claninfo`, `@joinclan`, `@leaveclan`, `@reloadclans`, `@cvcon` and `@cvcoff`.
  - New script commands: `clan_join()`, `clan_leave()` and `clan_master()`; extended `strcharinfo()` and `getcharid()`.
  - Configuration changes: see `conf/clans.conf`, `conf/map/logs.conf`, `db/clans.conf`, `db/*/map_zone_db.conf`.
  - Note: This requires the SQL migrations `2017-06-04--15-04.sql` and `2017-06-04--15-05.sql`.
  - Note: The `npc/re/other/clans.txt` script is now loaded by default in renewal mode.
- Added several (status-icon related) constants to the script engine (through the new `constants.inc` file). (part of #1718)
- Implemented the missing HPM interfaces in the login server (account, ipban, lchrif), added the missing variables into the login interfaces. (#1963, issue #1908)
  - The `_sql` suffix has been removed from the source files in the login server.
  - Functions in `account.c` and `loginlog.c` have been prefixed with `account_` and `loginlog_` respectively.
  - The `chrif_` functions of the login server have been renamed to `lchrif_`.
  - The `server[]` array has been moved to `login->dbs->server[]`.
  - The `account` (account.h), `ipban` (ipban.h), `lchrif` (login.h), `loginlog` (loginlog.h)
  - Several `log_*` global variables have been moved to the loginlog interface, with their respective names.
  - The `account_engine[0]` variable has been moved to `login->dbs->account_engine` (note: this is not an array!)
- Added/updated packets support for clients from 2018-01-31 to 2018-02-07. (#1969)

### Changed
- Applied script standardization to the Bakonawa Lake instance script. (#1874)
- Applied script standardization to the Buwaya Cave instance script. (#1877)
- Applied script standardization to the Eclage Interior instance script. (#1878)
- Applied script standardization to the Hazy Forest instance script. (#1880)
- Applied script standardization to the Malangdo Culvert instance script. (#1881)

### Fixed
- Fixed compatibility issues with the 2013-12-23 client. (part of #1957, issue #1956)
- Prevented the leak of a hidden GM's presence through area packets. (#1200)
- Fixed an unterminated string in the `bg_message()` related packets, with certain client versions. (#1890)

## [v2018.01.14] `January 14 2018`
### Added
- Added support for the `AllowReproduce` flag in the skill DB. This supersedes the skill_reproduce_db. (#1943)
- Added support for the `ZC_PROGRESS_ACTOR` packet. The packet is exposed to the script engine through the `progressbar_unit()` command (available on PACKETVER 20130821 and newer). (#1929)
- Added support for the new item drop packet for the Zero clients. The packet is controlled by the `ShowDropEffect` and `DropEffectMode` item DB flags and ignored by non-Zero clients. (#1939)
- Added support for the new Map Server Change packet 0x0ac7. (part of #1948)

### Changed
- Always enabled assertions and null pointer checks. In order to disable them (very discouraged, as it may lead to security issues), it is now necessary to edit `nullpo.h`. (#1937)
- Disabled the address sanitizer's memory leak detector in the travis builds, since it produced failures in third libraries. (#1949, #1952)
- Applied script standardization to the Nydhogg's Nest instance script. (#1871)
- Split packet_keys.h into separate files for main clients and zero clients. (part of #1948)
- Split packets_shuffle.h into separate files for main clients and zero clients. (part of #1948)
- Replaced the custom bank unavailable error message with the actual bank check error packet. (part of #1948)
- Updated and corrected the party member and party info packets. (part of #1948)
- Updated README.md with more relevant badges and links (added Discord, removed Waffle, added more GitHub information). (#1951)

### Fixed
- Updated Xcode project to include the RODEX related files. (#1942)
- Fixed RODEX loading mails when it requires multiple packets. (#1945, issue #1933)

### Removed
- Removed the skill_reproduce_db, now superseded by the `AllowReproduce` skill flag. (part of #1943)

## [v2017.12.17] `December 17 2017`
### Added
- Implemented Homunculus Autofeeding, available on the 2017 clients. The feature can be disabled by flipping `features.enable_homun_autofeed` in feature.conf. (#1898)
- Added support for the newly released Ragnarok Zero clients. The client type is controlled with the `--enable-packetver-zero` configure-time flag (disabled by default). (#1923)

### Changed
- Applied script standardization to the Old Glast Heim instance script. (#1883)
- Split packets.h into two files: packets.h and packets_shuffle.h. (part of #1923)

### Fixed
- Corrected a wrong path displayed in an error message pointing to the map-server configuration. (#1913)
- Fixed the natural expiration of the Poison status when under the effect of Slow Poison. (#1925)

## [v2017.11.19+2] `November 28 2017` `PATCH 2`
### Fixed
- Fixed an item loading failure in RODEX. (#1917, issue #1912)
- Fixed invisible NPCs (such as `FAKE_NPC`) being displayed as novices. (#1918, issue #1916)

## [v2017.11.19+1] `November 24 2017` `PATCH 1`
### Fixed
- Suppressed assertions in the Skill DB accessors when called with `skill_id = 0` (normal attacks). (#1910, issue #1909)

## [v2017.11.19] `November 19 2017`
### Added
- Added several missing members to the login interface. (Part of #1891)
- Added support for colored character server population counter in the service selection list. Configurable through `users_count` in login-server.conf. (#1891)
- Added top-level command `miniboss_monster` to label monsters as minibosses, and to send them as such to the client. (part of #1889)
- Added support for 2017-10-25 - 2017-11-01 clients. (#1889)
- Added support to display NPCs with player classes, including equipment and styles (best with clients starting from 20170726). This extends `getunitdata()` and `setunitdata()` with support for `UDT_SEX`, `UDT_HAIRSTYLE`, `UDT_HAIRCOLOR`, `UDT_HEADBOTTOM`, `UDT_HEADMIDDLE`, `UDT_HEADTOP`, `UDT_CLOTHCOLOR`, `UDT_SHIELD`, `UDT_WEAPON`, `UDT_ROBE`, `UDT_BODY2`. (#1893)
- Added type constants for the `getiteminfo()` and `setiteminfo()` script commands. Existing third party code must be updated to use the new constants (see the pull request description and the command documentation for details). (#1902)
- Added global function `F_GetAmmoType()`, counterpart to `F_GetWeaponType()` for ammunitions. Both functions have now been updated to only check the subtype if the item type is correct (`IT_AMMO` and `IT_WEAPON` respectively). (part of #1902)
- Added support for the Skill Scale packet, available in client versions 20151223 and newer. (#1903)

### Changed
- Applied script standardization to the Octopus Cave instance script. (#1882)
- Applied script standardization to the Ghost Palace instance script. (#1879)
- Applied script standardization to the Sara's Memory instance script. (#1884)
- Extended the script command `setequipoption()` with the possibility to remove item options from an equipment piece. (#1865)
- Updated the `QTYPE_*` constants (`questinfo()`, `showevent()`) to support the new 2017 client icons. (#1894)
- Applied script standardization to the Orc's Memory instance script. (#1872)
- Applied script standardization to the Sealed Shrine instance script. (#1873)
- Extended the global function `F_GetArmorType()` to support costumes and shadow equipment. (#1836)
- Extended the script commands `has_instance()` and `has_instance2()` with suport to search instances of type `IOT_NONE`. (#1397)
- Applied script standardization and improvements to the Endless Tower instance script. (#1862)
- Cleared some confusion between skill IDs and indexes through the codebase. Rewritten Skill DB accessors in a safer, more readable way. (part of #1896)

### Deprecated
- The use of numeric type constants with `getiteminfo()` and `setiteminfo()` is deprecated. For technical reasons, no deprecation notice is displayed. (part of #1902)

### Removed
- The `MAX_SKILL` constant has been removed, in favor of the more clear `MAX_SKILL_DB`, to be used in all places that use the compacted Skill DB array. For use with the non-compacted clientside Skill IDs, the `MAX_SKILL_ID` constant is still available. (part of #1896)

### Fixed
- Fixed compilation warnings when compiling with gcc-7. (#1887)
- Fixed the display flag for monsters labeled as `boss_monster` to be that of MVP monsters instead of miniboss monsters. (part of #1889)
- Fixed a subtle error in case `skill->unit_group_newid` overflows, causing certain skill unit entries to get stuck and never get deleted correctly. This can manifest itself with some monster spawns becoming immune to certain AoE spells having the `UF_NOOVERLAP` flag (Storm Gust, Lord of Vermillion, etc). (#1896)
- Implemented MATK support in the `getiteminfo()` and `setiteminfo()`. This functionality was previously advertised as availble in the command documentation, but was not implemented. (part of #1902)
- Restored View Sprite support in the `getiteminfo()` and `setiteminfo()`. This functionality was lost with #1828. (part of #1902, issue #1895)
- Reimplemented the global function `F_GetArmorType()` to reflect the fact that  `ITEMINFO_LOC` returns a bitmask. The function now handles multi-slot headgears and other uncommon cases better. (part of #1902)
- Corrected some incorrect data types passed to the SQL `StmtBind` functions, causing query errors and data loss. Said functions will now have a runtime assertion to ensure the right data type is passed. Third party code needs to be updated to reflect this stricter requirement. (#1901, issue #1531)
- Corrected some RODEX related queries in case `MAX_SLOTS` or `MAX_ITEM_OPTIONS` are set to custom values. (part of #1901)

## [v2017.10.22+1] `October 22 2017` `PATCH 1`
### Fixed
- Fixed a wrong null pointer check in `logmes()`, which caused the command to never log and instead print debug information.

## [v2017.10.22] `October 22 2017`
### Added
- Added the script command `getmapinfo()`, which allows to obtain misc information about a map. (#1852)
- Added an option to restrict party leader changes to characters on the same map. Controlled by the setting `party_change_leader_same_map` (defaults to true). (#1812)
- Added initial support (shuffle packets, obfuscation keys) for clients 2017-09-27, 2017-10-02, 2017-10-11, 2017-10-18. (#1859)
- Added the `noautoloot` mapflag, allowing to disable the `@autoloot` functionality on a map by map basis. (#1833)

### Changed
- Extended the script command `logmes()` with an option to log to the `atcommandlog` table. (#1843)
- Updated RoDEX, with support for packetver `20170419` and newer. (#1859)
- Updated Exp-related packets and handling functions to support values larger than 2 billions (as seen in packetver `20170830` and newer). (#1859)
- Changed the diagnostic message in `skill_init_unit_layout()` to report the skill ID instead of its index. (#1854)

### Fixed
- Corrected the Kafra dialog in case a Doram without the Summoner's Basic Skill attempts to open the Storage. (#1864)
- Changed the cell stack counting algorithm to ignore invisible NPCs, improving the Dancer Quest experience as well as other cases of hidden NPCs blocking off certain cells. (#1827)
- Improved the handling of the `cardfix` value to make it more resistant to overflows, especially in renewal mode. Simplified the related renewal/pre-renewal conditional code. (#1825)
- Fixed some compilation warnings occurring in VS2017. (#1870)

### Other
- New versioning scheme and project changelogs/release notes (#1853)

[Unreleased]: https://github.com/HerculesWS/Hercules/compare/stable...master
[v2018.06.03]: https://github.com/HerculesWS/Hercules/compare/v2018.05.06...v2018.06.03
[v2018.05.06]: https://github.com/HerculesWS/Hercules/compare/v2018.04.08...v2018.05.06
[v2018.04.08]: https://github.com/HerculesWS/Hercules/compare/v2018.03.11...v2018.04.08
[v2018.03.11]: https://github.com/HerculesWS/Hercules/compare/v2018.02.11+1...v2018.03.11
[v2018.02.11+1]: https://github.com/HerculesWS/Hercules/compare/v2018.02.11...v2018.02.11+1
[v2018.02.11]: https://github.com/HerculesWS/Hercules/compare/v2018.01.14...v2018.02.11
[v2018.01.14]: https://github.com/HerculesWS/Hercules/compare/v2017.12.17...v2018.01.14
[v2017.12.17]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+2...v2017.12.17
[v2017.11.19+2]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+1...v2017.11.19+2
[v2017.11.19+1]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19...v2017.11.19+1
[v2017.11.19]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22+1...v2017.11.19
[v2017.10.22+1]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22...v2017.10.22+1
[v2017.10.22]: https://github.com/HerculesWS/Hercules/compare/6b1fe2d...v2017.10.22
