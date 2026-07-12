# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2025-2026 Hercules Dev Team
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

	$errorFileName = $programName.Replace("\\", "--").Replace("/", "--")

	Write-Host "Running server $programName and writing errors to error_$errorFileName.txt with arguments: $additionalArgs"

	$process = Start-Process -FilePath "$programName" -ArgumentList $additionalArgs -NoNewWindow -PassThru -RedirectStandardError "error_$errorFileName.txt"
	$handle = $process.Handle # Workaround a bug where handle becomes null after wait https://stackoverflow.com/questions/44057728/start-process-system-diagnostics-process-exitcode-is-null-with-nonewwindow
	$process.WaitForExit()

	$errorText = Get-Content "error_$errorFileName.txt" -TotalCount 10000
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
	$compileFlags = $args[1..($args.Length - 1)]
	./setup_env.ps1 -BuildDir build
	Push-Location "build"
	cmake -S .. -B . $compileFlags
	ninja all
	ninja install
	Pop-Location
}
elseif ($args[0] -eq "run") {
	$serverArgs = "--run-once"
	CatchProcessErrors "bin/api-server.exe" $serverArgs
	CatchProcessErrors "bin/login-server.exe" $serverArgs
	CatchProcessErrors "bin/char-server.exe" $serverArgs
	CatchProcessErrors "bin/map-server.exe" $serverArgs
}
elseif ($args[0] -eq "test") {
	$serverArgs = "--run-once"
	foreach ($plugin in @("HPMHooking", "httpsample", "constdb2doc", "db2sql", "generate-translations", "mapcache", "script_mapquit")) {
		$serverArgs += " --load-plugin $plugin"
	}
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --load-script npc/dev/test.txt --load-script npc/dev/ci_test.txt"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --constdb2doc"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --db2sql"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --itemdb2sql"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --mobdb2sql"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --generate-translations"
	CatchProcessErrors "bin/map-server.exe" "$serverArgs --fix-md5"
}
