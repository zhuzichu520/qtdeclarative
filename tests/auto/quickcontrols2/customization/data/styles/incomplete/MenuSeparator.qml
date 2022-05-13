// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T

T.MenuSeparator {
    id: control
    objectName: "menuseparator-incomplete"

    contentItem: Item {
        objectName: "menuseparator-contentItem-incomplete"
    }

    background: Item {
        objectName: "menuseparator-background-incomplete"
    }
}
