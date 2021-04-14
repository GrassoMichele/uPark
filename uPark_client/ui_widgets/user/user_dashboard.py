#! /usr/bin/python3
from ..common.park import Park
from ..common.bookings import Bookings
from ..common.profile import Profile
from .user_vehicles import UserVehicles

from ..dashboard import Dashboard

class UserDashboard(Dashboard):

    def __init__(self, user):
        button_names = ["Park", "Bookings in Progress", "Bookings Expired", "Vehicles", "Profile", "Logout"]
        super().__init__(user, button_names)
        self.initUI()


    def initUI(self):
        self.stack.addWidget(Park(self.https_session, self.user))
        self.stack.addWidget(Bookings(self.https_session, self.user, only_in_progress = True))
        self.stack.addWidget(Bookings(self.https_session, self.user, only_expired = True))
        self.stack.addWidget(UserVehicles(self.https_session, self.user))
        self.stack.addWidget(Profile(self.https_session, self.user))
