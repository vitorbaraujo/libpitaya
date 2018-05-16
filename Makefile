setup-go:
	@curl -sL -o ~/gimme https://raw.githubusercontent.com/travis-ci/gimme/master/gimme
	@chmod +x ~/gimme
	@~/gimme -k
	@~/gimme 1.10.2
	@echo Go installed version $(shell go version)
	@~/.gimme/versions/go1.10.2.linux.amd64/bin/go version
	@echo WHERE IS GO
	@whereis go

setup-node:
	@curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash
	@. ~/.nvm/nvm.sh && nvm install 10.0.0

setup-gyp:
	@git clone https://chromium.googlesource.com/external/gyp.git ~/gyp
	@cd ~/gyp && sudo python setup.py install

setup-ci: setup-gyp setup-node setup-go
	@gyp --depth=. pomelo.gyp -f make --generator-output=build -Duse_sys_openssl=false -Dbuild_type=Release -Duse_xcode=false

.PHONY: build

test-deps:
	@-(cd test/server && go get)

build:
	cd build && make

test: build test-deps
	./run-tests.sh
