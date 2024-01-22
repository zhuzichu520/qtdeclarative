/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmlextensionplugin.h"
#include "qqmlextensionplugin_p.h"

QT_BEGIN_NAMESPACE

/*!
    \since 5.0
    \inmodule QtQml
    \class QQmlExtensionPlugin
    \brief The QQmlExtensionPlugin class provides an abstract base for custom QML extension plugins
           with custom type registration functions.

    \ingroup plugins

    \note If you need to write a plugin manually (which is rare) you should always use
    \l{QQmlEngineExtensionPlugin}. QQmlExtensionPlugin only provides the registerTypes() and
    unregisterTypes() functions in addition. You should not use them, but rather declare your
    types with \l{QML_ELEMENT} and friends and have the build system take care of the registration.
*/

/*!
    \since 5.14
    \inmodule QtQml
    \class QQmlEngineExtensionPlugin
    \brief The QQmlEngineExtensionPlugin class provides an abstract base for custom QML extension
           plugins.

    \ingroup plugins

    \include qqmlextensionplugin.qdocinc

    The \l {Writing QML Extensions with C++} tutorial also contains a chapter
    on creating QML plugins.

    \sa QQmlEngine::importPlugin(), {How to Create Qt Plugins}
*/

/*!
    \fn void QQmlExtensionPlugin::registerTypes(const char *uri)

    Registers the QML types in the given \a uri. Subclasses should implement
    this to call qmlRegisterType() for all types which are provided by the extension
    plugin.

    The \a uri is an identifier for the plugin generated by the QML engine
    based on the name and path of the extension's plugin library.
*/

/*!
    \internal
*/
QQmlExtensionPlugin::QQmlExtensionPlugin(QObject *parent)
    : QObject(*(new QQmlExtensionPluginPrivate), parent)
{
}

/*!
    Constructs a QML extension plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_PLUGIN_METADATA() macro, so there is no need for calling it
    explicitly.
 */
QQmlEngineExtensionPlugin::QQmlEngineExtensionPlugin(QObject *parent)
    : QObject(parent)
{
}


/*!
  \internal
 */
QQmlExtensionPlugin::~QQmlExtensionPlugin() = default;

/*!
  \internal
 */
QQmlEngineExtensionPlugin::~QQmlEngineExtensionPlugin() = default;

/*!
    \since 5.1
    \internal
    \brief Returns the URL of the directory from which the extension is loaded.

    This is useful when the plugin also needs to load QML files or other
    assets from the same directory.
*/
QUrl QQmlExtensionPlugin::baseUrl() const
{
    Q_D(const QQmlExtensionPlugin);
    return d->baseUrl;
}

/*!
  \since 6.0

  Override this method to unregister types manually registered in registerTypes.
*/
void QQmlExtensionPlugin::unregisterTypes()
{

}

/*!
    Initializes the extension from the \a uri using the \a engine. Here an application
    plugin might, for example, expose some data or objects to QML,
    as context properties on the engine's root context.
 */
void QQmlExtensionPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);
}

/*!
    Initializes the extension from the \a uri using the \a engine. Here an application
    plugin might, for example, expose some data or objects to QML,
    as context properties on the engine's root context.
 */
void QQmlEngineExtensionPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);
}

/*!
  \class QQmlExtensionInterface
  \internal
  \inmodule QtQml
*/

/*!
  \class QQmlTypesExtensionInterface
  \internal
  \inmodule QtQml
*/

/*!
  \class QQmlEngineExtensionInterface
  \internal
  \inmodule QtQml
*/


/*!
    \macro Q_IMPORT_QML_PLUGIN(PluginName)
    \since 6.2
    \relates QQmlEngineExtensionPlugin

    Ensures the plugin whose metadata-declaring class is named \a PluginName
    is linked into static builds.

    \sa Q_IMPORT_PLUGIN
*/

QT_END_NAMESPACE

#include "moc_qqmlextensionplugin.cpp"
