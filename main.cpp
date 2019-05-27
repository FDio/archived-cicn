/******************************************************************************
    Simple Player:  this file is part of QtAV examples
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>
*   This file is part of QtAV
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <cmath>
#include <QtCore/QtDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QApplication>
#include <QQuickItem>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtGui/QScreen>
#include <QTouchDevice>
#include <QtAV/QtAV.h>
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif
#include "Common/QtQuick2ApplicationViewer.h"
#include "../Common/Common.h"
#include "UI/DASHPlayer.h"
#include "UI/DASHPlayerNoGUI.h"
#include "UI/ViperGui.h"
#include "UI/GraphDataSource.h"
#include "Websocket/WebSocketService.h"
#include <QQmlContext>

using namespace viper;
int main(int argc, char *argv[])
{
    bool nogui = false;
    for(int i = 0; i < argc; i++)
    {
        if(!strcmp(argv[i], "-nohead"))
        {
            nogui = true;
            break;
        }
    }
    if(nogui)
    {
        pthread_mutex_t mainMutex;
        pthread_cond_t mainCond;

        pthread_mutex_init(&mainMutex,NULL);
        pthread_cond_init(&mainCond, NULL);

        Debug("STARTING NO GUI\n");
        DASHPlayerNoGUI p(argc,argv, &mainCond, true);


        pthread_mutex_lock(&mainMutex);
        while(p.isRunning())
        {
            pthread_cond_wait(&mainCond, &mainMutex);
        }
        pthread_mutex_unlock(&mainMutex);

        return 0;
    }
    QOptions options(get_common_options());
    options.add(QLatin1String("Viper options"))
            ("scale", 1.0, QLatin1String("scale of graphics context. 0: auto"))
            ;
    options.parse(argc, argv);
    Config::setName(QString::fromLatin1("Viper"));
    do_common_options_before_qapp(options);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Viper"));
    app.setApplicationDisplayName(QStringLiteral("Viper"));
    QDir::setCurrent(qApp->applicationDirPath());
    qDebug() <<  "event dispatcher:" << QCoreApplication::eventDispatcher();
    do_common_options(options);
    qDebug() << "arguments======= " << app.arguments();
    qDebug() << "current dir: " << QDir::currentPath();
    set_opengl_backend(options.option(QStringLiteral("gl")).value().toString(), app.arguments().first());
    load_qm(QStringList() << QStringLiteral("Viper"), options.value(QStringLiteral("language")).toString());
    QtQuick2ApplicationViewer viewer;
    QString binDir = qApp->applicationDirPath();
    if (binDir.endsWith(QLatin1String(".app/Contents/MacOS"))) {
        binDir.remove(QLatin1String(".app/Contents/MacOS"));
        binDir = binDir.left(binDir.lastIndexOf(QLatin1String("/")));
    }
    QQmlEngine *engine = viewer.engine();
    if (!engine->importPathList().contains(binDir))
        engine->addImportPath(binDir);
    qDebug() << engine->importPathList();
    engine->rootContext()->setContextProperty(QStringLiteral("PlayerConfig"), &Config::instance());
    qDebug(">>>>>>>>devicePixelRatio: %f", qApp->devicePixelRatio());
    QScreen *sc = app.primaryScreen();
    qDebug() << "dpi phy: " << sc->physicalDotsPerInch() << ", logical: " << sc->logicalDotsPerInch() << ", dpr: " << sc->devicePixelRatio()
             << "; vis rect:" << sc->virtualGeometry();
    engine->rootContext()->setContextProperty(QStringLiteral("screenPixelDensity"), sc->physicalDotsPerInch()*sc->devicePixelRatio());
    qreal r = sc->physicalDotsPerInch()/sc->logicalDotsPerInch();
    const qreal kR =
        #if defined(Q_OS_ANDROID)
            2.0;
#elif defined(Q_OS_WINRT)
            1.2;
#else
            1.0;
#endif
    if (std::isinf(r) || std::isnan(r))
        r = kR;
    float sr = options.value(QStringLiteral("scale")).toFloat();
#if defined(Q_OS_ANDROID) || defined(Q_OS_WINRT)
    sr = r;
#if defined(Q_OS_WINPHONE)
    sr = kR;
#endif
    if (sr > 2.0)
        sr = 2.0; //FIXME
#endif
    if (qFuzzyIsNull(sr))
        sr = r;
    engine->rootContext()->setContextProperty(QStringLiteral("scaleRatio"), sr);
    qDebug() << "touch devices: " << QTouchDevice::devices();
    engine->rootContext()->setContextProperty(QStringLiteral("isTouchScreen"), false);
#ifdef Q_OS_WINPHONE
    engine->rootContext()->setContextProperty(QStringLiteral("isTouchScreen"), true);
#endif
    foreach (const QTouchDevice* dev, QTouchDevice::devices()) {
        if (dev->type() == QTouchDevice::TouchScreen) {
            engine->rootContext()->setContextProperty(QStringLiteral("isTouchScreen"), true);
            break;
        }
    }
    QString qml = QStringLiteral("qml/Viper/main.qml");
    if (QFile(qApp->applicationDirPath() + QLatin1String("/") + qml).exists())
        qml.prepend(qApp->applicationDirPath() + QLatin1String("/"));
    else
        qml.prepend(QLatin1String("qrc:///"));
    viewer.setMainQmlFile(qml);
    viewer.show();
    QOption op = options.option(QStringLiteral("width"));
    if (op.isSet())
        viewer.setWidth(op.value().toInt());
    op = options.option(QStringLiteral("height"));
    if (op.isSet())
        viewer.setHeight(op.value().toInt());
    op = options.option(QStringLiteral("x"));
    if (op.isSet())
        viewer.setX(op.value().toInt());
    op = options.option(QStringLiteral("y"));
    if (op.isSet())
        viewer.setY(op.value().toInt());
    if (options.value(QStringLiteral("fullscreen")).toBool())
        viewer.showFullScreen();
    viewer.setTitle(QStringLiteral("Viper"));
#if 1
    QString json = app.arguments().join(QStringLiteral("\",\""));
    json.prepend(QLatin1String("[\"")).append(QLatin1String("\"]"));
    json.replace(QLatin1String("\\"), QLatin1String("/")); //FIXME
    QMetaObject::invokeMethod(viewer.rootObject(), "init", Q_ARG(QVariant, json));
    //#else
    QObject *player = viewer.rootObject()->findChild<QObject*>(QStringLiteral("player"));
    AppEventFilter *ae = new AppEventFilter(player, player);
    qApp->installEventFilter(ae);
    QString file;
#ifdef Q_OS_ANDROID
    engine->rootContext()->setContextProperty("platform",  1);
    file = QAndroidJniObject::callStaticObjectMethod("org.viper.com.ViperActivity"
                                                     , "getUrl"
                                                     , "()Ljava/lang/String;")
            .toString();
#endif
    engine->rootContext()->setContextProperty("platform", 0);
    if (app.arguments().size() > 1) {
        file = options.value(QStringLiteral("file")).toString();
        if (file.isEmpty()) {
            if (argc > 1 && !app.arguments().last().startsWith(QLatin1Char('-')) && !app.arguments().at(argc-2).startsWith(QLatin1Char('-')))
                file = app.arguments().last();
        }
    }
    qDebug() << "file: " << file;
    if (player && !file.isEmpty()) {
        if (!file.startsWith(QLatin1String("file:")) && QFile(file).exists())
            file.prepend(QLatin1String("file:")); //qml use url and will add qrc: if no scheme
        file.replace(QLatin1String("\\"), QLatin1String("/"));
        player->setProperty("source", QUrl(file));
    }
#endif
    QObject::connect(&Config::instance(), SIGNAL(changed()), &Config::instance(), SLOT(save()));
    QObject::connect(viewer.rootObject(), SIGNAL(requestFullScreen()), &viewer, SLOT(showFullScreen()));
    QObject::connect(viewer.rootObject(), SIGNAL(requestNormalSize()), &viewer, SLOT(showNormal()));
    GraphDataSource graphDataSource(&viewer);
    viewer.rootContext()->setContextProperty("dataSource", &graphDataSource);
    WebSocketService webSocketService;
    webSocketService.setGraphDataSource(&graphDataSource);
    webSocketService.start();
    ViperGui gui(player);
    QVariantHash va_opt;
    va_opt["display"] = "X11";
    va_opt["copyMode"] = "ZeroCopy";
    QVariantHash opt;
    opt["VAAPI"] = va_opt;
    gui.setNowLabel(viewer.rootObject()->findChild<QObject*>(QStringLiteral("now")));
    gui.setLifeLabel(viewer.rootObject()->findChild<QObject*>(QStringLiteral("life")));
    gui.setProgressBar(viewer.rootObject()->findChild<QObject*>(QStringLiteral("progress")));
    gui.setPlayButton(viewer.rootObject()->findChild<QObject*>(QStringLiteral("playBtn")));
    gui.setGraphDataSource(&graphDataSource);
    gui.setRootObject(viewer.rootObject());
    DASHPlayer dashPlayer(argc, argv, gui, &Config::instance());
    engine->rootContext()->setContextProperty("dashPlayer",&dashPlayer);
    QMetaObject::invokeMethod(viewer.rootObject(), "initGraph", Q_ARG(QVariant, (&Config::instance())->graph()));
    QMetaObject::invokeMethod(viewer.rootObject(), "initRepeat", Q_ARG(QVariant, (&Config::instance())->repeat()));
    QMetaObject::invokeMethod(viewer.rootObject(), "initFullScreen", Q_ARG(QVariant, (&Config::instance())->fullScreen()));
    QMetaObject::invokeMethod(viewer.rootObject(), "setAdaptationLogic", Q_ARG(QVariant, (&Config::instance())->adaptationLogic()));
    QMetaObject::invokeMethod(viewer.rootObject(), "setIcn", Q_ARG(QVariant, (&Config::instance())->icn()));
    return app.exec();
}
