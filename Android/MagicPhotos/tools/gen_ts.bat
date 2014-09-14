PATH=%PATH%;C:\Qt\5.3\mingw482_32\bin

lupdate ..\MagicPhotos.pro -ts ..\translations\MagicPhotos_ru_RU.src.ts
lupdate ..\qml             -ts ..\translations\MagicPhotos_ru_RU.qml.ts

lconvert ..\translations\MagicPhotos_ru_RU.src.ts ..\translations\MagicPhotos_ru_RU.qml.ts -o ..\translations\MagicPhotos_ru_RU.ts
