PATH=%PATH%;C:\Qt\5.3\mingw482_32\bin

lupdate ..\MagicPhotos.pro -ts ..\translations\MagicPhotos_ru.src.ts
lupdate ..\qml             -ts ..\translations\MagicPhotos_ru.qml.ts

lconvert ..\translations\MagicPhotos_ru.src.ts ..\translations\MagicPhotos_ru.qml.ts -o ..\translations\MagicPhotos_ru.ts
