#!/usr/bin/python

from PyQt5.QtWidgets import (QWidget, QCalendarWidget, QLabel, QApplication, QHBoxLayout, QMessageBox, QListWidget,
                             QVBoxLayout, QTableWidget, QAbstractItemView, QDesktopWidget, QTableWidgetItem,
                             QDialog, QDialogButtonBox, QComboBox)
from PyQt5.QtCore import QDate, Qt, QObject, pyqtSignal
from PyQt5.QtGui import QBrush, QColor

import sys

from datetime import timedelta, datetime
from dateutil.tz import tzutc, tzlocal

import requests


class Table_signal(QObject):
    message = pyqtSignal(int,str, str)      # parking_slot_number, start_time, end_time


class Bookings_table(QTableWidget):

    def __init__(self, rows, columns, parent, signal):
        super().__init__(rows, columns, parent)

        self.signal = signal
        self.init_UI()

    def init_UI(self):
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable
        self.horizontalHeader().setDefaultSectionSize(65)               # column width
        self.horizontalHeader().setDefaultAlignment(Qt.AlignLeft)

        # free_parking slots are white
        self.cells_colors = {
                            "user_bookings" : "#FFA200",
                            "other_bookings" : "#A8ADAF",
                            "bookings_separation": "#D2D5D7",
                            "disability": "#138DF3"
                            }

        # table column names
        self.quarters_of_hour = [str(timedelta(minutes=15)*i) for i in range(97)]                           # index 96 = 1 day, 0:00:00 (24:00:00)
        for i, q in enumerate(self.quarters_of_hour):
        	q_split = q.split(":")
        	item = str("0"+q_split[0] if len(q_split[0]) == 1 else q_split[0]) + ":" + str(q_split[1])      # e.g. 1:00:00 -> 01:00
        	self.quarters_of_hour[i] = item

        self.setHorizontalHeaderLabels(self.quarters_of_hour)
        self.show()


    def mouseReleaseEvent(self, event):
        super().mouseReleaseEvent(event)                        # base behaviour on mouse release (e.g. clear selection)

        if event.button() == Qt.LeftButton:                     # Release event only if done with left button
            indexSelection = [(item.row() , item.column()) for item in self.selectedIndexes()]

            print("Selection info: ", len(indexSelection), indexSelection)

            if len(indexSelection) != 0:                        # at least one cell selected

                # check on selection validity, only with multiple cells selected
                error = False
                for i in range(1, len(indexSelection)):
                    # if selection on different rows or non contiguous cells
                    if (indexSelection[i][0] != indexSelection[i-1][0]) | indexSelection[i][1] not in [indexSelection[i-1][1] + 1, indexSelection[i-1][1] - 1]:
                        error = True
                if error == True:
                    QMessageBox.critical(self, "Error", "Not a valid booking request!")
                    return

                # if no errors occured, proceed
                # take the first and last cell in selection
                parking_slot_number = indexSelection[0][0]+1            # parking slot numbers start from one
                booking_start_time = self.quarters_of_hour[indexSelection[0][1]]        # format xx:xx              ADD :00?
                #print(booking_start_time, type(booking_start_time))
                booking_end_time = self.quarters_of_hour[indexSelection[-1][1]+1]
                #print(booking_end_time, type(booking_end_time))
                self.signal.message.emit(parking_slot_number, booking_start_time, booking_end_time)

        self.clearSelection()


    def item_paint(self, row_index, col_index, color, deactivate = True):
        self.setItem(row_index, col_index, QTableWidgetItem(""))
        cell = self.item(row_index, col_index)
        cell.setBackground(QBrush(QColor(color)))

        if deactivate == True:                                  # always true unless when disable user
            cell.setFlags(Qt.NoItemFlags)                       # cell no selectable, editable, ecc.

        # colors handling
        if color == self.cells_colors["other_bookings"]:
            cell.setToolTip("Other user booking.")
        elif color == self.cells_colors["user_bookings"]:
            cell.setToolTip("Your booking.")
        elif color == self.cells_colors["bookings_separation"]:
            cell.setToolTip("Time interval between one booking and another")
        elif color == self.cells_colors["disability"]:
            cell.setToolTip("Parking slot reserved for disability.")



class BookingDialog(QDialog):
    def __init__(self, start, end, duration, parking_slot, user_vehicles, slot_vehicle_type_id, amount, parent=None):
        super().__init__(parent=parent)

        self.setWindowTitle("Booking")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.accept)        # QDialog slot
        self.buttonBox.rejected.connect(self.reject)

        self.layout = QVBoxLayout()
        self.layout.addWidget(QLabel("<b><i>Booking info: </i></b>"), 0, Qt.AlignCenter)
        self.layout.addWidget(QLabel("<b>Start: </b>" + start))
        self.layout.addWidget(QLabel("<b>End: </b>" + end))
        self.layout.addWidget(QLabel("<b>Duration: </b>" + str(duration)))
        self.layout.addWidget(QLabel("<b>Parking slot: </b>" + str(parking_slot)))

        self.layout.addWidget(QLabel("<b>Vehicle: </b>"))
        # filter on user vehicles
        self.user_category_cmb = QComboBox()
        self.user_category_cmb.addItems([f"{vehicle['license_plate']} - {vehicle['brand']} - {vehicle['model']}" for vehicle in user_vehicles if vehicle["id_vehicle_type"]==slot_vehicle_type_id])
        self.layout.addWidget(self.user_category_cmb)

        self.layout.addWidget(QLabel("<b>Amount: " + amount + "€</b>"))

        self.layout.addSpacing(20)

        self.layout.addWidget(self.buttonBox)
        self.setLayout(self.layout)


    def selected_vehicle(self):
        return self.user_category_cmb.currentText()



class Park(QWidget):

    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):

        # DA SISTEMARE: session http from dashboard
        self.http_session = requests.Session()
        self.http_session.verify = "/home/michele/Desktop/uPark/uPark_client/utility/upark_server.crt"
        self.http_session.auth = ('antonio@gmail.com', 'cosicosi')
        #self.http_session.auth = ('giulia@live.it', 'password1')
        #self.http_session.auth = ('admin@admin', 'admin')

        # da sistemare, non sarà più necessaria dopo aver fatto login in quanto conoscerò già le info su user
        #response = self.http_session.get("https://localhost:50050/apis/users/" + "2")
        response = make_http_request(self.http_session, "get", "users/" + "2")
        if response:
            self.user = response.json()
        else:
            return                  # boh?
        # fine da sistemare

        self.table_signal = Table_signal()
        self.table_signal.message.connect(self.make_booking)

        hbox = QHBoxLayout(self)

        h_vbox = QVBoxLayout()
        h_vbox.addWidget(QLabel("Parking lots: "), 1, Qt.AlignBottom)
        self.parking_lots_list = QListWidget()
        self.parking_lots_list.setStyleSheet("font: 11pt Arial;")
        self.parking_lots_list.currentItemChanged.connect(lambda : self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()], True))
        h_vbox.addWidget(self.parking_lots_list, 2, Qt.AlignTop)

        hbox.addLayout(h_vbox, 1)

        vbox = QVBoxLayout()

        table_hbox = QHBoxLayout()
        self.tableWidget = Bookings_table(50,96, self, self.table_signal)           # 50 should be the parking lot number of slots
        table_hbox.addWidget(self.tableWidget, 11)

        self.vehicle_types_table = QTableWidget()
        self.vehicle_types_table.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable
        self.vehicle_types_table.setColumnCount(1)
        self.vehicle_types_table.setHorizontalHeaderLabels(["Vehicle type"])
        self.vehicle_types_table.verticalHeader().hide()                                    # possible modification with another column which items are parking slot numbers
        self.vehicle_types_table.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        table_hbox.addWidget(self.vehicle_types_table, 1)

        # synchronize scrolling between slots table and vehicle types table
        self.tableWidget.verticalScrollBar().valueChanged.connect(self.vehicle_types_table.verticalScrollBar().setValue);
        self.vehicle_types_table.verticalScrollBar().valueChanged.connect(self.tableWidget.verticalScrollBar().setValue);

        vbox.addLayout(table_hbox, 10)

        self.cal = QCalendarWidget(self)
        self.cal.setGridVisible(True)
        #self.cal.setMinimumDate(QDate.currentDate())		# not show previous days
        self.cal.selectionChanged.connect(lambda : self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()], False))
        v_hbox = QHBoxLayout()
        v_hbox.addStretch(1)
        v_hbox.addWidget(self.cal)
        v_hbox.addStretch(1)
        vbox.addLayout(v_hbox, 1)

        hbox.addLayout(vbox, 10)

        self.setLayout(hbox)

        self.get_parking_lots()
        #self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()]["id"])

        self.setWindowTitle("Parcheggia")
        self.full_screen()
        self.show()


    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)


    # get info on parking lots
    def get_parking_lots(self):

        response = make_http_request(self.http_session, "get", "parking_lots")
        if response:
            self.parking_lots = response.json()
        else:
            return

        if self.parking_lots:                       # dict={}, if dict -> false i.e. dict empty
            #print(self.parking_lots)

            for parking_lot in self.parking_lots:
                self.parking_lots_list.addItem(parking_lot["name"].capitalize())
                # get info on parking slots of parking lot
                response = make_http_request(self.http_session, "get", "parking_lots/" + str(parking_lot["id"]) + "/parking_slots")
                if response:
                    parking_lot["parking_slots"] = response.json()
            #print(self.parking_lots)

            # get info on vehicle type of parking_slots
            response = make_http_request(self.http_session, "get", "vehicle_types")
            if response:
                self.vehicle_types = response.json()
            #print(self.vehicle_types)

            # select first parking lot
            self.parking_lots_list.setCurrentRow(0)             # first item selected
            self.parking_lot = self.parking_lots[0]             # self.parking_lot = actually selected parking lot


    def disability_check(self):
        disabled_user = self.user["disability"]
        slots_row_indexes_disability = [slot["number"]-1 for slot in self.parking_lot["parking_slots"] if slot["reserved_disability"] == True]
        for row_index in slots_row_indexes_disability:
            for col_index in range(96):
                self.tableWidget.item_paint(row_index, col_index, self.tableWidget.cells_colors["disability"], not disabled_user)    # only disabled user can make bookings on reserved slots


    def get_bookings(self, parking_lot, selected_park_changed):
        #GET bookings?since=xxxx-xx-xx&until=xxxx-xx-xx&id_user=x&id_parking_lot=x

        self.parking_lot = parking_lot
        # clear previously selected info (also items background color). Removes all items not in the headers.
        self.tableWidget.clearContents()
        # check on slots reserved for disability and set color
        self.disability_check()

        # selected_park_changed=True when there is a change in selected parking lot. Info on parking slots needs to be updated only in this case. Otherwise in case of date change is equal to False.
        if selected_park_changed:
            self.tableWidget.setRowCount(self.parking_lot["num_parking_slots"])

            # shows the associated vehicle type for each parking slot
            self.vehicle_types_table.clearContents()
            self.vehicle_types_table.setRowCount(self.tableWidget.rowCount())           # each parking slot has associated exactly one vehicle type
            for i in range(self.vehicle_types_table.rowCount()):
                vehicle_type_name = [vehicle_type["name"] for vehicle_type in self.vehicle_types if vehicle_type["id"] == self.parking_lot["parking_slots"][i]["id_vehicle_type"]][0]
                #print(vehicle_type_name)
                self.vehicle_types_table.setItem(i, 0, QTableWidgetItem(vehicle_type_name.capitalize()))


        #day = str(self.cal.selectedDate().toString(Qt.ISODate))
        day = self.cal.selectedDate().toString(Qt.ISODate)
        #print(day)

        # start: from local 00:00:00 to utc (e.g. local 2021-04-01 00:00:00 = UTC 2021-03-31 22:00)
        # end: from local 23:59:00 to utc
        bookings_start_datetime = datetime_to_UTC(day, "00:00")
        bookings_end_datetime = datetime_to_UTC(day, "23:59")   # 23:59 for include bookings of time xx:45, 23:45 is enough
        # bookings_start_datetime could be different from bookings_end_datetime (max 1 day)
        print("UTC - Start validity: ", bookings_start_datetime, " ; End validity: ", bookings_end_datetime, " ; Id parking lot: ", self.parking_lot["id"])

        response = make_http_request(self.http_session, "get", "bookings", params = {"since":bookings_start_datetime.split()[0], "until":bookings_end_datetime.split()[0], "id_parking_lot":self.parking_lot["id"]})
        if response:
            bookings = response.json()

        # bookings are in UTC
        if bookings:
            print([f"{booking['datetime_start']} - {booking['datetime_end']}" for booking in bookings])

            for booking in bookings:
                print("-----------------\n", booking)

                # filter booking on temporal validity
                # discard all bookings ending before bookings_start_datetime => booking['datetime_end'] <= bookings_start_datetime
                if (datetime.strptime(f"{booking['datetime_end']}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{bookings_start_datetime}", "%Y-%m-%d %H:%M:%S")).total_seconds() <= 0:
                    #print("ops")
                    continue
                # discard all bookings starting after bookings_end_datetime => booking['datetime_start'] >= bookings_end_datetime
                elif (datetime.strptime(f"{booking['datetime_start']}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{bookings_end_datetime}", "%Y-%m-%d %H:%M:%S")).total_seconds() >= 0:
                    #print("ops1")
                    continue
                # valid booking
                else:
                    # convert booking datetime from UTC to local
                    booking_start = datetime_UTC_to_local(booking['datetime_start'])
                    print("Local booking start: ", booking_start)

                    booking_end = datetime_UTC_to_local(booking['datetime_end'])
                    print("Local booking end: ", booking_end)

                    booking_start = booking_start if booking_start.split()[0] == day else day + " 00:00:00"      # if booking start day in localtime is different from booking end day in localtime (booking on multiple days) select all table cells.
                    print("New start: ", booking_start)

                    # Set end time to 24:00:00 for bookings which end day is temporally subsequent to the selected day.
                    booking_end = booking_end if booking_end.split()[0] == day else day + " 24:00:00"
                    print("New end: ", booking_end)

                    # show booking in table
                    booking_start_time_split = booking_start.split()[1].split(":")          # list with booking time info
                    booking_end_time_split = booking_end.split()[1].split(":")

                    # find table row index
                    table_row_index = [slot["number"]-1 for slot in self.parking_lot["parking_slots"] if slot["id"] == booking["id_parking_slot"]][0]

                    # idea with classes: self.parking_lot["parking_slots"].index(ParkingSlot(booking["id_parking_slot"]))

                    # find table column indexes
                    start_table_col_index = int(booking_start_time_split[0])*4 + int(booking_start_time_split[1])//15      # table column index = hour*4 + minutes/15
                    end_table_col_index = int(booking_end_time_split[0])*4 + int(booking_end_time_split[1])//15

                    # show booking in table, different color for booking owner user
                    color = self.tableWidget.cells_colors["user_bookings"] if booking["id_user"]==self.user["id"] else self.tableWidget.cells_colors["other_bookings"]
                    for i in range(start_table_col_index, end_table_col_index):
                        self.tableWidget.item_paint(table_row_index, i, color)

                    # deny bookings 15 minutes before and after existing bookings
                    if (start_table_col_index > 0):
                        self.tableWidget.item_paint(table_row_index, start_table_col_index-1, self.tableWidget.cells_colors["bookings_separation"])
                    if (end_table_col_index < 96):
                        self.tableWidget.item_paint(table_row_index, end_table_col_index, self.tableWidget.cells_colors["bookings_separation"])


    def get_booking_amount(self, utc_start_datetime_string, utc_end_datetime_string, slot_vehicle_type_id):
        # amount = hourly_rate * rate_percentage * time_interval

        # hourly rate
        user_category_id = self.user["id_user_category"]
        # obtain user_categories
        response = make_http_request(self.http_session, "get", "user_categories")
        if response:
            user_categories = response.json()

        if user_categories:             # not empty
            hourly_rate_id = [user_category["id_hourly_rate"] for user_category in user_categories if user_category["id"]==user_category_id][0]

        # obtain hourly rates and filter
        response = make_http_request(self.http_session, "get", "hourly_rates")
        if response:
            hourly_rates = response.json()

        if hourly_rates:
            hourly_rate_amount = float([hourly_rate["amount"] for hourly_rate in hourly_rates if hourly_rate["id"]==hourly_rate_id][0])
        #print(hourly_rate_amount)

        # rate_percentage
        rate_percentage = float([vehicle_type["rate_percentage"] for vehicle_type in self.vehicle_types if vehicle_type["id"]==slot_vehicle_type_id][0])
        #print(rate_percentage)

        # time_inverval = utc_end_datetime_string - utc_start_datetime_string in secondi o minuti
        time_interval_seconds = (datetime.strptime(f"{utc_end_datetime_string}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{utc_start_datetime_string}", "%Y-%m-%d %H:%M:%S")).total_seconds()
        #print(time_interval_seconds)

        booking_amount = "{:.2f}".format((hourly_rate_amount/3600) * rate_percentage * time_interval_seconds)
        return booking_amount


    def make_booking(self, parking_slot_number, start_time, end_time):
        print("********************")
        day = str(self.cal.selectedDate().toString(Qt.ISODate))

        # booking start and end in UTC
        utc_start_datetime_string = datetime_to_UTC(day, start_time)
        utc_end_datetime_string = datetime_to_UTC(day, end_time)

        #print(utc_start_datetime_string, utc_end_datetime_string)

        # booking duration, work with timezone and daylight saving time
        booking_duration = (datetime.strptime(f"{utc_end_datetime_string}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{utc_start_datetime_string}", "%Y-%m-%d %H:%M:%S"))

        response = make_http_request(self.http_session, "get", "users/" + str(self.user["id"]) + "/vehicles")
        if response:
            user_vehicles = response.json()
        else:
            user_vehicles = []

        # obtain slot vehicle type name
        slot_vehicle_type = self.vehicle_types_table.item(self.tableWidget.currentRow(), 0).text()
        #print(slot_vehicle_type)
        slot_vehicle_type_id = [vehicle_type["id"] for vehicle_type in self.vehicle_types if vehicle_type["name"].lower()==slot_vehicle_type.lower()][0]

        # it's necessary convert back from UTC to local time zone to handle daylight saving time
        booking_amount = self.get_booking_amount(utc_start_datetime_string, utc_end_datetime_string, slot_vehicle_type_id)
        print(booking_amount)

        datetime_start = datetime_UTC_to_local(utc_start_datetime_string)
        datetime_end = datetime_UTC_to_local(utc_end_datetime_string)
        booking_dialog = BookingDialog(datetime_start, datetime_end, booking_duration, parking_slot_number, user_vehicles, slot_vehicle_type_id, booking_amount, self)
        # if clicked OK button
        if booking_dialog.exec_():
            vehicle = booking_dialog.selected_vehicle()
            vehicle_id = [user_vehicle["id"] for user_vehicle in user_vehicles if user_vehicle["license_plate"]==vehicle.split()[0]][0]
            #print(vehicle_id)
            #print(self.parking_lot)

            booking =   {
                        "datetime_start": utc_start_datetime_string,
                        "datetime_end": utc_end_datetime_string,
                        "id_vehicle": vehicle_id,
                        "id_parking_slot": int([slot["id"] for slot in self.parking_lot["parking_slots"] if slot["number"] == parking_slot_number][0])
                        }

            response = make_http_request(self.http_session, "post", "users/" + str(self.user["id"]) + "/bookings", json = booking)
            if response:
                json_response = response.json()
                QMessageBox.information(self, "Booking response", json_response["message"] + "\nPrice: " + str(json_response["amount"]))
                self.get_bookings(self.parking_lot, False)



def make_http_request(http_session, method, relative_uri, json=None, params=None):

    try:
        response = http_session.request(method, "https://localhost:50050/apis/" + relative_uri, json=json, params=params)
        response.raise_for_status()

        return response

    except (requests.exceptions.HTTPError, Exception) as err:
        message = response.text if isinstance(err, requests.exceptions.HTTPError) else str(err)
        QMessageBox.critical(None, "Alert", message)
        return None


# Also pass day as parameter to handle Daylight saving time
def datetime_to_UTC(day, time):
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
    utc_datetime_string = utc_datetime.strftime('%Y-%m-%d %H:%M:%S')

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



if __name__ == '__main__':
    app = QApplication(sys.argv)
    parcheggia = Park()
    sys.exit(app.exec_())
