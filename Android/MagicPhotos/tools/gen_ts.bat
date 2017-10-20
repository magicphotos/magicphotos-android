PATH=%PATH%;C:\Qt\5.9.2\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin

lupdate ..\MagicPhotos.pro -ts ..\translations\MagicPhotos_ru.src.ts
lupdate ..\qml             -ts ..\translations\MagicPhotos_ru.qml.ts

lupdate ..\MagicPhotos.pro -ts ..\translations\MagicPhotos_de.src.ts
lupdate ..\qml             -ts ..\translations\MagicPhotos_de.qml.ts

lupdate ..\MagicPhotos.pro -ts ..\translations\MagicPhotos_fr.src.ts
lupdate ..\qml             -ts ..\translations\MagicPhotos_fr.qml.ts

lconvert ..\translations\MagicPhotos_ru.src.ts ..\translations\MagicPhotos_ru.qml.ts -o ..\translations\MagicPhotos_ru.ts
lconvert ..\translations\MagicPhotos_de.src.ts ..\translations\MagicPhotos_de.qml.ts -o ..\translations\MagicPhotos_de.ts
lconvert ..\translations\MagicPhotos_fr.src.ts ..\translations\MagicPhotos_fr.qml.ts -o ..\translations\MagicPhotos_fr.ts
