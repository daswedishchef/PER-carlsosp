# PER-carlsosp
### This is my current work on the Predation Event Recorders for National Marine Fisheries Service research

# GOALS
- Visually record predation events of juvenile salmon
- Log PIT tag in fish via RFID
- Record biophysical data at time of event
  - Location
  - Temperature
  - Depth
  - Lux
  
# HARDWARE
![image](https://github.com/daswedishchef/PER-carlsosp/blob/master/Image2.jpg)

**Current image features some dated hardware**

- Arduino UNO
- Adatafruit GPS/SD shield
- Action cam (IR Filter removed)
- Reed Switch
- ID12LA RFID
- ID12LA usb breakout
- Bar30-r1-rp temp and depth

# OPERATION
### CURRENT
The box is lowered into the water with fish attached. Box records fish and biophysical data when reed switch is triggered. Then collected after a given period of time to review data.
### FUTURE
Boxes will be lowered into water until notification is recieved that the reed switch is triggered. Retreive boxes, connect to its wifi to upload videos/photos. Charge if needed and reset.

# FUTURE DESIGN

Once we can get a prototype working I think we should move to an esp32 based system. It will allow for more horsepower and its sleep functions are a bit better. We can also use the wifi to extract the contents of the SD card inside the box without having to open it. The second big thing we should incorporate is LoRa functionality. Currently, there is no way to tell if the box has been triggered so there could be some inefficiencies when going out to retrieve them. The box will also log its biophysical data 8 times daily (as well as when the reed switch is triggered) through an on shore gateways that will update influxDB and grafana to track changes over time.
