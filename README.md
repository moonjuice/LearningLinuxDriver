# LearningLinuxDriver
## build code
```
make
```
## mount or unmount module
```
# mount
sudo insmod ./hx711.ko && sudo chmod 666 /dev/HX711 && sudo chmod 666 /dev/HX711_OFFSET

# unmount
sudo rmmod hx711
```
## control led
```
# write value
echo -n '1' > /dev/LED_0
echo -n '0' > /dev/LED_0

# read value
cat /dev/LED_0
```
## check log
```
dmesg | grep moon
```
## Show on Desktop App
```
cd DesktopApp
# before start application
npm install

# start application
npm start
```

## Show on Mobile phone
```
cd DesktopApp
# before start application
npm install

# start application
sudo node ble.js
```