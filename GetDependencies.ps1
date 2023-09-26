$SQLITE_VERSION = "3.43.0"
$OPENSSL_VERSION = "1.1.1w"
# Used for SQLite download URL
$SQLITE_VERSION_RELEASE_YEAR = "2023"

$SQLITE_VERSION_LIST = $SQLITE_VERSION.Split(".")
$SQLITE = "sqlite-amalgamation-{0}{1:D2}{2:D2}{3:D2}" -f $SQLITE_VERSION_LIST[0], [int]$SQLITE_VERSION_LIST[1], [int]$SQLITE_VERSION_LIST[2], [int]$SQLITE_VERSION_LIST[3]
$OPENSSL = "openssl-$OPENSSL_VERSION"

$BASEDIR = Get-Location
$DEPDIR = Join-Path -Path $BASEDIR -ChildPath "deps"
$TEMPDIR = Join-Path -Path $BASEDIR -ChildPath "temp"

# usage helper
function usage {
    Write-Host "Usage: .\GetDependencies.ps1 [-help]"
    exit 1
}

# download helper
function Download-File {
    param(
        [string]$url,
        [string]$output
    )

    try {
        Invoke-WebRequest -Uri $url -OutFile $output
    } catch {
        Write-Host "Error: Download failed. Please ensure you have an internet connection and the provided URL is correct." -ForegroundColor Red
        exit 1
    }
}

# dependencies: SQLite
function Get-SQLite {
    if (Test-Path "$DEPDIR\sqlite-amalgamation") {
        Write-Host "SQLite folder already exists locally. The downloaded copy will not be used."
        return
    }

    Set-Location $TEMPDIR
    Download-File "https://www.sqlite.org/$SQLITE_VERSION_RELEASE_YEAR/$SQLITE.zip" "$TEMPDIR\$SQLITE.zip"
    Set-Location $BASEDIR

    Expand-Archive -Path "$TEMPDIR\$SQLITE.zip" -DestinationPath $TEMPDIR
    Move-Item -Path "$TEMPDIR\$SQLITE" -Destination "$DEPDIR\sqlite-amalgamation"
}

# dependencies: openssl
function Get-OpenSSL {
    if (Test-Path "$DEPDIR\openssl") {
        Write-Host "OpenSSL folder already exists locally. The downloaded copy will not be used."
        return
    }

    if (-Not (Get-Command -Name "perl" -ErrorAction SilentlyContinue)) {
        Write-Host "Perl command not found. Please install it and try again."
        return
    }

    if (-Not (Get-Command -Name "nmake" -ErrorAction SilentlyContinue)) {
        Write-Host "NMake command not found. Please install it and try again."
        return
    }

    if (-Not (Get-Command -Name "nasm" -ErrorAction SilentlyContinue)) {
        Write-Host "nasm command not found. Please install it and try again."
        return
    }

    Set-Location $TEMPDIR
    Download-File "https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz" "$TEMPDIR\$OPENSSL.tar.gz"
    tar -zxf "$TEMPDIR\$OPENSSL.tar.gz" --directory $TEMPDIR
    Set-Location $BASEDIR

    # Build OpenSSL and get files
    Set-Location "$TEMPDIR\$OPENSSL"

    Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    Enter-VsDevShell 5d11a4c6 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

    perl ./Configure no-shared --release --api=1.1.0 no-deprecated no-ssl2 no-ssl3 no-md2 no-rc4 no-idea no-camellia no-ec no-engine no-tests VC-WIN64A
    nmake

    New-Item -ItemType Directory -Path "$DEPDIR\openssl"
    Set-Location $DEPDIR
    Copy-Item -Path "$TEMPDIR\$OPENSSL\libcrypto.lib" -Destination "$DEPDIR\openssl\libcrypto.lib"
    Copy-Item -Path "$TEMPDIR\$OPENSSL\libssl.lib" -Destination "$DEPDIR\openssl\libssl.lib"
    Copy-Item -Path "$TEMPDIR\$OPENSSL\include" -Destination "$DEPDIR\openssl\include"
}

#####################################
#                                   #
#   Maybe we can add here Perl,     #
#   NASM and NMake installers too   #
#                                   #
#####################################

# main process
function Get-Dependencies {
    if ($args.Count -eq 1 -and $args[0] -eq "-help") {
        usage
        exit 0
    } elseif ($args[0] -ne "") {
        Write-Host "Error: Invalid argument. Use .\GetDependencies.ps1 -help for usage instructions." -ForegroundColor Red
        exit 1
    }

    if (!(Test-Path $TEMPDIR)) {
        New-Item -Path $TEMPDIR -ItemType Directory | Out-Null
    }

    if (!(Test-Path $DEPDIR)) {
        New-Item -Path $DEPDIR -ItemType Directory | Out-Null
    }

    Get-SQLite
    Get-OpenSSL
    Set-Location $BASEDIR
    Remove-Item -Path $TEMPDIR -Recurse -Force
}

# Start the script process
Get-Dependencies $args