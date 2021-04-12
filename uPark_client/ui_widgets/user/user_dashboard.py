#! /usr/bin/python3
from PyQt5.QtWidgets import QWidget, QStackedWidget, QHBoxLayout, QVBoxLayout, QDesktopWidget, \
                             QLabel, QApplication, QMainWindow, QPushButton, QFrame, QMessageBox, \
                             QSpacerItem, QSizePolicy

from PyQt5.QtCore import Qt, QRect, pyqtSignal, QObject
from PyQt5.QtGui import QFont, QBrush, QPalette, QColor

import sys
import requests

from .park import Park
from .bookings_in_progress import BookingsInProgress
from .bookings_expired import BookingsExpired
from .user_vehicles import UserVehicles
from .profile import Profile

class UserDashboard_signal(QObject):
    close = pyqtSignal()

class UserDashboard(QMainWindow):

    def __init__(self, user):
        super().__init__()
        self.user = user
        self.https_session =requests.Session()
        self.https_session.verify = "utility/upark_server.crt"
        self.https_session.auth = (self.user.get_email(), self.user.get_password())
        self.signal = UserDashboard_signal()
        self.initUI()

    def initUI(self):
        self.widget = QWidget(self)
        self.stack = QStackedWidget(self)
        self.set_object_margins(self.stack, top=11)

        self.selected_button_stylesheet = "background-color: #9BCAEE; border-left: 8px solid #3399FF"

        hbox = QHBoxLayout()
        self.set_object_margins(hbox, left=0, top=0)

        self.frame = QFrame(self)
        self.frame.setStyleSheet("*{border: 0px; border-right:1px solid #B3BDCC;} QPushButton{font-family: Ubuntu; padding: 40px; border: 0px; border-style: hidden hidden solid hidden; border-width: 0px 0px 1px 8px; border-color: #B3BDCC}")

        self.buttons_names = ["Park", "Bookings in Progress", "Bookings Expired", "Vehicles", "Profile", "Logout"]
        self.buttons = []

        vbox = QVBoxLayout()
        vbox.setSpacing(0)
        self.set_object_margins(vbox, left=0, top=0, right=0)

        for name in self.buttons_names:

            self.buttons.append(QPushButton(name))
            #self.buttons[-1].setFlat(True)
            self.buttons[-1].clicked.connect(self.change_widget)
            vbox.addWidget(self.buttons[-1])

        self.buttons[0].setStyleSheet(self.selected_button_stylesheet)

        spacer_item = QSpacerItem(1,1, vPolicy = QSizePolicy.Expanding)
        vbox.addSpacerItem(spacer_item)

        self.frame.setLayout(vbox)

        hbox.addWidget(self.frame, 1)

        self.stack.addWidget(Park(self.https_session, self.user))
        self.stack.addWidget(BookingsInProgress(self.https_session, self.user))
        self.stack.addWidget(BookingsExpired(self.https_session, self.user))
        self.stack.addWidget(UserVehicles(self.https_session, self.user))
        self.stack.addWidget(Profile(self.https_session, self.user))

        hbox.addWidget(self.stack, 20)

        self.widget.setLayout(hbox)

        self.setCentralWidget(self.widget)

        self.setWindowTitle('Dashboard')
        self.full_screen()
        self.show()


    def set_object_margins(self, object, left=None, top=None, right=None, bottom=None):
        margins = object.contentsMargins()
        if left:
            margins.setLeft(left)
        if top:
            margins.setTop(top)
        if right:
            margins.setRight(right)
        if bottom:
            margins.setBottom(bottom)
        object.setContentsMargins(margins)


    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)


    def change_widget(self):
        button_sender = self.sender()
        try:
            widget_index = self.buttons_names.index(button_sender.text())
        except ValueError:
            return
        else:
            # sidebar background color
            for button in self.buttons:
                if button != button_sender:
                    button.setStyleSheet("")
                else:
                    button.setStyleSheet(self.selected_button_stylesheet)

            if button_sender.text() != "Logout":
                self.stack.setCurrentIndex(widget_index)

            else:
                reply = QMessageBox.question(self, 'Dashboard',
                                             f"{self.user.get_name()}, are you sure to logout?", QMessageBox.Yes |
                                             QMessageBox.No, QMessageBox.No)
                if reply == QMessageBox.Yes:
                    self.signal.close.emit()
                else:
                    button_sender.setStyleSheet("")
                    self.buttons[self.stack.currentIndex()].setStyleSheet(self.selected_button_stylesheet)

# def main():
#     app = QApplication(sys.argv)
#     ex = userDashboard()
#
#     sys.exit(app.exec_())
#
#
# if __name__ == '__main__':
#     main()
