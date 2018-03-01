PACKAGE=github.com/bmanojlovic/sendsms

include golang.mk

windows:
	GOOS=windows GOARCH=amd64 make build

darwin: macos
macos:
	GOOS=darwin GOARCH=amd64 make build