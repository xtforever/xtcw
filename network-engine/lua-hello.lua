function check_func(t1,t2)
	print(" arg1: " .. t1 );
	print(" arg2: " .. t2 );
	return t1+t2;
end

for a=1,10
do
	a="world";
	print("lua hello " .. a );
end

a = check_func(100,200);

howdy( "what", "should", "we", "do", a );
circle( 50,60,70 );

tab = measure_screen();

print( "name is: " .. tab["name"] );
print( "ip   is: " .. tab["ip"] );
print( "date is: " .. tab["date"] );

clrscr();

for i=1,100,3
do
	circle( 50+i, 60+i, 170 - i );
end

