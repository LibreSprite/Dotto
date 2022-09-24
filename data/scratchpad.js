// Dotto JS Scratchpad - Press Esc again to hide, Ctrl+Enter to run

var image = app.activeCell.composite;
for (var y = 0; y < image.height; ++y) {
    for (var x = 0; x < image.width; ++x) {
        image.setPixel(x, y, Math.random() * 0xFFFFFF | 0xFF000000);
    }
}
