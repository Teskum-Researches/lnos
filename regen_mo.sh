#!/bin/bash
set -e

mkdir -p locale/ru/LC_MESSAGES

msgfmt \
  po/ru.po \
  -o locale/ru/LC_MESSAGES/lnos.mo