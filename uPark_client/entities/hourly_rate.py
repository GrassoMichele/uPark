class HourlyRate:
    def __init__(self, id = None, amount = None):
        self._id = id
        self._amount = amount


    def set_id(self, id):
        self._id = id

    def set_amount(self, amount):
        self._amount = amount


    def get_id(self):
        return self._id

    def get_amount(self):
        return self._amount

    def __eq__(self, other):
        if isinstance(other, HourlyRate):
            return self._id == other._id
        return NotImplemented

    def __str__(self):
        hourly_rate_info = (f"Hourly rate Id: {self._id}, amount: {self._amount}")
        return hourly_rate_info
