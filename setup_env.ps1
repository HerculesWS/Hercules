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

<#
.SYNOPSIS
  Set up build environment for Hercules
.DESCRIPTION
  This script sets up the build environment for Hercules:
    - If the Visual Studio environment is not yet configured, it will be auto-detected and configured.
      - The CPU type will be autodetected and the most appropriate target architecture will be selected.
      - The latest VS version installed will be auto-detected unless the VCINSTALLDIR or VSINSTALLDIR environment variables are already configured.
    - If conan is not installed, it will be configured in a local venv and activated.
    - If a CMake configuration is not detected in the build directory, a new default configuration will be created.
      - Ninja will be configured as build tool.
      - The RelWithDebInfo mode will be used unless otherwise specified.
      - The build output will be configured into the "build" directory unless otherwise specified.

  This script can be safely reused with the same arguments to re-enter a configured build by setting up the appropriate environment variables.

  Note: this script should be executed from a PowerShell terminal window and not from Windows Explorer.
#>

Param(
  # Specify a different build directory name (default: "build")
  [String] $BuildDir = "build",
  # Specify a different build type (one of "Debug", "Release", "RelWithDebInfo", default: "RelWithDebInfo")
  [String] $BuildType = "RelWithDebInfo",
  # Override the auto-detected CPU architecture (one of "auto", "x64", "x86", "x64_x86", "x86_x64", "x64_arm", "x64_arm64", "x86_arm", "x86_arm64", "arm64", "arm64_x64", "arm64_x86", "arm64_arm")
  [String] $Arch = "auto"
)

if ($PSVersionTable.PSVersion.Major -lt 3) {
  Throw "This script requires at least PowerShell version 3.0"
}

# differentiate between right click -> run with powershell and execution from within a shell
$exe_path = $(gwmi win32_process -Filter "processid='$PID'").ExecutablePath
$cmd_line = $(gwmi win32_process -Filter "processid='$PID'").CommandLine
$script_path = $MyInvocation.MyCommand.Path
if ($cmd_line -match "`"$([regex]::escape($exe_path)).*$([regex]::escape($script_path))\s*`"") {
  $is_cli = $False
} else {
  $is_cli = $True
}
if (!$is_cli) {
  Write-Host "NOTE:`n"
  Write-Host "The purpose of this script is to set up the current shell with all the required variables to build Hercules."
  Write-Host "As such it should be executed from a PowerShell window. It's meaningless to execute it from the Explorer GUI."
  Write-Host -nonewline "`n`nPress any key or close this window to cancel...";
  $x = [console]::ReadKey($true);
}

$herc_path = $PSScriptRoot
if (-not (Test-Path "${herc_path}/CMakeLists.txt") -or -not (Test-Path "${herc_path}/run-server.bat")) {
  Throw "Hercules path not found. The script must be in the Hercules repository root"
}

if (-not ($Arch -in ("auto", "x64", "x86", "x64_x86", "x86_x64", "x64_arm", "x64_arm64", "x86_arm", "x86_arm64", "arm64", "arm64_x64", "arm64_x86", "arm64_arm"))) {
  Throw "Unknown Arch specified ($Arch)"
}
if (-not ($BuildType -in ("Release", "Debug", "RelWithDebInfo"))) {
  Throw "Unknown BuildType specified ($BuildType)"
}

# Visual Studio environment detection and setup

if ("${env:VSCMD_ARG_TGT_ARCH}" -ne "" -and "${env:VSCMD_ARG_HOST_ARCH}" -ne "" -and "${env:VcToolsVersion}" -ne "") {
  Write-Host "Detected VS environment already configured. Skipping VS detection."
  if ($Arch -ne "auto" -and $Arch -ne "${env:VSCMD_ARG_TGT_ARCH}" -and $Arch -ne "${env:VSCMD_ARG_HOST_ARCH}_${env:VSCMD_ARG_TGT_ARCH}") {
    Throw "The architecture of the detected VS environment ($(if ("${env:VSCMD_ARG_HOST_ARCH}" -eq "${env:VSCMD_ARG_TGT_ARCH}") { "${env:VSCMD_ARG_HOST_ARCH}" } else { "${env:VSCMD_ARG_HOST_ARCH}_${env:VSCMD_ARG_TGT_ARCH}" })) doesn't match the requested one (${Arch})"
  }
} else {
  Write-Host "Detecting VS installation..."
  $vcvars_path = ""
  if ("${env:VCINSTALLDIR}" -ne "" -and (Test-Path -Path "${env:VCINSTALLDIR}/Auxiliary/Build/vcvarsall.bat")) {
    # VC installation path (normally ${VS installation path}/VC)
    $vcvars_path = "${env:VCINSTALLDIR}/Auxiliary/Build"
  } elseif ("${env:VSINSTALLDIR}" -ne "" -and (Test-Path -Path "${env:VSINSTALLDIR}/VC/Auxiliary/Build/vcvarsall.bat")) {
    # VS installation path
    $vcvars_path = "${env:VSINSTALLDIR}/VC/Auxiliary/Build"
  } elseif (Test-Path -Path "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe") {
    # VS detector script targeting the latest version (including BuildTools only installations)
    $vs_path = (& "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe" -products * -latest -property installationPath)
    if ($vs_path -ne "" -and (Test-Path -Path "${vs_path}/VC/Auxiliary/Build/vcvarsall.bat")) {
      $vcvars_path = "${vs_path}/VC/Auxiliary/Build"
    } else {
      $vs_path = ""
    }
  }
  if ($vcvars_path -ne "") {
    Write-Host "Found vcvarsall.bat script: $vcvars_path"
  } else {
    Throw "Visual Studio installation not detected. Set the `$env:VSINSTALLDIR variable to your VS installation's directory"
  }

  Push-Location -Path "${vcvars_path}"
  if ($Arch -eq "auto") {
    if ("${env:VSCMD_ARG_HOST_ARCH}" -ne "") {
      if ("${env:VSCMD_ARG_TGT_ARCH}" -eq "" -or "${env:VSCMD_ARG_HOST_ARCH}" -eq "${env:VSCMD_ARG_TGT_ARCH}") {
        $Arch = "${env:VSCMD_ARG_HOST_ARCH}"
      } else {
        $Arch = "${env:VSCMD_ARG_HOST_ARCH}_${env:VSCMD_ARG_TGT_ARCH}"
      }
    } else {
      $Arch = [System.Runtime.InteropServices.RuntimeInformation,mscorlib]::OSArchitecture.ToString().ToLower()
    }
  }
  Write-Host "Setting up VS environment (architecture: ${Arch})"
  cmd.exe /c "vcvarsall.bat ${Arch} & set" |
  Foreach-Object {
      if ($_ -match "=") {
          $v = $_.split("="); Set-Item -Force -Path "ENV:\$($v[0])" -Value "$($v[1])"
      }
  }
  Pop-Location
}
if (-not ("${env:VSCMD_ARG_TGT_ARCH}" -in ("x86", "x64", "arm64", "arm"))) {
  Throw "VS environment configuration failed."
}
Write-Host "VS environment configured (Host: ${env:VSCMD_ARG_HOST_ARCH}, Target: ${env:VSCMD_ARG_TGT_ARCH}, Version: ${env:VCToolsVersion})"

# Conan detection and setup

Write-Host "Detecting conan 2 installation..."

if ((Get-Command conan -errorAction SilentlyContinue) -and ("$(conan --version)".replace("Conan version ", "") -ge 2)) {
  Write-Host "Detected conan 2 in the current environment. Venv setup will not be necessary."
} elseif ((Test-Path "${herc_path}/.venv/Scripts/conan.exe") -and ($(& "${herc_path}/.venv/Scripts/conan.exe").replace("Conan version ", "") -ge 2)) {
  Write-Host "Detected conan 2 installed in venv. The venv will be activated."
  & "${herc_path}/.venv/Scripts/activate.ps1"
} elseif ((Test-Path "${herc_path}/.venv/Scripts/activate.ps1") -and (Test-Path "${herc_path}/.venv/Scripts/pip.exe")) {
  Write-Host "Detected local venv. Installing conan 2 in it."
  & "${herc_path}/.venv/Scripts/pip.exe" install conan
  & "${herc_path}/.venv/Scripts/activate.ps1"
} elseif (Test-Path "${herc_path}/.venv") {
  Throw "Detected invalid local venv in `"${herc_path}/.venv`". Setup cannot proceed."
} elseif (Get-Command python3 -errorAction SilentlyContinue) {
  Write-Host "Setting up venv for conan 2"
  & python3 -m venv "${herc_path}/.venv"
  & "${herc_path}/.venv/Scripts/pip.exe" install conan
  & "${herc_path}/.venv/Scripts/activate.ps1"
} else {
  Throw "Python3 not found. Setup of conan cannot proceed."
}

if (-not (Get-Command conan -errorAction SilentlyContinue) -or -not ("$(conan --version)".replace("Conan version ", "") -ge 2)) {
  Throw "Conan 2 installation or venv setup failed."
}
Write-Host "Using conan from $((Get-Command conan -errorAction SilentlyContinue).Source) (version: $("$(conan --version)".replace('Conan version ', '')))"

# CMake detection and setup
if (Test-Path "${BuildDir}/CMakeCache.txt") {
  Write-Host "Detected CMake environment already configured in ${BuildDir}."
} else {
  if (-not (Get-Command cmake -errorAction SilentlyContinue)) {
    Throw "CMake installation not detected. Configuration cannot proceed."
  }
  if (-not (Get-Command ninja -errorAction SilentlyContinue)) {
    Throw "Ninja installation not detected. Configuration cannot proceed."
  }
  Write-Host "Setting up environment in ${BuildDir} using CMake from $((Get-Command cmake -errorAction SilentlyContinue).Source) and ninja from $((Get-Command ninja -errorAction SilentlyContinue).Source)"
  if (-not (Test-Path "${BuildDir}")) {
    New-Item -Path "${BuildDir}" -ItemType Directory
  }
  & cmake -S "${herc_path}" -B "${BuildDir}" -G Ninja "-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake_modules/conan_provider.cmake" "-DCMAKE_BUILD_TYPE=${BuildType}" -DENABLE_TESTING=ON
  if (-not $? -or -not (Test-Path "${BuildDir}/CMakeCache.txt")) {
    Throw "Failed to generate build environment in ${BuildDir}"
  }
}
Write-Host "`n`n"
Write-Host "Build environment ready in ${BuildDir}"
