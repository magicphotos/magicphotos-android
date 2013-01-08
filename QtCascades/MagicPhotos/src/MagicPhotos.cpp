#include "MagicPhotos.hpp"

#include <bb/cascades/pickers/FilePicker>
#include <bb/cascades/pickers/FilePickerMode>
#include <bb/cascades/pickers/FilePickerSortFlag>
#include <bb/cascades/pickers/FilePickerSortOrder>
#include <bb/cascades/pickers/FilePickerViewMode>
#include <bb/cascades/pickers/FileType>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>

#include "messagebox.h"
#include "decolorizeeditor.h"
#include "sketcheditor.h"

using namespace bb::cascades;

MagicPhotos::MagicPhotos(bb::cascades::Application *app) : QObject(app)
{
	qmlRegisterType<bb::cascades::pickers::FilePicker>("FilePicker", 1, 0, "FilePicker");

	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerMode>("FilePicker", 1, 0, "FilePickerMode", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerSortFlag>("FilePicker", 1, 0, "FilePickerSortFlag", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerSortOrder>("FilePicker", 1, 0, "FilePickerSortOrder", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FileType>("FilePicker", 1, 0, "FileType", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerViewMode>("FilePicker", 1, 0, "FilePickerViewMode", "");

	qmlRegisterType<DecolorizeEditor>("ImageEditor", 1, 0, "DecolorizeEditor");
	qmlRegisterType<SketchEditor>("ImageEditor", 1, 0, "SketchEditor");
	qmlRegisterType<SketchPreviewGenerator>("ImageEditor", 1, 0, "SketchPreviewGenerator");

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    qml->setContextProperty("MessageBox", new MessageBox());

    AbstractPane *root = qml->createRootObject<AbstractPane>();

    app->setScene(root);
}
