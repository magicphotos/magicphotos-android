function getSetting(key, defaultValue) {
    var value = defaultValue;
    var db    = openDatabaseSync("MagicPhotosDB", "1.0", "MagicPhotosDB", 1000000);

    db.transaction(
                function(tx) {
                    tx.executeSql("CREATE TABLE IF NOT EXISTS SETTINGS(KEY TEXT PRIMARY KEY, VALUE TEXT)");

                    var res = tx.executeSql("SELECT VALUE FROM SETTINGS WHERE KEY=?", [key]);

                    if (res.rows.length !== 0) {
                        value = res.rows.item(0).VALUE;
                    }
                }
    );

    return value;
}

function setSetting(key, value) {
    var db = openDatabaseSync("MagicPhotosDB", "1.0", "MagicPhotosDB", 1000000);

    db.transaction(
                function(tx) {
                    tx.executeSql("REPLACE INTO SETTINGS (KEY, VALUE) VALUES (?, ?)", [key, value]);
                }
    );
}
