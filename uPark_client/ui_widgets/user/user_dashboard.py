#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QStackedWidget, QHBoxLayout, QVBoxLayout, QDesktopWidget, \
                             QLabel, QApplication, QMainWindow, QPushButton, QFrame

from PyQt5.QtCore import Qt, QRect
from PyQt5.QtGui import QFont, QBrush, QPalette, QColor

import sys
import requests

from .park import Park

class userDashboard(QMainWindow):

    def __init__(self, user):
        super().__init__()
        self.user = user
        self.https_session =requests.Session()
        self.https_session.verify = "utility/upark_server.crt"
        self.https_session.auth = (self.user.get_email(), self.user.get_password())
        self.initUI()

    def initUI(self):
        self.widget = QWidget(self)
        self.stack = QStackedWidget(self)

        hbox = QHBoxLayout()

        self.frame = QFrame(self)
        self.frame.setStyleSheet("QPushButton {font-family: Ubuntu; padding: 40px}")

        self.buttons_names = ["Park", "Bookings in Progress", "Bookings Expired", "Vehicles", "Profile", "Logout"]
        self.buttons = []

        vbox = QVBoxLayout()

        # hline = QFrame()
        # hline.setFrameShape(QFrame.HLine)
        # hline.setFrameShadow(QFrame.Sunken)
        # hline.setLineWidth(1)

        for name in self.buttons_names:
            hline = QFrame()
            hline.setFrameShape(QFrame.HLine)
            hline.setFrameShadow(QFrame.Sunken)
            hline.setLineWidth(1)

            self.buttons.append(QPushButton(name))
            self.buttons[-1].setFlat(True)
            self.buttons[-1].clicked.connect(self.change_widget)
            vbox.addWidget(self.buttons[-1])

            if name != self.buttons_names[-1]:
                vbox.addWidget(hline, 1)

        self.frame.setLayout(vbox)

        hbox.addWidget(self.frame, 1)

        vline = QFrame()
        vline.setFrameShape(QFrame.VLine)
        vline.setFrameShadow(QFrame.Sunken)
        vline.setLineWidth(1)

        hbox.addWidget(vline, 1)

        self.park_widget = Park(self.https_session, self.user)
        self.booking_in_progress = QWidget()
        self.stack.addWidget(self.park_widget)
        self.stack.addWidget(self.booking_in_progress)
        #self.stack.setCurrentIndex(1)
        #setCurrentWidget(QWidget *widget)
        hbox.addWidget(self.stack, 10)


        self.widget.setLayout(hbox)

        self.setCentralWidget(self.widget)


        self.setWindowTitle('Dashboard')
        self.full_screen()
        self.show()

    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)

    def change_widget(self):
        button_sender = self.sender()
        widget_index = self.buttons_names.index(button_sender.text())
        self.stack.setCurrentIndex(widget_index)

# def main():
#     app = QApplication(sys.argv)
#     ex = userDashboard()
#
#     sys.exit(app.exec_())
#
#
# if __name__ == '__main__':
#     main()
