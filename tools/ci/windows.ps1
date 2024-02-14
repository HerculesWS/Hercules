# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2024 Hercules Dev Team
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

# Helper script for the Windows CI build

function CreatePluginProject {
	param([string]$pluginName)

	Write-Host "Creating project for plugin $pluginName"

	$pluginFileSample = "vcproj\\plugin-sample.vcxproj"
	$pluginFile = "vcproj\\plugin-$pluginName.vcxproj"

	Copy-Item -Path $pluginFileSample -Destination $pluginFile
            (Get-Content $pluginFile) -replace "sample", $pluginName | Set-Content $pluginFile
}

function WritePropsFile {
	param([string[]]$definitions)

	Write-Host "Writing props file with definitions: $definitions"

	# Add preprocessor definitions for msbuild using a props file
	# https://learn.microsoft.com/en-us/visualstudio/msbuild/customize-cpp-builds?view=vs-2022
	$propsFile = Join-Path -Path (Get-Location) -ChildPath "vcproj\AdditionalDefinitions.props"
@"
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemDefinitionGroup>
    <ClCompile>
      <PreprocessorDefinitions>%(PreprocessorDefinitions);$($definitions -join ";")</PreprocessorDefinitions>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
"@ | Out-File -FilePath $propsFile -Encoding utf8
	return $propsFile
}

function CatchProcessErrors {
	param([string]$programName, [string]$additionalArgs)

	Write-Host "Running server $programName with arguments: $additionalArgs"

	$process = Start-Process -FilePath "$programName" -ArgumentList $additionalArgs -NoNewWindow -PassThru -RedirectStandardError "error_$programName.txt"
	$process.WaitForExit()

	$errorText = Get-Content "error_$programName.txt" -TotalCount 10000
	if ($null -ne $errorText -and $errorText -ne "") {
		Write-Host "Errors found in running server $programName"
		Write-Host "$errorText"
		exit 1
	}

	if ($process.ExitCode -ne 0) {
		Write-Host "$programName failed with exit code $($process.ExitCode)"
		exit 1
	}
}

Write-Host "Arguments: $args"
if ($args[0] -eq "build") {
	$propsFile = WritePropsFile $args[1..($args.Length - 1)]
	CatchProcessErrors msbuild "-m Hercules.sln /p:ForceImportBeforeCppTargets=$propsFile"

	foreach ($plugin in @("httpsample", "constdb2doc", "db2sql", "dbghelpplug", "generate-translations", "mapcache", "script_mapquit")) {
		CreatePluginProject $plugin
		CatchProcessErrors msbuild "-m vcproj\\plugin-$plugin.vcxproj /p:ForceImportBeforeCppTargets=$propsFile"
	}
}
elseif ($args[0] -eq "run") {
	$serverArgs = "--run-once"
	CatchProcessErrors "api-server.exe" $serverArgs
	CatchProcessErrors "login-server.exe" $serverArgs
	CatchProcessErrors "char-server.exe" $serverArgs
	CatchProcessErrors "map-server.exe" $serverArgs
}
elseif ($args[0] -eq "test") {
	$serverArgs = "--run-once"
	foreach ($plugin in @("HPMHooking", "httpsample", "constdb2doc", "db2sql", "dbghelpplug", "generate-translations", "mapcache", "script_mapquit")) {
		$serverArgs += " --load-plugin $plugin"
	}
	CatchProcessErrors "map-server.exe" "$serverArgs --load-script npc/dev/test.txt --load-script npc/dev/ci_test.txt"
	CatchProcessErrors "map-server.exe" "$serverArgs --constdb2doc"
	CatchProcessErrors "map-server.exe" "$serverArgs --db2sql"
	CatchProcessErrors "map-server.exe" "$serverArgs --itemdb2sql"
	CatchProcessErrors "map-server.exe" "$serverArgs --mobdb2sql"
	CatchProcessErrors "map-server.exe" "$serverArgs --generate-translations"
	CatchProcessErrors "map-server.exe" "$serverArgs --fix-md5"
}
