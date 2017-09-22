FROM sbx320/build-environment:latest

RUN 	python2 configure.py --ninja && \
	python3 build.py
