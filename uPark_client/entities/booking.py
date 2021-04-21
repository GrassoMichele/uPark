class Booking:
    def __init__(self, id = None, datetime_start = None, datetime_end = None, entry_time = None, exit_time = None, amount = None, id_user = None, id_vehicle = None, id_parking_slot = None, note = ""):
        self._id = id
        self._datetime_start = datetime_start
        self._datetime_end = datetime_end
        self._entry_time = entry_time
        self._exit_time = exit_time
        self._amount = amount
        self._id_user = id_user
        self._id_vehicle = id_vehicle
        self._id_parking_slot = id_parking_slot
        self._note = note


    def set_id(self, id):
        self._id = id

    def set_datetime_start(self, datetime_start):
        self._datetime_start = datetime_start

    def set_datetime_end(self, datetime_end):
        self._datetime_end = datetime_end

    def set_entry_time(self, entry_time):
        self._entry_time = entry_time

    def set_exit_time(self, exit_time):
        self._exit_time = exit_time

    # def set_amount(self, amount):
    #     self._amount = amount

    # def set_id_user(self, id_user):
    #     self._id_user = id_user
    #
    # def set_id_vehicle(self, id_vehicle):
    #     self._id_vehicle = id_vehicle
    #
    # def set_id_parking_slot(self, id_parking_slot):
    #     self._id_parking_slot = id_parking_slot

    def set_note(self, note):
        self._note = note


    def get_id(self):
        return self._id

    def get_datetime_start(self):
        return self._datetime_start

    def get_datetime_end(self):
        return self._datetime_end

    def get_entry_time(self):
        return self._entry_time

    def get_exit_time(self):
        return self._exit_time

    def get_amount(self):
        return self._amount

    def get_id_user(self):
        return self._id_user

    def get_id_vehicle(self):
        return self._id_vehicle

    def get_id_parking_slot(self):
        return self._id_parking_slot

    def get_note(self):
        return self._note

    def __eq__(self, other):
        if isinstance(other, Booking):
            return self._id == other._id
        return NotImplemented

    def __str__(self):
        booking_info = (f"Booking Id: {self._id}, datetime start: {self._datetime_start}, datetime end: {self._datetime_end}, entry time: {self._entry_time}, "
        f"exit time: {self._exit_time}, amount: {self._amount}, id user: {self._id_user}, id vehicle: {self._id_vehicle}, "
        f"id parking slot: {self._id_parking_slot}, note: {self._note}")
        return booking_info
