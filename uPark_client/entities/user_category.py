class UserCategory:
    def __init__(self, id = None, name = None, id_hourly_rate = None, service_validity_start = None, service_validity_end = None):
        self._id = id
        self._name = name
        self._id_hourly_rate = id_hourly_rate
        self._service_validity_start = service_validity_start
        self._service_validity_end = service_validity_end


    def set_id(self, id):
        self._id = id

    # def set_name(self, name):
    #     self._name = name

    def set_id_hourly_rate(self, id_hourly_rate):
        self._id_hourly_rate = id_hourly_rate

    def set_service_validity_start(self, service_validity_start):
        self._service_validity_start = service_validity_start

    def set_service_validity_end(self, service_validity_end):
        self._service_validity_end = service_validity_end


    def get_id(self):
        return self._id

    def get_name(self):
        return self._name

    def get_id_hourly_rate(self):
        return self._id_hourly_rate

    def get_service_validity_start(self):
        return self._service_validity_start

    def get_service_validity_end(self):
        return self._service_validity_end

    def __eq__(self, other):
        if isinstance(other, UserCategory):
            return (self._id == other._id) or (self._name == other._name)
        return NotImplemented

    def __str__(self):
        user_category_info = (f"User category Id: {self._id}, name: {self._name}, id hourly rate: {self._id_hourly_rate}, service validity start: {self._service_validity_start}, "
        f"service validity end: {self._service_validity_end}")
        return user_category_info
