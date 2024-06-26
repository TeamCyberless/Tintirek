
param(
    [string]$tempPath,
    [string]$depsPath,
    [string]$openssl
)

Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell 5d11a4c6 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

if (-Not (Get-Command -Name "perl" -ErrorAction SilentlyContinue)) {
    Write-Host "Perl command not found. Please install it and try again."
    exit 11
}

if (-Not (Get-Command -Name "nmake" -ErrorAction SilentlyContinue)) {
    Write-Host "NMake command not found. Please install it and try again."
    exit 12
}

if (-Not (Get-Command -Name "nasm" -ErrorAction SilentlyContinue)) {
    Write-Host "nasm command not found. Please install it and try again."
    exit 13
}

Set-Location $tempPath/$openssl
perl ./Configure shared --prefix=$depsPath/openssl --debug no-md2 no-rc4 no-idea no-camellia no-ec no-engine no-tests VC-WIN64A
nmake
nmake install