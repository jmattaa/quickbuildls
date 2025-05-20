package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"

	"github.com/jmattaa/quickbuildls/logger"
	"github.com/jmattaa/quickbuildls/lsp"
)

func main() {
	logger, err := logger.New("/Users/jonathan/dev/quickbuildls/quickbuildls.log")
	if err != nil {
		panic(err)
	}
	defer logger.Close()
	logger.Write([]byte("LSP started\n"))

	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(lsp.Split)

	for scanner.Scan() {
		req := scanner.Bytes()
		content, l, err := lsp.Decode(req)
		if err != nil {
			logger.Write([]byte(err.Error() + "\n"))
			continue
		}
		handlereq(content, l, logger)
	}
}

func handlereq(c []byte, msg lsp.LspMessage, logger *logger.Logger) {
	switch msg.Method {
	case "initialize":
		msg := lsp.NewInitializeResponse(msg.ID)
		reply, err := lsp.Encode(msg)
		if err != nil {
			logger.Write([]byte(err.Error() + "\n"))
			return
		}

		os.Stdout.Write([]byte(reply))
	case "textDocument/didOpen":
		var req lsp.TextDocumentDidOpenNotification
		err := json.Unmarshal(c, &req)
		if err != nil {
			logger.Write([]byte(err.Error() + "\n"))
			return
		}
	}
}
