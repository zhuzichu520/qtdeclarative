import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 2.0

Window {
    visible: true

    TextArea {
        id: textarea
        objectName: "textarea"
        text: "TextArea"
        Accessible.name: text
    }
}
