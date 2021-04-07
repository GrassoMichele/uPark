#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QDesktopWidget, \
                             QLabel, QApplication, QMainWindow, QPushButton, QFrame

from PyQt5.QtCore import Qt, QRect
from PyQt5.QtGui import QFont

import sys

from .park import Park

class userDashboard(QMainWindow):

    def __init__(self, user):
        super().__init__()
        self.user = user
        self.initUI()

    def initUI(self):
        self.widget = QWidget(self)

        hbox = QHBoxLayout()

        vbox = QVBoxLayout()
        vbox.addWidget(QPushButton("arrivederci!"))
        vbox.addWidget(QPushButton("benvenuto!"))

        hbox.addLayout(vbox, 1)

        line = QFrame()
        line.setFrameShape(QFrame.VLine)
        line.setFrameShadow(QFrame.Sunken)
        line.setLineWidth(1)
        hbox.addWidget(line, 1)

        # stackWidget
        hbox.addWidget(Park(),10)

        # hbox.setStretch(0,1)
        # hbox.setStretch(1,1)
        # hbox.setStretch(2,10)


        self.widget.setLayout(hbox)

        self.setCentralWidget(self.widget)


        self.setWindowTitle('Dashboard')
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
    ex = userDashboard()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
