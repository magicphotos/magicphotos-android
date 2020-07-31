@ECHO OFF

SET PATH=%PATH%;C:\Qt\5.12.9\mingw73_64\bin;C:\Qt\Tools\mingw730_64\bin

FOR /R ..\qml %%F IN (*.qml) DO (
    qmllint "%%F"
)
