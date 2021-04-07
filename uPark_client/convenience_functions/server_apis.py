from PyQt5.QtWidgets import QMessageBox

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
