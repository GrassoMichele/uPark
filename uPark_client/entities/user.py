class User:
    def __init__(self, id = None, email = None, name = None, surname = None, password = None, wallet = 0.00, disability = False, active_account = False, id_user_category = 2):    # id_user_category = 2 -> Student
        self._id = id
        self._email = email
        self._name = name
        self._surname = surname
        self._password = password
        self._wallet = wallet
        self._disability = disability
        self._active_account = active_account
        self._id_user_category = id_user_category


    def set_id(self, id):
        self._id = id

    def set_email(self, email):
        self._email = email

    def set_name(self, name):
        self._name = name

    def set_surname(self, surname):
        self._surname = surname

    def set_password(self, password):
        self._password = password

    def set_wallet(self, wallet):
        self._wallet = wallet

    def set_disability(self, disability):
        self._disability = disability

    def set_active_account(self, active_account):
        self._active_account = active_account

    def set_id_user_category(self, id_user_category):
        self._id_user_category = id_user_category


    def get_id(self):
        return self._id

    def get_email(self):
        return self._email

    def get_name(self):
        return self._name

    def get_surname(self):
        return self._surname

    def get_password(self):
        return self._password

    def get_wallet(self):
        return self._wallet

    def get_disability(self):
        return self._disability

    def get_active_account(self):
        return self._active_account

    def get_id_user_category(self):
        return self._id_user_category

    def __eq__(self, other):
        if isinstance(other, User):
            return self._id == other._id
        return NotImplemented

    def __str__(self):
        user_info = (f"User Id: {self._id}, email: {self._email}, name: {self._name}, surname: {self._surname}, "
        f"password: {self._password}, wallet: {self._wallet}, disability: {self._disability}, active accont: {self._active_account}, "
        f"id_user_category: {self._id_user_category}")
        return user_info
