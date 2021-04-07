#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QDesktopWidget, \
                             QLabel, QApplication, QMainWindow, QPushButton, QFrame

from PyQt5.QtCore import Qt, QRect
from PyQt5.QtGui import QFont, QPixmap

import sys

from ui_widgets.forms.signup import SignUp
from ui_widgets.forms.login import Login

class Home(QMainWindow):

    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.setObjectName("MainWindow")
        self.setStyleSheet("#MainWindow { border-image: url(utility/pictures/home.jpg) 0 0 0 0 stretch stretch; }")

        self.signup = None
        self.login = None

        self.widget = QWidget(self)

        vbox = QVBoxLayout()

        vbox.addStretch()

        app_name = QLabel("uPark")
        app_name.setPixmap(QPixmap("utility/pictures/uPark_logo.svg"))
        app_name.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)

        vbox.addWidget(app_name)

        vbox.addStretch()

        hbox = QHBoxLayout()

        signup_btn = QPushButton("Sign-up")
        signup_btn.setFont(QFont("Ubuntu", 20))
        signup_btn.setStyleSheet("background-color : #3D3C3C; color : #FFFFFF; padding : 23px;")
        signup_btn.clicked.connect(self.show_signup)

        login_btn = QPushButton("Login")
        login_btn.setStyleSheet("background-color : #3D3C3C; color : #FFFFFF; padding : 23px;")
        login_btn.setFont(QFont("Ubuntu", 20))
        login_btn.clicked.connect(self.show_login)

        hbox.addStretch()
        hbox.addWidget(signup_btn)
        hbox.addStretch()
        hbox.addWidget(login_btn)
        hbox.addStretch()

        vbox.addLayout(hbox)

        vbox.addStretch()

        self.widget.setLayout(vbox)

        self.setCentralWidget(self.widget)

        self.setWindowTitle('uPark | Home')
        self.full_screen()
        self.show()

    def set_signup_close(self):
        self.signup.close()

    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)

    def show_signup(self):
        if self.signup is None:
            try:
                self.signup = SignUp()
                #self.signup.setParent()
                self.signup.signals.close.connect(self.set_signup_close)
            except Exception:
                self.signup = None

        if self.signup is not None:
            self.signup.show()


    def show_login(self):
        if self.login is None:
            try:
                self.login = Login()
                self.login.signals.close.connect(self.login.close)
            except Exception:
                self.login = None


        if self.login is not None:
            self.login.show()
            self.login.signals.home_close.connect(self.close)


def main():
    app = QApplication(sys.argv)
    ex = Home()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
