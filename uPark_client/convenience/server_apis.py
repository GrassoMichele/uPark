from PyQt5.QtWidgets import QMessageBox
from entities.user import User
from entities.user_category import UserCategory

import requests


def make_http_request(http_session, method, relative_uri, json=None, params=None, show_messagebox = True):
    try:
        response = http_session.request(method, "https://localhost:50050/apis/" + relative_uri, json=json, params=params)
        response.raise_for_status()
        return response

    except (requests.exceptions.HTTPError, Exception) as err:
        message = response.text if isinstance(err, requests.exceptions.HTTPError) else str(err)
        if show_messagebox:
            QMessageBox.critical(None, "Alert", message)
        return None


def user_is_admin(user, https_session):
    response = make_http_request(https_session, "get", "user_categories")

    if response.json():
        user_categories = [UserCategory(**user_category) for user_category in response.json()]

        try:
            user_category_index = user_categories.index(UserCategory(id = user.get_id_user_category()))
        except ValueError:
            return
        else:
            return True if user_categories[user_category_index].get_name() == "Admin" else False
    else:
        return
