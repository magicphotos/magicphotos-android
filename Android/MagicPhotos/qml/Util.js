function mapSizeToDevice(screen_dpi, size) {
    if (screen_dpi >= 640) {
        return Math.floor(size * 4);
    } else if (screen_dpi >= 480) {
        return Math.floor(size * 3);
    } else if (screen_dpi >= 320) {
        return Math.floor(size * 2);
    } else if (screen_dpi >= 240) {
        return Math.floor(size * 1.5);
    } else {
        return Math.floor(size);
    }
}
