#!/bin/bash

aclocal
autoheader
automake --add-missing --copy --foreign
autoconf
