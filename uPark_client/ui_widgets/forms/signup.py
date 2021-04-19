#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, \
                            QLineEdit, QComboBox, QMessageBox, QLabel, QPushButton, \
                            QDesktopWidget

from PyQt5.QtGui import QIcon

from PyQt5.QtCore import pyqtSignal, QObject, QTimer

import requests, time

from convenience.server_apis import make_http_request
from entities.user_category import UserCategory

# signal used to delete the signup instance from home class in case of exceptions (e.g. server down )
class SignUp_signals(QObject):
    close = pyqtSignal()

class SignUp(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.signals = SignUp_signals()

        self.https_session = requests.Session()
        self.https_session.verify = "utility/upark_server.crt"

        formLayout = QFormLayout()
        vbox = QVBoxLayout()

        self.email_le = QLineEdit()
        self.name_le = QLineEdit()
        self.surname_le = QLineEdit()

        hbox = QHBoxLayout()

        self.password_le = QLineEdit()
        self.password_le.setEchoMode(QLineEdit.Password)

        self.password_eye_btn = QPushButton(QIcon("utility/pictures/eye.svg"),"")
        self.password_eye_btn.clicked.connect(self.show_password)

        hbox.addWidget(self.password_le)
        hbox.addWidget(self.password_eye_btn)

        self.signup_btn = QPushButton("Sign-Up")
        self.signup_btn.clicked.connect(self.signup_submit)

        self.user_category_cmb = QComboBox()

        self.upark_code_le = QLineEdit()

        formLayout.addRow("Email: ", self.email_le)
        formLayout.addRow("Name: ",  self.name_le)
        formLayout.addRow("Surname: ",  self.surname_le)
        formLayout.addRow("Password: ",  hbox)
        formLayout.addRow("Category: ",  self.user_category_cmb)
        formLayout.addRow("uPark Code: ", self.upark_code_le)

        vbox.addLayout(formLayout)

        vbox.addWidget(self.signup_btn)

        self.setLayout(vbox)

        self.setFixedSize(500,300)
        self.setWindowTitle('Signup')
        self.move(QDesktopWidget().availableGeometry().center() - self.frameGeometry().center())


    def hideEvent(self, event):
        self.email_le.clear()
        self.name_le.clear()
        self.surname_le.clear()
        self.password_le.clear()
        self.password_le.setEchoMode(QLineEdit.Password)
        self.user_category_cmb.setCurrentIndex(0)
        self.upark_code_le.clear()


    def show_password(self):
        if self.password_le.echoMode() == QLineEdit.Password:
            self.password_le.setEchoMode(QLineEdit.Normal)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/closed_eye.svg"))
        else:
            self.password_le.setEchoMode(QLineEdit.Password)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/eye.svg"))


    def load_user_categories(self):
            response = make_http_request(self.https_session, "get", "user_categories", show_messagebox = False)

            if response:
                self.user_categories = [UserCategory(**user_category) for user_category in response.json() if user_category["name"] != "Admin"]
                self.user_category_cmb.clear()
                self.user_category_cmb.addItems([user_category.get_name() for user_category in self.user_categories])

            else:
                QMessageBox.critical(None, "Alert", "Server error. Retry later.")
                raise Exception("Http Error")


    def showEvent(self, event):
        try:
            self.load_user_categories()
        except Exception:
            QTimer.singleShot(10,self.hide)   # after 10 milliseconds hide the signup QtWidget


    def signup_submit(self):
        # searching for selected user_category info
        user_category_selected = self.user_categories[self.user_category_cmb.currentIndex()]

        print(user_category_selected)


        user_dict = {
                    "email": self.email_le.text(),
                    "name": self.name_le.text(),
                    "surname": self.surname_le.text(),
                    "password": self.password_le.text(),
                    "id_user_category": user_category_selected.get_id(),
                    "upark_code": self.upark_code_le.text()
                    }

        try:
            response = self.https_session.post('https://localhost:50050/apis/users/signup', json = user_dict)
            response.raise_for_status()

        except (requests.exceptions.HTTPError, Exception) as err:
            message = response.text if isinstance(err, requests.exceptions.HTTPError) else str(err)
            QMessageBox.critical(self, "Sign-up response", message)
            self.signals.close.emit()

        else:
            response_json = response.json()         # https response body as json
            QMessageBox.about(self, "Sign-up response", response_json["message"])
            self.signals.close.emit()
