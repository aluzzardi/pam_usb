#!/usr/bin/bash

# @todo: this will fail if other devices are present. its missing a step to actually determine the number used internally
# @todo: check if the numbering used in debconf is correct after all

echo -e "Test:\t\t\tpamusb-conf doesn't add user(s) twice"
sudo pamusb-conf --add-user=`whoami` --device=0 --yes | grep "already added" > /dev/null && echo -e "Result:\t\t\tPASSED!" || exit 1