package rpc

import (
	"bytes"
	"encoding/json"
	"fmt"
	"strconv"
)

type LspMessage struct {
	JSONRPC string `json:"jsonrpc"`
	Method  string `json:"method"`
	ID      int    `json:"id"`
	Params  any    `json:"params"`
}

func Encode(msg any) (string, error) {
	c, err := json.Marshal(msg)
	if err != nil {
		return "", err
	}

	return fmt.Sprintf("Content-Length: %d\r\n\r\n%s", len(c), c), nil
}

func Decode(msg string) (LspMessage, error) {
	header, content, found := bytes.Cut([]byte(msg), []byte("\r\n\r\n"))
	if !found {
		return LspMessage{}, fmt.Errorf("Couldn't parse message, missing header")
	}

	contentLenBytes := header[len("Content-Length: "):]
	contentlen, err := strconv.Atoi(string(contentLenBytes))
	if err != nil {
		return LspMessage{}, err
	}

	var r LspMessage
	err = json.Unmarshal(content[:contentlen], &r)
	return r, err
}

func Split(data []byte, _ bool) (advance int, token []byte, err error) {
	header, content, found := bytes.Cut(data, []byte("\r\n\r\n"))
	if !found {
		return 0, nil, nil // do nothing
	}

	contentLenBytes := header[len("Content-Length: "):]
	contentlen, err := strconv.Atoi(string(contentLenBytes))
	if err != nil {
		return 0, nil, err
	}

	if len(content) < contentlen {
		return 0, nil, nil // wait until we get everything
	}

	totallen := len(header) + len("\r\n\r\n") + contentlen
	return  totallen, data[:totallen], nil
}
