package main

import (
	"bufio"
	"os"

	"github.com/jmattaa/quickbuildls/logger"
	"github.com/jmattaa/quickbuildls/rpc"
)

func main() {
	logger, err := logger.New("/Users/jonathan/dev/quickbuildls/quickbuildls.log")
	if err != nil {
		panic(err)
	}
	defer logger.Close()
	logger.Write([]byte("LSP started\n"))

	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(rpc.Split)

	for scanner.Scan() {
		msg := scanner.Text()
		handlemsg(msg, logger)
	}
}

func handlemsg(msg string, logger *logger.Logger) {
	_, err := rpc.Decode(msg)
	if err != nil {
		logger.Write([]byte("Error decoding message: " + err.Error() + "\n"))
		return
	}

	logger.Write([]byte("Received message: " + msg + "\n"))
}
