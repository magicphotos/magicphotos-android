@ECHO OFF

SET JAVA_HOME=C:\Program Files\Java\jdk1.8.0_251
SET ANDROID_SDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk
SET ANDROID_NDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk\ndk\21.0.6113669
SET ANDROID_NDK_HOST=windows-x86_64
SET ANDROID_PLATFORM=android-29

SET PATH=%PATH%;C:\Qt\5.12.10\android_armv7\bin;%ANDROID_NDK_ROOT%\prebuilt\%ANDROID_NDK_HOST%\bin

    CD .. ^
 && (IF NOT EXIST .gradlewbuild (MKDIR .gradlewbuild && ATTRIB +H .gradlewbuild)) ^
 && CD .gradlewbuild ^
 && (IF EXIST android-build RMDIR /S /Q android-build) ^
 && qmake.exe ..\magicphotos.pro ^
 && make.exe clean ^
 && make.exe -j%NUMBER_OF_PROCESSORS% all ^
 && make.exe install INSTALL_ROOT=android-build ^
 && androiddeployqt.exe --input android-libmagicphotos.so-deployment-settings.json --output android-build --android-platform "%ANDROID_PLATFORM%" --deployment bundled --gradle --no-gdbserver ^
 && CD android-build ^
 && gradlew.bat lint
