#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QDesktopWidget, \
                             QLabel, QApplication, QMainWindow, QPushButton

from PyQt5.QtCore import Qt, QRect
from PyQt5.QtGui import QFont

import sys


class adminDashboard(QMainWindow):

    def __init__(self, user):
        super().__init__()
        self.user = user
        self.initUI()

    def initUI(self):
        self.setObjectName("MainWindow")
        #self.setStyleSheet("#MainWindow { border-image: url(utility/pictures/home.jpg) 0 0 0 0 stretch stretch; }")

        self.signup = None
        self.login = None

        self.widget = QWidget(self)

        vbox = QVBoxLayout()

        vbox.addStretch()

        app_name = QLabel("ADMIN DASHBOARD")  #si
        app_name.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        app_name.setFont(QFont("Arial", 40))
        app_name.setStyleSheet("color: black; ")

        vbox.addWidget(app_name)

        vbox.addStretch()

        hbox = QHBoxLayout()

        signup_btn = QPushButton("Sign-up")
        signup_btn.clicked.connect(self.showSignup)

        login_btn = QPushButton("Login")
        login_btn.clicked.connect(self.showLogin)

        hbox.addStretch()
        hbox.addWidget(signup_btn)
        hbox.addStretch()
        hbox.addWidget(login_btn)
        hbox.addStretch()

        vbox.addLayout(hbox)

        vbox.addStretch()

        self.widget.setLayout(vbox)

        self.setCentralWidget(self.widget)

        self.setWindowTitle('Dashboard')   #si
        self.full_screen()
        self.show()

    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)

    def showSignup(self):
        if self.signup is None:
            try:
                self.signup = SignUp()
            except Exception:
                self.signup = None
                self.close()
        if self.signup is not None:
            self.signup.show()

    def showLogin(self):
        if self.login is None:
            try:
                self.login = Login()
            except Exception:
                self.login = None
                self.close()
        if self.login is not None:
            self.login.show()


def main():
    app = QApplication(sys.argv)
    ex = adminDashboard()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
