#! /usr/bin/python3

from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, \
                            QLineEdit, QComboBox, QMessageBox, QLabel, QPushButton

from PyQt5.QtGui import QIcon

import requests
from urllib3.exceptions import HTTPError

class SignUp(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):

        self.http_session = requests.Session()
        self.http_session.verify = "utility/upark_server.crt"

        formLayout = QFormLayout()
        vbox = QVBoxLayout()

        self.email_le = QLineEdit()
        self.name_le = QLineEdit()
        self.surname_le = QLineEdit()

        hbox = QHBoxLayout()

        self.password_le = QLineEdit()
        self.password_le.setEchoMode(QLineEdit.Password)

        self.password_eye_btn = QPushButton(QIcon("utility/pictures/eye.svg"),"")
        self.password_eye_btn.clicked.connect(self.showPassword)

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


    def hideEvent(self, event):
        self.email_le.clear()
        self.name_le.clear()
        self.surname_le.clear()
        self.password_le.clear()
        self.password_le.setEchoMode(QLineEdit.Password)
        self.user_category_cmb.setCurrentIndex(0)
        self.upark_code_le.clear()


    def showPassword(self):
        if self.password_le.echoMode() == QLineEdit.Password:
            self.password_le.setEchoMode(QLineEdit.Normal)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/closed_eye.svg"))
        else:
            self.password_le.setEchoMode(QLineEdit.Password)
            self.password_eye_btn.setIcon(QIcon("utility/pictures/eye.svg"))


    def load_user_categories(self):
        #https://localhost:50050/apis/user_categories
        try:
            response = self.http_session.get("https://localhost:50050/apis/user_categories")
            # If the response was successful, no Exception will be raised
            response.raise_for_status()
        except HTTPError as http_err:
            print(f"HTTP error occurred: {http_err}")
        except Exception as err:
            QMessageBox.critical(None, "Alert", "Server error. Retry later.")           # parent = None to display QMessageBox centered
            raise Exception("Error")
            #print(f"Other error occurred: {err}")
        else:
            self.user_categories = response.json()         # http response body as json
            self.user_category_cmb.addItems([user_category["name"] for user_category in self.user_categories if user_category["name"]!="Admin"])


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
            response = self.http_session.post('https://localhost:50050/apis/users/signup', json = user_dict)
            response.raise_for_status()
        except HTTPError as http_err:
            QMessageBox.critical(self, "Sign-up response", response.text)
        except Exception as err:
            QMessageBox.critical(self, "Sign-up response", str(err))
        else:
            response_json = response.json()         # http response body as json
            QMessageBox.about(self, "Sign-up response", response_json["message"])
