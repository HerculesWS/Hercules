#/bin/sh
#Hi my naem is Kirt and I liek anime

ulimit -Sc unlimited

while [ 2 ] ; do
if [ -f .stopserver2 ] ; then
echo server marked down >> server-log.txt
else
echo restarting server at time at `date +"%m-%d-%H:%M-%S"`>> start-log.txt
./login-server
fi

sleep 5

done
