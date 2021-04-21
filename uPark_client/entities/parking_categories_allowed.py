class ParkingCategoriesAllowed:
    def __init__(self, id = None, id_parking_lot = None, id_user_category = None):
        self._id = id
        self._id_parking_lot = id_parking_lot
        self._id_user_category= id_user_category


    def set_id(self, id):
        self._id = id

    # def set_id_parking_lot(self, id_parking_lot):
    #     self._id_parking_lot = id_parking_lot
    #
    # def set_id_user_category(self, id_user_category):
    #     self._id_user_category = id_user_category


    def get_id(self):
        return self._id

    def get_id_parking_lot(self):
        return self._id_parking_lot

    def get_id_user_category(self):
        return self._id_user_category

    def __eq__(self, other):
        if isinstance(other, ParkingCategoriesAllowed):
            return self._id == other._id
        return NotImplemented

    def __str__(self):
        parking_categories_allowed_info = (f"Parking categories allowed Id: {self._id}, id parking lot: {self._id_parking_lot}, id user category: {self._id_user_category}")
        return parking_categories_allowed_info
