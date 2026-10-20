import QtQuick
import QtQuick.Controls

Window {
    visible: true
    width: 128
    height: 160
    color: "#103048"

    Text {
        y: 4
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: "QPA ST7735"
        color: "#ffffff"
        font.pixelSize: 13
        font.bold: true
    }

    Text {
        id: clock
        y: 26
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: Qt.formatDateTime(new Date(), "HH:mm:ss")
        color: "#ffd866"
        font.pixelSize: 22
    }

    Text {
        id: ticks
        y: 56
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: "ticks: 0"
        color: "#7dff7d"
        font.pixelSize: 13
    }

    Button {
        id: clickButton
        x: 29
        y: 80
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
        y: 120
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: "0"
        color: "#ff8800"
        font.pixelSize: 26
    }

    Text {
        id: footer
        y: 150
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: "Orange Pi Zero 3"
        color: "#5fd0ff"
        font.pixelSize: 10
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
