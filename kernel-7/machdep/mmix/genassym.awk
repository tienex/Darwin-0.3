#!/usr/bin/awk -f
# Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
#
# Extract assembly constants from genassym.c output
#
# Input: C preprocessor output from genassym.c
# Output: Assembly language constant definitions

/^int[ 	]/ {
	printf("#define\t%s\t%s\n", $2, $4);
}
