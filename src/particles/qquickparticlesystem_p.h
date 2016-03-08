/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick/QQuickItem>
#include <QElapsedTimer>
#include <QVector>
#include <QHash>
#include <QPointer>
#include <QSignalMapper>
#include <private/qquicksprite_p.h>
#include <QAbstractAnimation>
#include <QtQml/qqml.h>
#include <private/qv8engine_p.h> //For QQmlV4Handle
#include "qtquickparticlesglobal_p.h"

QT_BEGIN_NAMESPACE

template<class T, int Prealloc>
class QQuickParticleVarLengthArray: public QVarLengthArray<T, Prealloc>
{
public:
    void insert(const T &element)
    {
        if (!this->contains(element)) {
            this->append(element);
        }
    }

    bool removeOne(const T &element)
    {
        for (int i = 0; i < this->size(); ++i) {
            if (this->at(i) == element) {
                this->remove(i);
                return true;
            }
        }

        return false;
    }
};

class QQuickParticleSystem;
class QQuickParticleAffector;
class QQuickParticleEmitter;
class QQuickParticlePainter;
class QQuickParticleData;
class QQuickParticleSystemAnimation;
class QQuickStochasticEngine;
class QQuickSprite;
class QQuickV4ParticleData;
class QQuickParticleGroup;
class QQuickImageParticle;

struct QQuickParticleDataHeapNode{
    int time;//in ms
    QSet<QQuickParticleData*> data;//Set ptrs instead?
};

class Q_QUICKPARTICLES_PRIVATE_EXPORT QQuickParticleDataHeap {
    //Idea is to do a binary heap, but which also stores a set of int,Node* so that if the int already exists, you can
    //add it to the data* list. Pops return the whole list at once.
public:
    QQuickParticleDataHeap();
    void insert(QQuickParticleData* data);
    void insertTimed(QQuickParticleData* data, int time);

    int top();

    QSet<QQuickParticleData*> pop();

    void clear();

    bool contains(QQuickParticleData*);//O(n), for debugging purposes only
private:
    void grow();
    void swap(int, int);
    void bubbleUp(int);
    void bubbleDown(int);
    int m_size;
    int m_end;
    QQuickParticleDataHeapNode m_tmp;
    QVector<QQuickParticleDataHeapNode> m_data;
    QHash<int,int> m_lookups;
};

class Q_QUICKPARTICLES_PRIVATE_EXPORT QQuickParticleGroupData {
public: // types
    typedef int ID;
    enum { InvalidID = -1, DefaultGroupID = 0 };

public:
    QQuickParticleGroupData(const QString &name, QQuickParticleSystem* sys);
    ~QQuickParticleGroupData();

    int size()
    { return m_size; }

    QString name();

    void setSize(int newSize);

    const ID index;
    QQuickParticleVarLengthArray<QQuickParticlePainter*, 4> painters;//TODO: What if they are dynamically removed?

    //TODO: Refactor particle data list out into a separate class
    QVector<QQuickParticleData*> data;
    QSet<int> reusableIndexes;
    QQuickParticleDataHeap dataHeap;
    bool recycle(); //Force recycling round, returns true if all indexes are now reusable

    void initList();
    void kill(QQuickParticleData* d);

    //After calling this, initialize, then call prepareRecycler(d)
    QQuickParticleData* newDatum(bool respectsLimits);

    //TODO: Find and clean up those that don't get added to the recycler (currently they get lost)
    void prepareRecycler(QQuickParticleData* d);

private:
    int m_size;
    QQuickParticleSystem* m_system;
};

struct Color4ub {
    uchar r;
    uchar g;
    uchar b;
    uchar a;
};

class Q_QUICKPARTICLES_PRIVATE_EXPORT QQuickParticleData {
public:
    //TODO: QObject like memory management (without the cost, just attached to system)
    QQuickParticleData();
    ~QQuickParticleData();

    QQuickParticleData(const QQuickParticleData &other);
    QQuickParticleData &operator=(const QQuickParticleData &other);

    //Convenience functions for working backwards, because parameters are from the start of particle life
    //If setting multiple parameters at once, doing the conversion yourself will be faster.

    //sets the x accleration without affecting the instantaneous x velocity or position
    void setInstantaneousAX(qreal ax, QQuickParticleSystem *particleSystem);
    //sets the x velocity without affecting the instantaneous x postion
    void setInstantaneousVX(qreal vx, QQuickParticleSystem *particleSystem);
    //sets the instantaneous x postion
    void setInstantaneousX(qreal x, QQuickParticleSystem *particleSystem);
    //sets the y accleration without affecting the instantaneous y velocity or position
    void setInstantaneousAY(qreal ay, QQuickParticleSystem *particleSystem);
    //sets the y velocity without affecting the instantaneous y postion
    void setInstantaneousVY(qreal vy, QQuickParticleSystem *particleSystem);
    //sets the instantaneous Y postion
    void setInstantaneousY(qreal y, QQuickParticleSystem *particleSystem);

    //TODO: Slight caching?
    qreal curX(QQuickParticleSystem *particleSystem) const;
    qreal curVX(QQuickParticleSystem *particleSystem) const;
    qreal curAX() const { return ax; }
    qreal curAX(QQuickParticleSystem *) const { return ax; } // used by the macros in qquickv4particledata.cpp
    qreal curY(QQuickParticleSystem *particleSystem) const;
    qreal curVY(QQuickParticleSystem *particleSystem) const;
    qreal curAY() const { return ay; }
    qreal curAY(QQuickParticleSystem *) const { return ay; } // used by the macros in qquickv4particledata.cpp

    int index;
    int systemIndex;

    //General Position Stuff
    float x;
    float y;
    float t;
    float lifeSpan;
    float size;
    float endSize;
    float vx;
    float vy;
    float ax;
    float ay;

    //Painter-specific stuff, now universally shared
    //Used by ImageParticle color mode
    Color4ub color;
    //Used by ImageParticle deform mode
    float xx;
    float xy;
    float yx;
    float yy;
    float rotation;
    float rotationVelocity;
    float autoRotate;//Assume that GPUs prefer floats to bools
    //Used by ImageParticle Sprite mode
    float animIdx;
    float frameDuration;
    float frameAt;//Used for duration -1
    float frameCount;
    float animT;
    float animX;
    float animY;
    float animWidth;
    float animHeight;

    QQuickParticleGroupData::ID groupId;

    //Used by ImageParticle data shadowing
    QQuickImageParticle* colorOwner;
    QQuickImageParticle* rotationOwner;
    QQuickImageParticle* deformationOwner;
    QQuickImageParticle* animationOwner;

    //Used by ItemParticle
    QQuickItem* delegate;
    int modelIndex;
    //Used by custom affectors
    float update;
    //Used by CustomParticle
    float r;

    // 4 bytes wasted


    void debugDump(QQuickParticleSystem *particleSystem);
    bool stillAlive(QQuickParticleSystem *particleSystem);//Only checks end, because usually that's all you need and it's a little faster.
    bool alive(QQuickParticleSystem *particleSystem);
    float lifeLeft(QQuickParticleSystem *particleSystem);
    float curSize(QQuickParticleSystem *particleSystem);
    void clone(const QQuickParticleData& other);//Not =, leaves meta-data like index
    QQmlV4Handle v4Value(QQuickParticleSystem *particleSystem);
    void extendLife(float time, QQuickParticleSystem *particleSystem);

    static inline Q_DECL_CONSTEXPR qreal EPSILON() Q_DECL_NOTHROW { return 0.001; }

private:
    QQuickV4ParticleData* v8Datum;
};

class Q_QUICKPARTICLES_PRIVATE_EXPORT QQuickParticleSystem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    explicit QQuickParticleSystem(QQuickItem *parent = 0);
    ~QQuickParticleSystem();

    bool isRunning() const
    {
        return m_running;
    }

    int count(){ return particleCount; }

    static const int maxLife = 600000;

Q_SIGNALS:

    void systemInitialized();
    void runningChanged(bool arg);
    void pausedChanged(bool arg);
    void emptyChanged(bool arg);

public Q_SLOTS:
    void start(){setRunning(true);}
    void stop(){setRunning(false);}
    void restart(){setRunning(false);setRunning(true);}
    void pause(){setPaused(true);}
    void resume(){setPaused(false);}

    void reset();
    void setRunning(bool arg);
    void setPaused(bool arg);

    virtual int duration() const { return -1; }


protected:
    //This one only once per frame (effectively)
    void componentComplete();

private Q_SLOTS:
    void emittersChanged();
    void loadPainter(QObject* p);
    void createEngine(); //Not invoked by sprite engine, unlike Sprite uses
    void particleStateChange(int idx);

public:
    //These can be called multiple times per frame, performance critical
    void emitParticle(QQuickParticleData* p, QQuickParticleEmitter *particleEmitter);
    QQuickParticleData* newDatum(int groupId, bool respectLimits = true, int sysIdx = -1);
    void finishNewDatum(QQuickParticleData*);
    void moveGroups(QQuickParticleData *d, int newGIdx);
    int nextSystemIndex();

    //This one only once per painter per frame
    int systemSync(QQuickParticlePainter* p);

    //Data members here for ease of related class and auto-test usage. Not "public" API. TODO: d_ptrize
    QSet<QQuickParticleData*> needsReset;
    QVector<QQuickParticleData*> bySysIdx; //Another reference to the data (data owned by group), but by sysIdx
    QQuickStochasticEngine* stateEngine;

    QHash<QString, int> groupIds;
    QVarLengthArray<QQuickParticleGroupData*, 32> groupData;
    int nextFreeGroupId;
    int registerParticleGroupData(const QString &name, QQuickParticleGroupData *pgd);

    //Also only here for auto-test usage
    void updateCurrentTime( int currentTime );
    QQuickParticleSystemAnimation* m_animation;
    bool m_running;
    bool m_debugMode;

    int timeInt;
    bool initialized;
    int particleCount;

    void registerParticlePainter(QQuickParticlePainter* p);
    void registerParticleEmitter(QQuickParticleEmitter* e);
    void registerParticleAffector(QQuickParticleAffector* a);
    void registerParticleGroup(QQuickParticleGroup* g);

    static void statePropertyRedirect(QQmlListProperty<QObject> *prop, QObject *value);
    static void stateRedirect(QQuickParticleGroup* group, QQuickParticleSystem* sys, QObject *value);
    bool isPaused() const
    {
        return m_paused;
    }

    bool isEmpty() const
    {
        return m_empty;
    }

private:
    void searchNextFreeGroupId();

private:
    void initializeSystem();
    void initGroups();
    QList<QPointer<QQuickParticleEmitter> > m_emitters;
    QList<QPointer<QQuickParticleAffector> > m_affectors;
    QList<QPointer<QQuickParticlePainter> > m_painters;
    QList<QPointer<QQuickParticlePainter> > m_syncList;
    QList<QQuickParticleGroup*> m_groups;
    int m_nextIndex;
    QSet<int> m_reusableIndexes;
    bool m_componentComplete;

    QSignalMapper m_painterMapper;
    QSignalMapper m_emitterMapper;
    bool m_paused;
    bool m_allDead;
    bool m_empty;
};

// Internally, this animation drives all the timing. Painters sync up in their updatePaintNode
class QQuickParticleSystemAnimation : public QAbstractAnimation
{
    Q_OBJECT
public:
    QQuickParticleSystemAnimation(QQuickParticleSystem* system)
        : QAbstractAnimation(static_cast<QObject*>(system)), m_system(system)
    { }
protected:
    virtual void updateCurrentTime( int t )
    {
        m_system->updateCurrentTime(t);
    }

    virtual int duration() const
    {
        return -1;
    }

private:
    QQuickParticleSystem* m_system;
};

inline bool QQuickParticleData::stillAlive(QQuickParticleSystem* system)
{
    if (!system)
        return false;
    return (t + lifeSpan - EPSILON()) > ((qreal)system->timeInt/1000.0);
}

inline bool QQuickParticleData::alive(QQuickParticleSystem* system)
{
    if (!system)
        return false;
    qreal st = ((qreal)system->timeInt/1000.0);
    return (t + EPSILON()) < st && (t + lifeSpan - EPSILON()) > st;
}

QT_END_NAMESPACE

#endif // PARTICLESYSTEM_H


