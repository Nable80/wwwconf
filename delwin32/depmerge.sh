#!/bin/bash

awk '/\\$/ {ORS=""; for (i = 1; i < NF; ++i) print $i, "";} !/\\$/ {ORS="\n"; print}'
