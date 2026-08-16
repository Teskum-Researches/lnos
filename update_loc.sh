#!/bin/bash
set -e

./regen_pot.sh
./update_po.sh
./regen_mo.sh

echo "gettext: done"