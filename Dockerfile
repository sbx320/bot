FROM sbx320/build-environment:latest

ADD . /build 
RUN 	cd build && \
	python2 configure.py --ninja && \
	python3 build.py && \
	cp /build/build/rd2lbot /rd2lbot && \
	rm -rf build 
	
