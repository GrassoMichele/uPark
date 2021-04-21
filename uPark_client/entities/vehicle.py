class Vehicle:
    def __init__(self, id = None, license_plate = None, brand = None, model = None, id_user = None, id_vehicle_type = None):
        self._id = id
        self._license_plate = license_plate
        self._brand = brand
        self._model = model
        self._id_user = id_user
        self._id_vehicle_type = id_vehicle_type


    def set_id(self, id):
        self._id = id

    # def set_license_plate(self, license_plate):
    #     self._license_plate = license_plate

    def set_brand(self, brand):
        self._brand = brand

    def set_model(self, model):
        self._model = model

    # def set_id_user(self, id_user):
    #     self._id_user = id_user

    # def set_id_vehicle_type(self, id_vehicle_type):
    #     self._id_vehicle_type = id_vehicle_type


    def get_id(self):
        return self._id

    def get_license_plate(self):
        return self._license_plate

    def get_brand(self):
        return self._brand

    def get_model(self):
        return self._model

    def get_id_user(self):
        return self._id_user

    def get_id_vehicle_type(self):
        return self._id_vehicle_type

    def __eq__(self, other):
        if isinstance(other, Vehicle):
            return (self._id == other._id) or (self._license_plate == other._license_plate)
        return NotImplemented

    def __str__(self):
        vehicle_info = (f"Vehicle Id: {self._id}, license plate: {self._license_plate}, brand: {self._brand}, model: {self._model}, "
        f"id user: {self._id_user}, id vehicle type: {self._id_vehicle_type}")
        return vehicle_info
