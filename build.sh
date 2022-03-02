#!/bin/bash

BUILD_DIR=. \
SEL4CP_SDK=/home/user/sel4cp/release/sel4cp-sdk-1.2.6/ \
SEL4CP_BOARD=zcu102 \
SEL4CP_CONFIG=debug \
make
