try {
Remove-Item -Recurse -Force "x64\Release"
} catch {}

.\prebuild.ps1

$env:_CL_="/MD"
msbuild.exe GlosSI.sln /t:Build /p:Configuration=Release /p:Platform=x64

cd ./SteamTweaks
npm i
npm run build
cd ..

New-Item -ItemType Directory -Force -Path "x64\Release"
cd ./x64/Release/

$env:Path += ';..\..\Qt\6.3.1\msvc2019_64\bin'
# $env:Path += ';C:\Program Files\7-Zip'

Get-ChildItem -Rec | Where {$_.Extension -match "lib"} | Remove-Item
$env:Path = "$env:QTDIR\bin;$env:Path"

windeployqt.exe --release --qmldir ../../GlosSIConfig/qml ./GlosSIConfig.exe

Get-ChildItem -Path "..\..\..\" -Recurse -Include "ViGEmBusSetup_x64.exe","HidHideSetup.exe","vc_redist.x64.exe"

# Copy-Item "..\..\deps\SFML\out\Release\lib\sfml-graphics-2.dll" -Destination "."
# Copy-Item "..\..\deps\SFML\out\Release\lib\sfml-system-2.dll" -Destination "."
# Copy-Item "..\..\deps\SFML\out\Release\lib\sfml-window-2.dll" -Destination "."
# Copy-Item "..\..\GlosSIConfig\GetAUMIDs.ps1" -Destination "."
# Copy-Item "..\..\HidHideSetup.exe" -Destination "."
# Copy-Item "..\..\ViGEmBusSetup_x64.exe" -Destination "."
# Copy-Item "..\..\vc_redist.x64.exe" -Destination "."
# Copy-Item "..\..\LICENSE" -Destination "./LICENSE"
# Copy-Item "..\..\QT_License" -Destination "./QT_License"
# Copy-Item "..\..\THIRD_PARTY_LICENSES.txt" -Destination "./THIRD_PARTY_LICENSES.txt"
# Copy-Item "..\..\SteamTweaks\dist" -Destination "./SteamTweaks" -Recurse
#
#
# # 7z a GlosSI-snapshot.zip *
#
# cd ../..

