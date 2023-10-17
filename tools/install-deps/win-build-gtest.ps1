
param(
    [string]$tempPath,
    [string]$depsPath,
    [string]$gtest
)

Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell 5d11a4c6 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

if (-Not (Get-Command -Name "cmake" -ErrorAction SilentlyContinue)) {
    Write-Host "Perl command not found. Please install it and try again."
    exit 11
}

if (-Not (Get-Command -Name "ninja" -ErrorAction SilentlyContinue)) {
    Write-Host "NMake command not found. Please install it and try again."
    exit 12
}

Set-Location $tempPath/$gtest
cmake . -DBUILD_GMOCK=OFF
cmake --build . --config Release

New-Item -ItemType Directory -Path "$depsPath\googletest"
Copy-Item -Path "$tempPath\$gtest\lib\Release" "$depsPath\googletest\lib" -Recurse
Copy-Item -Path "$tempPath\$gtest\googletest\include" "$depsPath\googletest\include" -Recurse