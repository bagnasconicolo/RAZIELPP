/****************************************************************************
** Meta object code from reading C++ file 'NDVIApp.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/NDVIApp.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NDVIApp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_NDVIApp_t {
    QByteArrayData data[21];
    char stringdata0[228];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_NDVIApp_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_NDVIApp_t qt_meta_stringdata_NDVIApp = {
    {
QT_MOC_LITERAL(0, 0, 7), // "NDVIApp"
QT_MOC_LITERAL(1, 8, 11), // "startCamera"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 10), // "stopCamera"
QT_MOC_LITERAL(4, 32, 12), // "onFrameReady"
QT_MOC_LITERAL(5, 45, 7), // "cv::Mat"
QT_MOC_LITERAL(6, 53, 5), // "frame"
QT_MOC_LITERAL(7, 59, 16), // "onCaptureStopped"
QT_MOC_LITERAL(8, 76, 14), // "onPreviewTimer"
QT_MOC_LITERAL(9, 91, 13), // "changePalette"
QT_MOC_LITERAL(10, 105, 4), // "name"
QT_MOC_LITERAL(11, 110, 12), // "takeSnapshot"
QT_MOC_LITERAL(12, 123, 15), // "toggleRecording"
QT_MOC_LITERAL(13, 139, 7), // "checked"
QT_MOC_LITERAL(14, 147, 13), // "autoCalibrate"
QT_MOC_LITERAL(15, 161, 16), // "chooseCrossColor"
QT_MOC_LITERAL(16, 178, 14), // "chooseRoiColor"
QT_MOC_LITERAL(17, 193, 13), // "onZoomChanged"
QT_MOC_LITERAL(18, 207, 5), // "value"
QT_MOC_LITERAL(19, 213, 10), // "logMessage"
QT_MOC_LITERAL(20, 224, 3) // "msg"

    },
    "NDVIApp\0startCamera\0\0stopCamera\0"
    "onFrameReady\0cv::Mat\0frame\0onCaptureStopped\0"
    "onPreviewTimer\0changePalette\0name\0"
    "takeSnapshot\0toggleRecording\0checked\0"
    "autoCalibrate\0chooseCrossColor\0"
    "chooseRoiColor\0onZoomChanged\0value\0"
    "logMessage\0msg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_NDVIApp[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    1,   81,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    1,   86,    2, 0x08 /* Private */,
      11,    0,   89,    2, 0x08 /* Private */,
      12,    1,   90,    2, 0x08 /* Private */,
      14,    0,   93,    2, 0x08 /* Private */,
      15,    0,   94,    2, 0x08 /* Private */,
      16,    0,   95,    2, 0x08 /* Private */,
      17,    1,   96,    2, 0x08 /* Private */,
      19,    1,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void, QMetaType::QString,   20,

       0        // eod
};

void NDVIApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<NDVIApp *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->startCamera(); break;
        case 1: _t->stopCamera(); break;
        case 2: _t->onFrameReady((*reinterpret_cast< const cv::Mat(*)>(_a[1]))); break;
        case 3: _t->onCaptureStopped(); break;
        case 4: _t->onPreviewTimer(); break;
        case 5: _t->changePalette((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->takeSnapshot(); break;
        case 7: _t->toggleRecording((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->autoCalibrate(); break;
        case 9: _t->chooseCrossColor(); break;
        case 10: _t->chooseRoiColor(); break;
        case 11: _t->onZoomChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->logMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< cv::Mat >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject NDVIApp::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_NDVIApp.data,
    qt_meta_data_NDVIApp,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *NDVIApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NDVIApp::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NDVIApp.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int NDVIApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
