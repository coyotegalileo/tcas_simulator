g++ -std=c++11 -g -Iinclude ../Radar/radar.cpp ../airplane/airplane.cpp ../converts/converts.cpp ../checksum/checksum.cpp   -o radar
g++ -std=c++11 -g -Iinclude ../TCAS/TCAS.cpp ../checksum/checksum.cpp ../airplane/airplane.cpp ../converts/converts.cpp -o tcas
g++ -std=c++11 -g -Iinclude ../junto/junto.cpp ../airplane/airplane.cpp ../checksum/checksum.cpp ../converts/converts.cpp ../auto_pilot/determine_velocity.cpp  -o junto
