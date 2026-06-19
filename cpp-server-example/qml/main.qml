import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    objectName: "mainWindow"
    property string objID: "main_window_001"
    width: 800
    height: 600
    visible: true
    title: "QtAutoTest Demo"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Label {
            text: "QtAutoTest Demo"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        GroupBox {
            title: "User Information"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    spacing: 10
                    Label { text: "Username:"; Layout.preferredWidth: 100 }
                    TextField {
                        id: usernameField
                        objectName: "usernameField"
                        property string objID: "input_username_001"
                        placeholderText: "Enter username"
                        Layout.fillWidth: true
                        implicitHeight: 40
                    }
                }

                RowLayout {
                    spacing: 10
                    Label { text: "Password:"; Layout.preferredWidth: 100 }
                    TextField {
                        id: passwordField
                        objectName: "passwordField"
                        property string objID: "input_password_001"
                        placeholderText: "Enter password"
                        echoMode: TextInput.Password
                        Layout.fillWidth: true
                        implicitHeight: 40
                    }
                }

                CheckBox {
                    id: rememberCheckbox
                    objectName: "rememberCheckbox"
                    property string objID: "checkbox_remember_001"
                    text: "Remember me"
                }
            }
        }

        Button {
            id: loginButton
            objectName: "loginButton"
            property string objID: "btn_login_001"
            text: "Login"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50

            onClicked: {
                if (usernameField.text === "admin" && passwordField.text === "password") {
                    loginSuccessDialog.open()
                } else {
                    loginFailedDialog.open()
                }
            }
        }

        Label {
            objectName: "statusLabel"
            property string objID: "label_status_001"
            text: "Ready"
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Dialog {
        id: loginSuccessDialog
        objectName: "loginSuccessDialog"
        property string objID: "dialog_login_success_001"
        title: "Success"
        modal: true
        anchors.centerIn: parent

        Label {
            objectName: "loginSuccessMessage"
            property string objID: "label_login_success_001"
            text: "Welcome, admin!"
        }

        standardButtons: Dialog.Ok
    }

    Dialog {
        id: loginFailedDialog
        objectName: "loginFailedDialog"
        property string objID: "dialog_login_failed_001"
        title: "Error"
        modal: true
        anchors.centerIn: parent

        Label {
            objectName: "loginFailedMessage"
            property string objID: "label_login_failed_001"
            text: "Invalid username or password!"
        }

        standardButtons: Dialog.Ok
    }
}
