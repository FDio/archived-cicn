#!/bin/bash
lxc exec u1srv1 -- bash -c "iget http://u3srv1/$1" &
lxc exec u1srv2 -- bash -c "iget http://u3srv1/$1" &
lxc exec u2srv1 -- bash -c "iget http://u3srv1/$1" &
wait $(jobs -p)
