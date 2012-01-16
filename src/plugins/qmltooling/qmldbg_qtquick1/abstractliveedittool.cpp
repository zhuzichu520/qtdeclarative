/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "abstractliveedittool.h"
#include "qdeclarativeviewinspector_p.h"

#include <QDeclarativeEngine>

#include <QtDebug>
#include <QGraphicsItem>
#include <QDeclarativeItem>

namespace QmlJSDebugger {
namespace QtQuick1 {

AbstractLiveEditTool::AbstractLiveEditTool(QDeclarativeViewInspector *editorView)
    : AbstractTool(editorView)
{
}


AbstractLiveEditTool::~AbstractLiveEditTool()
{
}

QDeclarativeViewInspector *AbstractLiveEditTool::inspector() const
{
    return static_cast<QDeclarativeViewInspector*>(AbstractTool::inspector());
}

QDeclarativeView *AbstractLiveEditTool::view() const
{
    return inspector()->declarativeView();
}

QGraphicsScene* AbstractLiveEditTool::scene() const
{
    return view()->scene();
}

void AbstractLiveEditTool::updateSelectedItems()
{
    selectedItemsChanged(items());
}

QList<QGraphicsItem*> AbstractLiveEditTool::items() const
{
    return inspector()->selectedItems();
}

bool AbstractLiveEditTool::topItemIsMovable(const QList<QGraphicsItem*> & itemList)
{
    QGraphicsItem *firstSelectableItem = topMovableGraphicsItem(itemList);
    if (firstSelectableItem == 0)
        return false;
    if (toQDeclarativeItem(firstSelectableItem) != 0)
        return true;

    return false;

}

bool AbstractLiveEditTool::topSelectedItemIsMovable(const QList<QGraphicsItem*> &itemList)
{
    QList<QGraphicsItem*> selectedItems = inspector()->selectedItems();

    foreach (QGraphicsItem *item, itemList) {
        QDeclarativeItem *declarativeItem = toQDeclarativeItem(item);
        if (declarativeItem
                && selectedItems.contains(declarativeItem)
                /*&& (declarativeItem->qmlItemNode().hasShowContent() || selectNonContentItems)*/)
            return true;
    }

    return false;

}

bool AbstractLiveEditTool::topItemIsResizeHandle(const QList<QGraphicsItem*> &/*itemList*/)
{
    return false;
}

QDeclarativeItem *AbstractLiveEditTool::toQDeclarativeItem(QGraphicsItem *item)
{
    return qobject_cast<QDeclarativeItem*>(item->toGraphicsObject());
}

QGraphicsItem *AbstractLiveEditTool::topMovableGraphicsItem(const QList<QGraphicsItem*> &itemList)
{
    foreach (QGraphicsItem *item, itemList) {
        if (item->flags().testFlag(QGraphicsItem::ItemIsMovable))
            return item;
    }
    return 0;
}

QDeclarativeItem *AbstractLiveEditTool::topMovableDeclarativeItem(const QList<QGraphicsItem*>
                                                                  &itemList)
{
    foreach (QGraphicsItem *item, itemList) {
        QDeclarativeItem *declarativeItem = toQDeclarativeItem(item);
        if (declarativeItem /*&& (declarativeItem->qmlItemNode().hasShowContent())*/)
            return declarativeItem;
    }

    return 0;
}

QList<QGraphicsObject*> AbstractLiveEditTool::toGraphicsObjectList(const QList<QGraphicsItem*>
                                                                   &itemList)
{
    QList<QGraphicsObject*> gfxObjects;
    foreach (QGraphicsItem *item, itemList) {
        QGraphicsObject *obj = item->toGraphicsObject();
        if (obj)
            gfxObjects << obj;
    }

    return gfxObjects;
}

QString AbstractLiveEditTool::titleForItem(QGraphicsItem *item)
{
    QString className(QLatin1String("QGraphicsItem"));
    QString objectStringId;

    QString constructedName;

    QGraphicsObject *gfxObject = item->toGraphicsObject();
    if (gfxObject) {
        className = QLatin1String(gfxObject->metaObject()->className());

        className.remove(QRegExp(QLatin1String("_QMLTYPE_\\d+")));
        className.remove(QRegExp(QLatin1String("_QML_\\d+")));
        if (className.startsWith(QLatin1String("QDeclarative")))
            className = className.remove(QLatin1String("QDeclarative"));

        QDeclarativeItem *declarativeItem = qobject_cast<QDeclarativeItem*>(gfxObject);
        if (declarativeItem) {
            objectStringId = inspector()->idStringForObject(declarativeItem);
        }

        if (!objectStringId.isEmpty()) {
            constructedName = objectStringId + QLatin1String(" (") + className + QLatin1Char(')');
        } else {
            if (!gfxObject->objectName().isEmpty()) {
                constructedName = gfxObject->objectName() + QLatin1String(" (") + className + QLatin1Char(')');
            } else {
                constructedName = className;
            }
        }
    }

    return constructedName;
}

} // namespace QtQuick1
} // namespace QmlJSDebugger
