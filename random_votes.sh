#!/bin/bash

CANDIDATES=4
RANGE=4

for i in {1..268}
do
    rand_candidate=$((RANDOM % CANDIDATES))
    out/vote_cmd.o voter_$i $rand_candidate
done