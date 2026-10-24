import QtQuick
import QtQuick.Controls

Window {
    visible: true
    width: 128
    height: 160
    color: "#103048"

    // весь интерфейс прокручивается колесом мыши; контент выше экрана
    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Column {
            spacing: 4
            topPadding: 4
            width: 128

            Text {
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "QPA ST7735"
                color: "#ffffff"
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                id: clock
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: Qt.formatDateTime(new Date(), "HH:mm:ss")
                color: "#ffd866"
                font.pixelSize: 22
            }

            Text {
                id: ticks
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "ticks: 0"
                color: "#7dff7d"
                font.pixelSize: 13
            }

            Button {
                id: clickButton
                anchors.horizontalCenter: parent.horizontalCenter
                width: 70
                height: 32
                text: "Click me"
                font.pixelSize: 11

                onClicked: {
                    counter.text = Number(counter.text) + 1 + "";
                    footer.color = "#ff8800";
                }
            }

            Text {
                id: counter
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "0"
                color: "#ff8800"
                font.pixelSize: 26
            }

            Text {
                id: footer
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "Orange Pi Zero 3"
                color: "#5fd0ff"
                font.pixelSize: 10
            }

            TextInput {
                id: input
                x: 4
                width: 120
                height: 22
                padding: 3
                text: "type here"
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

            Text {
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "scroll + type below"
                color: "#8899aa"
                font.pixelSize: 10
            }

            Text {
                width: 128
                horizontalAlignment: Text.AlignHCenter
                text: "end of demo"
                color: "#8899aa"
                font.pixelSize: 10
            }
        }
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: clock.text = Qt.formatDateTime(new Date(), "HH:mm:ss")
    }

    Timer {
        interval: 500
        running: true
        repeat: true
        property int count: 0
        onTriggered: {
            count += 1;
            ticks.text = "ticks: " + count;
        }
    }
}
