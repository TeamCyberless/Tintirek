$SQLITE_VERSION = "3.43.0"
# Used for SQLite download URL
$SQLITE_VERSION_RELEASE_YEAR = "2023"

$SQLITE_VERSION_LIST = $SQLITE_VERSION.Split(".")
$SQLITE = "sqlite-amalgamation-{0}{1:D2}{2:D2}{3:D2}" -f $SQLITE_VERSION_LIST[0], [int]$SQLITE_VERSION_LIST[1], [int]$SQLITE_VERSION_LIST[2], [int]$SQLITE_VERSION_LIST[3]

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
    if (Test-Path "$DEPDIR\sqlite-amalgamation-3430000") {
        Write-Host "SQLite folder already exists locally. The downloaded copy will not be used."
        return
    }

    Set-Location $TEMPDIR
    Download-File "https://www.sqlite.org/$SQLITE_VERSION_RELEASE_YEAR/$SQLITE.zip" "$TEMPDIR\$SQLITE.zip"
    Set-Location $BASEDIR

    Expand-Archive -Path "$TEMPDIR\$SQLITE.zip" -DestinationPath $TEMPDIR
    Move-Item -Path "$TEMPDIR\$SQLITE" -Destination "$DEPDIR\sqlite-amalgamation"
}

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
    Remove-Item -Path $TEMPDIR -Recurse -Force
}

# Start the script process
Get-Dependencies $args