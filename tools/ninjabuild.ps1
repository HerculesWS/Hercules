# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2026 Hercules Dev Team
# Copyright (C) 2026 Haru
#
# Hercules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This is a wrapper for the ninja command, to provide colorful output.
# It's intended to be invoked as `ninjac` (accepting the same arguments as `ninja`), after setting up through setup_env.ps1

$startTime = (Get-Date)

Set-Item -Force -Path "ENV:NINJA_STATUS" -Value "[%f+%r/%t - %e] "
Set-Item -Force -Path "ENV:CLICOLOR_FORCE" -Value "1"

$clr = @{
    none        = "";
    reset       = "$([char]27)[0m";
    black       = "$([char]27)[30m";
    darkred     = "$([char]27)[31m";
    darkgreen   = "$([char]27)[32m";
    darkyellow  = "$([char]27)[33m";
    darkblue    = "$([char]27)[34m";
    darkmagenta = "$([char]27)[35m";
    darkcyan    = "$([char]27)[36m";
    gray        = "$([char]27)[37m";
    darkgray    = "$([char]27)[30;1m";
    red         = "$([char]27)[31;1m";
    green       = "$([char]27)[32;1m";
    yellow      = "$([char]27)[33;1m";
    blue        = "$([char]27)[34;1m";
    magenta     = "$([char]27)[35;1m";
    cyan        = "$([char]27)[36;1m";
    white       = "$([char]27)[37;1m";
};
Write-Output "$($clr['darkmagenta'])- 8<- - - - - - - - - - - - BUILD START - - - - - - - - - - - - -$($clr['reset'])";
ninja $args |
Foreach-Object {
        $line = $_;
        $color = 'none';
        $prefix = '';
        if ($line -match '^(.*?)(?:\((\d+)\))?\s*:\s+(warning|error|fatal error)\s+([A-Z0-9]+):\s+(.*)$') {
            $str = $Matches.3;
            if ($str -match 'warning') {
                $color = 'darkyellow';
            } elseif ($str -match 'error') {
                $color = 'darkred';
            }
        } elseif ($line -match '^ninja:') {
            $color = 'darkgray';
        } elseif ($line -match '^(\[\d+\+\d+\/\d+(?: - [0-9.]+)?\]) (\w+)(.*)$') {
            $prefix = "$($clr['white'])$($Matches.1)$($clr['reset']) ";
            $line = "$($Matches.2)$($Matches.3)";
            $str = $Matches.2;
            if ($str -match 'Automatic') {
                $color = 'blue';
            } elseif ($str -match 'Generating') {
                $color = 'darkblue';
            } elseif ($str -match 'Building') {
                $color = 'gray';
            } elseif ($str -match 'Linking') {
                $color = 'darkcyan';
            }
        }
        return "$($prefix)$($clr[$color])$($line)$($clr['reset'])";
} | Write-Host
$result = $?
Write-Host "$($clr['darkmagenta'])- 8<- - - - - - - - - - - - BUILD END - - - - - - - - - - - - - -$($clr['reset'])";

if (-not $result) {
    Write-Host "Build failed"
    exit $result
}

$Elapsed = (Get-Date) - $startTime

Write-Host "Build completed in ${Elapsed}"
