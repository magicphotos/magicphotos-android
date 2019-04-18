@ECHO OFF

SET ANDROID_NDK_ROOT=D:\Users\%USERNAME%\Documents\Android\android-ndk-r18b
SET SONARCLOUD_ROOT=D:\Users\%USERNAME%\Documents\SonarCloud

PATH=%PATH%;C:\Qt\5.12.2\android_armv7\bin;%ANDROID_NDK_ROOT%\prebuilt\windows\bin;%SONARCLOUD_ROOT%\build-wrapper-win-x86;%SONARCLOUD_ROOT%\sonar-scanner-3.3.0.1492-windows\bin

IF NOT "%~1"=="" (
        CD .. ^
     && (IF NOT EXIST .sonarbuild (MKDIR .sonarbuild && ATTRIB +H .sonarbuild) || CD .) ^
     && CD .sonarbuild ^
     && qmake.exe ..\magicphotos.pro ^
     && build-wrapper-win-x86-64.exe --out-dir bw-output make.exe clean all ^
     && CD .. ^
     && sonar-scanner.bat -D"sonar.projectKey=magicphotos:magicphotos-android" ^
                          -D"sonar.projectName=MagicPhotos Android" ^
                          -D"sonar.projectVersion=master" ^
                          -D"sonar.organization=magicphotos-github" ^
                          -D"sonar.sources=src" ^
                          -D"sonar.sourceEncoding=UTF-8" ^
                          -D"sonar.cfamily.build-wrapper-output=.sonarbuild\\bw-output" ^
                          -D"sonar.host.url=https://sonarcloud.io" ^
                          -D"sonar.login=%~1"
) ELSE (
    ECHO Syntax: sonarcloud.bat SONARCLOUD_KEY
)
