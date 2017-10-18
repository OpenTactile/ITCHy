
TOOL = python3 setup.py

all:
	$(TOOL) build

run:
	LD_LIBRARY_PATH="../build-libITCHy" python3 itchpytest.py

repl:
	LD_LIBRARY_PATH="../build-libITCHy" python3 -i itchpytest.py

clean:
	$(TOOL) clean --all
