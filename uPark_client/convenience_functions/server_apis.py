import requests

def get_user_categories(https_session):
    message, user_categories = None, None

    try:
        response = https_session.get("https://localhost:50050/apis/user_categories")
        # If the response was successful (status codes between [200-400)), no Exception will be raised
        response.raise_for_status()

    except (requests.exceptions.HTTPError, Exception) as err:
        message = (response.text, "Http request error!") if isinstance(err, requests.exceptions.HTTPError) else (str(err), "Server error. Retry later.")

    else:
        user_categories = response.json()                                       # http response body as json

    return user_categories, message
