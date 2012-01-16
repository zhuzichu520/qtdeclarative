/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
#include <qtest.h>
#include <private/qdeclarativelistcompositor_p.h>

template<typename T, int N> int lengthOf(const T (&)[N]) { return N; }

typedef QDeclarativeListCompositor C;

struct Range
{
    Range() {}
    Range(void *list, int index, int count, int flags)
        : list(list), index(index), count(count), flags(flags) {}
    void *list;
    int index;
    int count;
    int flags;
};

template <typename T>  struct Array
{
    Array() : array(0), count(0) {}
    template<int N> Array(const T (&array)[N]) : array(array), count(N) {}

    T operator [](int index) const { return array[index]; }

    const T *array;
    int count;
};

typedef Array<int> IndexArray;
typedef Array<const void *> ListArray;

typedef QVector<QDeclarativeListCompositor::Remove> RemoveList;
typedef QVector<QDeclarativeListCompositor::Insert> InsertList;
typedef QVector<QDeclarativeListCompositor::Change> ChangeList;

typedef QVector<Range> RangeList;

Q_DECLARE_METATYPE(RangeList)
Q_DECLARE_METATYPE(RemoveList)
Q_DECLARE_METATYPE(InsertList)
Q_DECLARE_METATYPE(ChangeList)
Q_DECLARE_METATYPE(void *)
Q_DECLARE_METATYPE(IndexArray)
Q_DECLARE_METATYPE(ListArray)
Q_DECLARE_METATYPE(C::Group)

QT_BEGIN_NAMESPACE
bool operator ==(const C::Change &left, const C::Change &right)
{
    return left.index[3] == right.index[3]
            && left.index[2] == right.index[2]
            && left.index[1] == right.index[1]
            && left.index[0] == right.index[0]
            && left.count == right.count
            && left.groups() == right.groups()
            && left.inCache() == right.inCache()
            && (left.moveId == -1) == (right.moveId == -1);
}
QT_END_NAMESPACE

static const C::Group Visible = C::Group(2);
static const C::Group Selection = C::Group(3);

class tst_qdeclarativelistcompositor : public QObject
{
    Q_OBJECT

    enum {
        VisibleFlag   = 0x04,
        SelectionFlag = 0x08
    };

    void populateChange(
            C::Change &change, int sIndex, int vIndex, int dIndex, int cIndex, int count, int flags, int moveId)
    {
        change.index[Selection] = sIndex;
        change.index[Visible] = vIndex;
        change.index[C::Default] = dIndex;
        change.index[C::Cache] = cIndex;
        change.count = count;
        change.flags = flags;
        change.moveId = moveId;
    }

    C::Remove Remove(
            int sIndex, int vIndex, int dIndex, int cIndex, int count, int flags, int moveId = -1)
    {
        C::Remove remove;
        populateChange(remove, sIndex, vIndex, dIndex, cIndex, count, flags, moveId);
        return remove;
    }

    C::Insert Insert(
            int sIndex, int vIndex, int dIndex, int cIndex, int count, int flags, int moveId = -1)
    {
        C::Insert insert;
        populateChange(insert, sIndex, vIndex, dIndex, cIndex, count, flags, moveId);
        return insert;
    }

    C::Change Change(
        int sIndex, int vIndex, int dIndex, int cIndex, int count, int flags, int moveId = -1)
    {
        C::Change change;
        populateChange(change, sIndex, vIndex, dIndex, cIndex, count, flags, moveId);
        return change;
    }

private slots:
    void find_data();
    void find();
    void findInsertPosition_data();
    void findInsertPosition();
    void insert();
    void clearFlags_data();
    void clearFlags();
    void setFlags_data();
    void setFlags();
    void move_data();
    void move();
    void moveFromEnd();
    void clear();
    void listItemsInserted_data();
    void listItemsInserted();
    void listItemsRemoved_data();
    void listItemsRemoved();
    void listItemsMoved_data();
    void listItemsMoved();
    void listItemsChanged_data();
    void listItemsChanged();
};

void tst_qdeclarativelistcompositor::find_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<C::Group>("startGroup");
    QTest::addColumn<int>("startIndex");
    QTest::addColumn<C::Group>("group");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("selectionIndex");
    QTest::addColumn<int>("visibleIndex");
    QTest::addColumn<int>("defaultIndex");
    QTest::addColumn<int>("cacheIndex");
    QTest::addColumn<uint>("rangeFlags");
    QTest::addColumn<int>("rangeIndex");

    int listA; void *a = &listA;

    QTest::newRow("Start")
            << (RangeList()
                << Range(a, 0, 1, int(C::PrependFlag |  SelectionFlag | C::DefaultFlag | C::CacheFlag))
                << Range(a, 1, 1, int(C::AppendFlag | C::PrependFlag | C::CacheFlag))
                << Range(0, 0, 1, int(VisibleFlag| C::CacheFlag)))
            << C::Cache << 2
            << Selection << 0
            << 0 << 0 << 0 << 0
            << uint(C::PrependFlag |  SelectionFlag | C::DefaultFlag | C::CacheFlag) << 0;
}

void tst_qdeclarativelistcompositor::find()
{
    QFETCH(RangeList, ranges);
    QFETCH(C::Group, startGroup);
    QFETCH(int, startIndex);
    QFETCH(C::Group, group);
    QFETCH(int, index);
    QFETCH(int, cacheIndex);
    QFETCH(int, defaultIndex);
    QFETCH(int, visibleIndex);
    QFETCH(int, selectionIndex);
    QFETCH(uint, rangeFlags);
    QFETCH(int, rangeIndex);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    compositor.find(startGroup, startIndex);

    QDeclarativeListCompositor::iterator it = compositor.find(group, index);
    QCOMPARE(it.index[C::Cache], cacheIndex);
    QCOMPARE(it.index[C::Default], defaultIndex);
    QCOMPARE(it.index[Visible], visibleIndex);
    QCOMPARE(it.index[Selection], selectionIndex);
    QCOMPARE(it->flags, rangeFlags);
    QCOMPARE(it->index, rangeIndex);
}

void tst_qdeclarativelistcompositor::findInsertPosition_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<C::Group>("group");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("selectionIndex");
    QTest::addColumn<int>("visibleIndex");
    QTest::addColumn<int>("defaultIndex");
    QTest::addColumn<int>("cacheIndex");
    QTest::addColumn<uint>("rangeFlags");
    QTest::addColumn<int>("rangeIndex");

    int listA; void *a = &listA;

    QTest::newRow("Start")
            << (RangeList()
                << Range(a, 0, 1, int(C::PrependFlag |  SelectionFlag | C::DefaultFlag | C::CacheFlag))
                << Range(a, 1, 1, int(C::AppendFlag | C::PrependFlag | C::CacheFlag))
                << Range(0, 0, 1, int(VisibleFlag| C::CacheFlag)))
            << Selection << 0
            << 0 << 0 << 0 << 0
            << uint(C::PrependFlag |  SelectionFlag | C::DefaultFlag | C::CacheFlag) << 0;
    QTest::newRow("1")
            << (RangeList()
                << Range(a, 0, 1, int(C::PrependFlag |  SelectionFlag | C::DefaultFlag | C::CacheFlag))
                << Range(a, 1, 1, int(C::AppendFlag | C::PrependFlag | C::CacheFlag))
                << Range(0, 0, 1, int(VisibleFlag| C::CacheFlag)))
            << Selection << 1
            << 1 << 0 << 1 << 1
            << uint(C::AppendFlag | C::PrependFlag | C::CacheFlag) << 1;
}

void tst_qdeclarativelistcompositor::findInsertPosition()
{
    QFETCH(RangeList, ranges);
    QFETCH(C::Group, group);
    QFETCH(int, index);
    QFETCH(int, cacheIndex);
    QFETCH(int, defaultIndex);
    QFETCH(int, visibleIndex);
    QFETCH(int, selectionIndex);
    QFETCH(uint, rangeFlags);
    QFETCH(int, rangeIndex);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QDeclarativeListCompositor::insert_iterator it = compositor.findInsertPosition(group, index);
    QCOMPARE(it.index[C::Cache], cacheIndex);
    QCOMPARE(it.index[C::Default], defaultIndex);
    QCOMPARE(it.index[Visible], visibleIndex);
    QCOMPARE(it.index[Selection], selectionIndex);
    QCOMPARE(it->flags, rangeFlags);
    QCOMPARE(it->index, rangeIndex);
}

void tst_qdeclarativelistcompositor::insert()
{
    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    C::iterator it;

    int listA; int *a = &listA;
    int listB; int *b = &listB;
    int listC; int *c = &listC;

    {
        compositor.append(a, 0, 12, C::AppendFlag | C::PrependFlag | C::DefaultFlag);
        const int indexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        const int *lists[]  = {a,a,a,a,a,a,a,a,a,a, a, a};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } {
        compositor.append(b, 4, 4, C::DefaultFlag);
        const int indexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7};
        const int *lists[]  = {a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } { // Insert at end.
        compositor.insert(
                C::Default, 16, c, 2, 2, C::DefaultFlag);
        const int indexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7,2,3};
        const int *lists[]  = {a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b,c,c};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } { // Insert at start
        compositor.insert(
                C::Default, 0, c, 6, 4, C::DefaultFlag);
        const int indexes[] = {6,7,8,9,0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7,2,3};
        const int *lists[]  = {c,c,c,c,a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b,c,c};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } { // Insert after static range.
        compositor.insert(
                C::Default, 4, b, 0, 8, C::AppendFlag | C::PrependFlag | C::DefaultFlag);
        const int indexes[] = {6,7,8,9,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7,2,3};
        const int *lists[]  = {c,c,c,c,b,b,b,b,b,b,b,b,a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b,c,c};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } { // Insert at end of dynamic range.
        compositor.insert(
                C::Default, 12, c, 0, 4, C::AppendFlag | C::PrependFlag | C::DefaultFlag);
        const int indexes[] = {6,7,8,9,0,1,2,3,4,5,6,7,0,1,2,3,0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7,2,3};
        const int *lists[]  = {c,c,c,c,b,b,b,b,b,b,b,b,c,c,c,c,a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b,c,c};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    } { // Insert into range.
        compositor.insert(
                C::Default, 8, c, 0, 4, C::AppendFlag | C::PrependFlag | C::DefaultFlag);
        const int indexes[] = {6,7,8,9,0,1,2,3,0,1,2,3,4,5,6,7,0,1,2,3,0,1,2,3,4,5,6,7,8,9,10,11,4,5,6,7,2,3};
        const int *lists[]  = {c,c,c,c,b,b,b,b,c,c,c,c,b,b,b,b,c,c,c,c,a,a,a,a,a,a,a,a,a,a, a, a,b,b,b,b,c,c};
        QCOMPARE(compositor.count(C::Default), lengthOf(indexes));
        for (int i = 0; i < lengthOf(indexes); ++i) {
            it = compositor.find(C::Default, i);
            QCOMPARE(it.list<int>(), lists[i]);
            if (lists[i]) QCOMPARE(it.modelIndex(), indexes[i]);
        }
    }
}

void tst_qdeclarativelistcompositor::clearFlags_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<C::Group>("group");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("flags");
    QTest::addColumn<RemoveList>("expectedRemoves");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<ListArray>("cacheLists");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<ListArray>("defaultLists");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<ListArray>("visibleLists");
    QTest::addColumn<IndexArray>("selectionIndexes");
    QTest::addColumn<ListArray>("selectionLists");

    int listA; void *a = &listA;

    {   static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int visibleIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int selectionIndexes[] = {0,1,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *selectionLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        QTest::newRow("Default, 2, 2, Selection")
                << (RangeList()
                    << Range(a, 0, 12, int(C::AppendFlag | C::PrependFlag |  SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag))
                    << Range(0, 0, 4, int(SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)))
                << C::Default << 2 << 2 << int(SelectionFlag)
                << (RemoveList()
                    << Remove(2, 2, 2, 2, 2, SelectionFlag | C::CacheFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int visibleIndexes[] = {0,2,3,5,6,7,8,9,10,11,0,0,0,0};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int selectionIndexes[] = {0,1,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *selectionLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        QTest::newRow("Selection, 1, 2, Visible")
                << (RangeList()
                    << Range(a, 0, 2, int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 2, 2, int(C::PrependFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 4, 8, int(C::AppendFlag | C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(0, 0, 4, int(SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)))
                << Selection << 1 << 2 << int(VisibleFlag)
                << (RemoveList()
                    << Remove(1, 1, 1, 1, 1, VisibleFlag | C::CacheFlag)
                    << Remove(2, 3, 4, 4, 1, VisibleFlag | C::CacheFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0,0};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0};
        static const int visibleIndexes[] = {0,2,3,5,6,7,8,9,10,11,0,0,0};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0};
        static const int selectionIndexes[] = {0,1,4,5,6,7,8,9,10,11,0,0,0};
        static const void *selectionLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0};
        QTest::newRow("Default, 13, 1, Prepend | Selection | Visible | Default")
                << (RangeList()
                    << Range(a, 0, 1, int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 1, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 2, 2, int(C::PrependFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 4, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 5, 7, int(C::AppendFlag | C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(0, 0, 4, int(SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)))
                << C::Default << 13 << 1 << int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag)
                << (RemoveList()
                    << Remove(11, 11, 13, 13, 1, SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,0};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11,0,0,0};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a,0,0,0};
        static const int visibleIndexes[] = {0,2,3,5,6,7,8,9,10,11,0,0,0};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0};
        static const int selectionIndexes[] = {0,1,4,5,6,7,8,9,10,11,0,0,0};
        static const void *selectionLists[] = {a,a,a,a,a,a,a,a, a, a,0,0,0};
        QTest::newRow("Cache, 11, 4, Cache")
                << (RangeList()
                    << Range(a, 0, 1, int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 1, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 2, 2, int(C::PrependFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 4, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 5, 7, int(C::AppendFlag | C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(0, 0, 1, int(C::CacheFlag))
                    << Range(0, 0, 3, int(SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)))
                << C::Cache << 11 << 4 << int(C::CacheFlag)
                << (RemoveList())
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,0};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,0};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a,0};
        static const int visibleIndexes[] = {0,2,3,5,6,7,8,9,10,0};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a, a,0};
        static const int selectionIndexes[] = {0,1,4,5,6,7,8,9,10,0};
        static const void *selectionLists[] = {a,a,a,a,a,a,a,a, a,0};
        QTest::newRow("Default, 11, 3, Default | Visible | Selection")
                << (RangeList()
                    << Range(a, 0, 1, int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 1, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 2, 2, int(C::PrependFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 4, 1, int(C::PrependFlag | SelectionFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 5, 6, int(C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag  | C::CacheFlag))
                    << Range(a, 11, 1, int(C::AppendFlag | C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag))
                    << Range(0, 0, 2, int(SelectionFlag | VisibleFlag | C::DefaultFlag))
                    << Range(0, 0, 1, int(SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)))
                << C::Default << 11 << 3 << int(C::DefaultFlag | VisibleFlag| SelectionFlag)
                << (RemoveList()
                    << Remove(9, 9, 11, 11, 1, SelectionFlag | VisibleFlag | C::DefaultFlag)
                    << Remove(9, 9, 11, 11, 2, SelectionFlag | VisibleFlag | C::DefaultFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    }
}

void tst_qdeclarativelistcompositor::clearFlags()
{
    QFETCH(RangeList, ranges);
    QFETCH(C::Group, group);
    QFETCH(int, index);
    QFETCH(int, count);
    QFETCH(int, flags);
    QFETCH(RemoveList, expectedRemoves);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(ListArray, cacheLists);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(ListArray, defaultLists);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(ListArray, visibleLists);
    QFETCH(IndexArray, selectionIndexes);
    QFETCH(ListArray, selectionLists);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Remove> removes;
    compositor.clearFlags(group, index, count, flags, &removes);

    QCOMPARE(removes, expectedRemoves);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Cache, i);
        QCOMPARE(it->list, cacheLists[i]);
        if (cacheLists[i])
            QCOMPARE(it.modelIndex(), cacheIndexes[i]);
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Default, i);
        QCOMPARE(it->list, defaultLists[i]);
        if (defaultLists[i])
            QCOMPARE(it.modelIndex(), defaultIndexes[i]);
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        C::iterator it = compositor.find(Visible, i);
        QCOMPARE(it->list, visibleLists[i]);
        if (visibleLists[i])
            QCOMPARE(it.modelIndex(), visibleIndexes[i]);
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        C::iterator it = compositor.find(Selection, i);
        QCOMPARE(it->list, selectionLists[i]);
        if (selectionLists[i])
            QCOMPARE(it.modelIndex(), selectionIndexes[i]);
    }
}

void tst_qdeclarativelistcompositor::setFlags_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<C::Group>("group");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("flags");
    QTest::addColumn<InsertList>("expectedInserts");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<ListArray>("cacheLists");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<ListArray>("defaultLists");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<ListArray>("visibleLists");
    QTest::addColumn<IndexArray>("selectionIndexes");
    QTest::addColumn<ListArray>("selectionLists");

    int listA; void *a = &listA;

    {   static const int cacheIndexes[] = {0,0,0,0};
        static const void *cacheLists[] = {0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        QTest::newRow("Default, 2, 2, Default")
                << (RangeList()
                    << Range(a, 0, 12, C::DefaultFlag)
                    << Range(0, 0, 4, C::CacheFlag))
                << C::Default << 2 << 2 << int(C::DefaultFlag)
                << (InsertList())
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,0,0,0};
        static const void *cacheLists[] = {0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        static const int visibleIndexes[] = {2,3};
        static const void *visibleLists[] = {a,a};
        QTest::newRow("Default, 2, 2, Visible")
                << (RangeList()
                    << Range(a, 0, 12, C::DefaultFlag)
                    << Range(0, 0, 4, C::CacheFlag))
                << C::Default << 2 << 2 << int(VisibleFlag)
                << (InsertList()
                    << Insert(0, 0, 2, 0, 2, VisibleFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {3,6,0,0,0,0};
        static const void *cacheLists[] = {a,a,0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        static const int visibleIndexes[] = {2,3,6,7};
        static const void *visibleLists[] = {a,a,a,a};
        static const int selectionIndexes[] = {3,6};
        static const void *selectionLists[] = {a,a};
        QTest::newRow("Visible, 1, 2, Selection | Cache")
                << (RangeList()
                    << Range(a, 0, 2, C::DefaultFlag)
                    << Range(a, 2, 2, VisibleFlag | C::DefaultFlag)
                    << Range(a, 4, 2, C::DefaultFlag)
                    << Range(a, 6, 2, VisibleFlag | C::DefaultFlag)
                    << Range(a, 8, 4, C::DefaultFlag)
                    << Range(0, 0, 4, C::CacheFlag))
                << Visible << 1 << 2 << int(SelectionFlag | C::CacheFlag)
                << (InsertList()
                    << Insert(0, 1, 3, 0, 1, SelectionFlag | C::CacheFlag)
                    << Insert(1, 2, 6, 1, 1, SelectionFlag | C::CacheFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {3,6,0,0,0,0};
        static const void *cacheLists[] = {a,a,0,0,0,0};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        static const int visibleIndexes[] = {2,3,6,7,0};
        static const void *visibleLists[] = {a,a,a,a,0};
        static const int selectionIndexes[] = {3,6};
        static const void *selectionLists[] = {a,a};
        QTest::newRow("Cache, 3, 1, Visible")
                << (RangeList()
                    << Range(a, 0, 2, C::DefaultFlag)
                    << Range(a, 2, 1, VisibleFlag | C::DefaultFlag)
                    << Range(a, 3, 1, SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 4, 2, C::DefaultFlag)
                    << Range(a, 6, 1, SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 7, 1, VisibleFlag | C::DefaultFlag)
                    << Range(a, 8, 4, C::DefaultFlag)
                    << Range(0, 0, 4, C::CacheFlag))
                << C::Cache << 3 << 1 << int(VisibleFlag)
                << (InsertList()
                    << Insert(2, 4, 12, 3, 1, VisibleFlag | C::CacheFlag))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    } { static const int cacheIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *cacheLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        static const int defaultIndexes[] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static const void *defaultLists[] = {a,a,a,a,a,a,a,a,a,a, a, a};
        static const int visibleIndexes[] = {0,1,3,4,5,6,7,8,9,10,11};
        static const void *visibleLists[] = {a,a,a,a,a,a,a,a,a, a, a};
        static const int selectionIndexes[] = {2,6,7,8,9};
        static const void *selectionLists[] = {a,a,a,a,a};
        QTest::newRow("Existing flag, sparse selection")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 1, C::PrependFlag | SelectionFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 3, 3, C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 6, 4, C::PrependFlag | SelectionFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a,10, 2, C::AppendFlag | C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag))
                << C::Cache << 3 << 1 << int(VisibleFlag)
                << InsertList()
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray(selectionIndexes) << ListArray(selectionLists);
    }
}

void tst_qdeclarativelistcompositor::setFlags()
{
    QFETCH(RangeList, ranges);
    QFETCH(C::Group, group);
    QFETCH(int, index);
    QFETCH(int, count);
    QFETCH(int, flags);
    QFETCH(InsertList, expectedInserts);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(ListArray, cacheLists);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(ListArray, defaultLists);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(ListArray, visibleLists);
    QFETCH(IndexArray, selectionIndexes);
    QFETCH(ListArray, selectionLists);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Insert> inserts;
    compositor.setFlags(group, index, count, flags, &inserts);

    QCOMPARE(inserts, expectedInserts);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Cache, i);
        QCOMPARE(it->list, cacheLists[i]);
        if (cacheLists[i])
            QCOMPARE(it.modelIndex(), cacheIndexes[i]);
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Default, i);
        QCOMPARE(it->list, defaultLists[i]);
        if (defaultLists[i])
            QCOMPARE(it.modelIndex(), defaultIndexes[i]);
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        C::iterator it = compositor.find(Visible, i);
        QCOMPARE(it->list, visibleLists[i]);
        if (visibleLists[i])
            QCOMPARE(it.modelIndex(), visibleIndexes[i]);
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        C::iterator it = compositor.find(Selection, i);
        QCOMPARE(it->list, selectionLists[i]);
        if (selectionLists[i])
            QCOMPARE(it.modelIndex(), selectionIndexes[i]);
    }
}

void tst_qdeclarativelistcompositor::move_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<C::Group>("fromGroup");
    QTest::addColumn<int>("from");
    QTest::addColumn<C::Group>("toGroup");
    QTest::addColumn<int>("to");
    QTest::addColumn<int>("count");
    QTest::addColumn<RemoveList>("expectedRemoves");
    QTest::addColumn<InsertList>("expectedInserts");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<ListArray>("cacheLists");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<ListArray>("defaultLists");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<ListArray>("visibleLists");
    QTest::addColumn<IndexArray>("selectionIndexes");
    QTest::addColumn<ListArray>("selectionLists");

    int listA; void *a = &listA;
    int listB; void *b = &listB;
    int listC; void *c = &listC;

    {   static const int cacheIndexes[] = {0,0,0,0,2,3};
        static const void *cacheLists[] = {0,0,0,0,c,c};
        static const int defaultIndexes[] = {0,0,1,2,3,4,5,0,1,2,3,4,5,1,2,3,0,1,2,3,4,5};
        static const void *defaultLists[] = {0,a,a,a,a,a,a,b,b,b,b,b,b,0,0,0,c,c,c,c,c,c};
        QTest::newRow("15, 0, 1")
                << (RangeList()
                    << Range(a, 0, 6, C::DefaultFlag)
                    << Range(b, 0, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(0, 0, 4, C::DefaultFlag | C::CacheFlag)
                    << Range(c, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(c, 2, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(c, 4, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << C::Default << 15 << C::Default << 0 << 1
                << (RemoveList()
                    << Remove(0, 0, 15, 3, 1, C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 0, 0, 1, C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,0,0,0,2,3};
        static const void *cacheLists[] = {0,0,0,0,c,c};
        static const int defaultIndexes[] = {0,1,0,1,2,3,4,5,0,1,2,3,4,5,2,3,0,1,2,3,4,5};
        static const void *defaultLists[] = {0,0,a,a,a,a,a,a,b,b,b,b,b,b,0,0,c,c,c,c,c,c};
        QTest::newRow("15, 1, 1")
                << (RangeList()
                    << Range(0, 0, 1, C::DefaultFlag | C::CacheFlag)
                    << Range(a, 0, 6, C::DefaultFlag)
                    << Range(b, 0, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(0, 0, 3, C::DefaultFlag | C::CacheFlag)
                    << Range(c, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(c, 2, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(c, 4, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << C::Default << 15 << C::Default << 1 << 1
                << (RemoveList()
                    << Remove(0, 0, 15, 3, 1, C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 1, 1, 1, C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,0,0,0,2,3};
        static const void *cacheLists[] = {0,0,0,0,c,c};
        static const int defaultIndexes[] = {0,1,2,0,1,3,4,5,0,1,2,3,4,5,2,3,0,1,2,3,4,5};
        static const void *defaultLists[] = {a,a,a,0,0,a,a,a,b,b,b,b,b,b,0,0,c,c,c,c,c,c};
        QTest::newRow("0, 3, 2")
                << (RangeList()
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(a, 0, 6, C::DefaultFlag)
                    << Range(b, 0, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(c, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(c, 2, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(c, 4, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << C::Default << 0 << C::Default << 3 << 2
                << (RemoveList()
                    << Remove(0, 0, 0, 0, 2, C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 3, 0, 2, C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,0,0,0,2,3};
        static const void *cacheLists[] = {0,0,0,0,c,c};
        static const int defaultIndexes[] = {0,5,0,1,2,3,4,5,0,1,0,1,2,2,3,3,4,1,2,3,4,5};
        static const void *defaultLists[] = {a,a,b,b,b,b,b,b,0,0,c,a,a,0,0,a,a,c,c,c,c,c};
        QTest::newRow("7, 1, 10")
                << (RangeList()
                    << Range(a, 0, 3, C::DefaultFlag)
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(a, 3, 3, C::DefaultFlag)
                    << Range(b, 0, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(c, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(c, 2, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(c, 4, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << C::Default << 7 << C::Default << 1 << 10
                << (RemoveList()
                    << Remove(0, 0, 7, 2, 1, C::DefaultFlag, 0)
                    << Remove(0, 0, 7, 2, 6, C::DefaultFlag, 1)
                    << Remove(0, 0, 7, 2, 2, C::DefaultFlag | C::CacheFlag, 2)
                    << Remove(0, 0, 7, 2, 1, C::DefaultFlag, 3))
                << (InsertList()
                    << Insert(0, 0, 1, 0, 1, C::DefaultFlag, 0)
                    << Insert(0, 0, 2, 0, 6, C::DefaultFlag, 1)
                    << Insert(0, 0, 8, 0, 2, C::DefaultFlag | C::CacheFlag, 2)
                    << Insert(0, 0, 10, 2, 1, C::DefaultFlag, 3))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,0,0,0,3,2};
        static const void *cacheLists[] = {0,0,0,0,c,c};
        static const int defaultIndexes[] = {0,5,0,1,2,3,4,5,0,1,0,1,2,2,3,3,4,3,4,5,1,2};
        static const void *defaultLists[] = {a,a,b,b,b,b,b,b,0,0,c,a,a,0,0,a,a,c,c,c,c,c};
        QTest::newRow("17, 20, 2")
                << (RangeList()
                    << Range(a, 0, 1, C::DefaultFlag)
                    << Range(a, 5, 1, C::DefaultFlag)
                    << Range(b, 0, 6, C::DefaultFlag)
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(c, 0, 1, C::DefaultFlag)
                    << Range(a, 1, 2, C::DefaultFlag)
                    << Range(0, 0, 2, C::DefaultFlag | C::CacheFlag)
                    << Range(a, 3, 2, C::DefaultFlag)
                    << Range(b, 0, 6, C::AppendFlag | C::PrependFlag)
                    << Range(c, 0, 1, C::PrependFlag)
                    << Range(c, 1, 1, C::PrependFlag | C::DefaultFlag)
                    << Range(c, 2, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(c, 4, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << C::Default << 17 << C::Default << 20 << 2
                << (RemoveList()
                    << Remove(0, 0, 17, 4, 1, C::DefaultFlag, 0)
                    << Remove(0, 0, 17, 4, 1, C::DefaultFlag | C::CacheFlag, 1))
                << (InsertList()
                    << Insert(0, 0, 20, 5, 1, C::DefaultFlag, 0)
                    << Insert(0, 0, 21, 5, 1, C::DefaultFlag | C::CacheFlag, 1))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {8,9,10,4,11,0,1,2,3,5,6,7};
        static const void *cacheLists[] = {a,a, a,a, a,a,a,a,a,a,a,a};
        static const int defaultIndexes[] = {8,9,10,4,11,0,1,2,3,5,6,7};
        static const void *defaultLists[] = {a,a, a,a, a,a,a,a,a,a,a,a};
        static const int visibleIndexes[] = {8,9,10,4,11,0,1,2,3,5,6,7};
        static const void *visibleLists[] = {a,a, a,a, a,a,a,a,a,a,a,a};
        QTest::newRow("3, 4, 5")
                << (RangeList()
                    << Range(a, 8, 4, VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 0, 2, C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 1, C::PrependFlag)
                    << Range(a, 2, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 3, 5, C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 8, 4, C::AppendFlag | C::PrependFlag))
                << C::Default << 3 << C::Default << 4 << 5
                << (RemoveList()
                    << Remove(0, 3, 3, 3, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 0)
                    << Remove(0, 3, 3, 3, 2, VisibleFlag | C::DefaultFlag | C::CacheFlag, 1)
                    << Remove(0, 3, 3, 3, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 2)
                    << Remove(0, 3, 3, 3, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 3))
                << (InsertList()
                    << Insert(0, 4, 4, 4, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 0)
                    << Insert(0, 5, 5, 5, 2, VisibleFlag | C::DefaultFlag | C::CacheFlag, 1)
                    << Insert(0, 7, 7, 7, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 2)
                    << Insert(0, 8, 8, 8, 1, VisibleFlag | C::DefaultFlag | C::CacheFlag, 3))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray(visibleIndexes) << ListArray(visibleLists)
                << IndexArray() << ListArray();
    } { static const int cacheIndexes[] = {0,1};
        static const void *cacheLists[] = {a,a};
        static const int defaultIndexes[] = {0,1};
        static const void *defaultLists[] = {a,a};
        QTest::newRow("0, 1, 1")
                << (RangeList()
                    << Range(a, 0, 1, C::PrependFlag)
                    << Range(a, 1, 1, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 0, C::AppendFlag | C::PrependFlag)
                    << Range(a, 0, 1, C::DefaultFlag | C::CacheFlag))
                << C::Default << 0 << C::Default << 1 << 1
                << (RemoveList()
                    << Remove(0, 0, 0, 0, 1, C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 1, 1, 1, C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes) << ListArray(cacheLists)
                << IndexArray(defaultIndexes) << ListArray(defaultLists)
                << IndexArray() << ListArray()
                << IndexArray() << ListArray();
    }
}

void tst_qdeclarativelistcompositor::move()
{
    QFETCH(RangeList, ranges);
    QFETCH(C::Group, fromGroup);
    QFETCH(int, from);
    QFETCH(C::Group, toGroup);
    QFETCH(int, to);
    QFETCH(int, count);
    QFETCH(RemoveList, expectedRemoves);
    QFETCH(InsertList, expectedInserts);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(ListArray, cacheLists);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(ListArray, defaultLists);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(ListArray, visibleLists);
    QFETCH(IndexArray, selectionIndexes);
    QFETCH(ListArray, selectionLists);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Remove> removes;
    QVector<C::Insert> inserts;
    compositor.move(fromGroup, from, toGroup, to, count, fromGroup, &removes, &inserts);

    QCOMPARE(removes, expectedRemoves);
    QCOMPARE(inserts, expectedInserts);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Cache, i);
        QCOMPARE(it->list, cacheLists[i]);
        if (cacheLists[i])
            QCOMPARE(it.modelIndex(), cacheIndexes[i]);
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        C::iterator it = compositor.find(C::Default, i);
        QCOMPARE(it->list, defaultLists[i]);
        if (defaultLists[i])
            QCOMPARE(it.modelIndex(), defaultIndexes[i]);
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        C::iterator it = compositor.find(Visible, i);
        QCOMPARE(it->list, visibleLists[i]);
        if (visibleLists[i])
            QCOMPARE(it.modelIndex(), visibleIndexes[i]);
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        C::iterator it = compositor.find(Selection, i);
        QCOMPARE(it->list, selectionLists[i]);
        if (selectionLists[i])
            QCOMPARE(it.modelIndex(), selectionIndexes[i]);
    }
}

void tst_qdeclarativelistcompositor::moveFromEnd()
{
    int listA; void *a = &listA;

    QDeclarativeListCompositor compositor;
    compositor.append(a, 0, 1, C::AppendFlag | C::PrependFlag | C::DefaultFlag);

    // Moving an item anchors it to that position.
    compositor.move(C::Default, 0, C::Default, 0, 1, C::Default);

    // The existing item is anchored at 0 so prepending an item to the source will append it here
    QVector<C::Insert> inserts;
    compositor.listItemsInserted(a, 0, 1, &inserts);

    QCOMPARE(inserts.count(), 1);
    QCOMPARE(inserts.at(0).index[1], 1);
    QCOMPARE(inserts.at(0).count, 1);

    C::iterator it;
    it = compositor.find(C::Default, 0);
    QCOMPARE(it.modelIndex(), 1);

    it = compositor.find(C::Default, 1);
    QCOMPARE(it.modelIndex(), 0);
}

void tst_qdeclarativelistcompositor::clear()
{
    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    int listA; void *a = &listA;
    int listB; void *b = &listB;

    compositor.append(a, 0, 8, C::AppendFlag | C::PrependFlag | VisibleFlag | C::DefaultFlag);
    compositor.append(b, 4, 5,  VisibleFlag | C::DefaultFlag);
    compositor.append(0, 0, 3,  VisibleFlag | C::DefaultFlag | C::CacheFlag);

    QCOMPARE(compositor.count(C::Default), 16);
    QCOMPARE(compositor.count(Visible), 16);
    QCOMPARE(compositor.count(C::Cache), 3);

    compositor.clear();
    QCOMPARE(compositor.count(C::Default), 0);
    QCOMPARE(compositor.count(Visible), 0);
    QCOMPARE(compositor.count(C::Cache), 0);
}

void tst_qdeclarativelistcompositor::listItemsInserted_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<void *>("list");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("count");
    QTest::addColumn<InsertList>("expectedInserts");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<IndexArray>("selectionIndexes");

    int listA; void *a = &listA;
    int listB; void *b = &listB;

    {   static const int defaultIndexes[] = {/*A*/0,1,5,6,/*B*/0,1,2,3,/*A*/2,3,4};
        QTest::newRow("A 10, 2")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << a << 10 << 2
                << InsertList()
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int defaultIndexes[] = {/*A*/0,1,5,6,/*B*/0,1,2,3,/*A*/2,3,4};
        QTest::newRow("B 10, 2")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << b << 10 << 2
                << InsertList()
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int defaultIndexes[] = {/*A*/0,1,2,3,7,8,/*B*/0,1,2,3,/*A*/4,5,6};
        static const int visibleIndexes[] = {/*A*/0,1};
        QTest::newRow("A 0, 2")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << a << 0 << 2
                << (InsertList()
                    << Insert(0, 0, 0, 0, 2, VisibleFlag | C::DefaultFlag))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray(visibleIndexes)
                << IndexArray();
    } { static const int defaultIndexes[] = {/*A*/0,1,2,3,5,8,9,/*B*/0,1,2,3,/*A*/4,6,7};
        static const int visibleIndexes[] = {/*A*/0,1,5};
        QTest::newRow("A 5, 1")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | VisibleFlag | C::DefaultFlag)
                    << Range(a, 2, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 4, 3, C::PrependFlag)
                    << Range(a, 7, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 4, 3, C::DefaultFlag))
                << a << 5 << 1
                << (InsertList()
                    << Insert(0, 2, 4, 0, 1, VisibleFlag | C::DefaultFlag))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray(visibleIndexes)
                << IndexArray();
    } { static const int defaultIndexes[] = {/*A*/0,1,2,3,5,8,9,10,11,/*B*/0,1,2,3,/*A*/4,6,7};
        static const int visibleIndexes[] = {/*A*/0,1,5,10,11};
        QTest::newRow("A 10, 2")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | VisibleFlag | C::DefaultFlag)
                    << Range(a, 2, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 4, 1, C::PrependFlag)
                    << Range(a, 5, 1, C::PrependFlag | VisibleFlag | C::DefaultFlag)
                    << Range(a, 6, 2, C::PrependFlag)
                    << Range(a, 8, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 4, 1, C::DefaultFlag)
                    << Range(a, 6, 2, C::DefaultFlag))
                << a << 10 << 2
                << (InsertList()
                    << Insert(0, 3, 7, 0, 2, VisibleFlag | C::DefaultFlag))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray(visibleIndexes)
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,-1,-1,-1,2,5,6,7,8,9};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6,7,8,9};
        static const int visibleIndexes[] = {/*A*/3,4};
        QTest::newRow("Insert after remove")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 3, C::CacheFlag)
                    << Range(a, 2, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 3 << 2
                << (InsertList()
                    << Insert(0, 0, 3, 6, 2, VisibleFlag | C::DefaultFlag))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray(visibleIndexes)
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,2,3,4};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6};
        static const int visibleIndexes[] = {/*A*/0,1,2,3,4,5,6};
        QTest::newRow("Consecutive appends")
                << (RangeList()
                    << Range(a, 0, 5, C::PrependFlag | VisibleFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 5, 1, C::PrependFlag | VisibleFlag | C::DefaultFlag)
                    << Range(a, 6, 0, C::AppendFlag | VisibleFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 6 << 1
                << (InsertList()
                    << Insert(0, 6, 6, 5, 1, VisibleFlag | C::DefaultFlag))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray(visibleIndexes)
                << IndexArray();
    }
}

void tst_qdeclarativelistcompositor::listItemsInserted()
{
    QFETCH(RangeList, ranges);
    QFETCH(void *, list);
    QFETCH(int, index);
    QFETCH(int, count);
    QFETCH(InsertList, expectedInserts);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(IndexArray, selectionIndexes);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Insert> inserts;
    compositor.listItemsInserted(list, index, count, &inserts);

    QCOMPARE(inserts, expectedInserts);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        if (cacheIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Cache, i).modelIndex(), cacheIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        if (defaultIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Default, i).modelIndex(), defaultIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        if (visibleIndexes[i] != -1) {
            QCOMPARE(compositor.find(Visible, i).modelIndex(), visibleIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        if (selectionIndexes[i] != -1) {
            QCOMPARE(compositor.find(Selection, i).modelIndex(), selectionIndexes[i]);
        }
    }
}

void tst_qdeclarativelistcompositor::listItemsRemoved_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<void *>("list");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("count");
    QTest::addColumn<RemoveList>("expectedRemoves");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<IndexArray>("selectionIndexes");

    int listA; void *a = &listA;
    int listB; void *b = &listB;

    {   static const int defaultIndexes[] = {/*A*/0,1,5,6,/*B*/0,1,2,3,/*A*/2,3,4};
        QTest::newRow("12, 2")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << a << 12 << 2
                << RemoveList()
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } {   static const int defaultIndexes[] = {/*A*/0,1,/*B*/0,1,2,3,/*A*/2,3};
        QTest::newRow("4, 3")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << a << 4 << 3
                << (RemoveList()
                    << Remove(0, 0, 2, 0, 2, C::DefaultFlag)
                    << Remove(0, 0, 8, 0, 1, C::DefaultFlag))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,-1,-1,-1,2,-1,-1,3,4,5};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5};
        QTest::newRow("Remove after remove")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 3, C::CacheFlag)
                    << Range(a, 2, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 3 << 2
                << (RemoveList()
                    << Remove(0, 0, 3, 6, 2, C::DefaultFlag | C::CacheFlag))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/-1,-1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6};
        QTest::newRow("Sparse remove")
                << (RangeList()
                    << Range(a,  0,  2, C::CacheFlag)
                    << Range(a,  0,  1, C::DefaultFlag | C::CacheFlag)
                    << Range(a,  0,  1, C::CacheFlag)
                    << Range(a,  1,  5, C::DefaultFlag | C::CacheFlag)
                    << Range(a,  0,  1, C::CacheFlag)
                    << Range(a,  6,  2, C::DefaultFlag | C::CacheFlag)
                    << Range(a,  0,  1, C::CacheFlag)
                    << Range(a,  8,  3, C::DefaultFlag | C::CacheFlag)
                    << Range(a,  0,  1, C::CacheFlag)
                    << Range(a, 11,  1, C::DefaultFlag | C::CacheFlag)
                    << Range(a, 12,  5, C::DefaultFlag))
                << a << 1 << 10
                << (RemoveList()
                    << Remove(0, 0, 1, 4, 5, C::DefaultFlag | C::CacheFlag)
                    << Remove(0, 0, 1,10, 2, C::DefaultFlag | C::CacheFlag)
                    << Remove(0, 0, 1,13, 3, C::DefaultFlag | C::CacheFlag))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    }
}

void tst_qdeclarativelistcompositor::listItemsRemoved()
{
    QFETCH(RangeList, ranges);
    QFETCH(void *, list);
    QFETCH(int, index);
    QFETCH(int, count);
    QFETCH(RemoveList, expectedRemoves);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(IndexArray, selectionIndexes);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Remove> removes;
    compositor.listItemsRemoved(list, index, count, &removes);

    QCOMPARE(removes, expectedRemoves);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        if (cacheIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Cache, i).modelIndex(), cacheIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        if (defaultIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Default, i).modelIndex(), defaultIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        if (visibleIndexes[i] != -1) {
            QCOMPARE(compositor.find(Visible, i).modelIndex(), visibleIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        if (selectionIndexes[i] != -1) {
            QCOMPARE(compositor.find(Selection, i).modelIndex(), selectionIndexes[i]);
        }
    }
}

void tst_qdeclarativelistcompositor::listItemsMoved_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<void *>("list");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<int>("count");
    QTest::addColumn<RemoveList>("expectedRemoves");
    QTest::addColumn<InsertList>("expectedInserts");
    QTest::addColumn<IndexArray>("cacheIndexes");
    QTest::addColumn<IndexArray>("defaultIndexes");
    QTest::addColumn<IndexArray>("visibleIndexes");
    QTest::addColumn<IndexArray>("selectionIndexes");

    int listA; void *a = &listA;
    int listB; void *b = &listB;

    {   static const int defaultIndexes[] = {/*A*/0,2,3,4,/*B*/0,1,2,3,/*A*/5,6,1};
        QTest::newRow("4, 1, 3")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 2, 3, C::PrependFlag)
                    << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 2, 3, C::DefaultFlag))
                << a << 4 << 1 << 3
                << (RemoveList()
                    << Remove(0, 0, 2, 0, 2, C::DefaultFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 1, 0, 2, C::DefaultFlag, 0))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int defaultIndexes[] = {/*A*/1,2,3,6,/*B*/0,1,2,3,/*A*/4,5,0};
        QTest::newRow("0, 6, 1")
                << (RangeList()
                    << Range(a, 0, 1, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 1, 1, C::PrependFlag)
                    << Range(a, 2, 3, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 5, 2, C::PrependFlag)
                    << Range(a, 7, 0, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                    << Range(b, 0, 4, C::DefaultFlag)
                    << Range(a, 5, 2, C::DefaultFlag)
                    << Range(a, 1, 1, C::DefaultFlag))
                << a << 0 << 6 << 1
                << (RemoveList()
                    << Remove(0, 0, 0, 0, 1, C::DefaultFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 3, 0, 1, C::DefaultFlag, 0))
                << IndexArray()
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,3,4};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6,7};
        QTest::newRow("6, 2, 1")
                << (RangeList()
                    << Range(a, 0, 4, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 4, 4, C::AppendFlag | C::PrependFlag | C::DefaultFlag))
                << a << 6 << 2 << 1
                << (RemoveList()
                    << Remove(0, 0, 6, 4, 1, C::DefaultFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 2, 2, 1, C::DefaultFlag, 0))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,-1,-1,-1,2,3,4,5,6,7};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6,7};
        QTest::newRow("Move after remove")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 3, C::CacheFlag)
                    << Range(a, 2, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 4 << 2 << 2
                << (RemoveList()
                    << Remove(0, 0, 4, 7, 2, C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 2, 5, 2, C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,5,6,7,8,9,10,11,12};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6,7,8,9,10,11,12};
        QTest::newRow("Move merge tail")
                << (RangeList()
                    << Range(a, 0, 10, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 10, 3, C::PrependFlag | C::DefaultFlag)
                    << Range(a, 13, 0, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 8 << 0 << 5
                << (RemoveList()
                    << Remove(0, 0, 8, 8, 2, C::DefaultFlag | C::CacheFlag, 0)
                    << Remove(0, 0, 8, 8, 3, C::DefaultFlag, 1))
                << (InsertList()
                    << Insert(0, 0, 0, 0, 2, C::DefaultFlag | C::CacheFlag, 0)
                    << Insert(0, 0, 2, 2, 3, C::DefaultFlag, 1))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    } { static const int cacheIndexes[] = {/*A*/0,1,2,3};
        static const int defaultIndexes[] = {/*A*/0,1,2,3};
        static const int selectionIndexes[] = {/*A*/3};
        QTest::newRow("Move selection")
                << (RangeList()
                    << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 2, 1, C::PrependFlag | SelectionFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 3, 1, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
                << a << 2 << 3 << 1
                << (RemoveList()
                    << Remove(0, 0, 2, 2, 1, C::PrependFlag | SelectionFlag | C::DefaultFlag | C::CacheFlag, 0))
                << (InsertList()
                    << Insert(0, 0, 3, 3, 1, C::PrependFlag | SelectionFlag | C::DefaultFlag | C::CacheFlag, 0))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray(selectionIndexes);
    } { static const int cacheIndexes[] = {/*A*/0,1,2,3,4,5,8,9};
        static const int defaultIndexes[] = {/*A*/0,1,2,3,4,5,6,7,8,9,10,11};
        QTest::newRow("move mixed cached items")
                << (RangeList()
                    << Range(a,  0,  1, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a,  1,  2, C::PrependFlag | C::DefaultFlag)
                    << Range(a,  3,  7, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                    << Range(a, 10,  2, C::PrependFlag | C::DefaultFlag))
                << a << 1 << 6 << 3
                << (RemoveList()
                    << Remove(0, 0, 1, 1, 2, C::PrependFlag | C::DefaultFlag, 0)
                    << Remove(0, 0, 1, 1, 1, C::PrependFlag | C::DefaultFlag | C::CacheFlag, 1))
                << (InsertList()
                    << Insert(0, 0, 6, 6, 2, C::PrependFlag | C::DefaultFlag, 0)
                    << Insert(0, 0, 8, 6, 1, C::PrependFlag | C::DefaultFlag | C::CacheFlag, 1))
                << IndexArray(cacheIndexes)
                << IndexArray(defaultIndexes)
                << IndexArray()
                << IndexArray();
    }
}

void tst_qdeclarativelistcompositor::listItemsMoved()
{
    QFETCH(RangeList, ranges);
    QFETCH(void *, list);
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(int, count);
    QFETCH(RemoveList, expectedRemoves);
    QFETCH(InsertList, expectedInserts);
    QFETCH(IndexArray, cacheIndexes);
    QFETCH(IndexArray, defaultIndexes);
    QFETCH(IndexArray, visibleIndexes);
    QFETCH(IndexArray, selectionIndexes);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Remove> removes;
    QVector<C::Insert> inserts;
    compositor.listItemsMoved(list, from, to, count, &removes, &inserts);

    QCOMPARE(removes, expectedRemoves);
    QCOMPARE(inserts, expectedInserts);

    QCOMPARE(compositor.count(C::Cache), cacheIndexes.count);
    for (int i = 0; i < cacheIndexes.count; ++i) {
        if (cacheIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Cache, i).modelIndex(), cacheIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(C::Default), defaultIndexes.count);
    for (int i = 0; i < defaultIndexes.count; ++i) {
        if (defaultIndexes[i] != -1) {
            QCOMPARE(compositor.find(C::Default, i).modelIndex(), defaultIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Visible), visibleIndexes.count);
    for (int i = 0; i < visibleIndexes.count; ++i) {
        if (visibleIndexes[i] != -1) {
            QCOMPARE(compositor.find(Visible, i).modelIndex(), visibleIndexes[i]);
        }
    }
    QCOMPARE(compositor.count(Selection), selectionIndexes.count);
    for (int i = 0; i < selectionIndexes.count; ++i) {
        if (selectionIndexes[i] != -1) {
            QCOMPARE(compositor.find(Selection, i).modelIndex(), selectionIndexes[i]);
        }
    }
}

void tst_qdeclarativelistcompositor::listItemsChanged_data()
{
    QTest::addColumn<RangeList>("ranges");
    QTest::addColumn<void *>("list");
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("count");
    QTest::addColumn<ChangeList>("expectedChanges");

    int listA; void *a = &listA;
    int listB; void *b = &listB;

    QTest::newRow("overlapping")
            << (RangeList()
                << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag)
                << Range(a, 2, 3, C::PrependFlag)
                << Range(a, 5, 2, C::AppendFlag | C::PrependFlag | C::DefaultFlag)
                << Range(b, 0, 4, C::DefaultFlag)
                << Range(a, 2, 3, C::DefaultFlag))
            << a << 3 << 4
            << (ChangeList()
                << Change(0, 0, 2, 0, 2, C::DefaultFlag)
                << Change(0, 0, 9, 0, 2, C::DefaultFlag));
    QTest::newRow("Change after remove")
            << (RangeList()
                << Range(a, 0, 2, C::PrependFlag | C::DefaultFlag | C::CacheFlag)
                << Range(a, 2, 3, C::CacheFlag)
                << Range(a, 2, 6, C::AppendFlag | C::PrependFlag | C::DefaultFlag | C::CacheFlag))
            << a << 3 << 2
            << (ChangeList()
                << Change(0, 0, 3, 6, 2, C::DefaultFlag | C::CacheFlag));
}

void tst_qdeclarativelistcompositor::listItemsChanged()
{
    QFETCH(RangeList, ranges);
    QFETCH(void *, list);
    QFETCH(int, index);
    QFETCH(int, count);
    QFETCH(ChangeList, expectedChanges);

    QDeclarativeListCompositor compositor;
    compositor.setGroupCount(4);
    compositor.setDefaultGroups(VisibleFlag | C::DefaultFlag);

    foreach (const Range &range, ranges)
        compositor.append(range.list, range.index, range.count, range.flags);

    QVector<C::Change> changes;
    compositor.listItemsChanged(list, index, count, &changes);

    QCOMPARE(changes, expectedChanges);
}

QTEST_MAIN(tst_qdeclarativelistcompositor)

#include "tst_qdeclarativelistcompositor.moc"


