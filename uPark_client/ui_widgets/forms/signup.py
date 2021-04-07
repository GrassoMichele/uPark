#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, \
                            QLineEdit, QComboBox, QMessageBox, QLabel, QPushButton, \
                            QDesktopWidget

from PyQt5.QtGui import QIcon

from PyQt5.QtCore import pyqtSignal, QObject

import requests

from convenience_functions.server_apis import get_user_categories

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


        self.signup_btn = QPushButton("Sign-up!")
        self.signup_btn.clicked.connect(self.signup_submit)

        self.user_category_cmb = QComboBox()

        # get categories
        self.load_user_categories()


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
            results = get_user_categories(self.https_session)
            self.user_categories = results[0]

            if self.user_categories is None:                         # something went wrong with https request
                message =  results[1]                                # message is a tuple
                print(f"HTTP error occurred: {message[0]}")
                QMessageBox.critical(None, "Alert", message[1])
                raise Exception("Http Error")

            else:
                self.user_category_cmb.addItems([user_category["name"] for user_category in self.user_categories if user_category["name"] != "Admin"])


    def signup_submit(self):
        # searching for selected user_category info
        category_dict = list(filter(lambda x: x["name"]==self.user_category_cmb.currentText(), self.user_categories))[0]

        user_dict = {
                    "email": self.email_le.text(),
                    "name": self.name_le.text(),
                    "surname": self.surname_le.text(),
                    "password": self.password_le.text(),
                    "id_user_category": category_dict["id"],
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
