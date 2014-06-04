#!/bin/bash
# Usage: ./install-package.sh IP Password
blackberry-deploy -installApp -launchApp -device $1 -password $2 Sachesi.bar
