[ "$ACTION" = add -a "$DEVTYPE" = usb_device -a "$PRODUCT" = 12d1/15c1/102 ] || exit 0

logger -t DEBUG "ME909s LTE Dongle detected"

vid=$(cat /sys$DEVPATH/idVendor)
pid=$(cat /sys$DEVPATH/idProduct)
mode=$(cat /sys$DEVPATH/bConfigurationValue)

[ "$mode" != 3 ] || exit 0

logger -t DEBUG "Setting right mode"

echo 3 > /sys$DEVPATH/bConfigurationValue
