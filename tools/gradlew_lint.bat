@ECHO OFF

SET JAVA_HOME=C:\Program Files (x86)\Java\jdk1.8.0_202
SET ANDROID_SDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk
SET ANDROID_NDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk\ndk\20.0.5594570
SET ANDROID_NDK_HOST=windows
SET ANDROID_PLATFORM=android-29

PATH=%PATH%;C:\Qt\5.12.8\android_armv7\bin;%ANDROID_NDK_ROOT%\prebuilt\windows\bin

    CD .. ^
 && (IF NOT EXIST .gradlewbuild (MKDIR .gradlewbuild && ATTRIB +H .gradlewbuild)) ^
 && CD .gradlewbuild ^
 && (IF EXIST android-build RMDIR /S /Q android-build) ^
 && qmake.exe ..\magicphotos.pro ^
 && make.exe clean ^
 && make.exe -j%NUMBER_OF_PROCESSORS% all ^
 && make.exe install INSTALL_ROOT=android-build ^
 && androiddeployqt.exe --input android-libmagicphotos.so-deployment-settings.json --output android-build --android-platform %ANDROID_PLATFORM% --deployment bundled --gradle --no-gdbserver ^
 && CD android-build ^
 && gradlew.bat lint
