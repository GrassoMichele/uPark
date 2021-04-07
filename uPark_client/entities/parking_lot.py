class ParkingLot:
    def __init__(self, id = None, name = None, street = None, num_parking_slots = None, parking_slots = None):
        self._id = id
        self._name = name
        self._street = street
        self._num_parking_slots = num_parking_slots
        self._parking_slots = parking_slots


    def set_id(self, id):
        self._id = id

    def set_name(self, name):
        self._name = name

    def set_street(self, street):
        self._street = street

    def set_num_parking_slots(self, num_parking_slots):
        self._num_parking_slots = num_parking_slots

    def set_parking_slots(self, parking_slots):
        self._parking_slots = parking_slots


    def get_id(self):
        return self._id

    def get_name(self):
        return self._name

    def get_street(self):
        return self._street

    def get_num_parking_slots(self):
        return self._num_parking_slots

    def get_parking_slots(self):
        return self._parking_slots

    def __eq__(self, other):
        if isinstance(other, ParkingLot):
            return self._id == other._id
        return NotImplemented

    def __str__(self):
        parking_lot_info = (f"Parking lot Id: {self._id}, name: {self._name}, street: {self._street}, num parking slots: {self._num_parking_slots}")
        return parking_lot_info
