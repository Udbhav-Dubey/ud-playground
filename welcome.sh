#!/bin/bash

sleep 2

hour=$(date +%H)

if [ "$hour" -lt 12 ]; then
  greet="Good morning,sir"
elif [ "$hour" -lt 18 ]; then
  greet="Good afternoon, sir"
else
  greet="Good evening, sir"
fi

notify-send -u normal "$greet" "Better to burn out than fade away."

