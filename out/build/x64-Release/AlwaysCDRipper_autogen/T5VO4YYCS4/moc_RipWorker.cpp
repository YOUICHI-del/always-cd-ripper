/****************************************************************************
** Meta object code from reading C++ file 'RipWorker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ripper/RipWorker.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RipWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN9RipWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto RipWorker::qt_create_metaobjectdata<qt_meta_tag_ZN9RipWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RipWorker",
        "sectorDone",
        "",
        "SectorResult",
        "sr",
        "trackStarted",
        "trackNum",
        "totalTracks",
        "title",
        "trackDone",
        "RipResult",
        "result",
        "metadataReady",
        "artist",
        "album",
        "year",
        "QPixmap",
        "coverArt",
        "QList<TrackInfo>",
        "tracks",
        "finished",
        "error",
        "message",
        "run"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sectorDone'
        QtMocHelpers::SignalData<void(SectorResult)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'trackStarted'
        QtMocHelpers::SignalData<void(int, int, QString)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'trackDone'
        QtMocHelpers::SignalData<void(RipResult)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Signal 'metadataReady'
        QtMocHelpers::SignalData<void(QString, QString, QString, QPixmap, QVector<TrackInfo>)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 }, { QMetaType::QString, 14 }, { QMetaType::QString, 15 }, { 0x80000000 | 16, 17 },
            { 0x80000000 | 18, 19 },
        }}),
        // Signal 'finished'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'error'
        QtMocHelpers::SignalData<void(QString)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Slot 'run'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RipWorker, qt_meta_tag_ZN9RipWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RipWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9RipWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9RipWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9RipWorkerE_t>.metaTypes,
    nullptr
} };

void RipWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RipWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sectorDone((*reinterpret_cast<std::add_pointer_t<SectorResult>>(_a[1]))); break;
        case 1: _t->trackStarted((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->trackDone((*reinterpret_cast<std::add_pointer_t<RipResult>>(_a[1]))); break;
        case 3: _t->metadataReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QPixmap>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QList<TrackInfo>>>(_a[5]))); break;
        case 4: _t->finished(); break;
        case 5: _t->error((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->run(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)(SectorResult )>(_a, &RipWorker::sectorDone, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)(int , int , QString )>(_a, &RipWorker::trackStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)(RipResult )>(_a, &RipWorker::trackDone, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)(QString , QString , QString , QPixmap , QVector<TrackInfo> )>(_a, &RipWorker::metadataReady, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)()>(_a, &RipWorker::finished, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RipWorker::*)(QString )>(_a, &RipWorker::error, 5))
            return;
    }
}

const QMetaObject *RipWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RipWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9RipWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RipWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void RipWorker::sectorDone(SectorResult _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RipWorker::trackStarted(int _t1, int _t2, QString _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void RipWorker::trackDone(RipResult _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void RipWorker::metadataReady(QString _t1, QString _t2, QString _t3, QPixmap _t4, QVector<TrackInfo> _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 4
void RipWorker::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void RipWorker::error(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
