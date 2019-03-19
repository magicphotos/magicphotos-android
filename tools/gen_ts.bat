PATH=%PATH%;C:\Qt\5.12.2\mingw73_64\bin;C:\Qt\Tools\mingw730_64\bin

lupdate ..\magicphotos.pro -ts ..\translations\magicphotos_ru.src.ts
lupdate ..\qml             -ts ..\translations\magicphotos_ru.qml.ts

lupdate ..\magicphotos.pro -ts ..\translations\magicphotos_de.src.ts
lupdate ..\qml             -ts ..\translations\magicphotos_de.qml.ts

lupdate ..\magicphotos.pro -ts ..\translations\magicphotos_fr.src.ts
lupdate ..\qml             -ts ..\translations\magicphotos_fr.qml.ts

lconvert ..\translations\magicphotos_ru.src.ts ..\translations\magicphotos_ru.qml.ts ..\translations\magicphotos_ru.qt.ts -o ..\translations\magicphotos_ru.ts
lconvert ..\translations\magicphotos_de.src.ts ..\translations\magicphotos_de.qml.ts ..\translations\magicphotos_de.qt.ts -o ..\translations\magicphotos_de.ts
lconvert ..\translations\magicphotos_fr.src.ts ..\translations\magicphotos_fr.qml.ts ..\translations\magicphotos_fr.qt.ts -o ..\translations\magicphotos_fr.ts
