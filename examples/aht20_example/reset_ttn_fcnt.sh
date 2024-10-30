# Reset the end device session with the last f-cnt-up
echo "Resetting device session with last f-cnt-up..."
ttn-lw-cli end-devices reset attiny-lorawan-test attiny84test --session.last-f-cnt-up

# Reset the end device session with the last a-f-cnt-down
echo "Resetting device session with last a-f-cnt-down..."
ttn-lw-cli end-devices reset attiny-lorawan-test attiny84test --session.last-a-f-cnt-down

#echo "Deactivating downlink transmission..."
#ttn-lw-cli end-devices set --application-id attiny-lorawan-test --device-id attiny84test --mac-settings.schedule-downlinks=false
