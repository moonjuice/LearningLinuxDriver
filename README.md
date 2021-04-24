# LearningLinuxDriver
## build code
```
make
```
## mount or unmount module
```
# mount
sudo insmod ./hx711.ko && sudo chmod 666 /dev/HX711

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

## User Application
```
* gcc -o read_hx711 read_hx711.c
* ./read_hx711
```