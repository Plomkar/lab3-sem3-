package main

import (
	"os"

	"dslab3/pkg/ds"
)

func main() {
	os.Exit(ds.RunCLI(os.Args[1:], os.Stdout, os.Stderr))
}
