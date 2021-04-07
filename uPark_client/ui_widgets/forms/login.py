#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, \
                            QLineEdit, QComboBox, QMessageBox, QLabel, QPushButton, \
                            QDesktopWidget

from PyQt5.QtGui import QIcon

from PyQt5.QtCore import pyqtSignal, QObject

import requests

from entities.user import User
from convenience_functions.server_apis import get_user_categories
from ui_widgets.user.user_dashboard import userDashboard
from ui_widgets.admin.admin_dashboard import adminDashboard

class Login_signals(QObject):
    close = pyqtSignal()
    home_close = pyqtSignal()

class Login(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.signals = Login_signals()

        self.user_dashboard = None
        self.admin_dashboard = None

        self.https_session = requests.Session()
        self.https_session.verify = "utility/upark_server.crt"

        formLayout = QFormLayout()
        vbox = QVBoxLayout()

        self.email_le = QLineEdit()

        hbox = QHBoxLayout()

        self.password_le = QLineEdit()
        self.password_le.setEchoMode(QLineEdit.Password)

        self.password_eye_btn = QPushButton(QIcon("utility/pictures/eye.svg"),"")
        self.password_eye_btn.clicked.connect(self.show_password)

        hbox.addWidget(self.password_le)
        hbox.addWidget(self.password_eye_btn)

        self.login_btn = QPushButton("Login!")
        self.login_btn.clicked.connect(self.login_submit)

        formLayout.addRow("Email: ", self.email_le)
        formLayout.addRow("Password: ",  hbox)

        vbox.addLayout(formLayout)

        vbox.addWidget(self.login_btn)

        self.setLayout(vbox)

        self.setFixedSize(500,150)
        self.setWindowTitle('Login')
        self.move(QDesktopWidget().availableGeometry().center() - self.frameGeometry().center())


    def hideEvent(self, event):
        self.email_le.clear()
        self.password_le.clear()
        self.password_le.setEchoMode(QLineEdit.Password)


    def show_password(self):
        if self.password_le.echoMode() == QLineEdit.Password:
            self.password_le.setEchoMode(QLineEdit.Normal)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/closed_eye.svg"))
        else:
            self.password_le.setEchoMode(QLineEdit.Password)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/eye.svg"))


    def login_submit(self):

        auth_info = (self.email_le.text(), self.password_le.text())

        try:
            response = requests.get('https://localhost:50050/apis/login', verify = "utility/upark_server.crt", auth = auth_info)
            response.raise_for_status()

        except (requests.exceptions.HTTPError, Exception) as err:
            message = response.text if isinstance(err, requests.exceptions.HTTPError) else str(err)
            QMessageBox.critical(self, "Login response", message)

        else:
            response_json = response.json()         # https response body as json
            QMessageBox.about(self, "Login response", response_json["message"])

            del response_json["message"]                      # call del to make the response_json matches with User constructor
            user = User(**response_json)                      # dictionary unpacking

            print(user)
            # user = User(response_json["id"], response_json["email"], response_json["name"],
            # response_json["surname"], response_json["password"], response_json["wallet"], response_json["disability"],
            # response_json["active_account"], response_json["id_user_category"])

            results = get_user_categories(self.https_session)
            user_categories = results[0]

            if user_categories is None:                         # something went wrong with https request
                message = results[1]                            # message is a tuple
                print(f"HTTP error occurred: {message[0]}")
                QMessageBox.critical(None, "Alert", message[1])
                self.signals.close.emit()

            else:
                id_admin = list(filter(lambda x: x["name"] == "Admin", user_categories))[0]["id"]

                if user.get_id_user_category() == id_admin:
                    self.admin_dashboard = adminDashboard(user)      #aggiungere la sessione
                else:
                    self.user_dashboard = userDashboard(user)

                self.signals.home_close.emit()
        finally:
            self.signals.close.emit()
