class VehicleType:
    def __init__(self, id = None, name = None, rate_percentage = None):
        self._id = id
        self._name = name
        self._rate_percentage = rate_percentage


    def set_id(self, id):
        self._id = id

    def set_name(self, name):
        self._name = name

    def set_rate_percentage(self, rate_percentage):
        self._rate_percentage = rate_percentage


    def get_id(self):
        return self._id

    def get_name(self):
        return self._name

    def get_rate_percentage(self):
        return self._rate_percentage

    def __eq__(self, other):
        if isinstance(other, VehicleType):
            return (self._id == other._id) or (self._name == other._name)
        return NotImplemented

    def __str__(self):
        vehicle_type_info = (f"Vehicle type Id: {self._id}, name: {self._name}, rate percentage: {self._rate_percentage}")
        return vehicle_type_info
