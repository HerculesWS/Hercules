# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project does not adhere to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
<!--
If you are reading this in a text editor, simply ignore this section
-->

## [v2017.10.22-1] `October 22 2017` `PATCH 1`
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
[v2017.10.22-1]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22...v2017.10.22-1
[v2017.10.22]: https://github.com/HerculesWS/Hercules/compare/6b1fe2d...v2017.10.22
