#! /usr/bin/python3
from .parking import Parking
from ..common.bookings import Bookings
from ..common.profile import Profile
from .other_settings.other_settings import OtherSettings
from .user_management import UserManagement

from ..dashboard import Dashboard

class AdminDashboard(Dashboard):

    def __init__(self, user):
        button_names = ["Parking Management", "Bookings in Progress", "Bookings Expired", "Other settings", "User Management", "Profile", "Logout"] #income
        super().__init__(user, button_names)
        self.initUI()

    def initUI(self):
        self.stack.addWidget(Parking(self.https_session, self.user))
        self.stack.addWidget(Bookings(self.https_session, self.user, only_in_progress = True))
        self.stack.addWidget(Bookings(self.https_session, self.user, only_expired = True))
        self.stack.addWidget(OtherSettings(self.https_session))
        self.stack.addWidget(UserManagement(self.https_session))
        self.stack.addWidget(Profile(self.https_session, self.user))
