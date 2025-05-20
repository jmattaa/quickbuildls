package lsp

type Request struct {
	RPC    string `json:"jsonrpc"` // should always be 2.0
	ID     int    `json:"id"`
	Method string `json:"method"`
}

type Response struct {
	RPC    string `json:"jsonrpc"`
	ID     *int   `json:"id,omitempty"`
}

type Notification struct {
	RPC    string `json:"jsonrpc"`
	Method string `json:"method"`
}
