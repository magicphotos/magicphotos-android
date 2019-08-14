@ECHO OFF

SET JAVA_HOME=C:\Program Files (x86)\Java\jdk1.8.0_202
SET ANDROID_SDK_ROOT=D:\Users\%USERNAME%\Documents\Android\android-sdk
SET ANDROID_NDK_ROOT=D:\Users\%USERNAME%\Documents\Android\android-ndk-r19c
SET SONARCLOUD_ROOT=D:\Users\%USERNAME%\Documents\SonarCloud
SET ANDROID_PLATFORM=android-28

PATH=%PATH%;C:\Qt\5.12.4\android_armv7\bin;%ANDROID_NDK_ROOT%\prebuilt\windows-x86_64\bin;%SONARCLOUD_ROOT%\build-wrapper-win-x86;%SONARCLOUD_ROOT%\sonar-scanner-3.3.0.1492-windows\bin

IF NOT "%~1"=="" (
        CD .. ^
     && (IF NOT EXIST .sonarbuild (MKDIR .sonarbuild && ATTRIB +H .sonarbuild) || CD .) ^
     && CD .sonarbuild ^
     && qmake.exe ..\magicphotos.pro ^
     && build-wrapper-win-x86-64.exe --out-dir bw-output make.exe clean all install INSTALL_ROOT=android-build ^
     && androiddeployqt.exe --input android-libmagicphotos.so-deployment-settings.json --output android-build --android-platform %ANDROID_PLATFORM% --deployment bundled --gradle --no-gdbserver ^
     && CD .. ^
     && sonar-scanner.bat -D"sonar.projectKey=magicphotos:magicphotos-android" ^
                          -D"sonar.projectName=MagicPhotos Android" ^
                          -D"sonar.organization=magicphotos-github" ^
                          -D"sonar.sources=." ^
                          -D"sonar.sourceEncoding=UTF-8" ^
                          -D"sonar.exclusions=qml/**/*,translations/*" ^
                          -D"sonar.cfamily.build-wrapper-output=.sonarbuild\\bw-output" ^
                          -D"sonar.java.source=1.7" ^
                          -D"sonar.java.binaries=.sonarbuild" ^
                          -D"sonar.host.url=https://sonarcloud.io" ^
                          -D"sonar.login=%~1"
) ELSE (
    ECHO Syntax: sonarcloud.bat SONARCLOUD_KEY
)
