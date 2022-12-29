#ifndef HARDWARE_FUNCTIONS_H
#define HARDWARE_FUNCTIONS_H

#include <QString>

#include <stdio.h>

#ifdef ANDROID
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAndroidJniObject>
#else
#include <QJniObject>
#endif
#endif

QString get_hdid();

#endif // HARDWARE_FUNCTIONS_H
