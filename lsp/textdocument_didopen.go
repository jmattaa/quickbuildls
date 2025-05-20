package lsp

type TextDocumentDidOpenNotification struct {
	Notification
	Params TextDocumentDidOpenParams `json:"params"`
}

type TextDocumentDidOpenParams struct {
	TextDocument TextDocumentItem `json:"textDocument"`
}
