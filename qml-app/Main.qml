import QtQuick
import QtQuick.Controls

Window {
    visible: true
    width: 128
    height: 160
    color: "#103048"

    Text {
        id: clock
        y: 4
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: Qt.formatDateTime(new Date(), "HH:mm:ss")
        color: "#ffd866"
        font.pixelSize: 16
        font.bold: true
    }

    // поле ввода: фокус по клику, выделение текста мышью; клавиатуры
    // в плагине пока нет, ввод не приходит
    TextInput {
        id: input
        x: 4
        y: 26
        width: 120
        height: 22
        padding: 3
        text: "select me"
        color: "#103048"
        font.pixelSize: 12
        activeFocusOnPress: true
        selectByMouse: true

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: "#f0f0f0"
            border.color: input.activeFocus ? "#7dff7d" : "#5fd0ff"
            border.width: input.activeFocus ? 2 : 1
            z: -1
        }
    }

    // прокрутка колесом мыши
    ScrollView {
        id: scroll
        y: 52
        width: parent.width
        height: 70
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        Column {
            spacing: 2
            Repeater {
                model: 16
                delegate: Text {
                    text: "line " + index
                    color: index % 2 ? "#7dff7d" : "#5fd0ff"
                    font.pixelSize: 12
                }
            }
        }
    }

    Button {
        id: clickButton
        x: 4
        y: 126
        width: 120
        height: 30
        text: "clicked: " + counter.text
        font.pixelSize: 11

        onClicked: {
            counter.text = Number(counter.text) + 1 + "";
        }
    }

    Text {
        id: counter
        y: 158
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: "0"
        color: "#ff8800"
        font.pixelSize: 10
    }
}
