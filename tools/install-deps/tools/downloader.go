package tools

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"errors"
)

func DownloadFile(fileURL, outputFileName string) error {
	fmt.Printf("Downloading %s.\n", outputFileName)
	resp, err := http.Get(fileURL)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return errors.New(fmt.Sprintf("HTTP Response Code:", resp.StatusCode))
	}

	file, err := os.Create(outputFileName)
	if err != nil {
		return err
	}
	defer file.Close()

	const chunkSize = 8192
	buffer := make([]byte, chunkSize)
	var totalBytesWritten int64
	progressBar := NewProgressBar(resp.ContentLength, 50)

	for {
		n, err := resp.Body.Read(buffer)
        if err != nil && err != io.EOF {
            return err
        }
        if n == 0 {
            break
        }

        _, err = file.Write(buffer[:n])
        if err != nil {
            return err
        }

		totalBytesWritten += int64(n)

		progressBar.Update(totalBytesWritten)
		fmt.Printf("\r%s", progressBar.Render())
	}

	progressBar.Update(resp.ContentLength)
	fmt.Printf("\r%s\nDownload Completed!\n", progressBar.Render())
	return nil
}