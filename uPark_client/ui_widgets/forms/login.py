#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, \
                            QLineEdit, QComboBox, QMessageBox, QLabel, QPushButton, \
                            QDesktopWidget

from PyQt5.QtGui import QIcon

from PyQt5.QtCore import pyqtSignal, QObject

import requests

from entities.user import User
from entities.user_category import UserCategory
from convenience_functions.server_apis import make_http_request
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

        self.https_session.auth = (self.email_le.text(), self.password_le.text())

        response = make_http_request(self.https_session, "get", "login")

        if response:
            response_json = response.json()                   # https response body as json
            QMessageBox.about(self, "Login", response_json["message"])

            del response_json["message"]                      # call del to make the response_json matches with User constructor
            user = User(**response_json)                      # dictionary unpacking

            #print(user)

            response_user_categories = make_http_request(self.https_session, "get", "user_categories", show_messagebox = False)

            if response_user_categories:
                user_categories = [UserCategory(**user_category) for user_category in response_user_categories.json()]

                try:
                    admin_list_index = user_categories.index(UserCategory(name = "Admin"))
                    admin_id = user_categories[admin_list_index].get_id()

                    if user.get_id_user_category() == admin_id:
                        self.admin_dashboard = adminDashboard(user)
                    else:
                        self.user_dashboard = userDashboard(user)

                except ValueError:
                    print("Error: No admin found in user_categories. Some problem occured in DB uPark!")

                finally:
                    self.signals.home_close.emit()

            else:
                QMessageBox.critical(None, "Alert", "Server error. Retry later.")
                #self.signals.close.emit()


        self.signals.close.emit()
