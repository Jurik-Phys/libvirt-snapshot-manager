// Begin appInfo.cpp

#include <QDebug>
#include "appInfo.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#ifndef APP_GIT_HASH
#define APP_GIT_HASH "unknown"
#endif

#ifndef APP_GIT_DIRTY
#define APP_GIT_DIRTY "-?????"
#endif

#ifndef APP_GIT_BRANCH
#define APP_GIT_BRANCH "unknown"
#endif

#ifndef APP_BASE_NAME
#define APP_BASE_NAME "Libvrit Snapshot Manager"
#endif

#ifndef APP_FULL_NAME
#define APP_FULL_NAME "Graphical Manager for External Snapshots (libvirt)"
#endif

#ifndef APP_AUTHOR_NAME
#define APP_AUTHOR_NAME "Yury Ovsyannikov"
#endif

#ifndef APP_AUTHOR_MAIL
#define APP_AUTHOR_MAIL "jurik.phys@gmail.com"
#endif

#ifndef APP_AUTHOR_SITE
#define APP_AUTHOR_SITE "https://jurik-phys.net"
#endif

QString AppInfo::appBaseName(){
    return QString(APP_BASE_NAME);
}

QString AppInfo::appFullName(){
    return QString(APP_FULL_NAME);
}

QString AppInfo::appBaseVersion(){
    return QString(APP_VERSION);
}

QString AppInfo::appFullVersion(){
    QString res(QString("v%1 (git-%2%3, %4)")
            .arg(APP_VERSION)
            .arg(APP_GIT_HASH)
            .arg(APP_GIT_DIRTY)
            .arg(APP_GIT_BRANCH));
    return res;
}

QString AppInfo::appAuthorName(){
    return QString(APP_AUTHOR_NAME);
}

QString AppInfo::appAuthorMail(){
    return QString(APP_AUTHOR_MAIL);
}

QString AppInfo::appAuthorSite(){
    return QString(APP_AUTHOR_SITE);
}

// End appInfo.cpp
