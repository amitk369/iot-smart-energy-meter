CC = clang
CFLAGS = -Wall -Wextra -O2 -I./firmware/include
SRC = firmware/src/pzem004t_modbus.c firmware/src/grid_protection.c firmware/src/mqtt_telemetry.c
TEST_SRC = tests/test_energy_meter.c

all: test

test:
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o run_tests -lm
	./run_tests

sim:
	python3 simulation/simulate_grid_telemetry.py

clean:
	rm -f run_tests
