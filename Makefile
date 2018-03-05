PACKAGE=github.com/bmanojlovic/sendsms

include golang.mk

windows:
	GOOS=windows GOARCH=amd64 make build

windows32:
	GOOS=windows GOARCH=386 make build

darwin: macos

macos:
	GOOS=darwin GOARCH=amd64 make build
