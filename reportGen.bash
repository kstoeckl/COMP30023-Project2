#!/bin/bash
for port in {1300..1320};do
	echo "Created Server"
	screen -dmS "server" ./server $port
	echo "Spawning clients"
	i=1300
	while [ $i -lt $port ]; do
		echo "$i"
	    screen -dmS "client$i" ./client localhost $port
	    sleep .2
	    let i=i+1 
	done
	screen -r server
	sleep .2

	echo $(tail -1 log.txt)
	echo $(tail -1 log.txt) >> reportdata.txt

	killall screen
done

echo "done"

