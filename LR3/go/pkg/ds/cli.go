package ds

import (
	"fmt"
	"io"
	"os"
	"strings"
)

func writeLine(writer io.Writer, text string) {
	_, _ = fmt.Fprintln(writer, text)
}

func RunCLI(args []string, stdout, stderr io.Writer) int {
	var filename, query string
	for i := 0; i < len(args); i++ {
		switch {
		case args[i] == "--file" && i+1 < len(args):
			i++
			filename = args[i]
		case args[i] == "--query" && i+1 < len(args):
			i++
			query = args[i]
		}
	}

	if query == "" {
		writeLine(stdout, "Usage: dsdb --file <file.json> --query '<COMMAND> [args]'")
		return 0
	}

	database := NewDatabase()
	if filename != "" {
		if _, err := os.Stat(filename); err == nil {
			if err := database.LoadFromFile(filename); err != nil {
				writeLine(stderr, "ERROR: "+err.Error())
				return 2
			}
		}
	}

	reply := database.Execute(query)
	writeLine(stdout, reply)
	if strings.HasPrefix(reply, "ERROR:") {
		return 1
	}
	if filename != "" {
		if err := database.SaveToFile(filename); err != nil {
			writeLine(stderr, "ERROR: "+err.Error())
			return 2
		}
	}
	return 0
}
