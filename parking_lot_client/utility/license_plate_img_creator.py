#! /usr/bin/python3
from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw
import sys

if len(sys.argv) == 2:
    license_plate_text = sys.argv[1]

    if len(license_plate_text)==7:
        license_plate_text = license_plate_text[0:2] + " " + license_plate_text[2:]

        license_plate_base_img = Image.open("license_plate_base.jpg")
        draw = ImageDraw.Draw(license_plate_base_img)
        #font = ImageFont.truetype('Helvetica-Bold.ttf', 110)
        font = ImageFont.truetype('Helvetica-Bold.ttf', 80)
        #draw.text((185, 160), license_plate_text, (0, 0, 0), font=font)
        draw.text((210, 190), license_plate_text, (0, 0, 0), font=font)
        license_plate_base_img.save("../vehicles/" + license_plate_text + ".jpg")
    else:
        print("license plate must contains 7 characters")
else:
    print("Please, insert a license plate!")
