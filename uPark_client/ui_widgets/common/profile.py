#!/usr/bin/python
from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox, QPushButton, QFormLayout, QFrame, \
                            QVBoxLayout, QHBoxLayout, QDialogButtonBox, QInputDialog, QLineEdit \

from PyQt5.QtCore import Qt

from PyQt5.QtGui import QPixmap

from convenience_functions.server_apis import make_http_request, user_is_admin

from entities.user import User
from entities.user_category import UserCategory

import requests


class Profile(QWidget):

    def __init__(self, https_session, user):
        super().__init__()
        self.https_session = https_session
        self.user = user
        self.initUI()

    def initUI(self):

        title = "Profile"

        vbox_main = QVBoxLayout()
        vbox_main.setSpacing(30)
        vbox_main.setContentsMargins(200,11,200,30)
        #vbox_main.setContentsMargins(200,0,200,30)

        text = QLabel(title)
        text.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        vbox_main.addWidget(text, 1, Qt.AlignTop | Qt.AlignHCenter)

        vbox_info = QVBoxLayout()
        vbox_info.setSpacing(10)

        picture = QLabel()
        pixmap = QPixmap("utility/pictures/user.png")
        picture.setPixmap(pixmap.scaled(100,100, Qt.KeepAspectRatio))
        picture.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
        vbox_info.addWidget(picture)

        # name, surname
        user_name_surname = QLabel(f"{self.user.get_name()} {self.user.get_surname()}")
        user_name_surname.setStyleSheet("font-family: Ubuntu; font-size: 25px;")
        vbox_info.addWidget(user_name_surname, 3, Qt.AlignHCenter)
        vbox_info.addSpacing(50)

        # formlayout (con hbox per modifica password)
        self.form_layout = QFormLayout()

        # password handling
        hbox_pass = QHBoxLayout()
        password = self.user.get_password()
        password = f"{password[0]}{'*'*(len(password)-2)}{password[-1]}"
        self.password_lbl = QLabel(password)
        hbox_pass.addWidget(self.password_lbl)
        hbox_pass.addStretch(2)
        update_pass = QPushButton("Edit")
        update_pass.clicked.connect(self.update_password)
        hbox_pass.addWidget(update_pass)
        hbox_pass.addStretch(2)


        self.form_layout.addRow("<b>Email: </b>", QLabel(self.user.get_email()))
        self.form_layout.addRow("<b>Name: </b>",  QLabel(self.user.get_name()))
        self.form_layout.addRow("<b>Surname: </b>",  QLabel(self.user.get_surname()))
        self.form_layout.addRow("<b>Password: </b>",  hbox_pass)
        self.wallet_lbl = QLabel(self.user.get_wallet())

        wallet_lbl_name = "<b>Wallet: </b>" if not user_is_admin(self.user, self.https_session) else "<b>uPark Income: </b>"
        self.form_layout.addRow(wallet_lbl_name, self.wallet_lbl)

        self.disability_lbl = QLabel(str(self.user.get_disability()))
        self.form_layout.addRow("<b>Disability: </b>", self.disability_lbl)

        self.active_account_lbl = QLabel(str(self.user.get_active_account()))
        self.form_layout.addRow("<b>Active account: </b>", self.active_account_lbl)

        self.form_layout.addRow("<b>Category: </b>", QLabel(self.get_user_category_name()))

        vbox_info.addLayout(self.form_layout, 5)

        vbox_info.addStretch(5)

        vbox_main.addLayout(vbox_info, 2)

        if not user_is_admin(self.user, self.https_session):
            add_money_button = QPushButton("Add money")
            add_money_button.clicked.connect(self.add_money)
            vbox_main.addWidget(add_money_button, Qt.AlignTop)

        vbox_main.addStretch(5)

        self.setLayout(vbox_main)

        self.setWindowTitle(title)
        self.show()


    def get_user_category_name(self):
        response = make_http_request(self.https_session, "get", "user_categories", show_messagebox = True)

        if response:
            user_categories = [UserCategory(**user_category) for user_category in response.json()]
            try:
                user_category_index = user_categories.index(UserCategory(id = self.user.get_id_user_category()))
                return user_categories[user_category_index].get_name()
            except ValueError:
                QMessageBox.critical(self, "Alert", "Server error on user category")
                return "---"


    def update_password(self):
        text, ok = QInputDialog.getText(self, "Update password", "New password: ", QLineEdit.Normal)
        new_password = str(text)
        if ok and len(new_password) < 8:
            # https request
            response = make_http_request(self.https_session, "put", "users/" + str(self.user.get_id()), json = {"password":new_password})
            if response:
                QMessageBox.information(self, "Server response", response.text)
                self.user.set_password(new_password)
                self.https_session.auth = (self.user.get_email(), new_password)

                # update info
                self.password_lbl.setText(f"{new_password[0]}{'*'*(len(new_password)-2)}{new_password[-1]}")

            #print(new_password)
        elif ok and len(new_password) == 0:
            QMessageBox.information(self, "uPark tip", "Password must contains at least 8 characters!")


    def add_money(self):
        amount, ok = QInputDialog.getDouble(self, "Add money", "Amount: ", 5.0, 0.0, 50.0, decimals=2)

        if ok:
            # https request
            response = make_http_request(self.https_session, "post", "users/" + str(self.user.get_id()) + "/add_money", json = {"amount":amount})
            if response:
                QMessageBox.information(self, "Server response", response.text)

                self.user.set_wallet(str(float(self.user.get_wallet()) + amount))

                # update info
                self.wallet_lbl.setText(self.user.get_wallet())

            #print(new_password)
        elif ok and len(new_password) == 0:
            QMessageBox.information(self, "uPark tip", "Password should not be blank!")


    def refresh(self):
        response = make_http_request(self.https_session, "get", "users/" + str(self.user.get_id()))
        if response.json():
            self.user = User(**response.json())
            # password
            password = self.user.get_password()
            password_hidden = f"{password[0]}{'*'*(len(password)-2)}{password[-1]}"
            if password_hidden != self.password_lbl.text():
                self.password_lbl.setText(password_hidden)
            # wallet
            if self.user.get_wallet() != self.wallet_lbl.text():
                self.wallet_lbl.setText(self.user.get_wallet())
            # disability
            if self.user.get_disability() != self.disability_lbl.text():
                self.disability_lbl.setText(str(self.user.get_disability()))
            # active account
            if self.user.get_active_account() != self.active_account_lbl.text():
                self.active_account_lbl.setText(str(self.user.get_active_account()))


    def showEvent(self, event):
        self.refresh()
