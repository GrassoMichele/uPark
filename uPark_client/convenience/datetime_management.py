from datetime import timedelta, datetime
from dateutil.tz import tzutc, tzlocal

# Also pass day as parameter to handle Daylight saving time
def datetime_to_UTC(day, time, is_query = False):
    time += ":00"               # columns format xx:xx

    # Auto-detect timezones:
    utc_zone = tzutc()
    local_zone = tzlocal()

    # Convert time string to datetime
    if (time != "1 day, 0:00:00"):     #timedelta(1,0) = 1 day, 0:00. This is the case of booking request ending at midnight (next day 0:00)
        local_datetime = datetime.strptime(f"{day} {time}", "%Y-%m-%d %H:%M:%S")
    else:
        local_datetime = datetime.strptime(day, "%Y-%m-%d") + timedelta(1,0)

    # Tell the datetime object that it's in local time zone since
    # datetime objects are 'naive' by default
    local_datetime = local_datetime.replace(tzinfo=local_zone)

    # detect daylight saving time
    #print("DST: ", local_datetime.dst())

    # Convert time to UTC
    utc_datetime = local_datetime.astimezone(utc_zone)
    # Generate UTC time string
    utc_datetime_string = utc_datetime.strftime('%Y-%m-%dT%H_%M_%S') if is_query else utc_datetime.strftime('%Y-%m-%d %H:%M:%S')


    return utc_datetime_string


def datetime_UTC_to_local(date_time):

    # Auto-detect zones:
    utc_zone = tzutc()
    local_zone = tzlocal()

    # Convert time string to datetime
    utc_datetime = datetime.strptime(f"{date_time}", "%Y-%m-%d %H:%M:%S")

    # Tell the datetime object that it's in UTC time zone
    utc_datetime = utc_datetime.replace(tzinfo=utc_zone)
    # Convert time to local
    local_datetime = utc_datetime.astimezone(local_zone)
    # Generate UTC time string
    local_datetime_string = local_datetime.strftime('%Y-%m-%d %H:%M:%S')

    return local_datetime_string
