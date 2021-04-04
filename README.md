# LearningLinuxDriver
## build code
```
make
```
## mount or unmount module
```
# mount
sudo insmod ./hello.ko && sudo chmod 666 /dev/LED_0

# unmount
sudo rmmod hello
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