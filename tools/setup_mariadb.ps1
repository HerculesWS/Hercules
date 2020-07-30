#Requires -Version 5.1

function Ask-Continue { Write-Output ""; pause; Write-Output "" }

Write-Output "This script will automatically install MariaDB and configure it for you."
Write-Output "You may interrupt the installation by pressing CTRL+C or closing this window."
Ask-Continue

if (-Not (Select-String -Quiet -SimpleMatch -Pattern "db_password: ""ragnarok""" -LiteralPath "$PSScriptRoot\..\conf\global\sql_connection.conf")) {
    Write-Output "WARNING: It seems you already configured the sql connection for your server."
    Write-Output "If you decide to continue, your settings will be overwritten."
    Ask-Continue
}

# step 1: install scoop
if (-Not (Get-Command scoop -errorAction SilentlyContinue)) {
    Set-ExecutionPolicy RemoteSigned -scope Process -Force # <= this will trigger a yes/no prompt if not already authorized
    Invoke-Expression (new-object net.webclient).downloadstring('https://get.scoop.sh')
    scoop update
}

# step 2: install mariadb
if (Test-Path $env:USERPROFILE\scoop\apps\mariadb) {
    # usually we'd want to capture the output of "scoop list mariadb", but it uses
    # Write-Host, so we can't, hence why we check manually for the folder
    Write-Output "WARNING: MariaDB is already installed!"
    Write-Output "If you decide to continue, your hercules user password will be overwritten."
    Ask-Continue
} elseif (Get-Command mysqld -errorAction SilentlyContinue) {
    Write-Output "ERROR: You already have a MySQL provider installed. To avoid conflict, MariaDB will not be installed."
    Write-Output "If you wish to continue you will have to uninstall your current MySQL provider."
    exit 1
} else {
    scoop install mariadb
}

# step 3: add the herc user, set up the new database
$userpw = -join ((48..57) + (97..122) | Get-Random -Count 32 | % {[char]$_})
$rootpw = -join ((48..57) + (97..122) | Get-Random -Count 32 | % {[char]$_})
$maria_job = Start-Process -NoNewWindow -FilePath "mysqld.exe" -ArgumentList "--console" -PassThru -RedirectStandardError "$PSScriptRoot\maria.out"

while (-Not $maria_job.HasExited) {
    if ($lt -Lt 1 -And (Select-String -Quiet -SimpleMatch -Pattern "ready for connections" -LiteralPath "$PSScriptRoot\maria.out")) {
@"
CREATE DATABASE IF NOT EXISTS hercules;
DROP USER IF EXISTS 'hercules'@'localhost';
DROP USER IF EXISTS 'hercules'@'127.0.0.1';
CREATE USER 'hercules'@'localhost' IDENTIFIED BY '$userpw';
CREATE USER 'hercules'@'127.0.0.1' IDENTIFIED BY '$userpw';
-- ALTER USER 'root'@'localhost' IDENTIFIED BY '$rootpw';
GRANT ALTER,CREATE,SELECT,INSERT,UPDATE,DELETE,DROP,INDEX ON `hercules`.* TO 'hercules'@'localhost';
GRANT ALTER,CREATE,SELECT,INSERT,UPDATE,DELETE,DROP,INDEX ON `hercules`.* TO 'hercules'@'127.0.0.1';
FLUSH PRIVILEGES;
USE `hercules`;
\. $PSScriptRoot\..\sql-files\main.sql
\. $PSScriptRoot\..\sql-files\logs.sql
shutdown;
\q
"@ | mysql.exe -u root
        $lt++
    }
    Start-Sleep 1
}

if ($lt -Lt 1) {
    Write-Output "ERROR: MariaDB could not execute the query."
    Write-Output "This might happen if your root user already has a password, or if the MySQL service is currently running."
    $maria_job.close()
    exit 1
}

# step 4: finish up
@"
sql_connection: {
    db_username: "hercules"
    db_password: "$userpw"
    db_database: "hercules"
}
"@ | Out-File -Encoding UTF8 -LiteralPath "$PSScriptRoot\..\conf\global\sql_connection.conf"
Remove-Item -Force -errorAction SilentlyContinue "$PSScriptRoot\maria.out"
& "$PSScriptRoot\install_mariadb.bat" # <= we need admin permissions, so we use an external script
Write-Output "========= ALL DONE ========="
Write-Output ""
Write-Output "Your hercules installation is now configured to use MariaDB."
Write-Output "You can find the password in conf\global\sql_connection.conf."
Write-Output ""
Write-Output "If you want to start MariaDB on boot, use services.msc and set ""MySQL"" to Automatic."
Write-Output ""
Write-Output "Make sure you set a password for the root user. You can do this from the command line or from HeidiSQL."
Write-Output "You can obtain HeidiSQL at https://www.microsoft.com/store/productId/9NXPRT2T0ZJF"
