class ParkingSlot:
    def __init__(self, id = None, number = None, id_parking_lot = None, id_vehicle_type = None, reserved_disability = False):
        self._id = id
        self._number = number
        self._id_parking_lot = id_parking_lot
        self._id_vehicle_type = id_vehicle_type
        self._reserved_disability = reserved_disability


    def set_id(self, id):
        self._id = id

    # def set_number(self, number):
    #     self._number = number
    #
    # def set_id_parking_lot(self, id_parking_lot):
    #     self._id_parking_lot = id_parking_lot

    def set_id_vehicle_type(self, id_vehicle_type):
        self._id_vehicle_type = id_vehicle_type

    def set_reserved_disability(self, reserved_disability):
        self._reserved_disability = reserved_disability


    def get_id(self):
        return self._id

    def get_number(self):
        return self._number

    def get_id_parking_lot(self):
        return self._id_parking_lot

    def get_id_vehicle_type(self):
        return self._id_vehicle_type

    def get_reserved_disability(self):
        return self._reserved_disability

    def __eq__(self, other):
        if isinstance(other, ParkingSlot):
            return (self._id == other._id) or (self._number == other._number)
        return NotImplemented

    def __str__(self):
        parking_slot_info = (f"Parking slot info Id: {self._id}, number: {self._number}, id parking lot: {self._id_parking_lot}, id vehicle type: {self._id_vehicle_type}, "
        f"reserved disability: {self._reserved_disability}")
        return parking_slot_info
