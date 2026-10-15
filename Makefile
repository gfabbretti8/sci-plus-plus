CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
PREFIX   ?= /usr/local

sci++: scipp.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

test: sci++
	./sci++ esempi/ciao.sci -o esempi/ciao && ./esempi/ciao

install: sci++
	install -m 755 sci++ $(PREFIX)/bin/sci++

clean:
	rm -f sci++ esempi/ciao

.PHONY: test install clean
