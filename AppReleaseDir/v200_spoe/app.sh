#telnetd &
cp /tools/* /tmp
cp /app/message /tmp
cp /app/upgrade /tmp
cd /app/hi3518
./load3518 -i
cd /tmp
./message&
cd /app
./App3518&

