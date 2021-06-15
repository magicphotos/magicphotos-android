SET PATH=%PATH%;C:\Qt\5.12.11\mingw73_64\bin;C:\Qt\Tools\mingw730_64\bin

lupdate -locations absolute ..\magicphotos.pro -ts ..\translations\magicphotos_de.src.ts

lupdate -locations absolute ..\magicphotos.pro -ts ..\translations\magicphotos_fr.src.ts

lupdate -locations absolute ..\magicphotos.pro -ts ..\translations\magicphotos_ru.src.ts

lupdate -locations absolute ..\magicphotos.pro -ts ..\translations\magicphotos_zh.src.ts

lconvert ..\translations\magicphotos_de.src.ts ..\translations\magicphotos_de.qt.ts -o ..\translations\magicphotos_de.ts
lconvert ..\translations\magicphotos_fr.src.ts ..\translations\magicphotos_fr.qt.ts -o ..\translations\magicphotos_fr.ts
lconvert ..\translations\magicphotos_ru.src.ts ..\translations\magicphotos_ru.qt.ts -o ..\translations\magicphotos_ru.ts
lconvert ..\translations\magicphotos_zh.src.ts ..\translations\magicphotos_zh.qt.ts -o ..\translations\magicphotos_zh.ts
