function mapSizeToDevice(pixel_density, size) {
    var device_dpi = pixel_density * 25.4;

    if (device_dpi > (640 - 32)) {
        return Math.floor(size * 4);
    } else if (device_dpi > (480 - 32)) {
        return Math.floor(size * 3);
    } else if (device_dpi > (320 - 16)) {
        return Math.floor(size * 2);
    } else if (device_dpi > (240 - 16)) {
        return Math.floor(size * 1.5);
    } else {
        return Math.floor(size);
    }
}
