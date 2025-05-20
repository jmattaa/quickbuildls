package logger

import (
	"os"
	"time"
)

type Logger struct {
	logfn   string
	logfile *os.File
}

func New(logfn string) (*Logger, error) {
	logger := &Logger{
		logfn: logfn,
	}

	logfile, err := os.OpenFile(logfn, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
	if err != nil {
		return nil, err
	}

	logger.logfile = logfile
	return logger, nil
}

func (l *Logger) Write(p []byte) (n int, err error) {
	ctime := time.Now().Format(time.RFC822)

	n, err = l.logfile.Write([]byte(ctime + "  " + string(p)))
	return
}

func (l *Logger) Close() error {
	return l.logfile.Close()
}
