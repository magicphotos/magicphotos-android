#include <memory>

#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "cartooneditor.h"

CartoonEditor::CartoonEditor(QQuickItem *parent) :
    EffectEditor(parent),
    Radius      (0),
    Threshold   (0)
{
}

int CartoonEditor::radius() const
{
    return Radius;
}

void CartoonEditor::setRadius(int radius)
{
    Radius = radius;
}

int CartoonEditor::threshold() const
{
    return Threshold;
}

void CartoonEditor::setThreshold(int threshold)
{
    Threshold = threshold;
}

void CartoonEditor::ProcessOpenedImage()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<CartoonImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                  generator.get(), &CartoonImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                 thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &CartoonImageGenerator::imageReady, this,            &CartoonEditor::setEffectedImage);
    connect(generator.get(), &CartoonImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &CartoonImageGenerator::finished,   generator.get(), &CartoonImageGenerator::deleteLater);

    generator->SetRadius(Radius);
    generator->SetThreshold(Threshold);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());
}

CartoonPreviewGenerator::CartoonPreviewGenerator(QQuickItem *parent) :
    PreviewGenerator(parent),
    Radius          (0),
    Threshold       (0)
{
}

int CartoonPreviewGenerator::radius() const
{
    return Radius;
}

void CartoonPreviewGenerator::setRadius(int radius)
{
    Radius = radius;

    if (!LoadedImage.isNull()) {
        if (ImageGeneratorRunning) {
            RestartImageGenerator = true;
        } else {
            StartImageGenerator();
        }
    }
}

int CartoonPreviewGenerator::threshold() const
{
    return Threshold;
}

void CartoonPreviewGenerator::setThreshold(int threshold)
{
    Threshold = threshold;

    if (!LoadedImage.isNull()) {
        if (ImageGeneratorRunning) {
            RestartImageGenerator = true;
        } else {
            StartImageGenerator();
        }
    }
}

void CartoonPreviewGenerator::StartImageGenerator()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<CartoonImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                  generator.get(), &CartoonImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                 thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &CartoonImageGenerator::imageReady, this,            &CartoonPreviewGenerator::setEffectedImage);
    connect(generator.get(), &CartoonImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &CartoonImageGenerator::finished,   generator.get(), &CartoonImageGenerator::deleteLater);

    generator->SetRadius(Radius);
    generator->SetThreshold(Threshold);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());

    ImageGeneratorRunning = true;

    emit generationStarted();
}

CartoonImageGenerator::CartoonImageGenerator(QObject *parent) :
    QObject  (parent),
    Radius   (0),
    Threshold(0)
{
}

void CartoonImageGenerator::SetRadius(int radius)
{
    Radius = radius;
}

void CartoonImageGenerator::SetThreshold(int threshold)
{
    Threshold = threshold;
}

void CartoonImageGenerator::SetInput(const QImage &input_image)
{
    InputImage = input_image;
}

void CartoonImageGenerator::start()
{
    QImage blur_image    = InputImage;
    QImage cartoon_image = InputImage;

    // Make Gaussian blur of original image, if applicable

    if (Radius != 0) {
        QImage::Format format = blur_image.format();

        blur_image = blur_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        constexpr int tab[] = {14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2};

        int alpha = Radius < 1 ? 16 : (Radius > 17 ? 1 : tab[Radius - 1]);

        int r1 = blur_image.rect().top();
        int r2 = blur_image.rect().bottom();
        int c1 = blur_image.rect().left();
        int c2 = blur_image.rect().right();

        int bpl = blur_image.bytesPerLine();

        int            rgba[4];
        unsigned char *p;

        for (int col = c1; col <= c2; col++) {
            p = blur_image.scanLine(r1) + col * 4;

            for (int i = 0; i < 4; i++) {
                rgba[i] = p[i] << 4;
            }

            p += bpl;

            for (int j = r1; j < r2; j++, p += bpl) {
                for (int i = 0; i < 4; i++) {
                    p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
                }
            }
        }

        for (int row = r1; row <= r2; row++) {
            p = blur_image.scanLine(row) + c1 * 4;

            for (int i = 0; i < 4; i++) {
                rgba[i] = p[i] << 4;
            }

            p += 4;

            for (int j = c1; j < c2; j++, p += 4) {
                for (int i = 0; i < 4; i++) {
                    p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
                }
            }
        }

        for (int col = c1; col <= c2; col++) {
            p = blur_image.scanLine(r2) + col * 4;

            for (int i = 0; i < 4; i++) {
                rgba[i] = p[i] << 4;
            }

            p -= bpl;

            for (int j = r1; j < r2; j++, p -= bpl) {
                for (int i = 0; i < 4; i++) {
                    p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
                }
            }
        }

        for (int row = r1; row <= r2; row++) {
            p = blur_image.scanLine(row) + c2 * 4;

            for (int i = 0; i < 4; i++) {
                rgba[i] = p[i] << 4;
            }

            p -= 4;

            for (int j = c1; j < c2; j++, p -= 4) {
                for (int i = 0; i < 4; i++) {
                    p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
                }
            }
        }

        blur_image = blur_image.convertToFormat(format);
    }

    // Apply Cartoon filter

    QVector src_buf(blur_image.width() * blur_image.height() * 4, 0);
    QVector dst_buf(blur_image.width() * blur_image.height() * 4, 0);

    for (int y = 0; y < blur_image.height(); y++) {
        for (int x = 0; x < blur_image.width(); x++) {
            QRgb color = blur_image.pixel(x, y);

            src_buf[(y * blur_image.width() + x) * 4]     = qBlue(color);
            src_buf[(y * blur_image.width() + x) * 4 + 1] = qGreen(color);
            src_buf[(y * blur_image.width() + x) * 4 + 2] = qRed(color);
            src_buf[(y * blur_image.width() + x) * 4 + 3] = qAlpha(color);
        }
    }

    int  offset;
    int  blue_g, green_g, red_g;
    int  blue, green, red;
    bool exceeds_threshold;

    for (int y = 1; y < blur_image.height() - 1; y++) {
        for (int x = 1; x < blur_image.width() - 1; x++) {
            offset = y * blur_image.width() * 4 + x * 4;

            blue_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            blue_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            offset++;

            green_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            green_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            offset++;

            red_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            red_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            if (blue_g + green_g + red_g > Threshold) {
                exceeds_threshold = true;
            } else {
                offset -= 2;

                blue_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                offset++;

                green_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                offset++;

                red_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                if (blue_g + green_g + red_g > Threshold) {
                    exceeds_threshold = true;
                } else {
                    offset -= 2;

                    blue_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    offset++;

                    green_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    offset++;

                    red_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    if (blue_g + green_g + red_g > Threshold) {
                        exceeds_threshold = true;
                    } else {
                        offset -= 2;

                        blue_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        blue_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        offset++;

                        green_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        green_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        offset++;

                        red_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        red_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        exceeds_threshold = (blue_g + green_g + red_g > Threshold);
                    }
                }
            }

            offset -= 2;

            if (exceeds_threshold) {
                blue  = 0;
                green = 0;
                red   = 0;
            } else {
                blue  = src_buf[offset];
                green = src_buf[offset + 1];
                red   = src_buf[offset + 2];
            }

            blue  = blue  > 255 ? 255 : (blue  < 0 ? 0 : blue);
            green = green > 255 ? 255 : (green < 0 ? 0 : green);
            red   = red   > 255 ? 255 : (red   < 0 ? 0 : red);

            dst_buf[offset]     = blue;
            dst_buf[offset + 1] = green;
            dst_buf[offset + 2] = red;
            dst_buf[offset + 3] = src_buf[offset + 3];
        }
    }

    for (int y = 0; y < cartoon_image.height(); y++) {
        for (int x = 0; x < cartoon_image.width(); x++) {
            cartoon_image.setPixel(x, y, qRgba(dst_buf[(y * cartoon_image.width() + x) * 4 + 2], dst_buf[(y * cartoon_image.width() + x) * 4 + 1],
                                               dst_buf[(y * cartoon_image.width() + x) * 4],     dst_buf[(y * cartoon_image.width() + x) * 4 + 3]));
        }
    }

    emit imageReady(cartoon_image);
    emit finished();
}
