function dp(dpi, size) {
    if (dpi >= 640) {
        return Math.floor(size * 4);
    } else if (dpi >= 480) {
        return Math.floor(size * 3);
    } else if (dpi >= 320) {
        return Math.floor(size * 2);
    } else if (dpi >= 240) {
        return Math.floor(size * 1.5);
    } else {
        return Math.floor(size);
    }
}

function generateImageFileName(save_directory) {
    var current_date = new Date();

    return "%1/%2-%3-%4_%5-%6-%7.jpg".arg(save_directory)
                                     .arg(current_date.getFullYear())
                                     .arg(current_date.getMonth() + 1)
                                     .arg(current_date.getDate())
                                     .arg(current_date.getHours())
                                     .arg(current_date.getMinutes())
                                     .arg(current_date.getSeconds());
}
