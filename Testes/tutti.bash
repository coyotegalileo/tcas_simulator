g++ -std=c++11 -g -Iinclude ../preProcessor/predisp.cpp ../airplane/airplane.cpp ../checksum/checksum.cpp ../converts/converts.cpp -o preout
g++ -std=c++11 -g -Iinclude ../auto_pilot/auto_pilot.cpp ../airplane/airplane.cpp ../checksum/checksum.cpp ../converts/converts.cpp ../auto_pilot/determine_velocity.cpp  -o autop
g++ -std=c++11 -g -Iinclude ../mux/mux.cpp ../airplane/airplane.cpp ../checksum/checksum.cpp ../converts/converts.cpp -o mux
g++ -std=c++11 -g -Iinclude ../Radar/radar.cpp ../airplane/airplane.cpp ../converts/converts.cpp ../checksum/checksum.cpp   -o radar
g++ -std=c++11 -g -Iinclude ../simulador/simulator.cpp ../checksum/checksum.cpp ../converts/converts.cpp ../airplane/airplane.cpp -o simulator
g++ -std=c++11 -g -Iinclude ../TCAS/TCAS.cpp ../checksum/checksum.cpp ../airplane/airplane.cpp ../converts/converts.cpp -o tcas
