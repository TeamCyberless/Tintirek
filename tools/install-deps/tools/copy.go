package tools

import (
	"io"
	"os"
	"path/filepath"
	"fmt"
)

type ProgressCallback func(completed int64)

func Copy(src, dst string) error {
	fmt.Printf("Copying %s.\n", src)
	totalFiles := countFiles(src)
	progressBar := NewProgressBar(totalFiles, 50)

	err := copyInternal(src, dst, totalFiles, func(completed int64) {
		progressBar.Update(completed)
		fmt.Printf("\r%s", progressBar.Render())
	})

	progressBar.Update(totalFiles)
	fmt.Printf("\r%s\nCopy Completed!\n", progressBar.Render())

	return err
}

func copyInternal(src, dst string, total int64, callback ProgressCallback) error {
	srcInfo, err := os.Stat(src)
	if err != nil {
		return err
	}

	if srcInfo.IsDir() {
		if err := os.MkdirAll(dst, 0755); err != nil {
			return err
		}

		entries, err := os.ReadDir(src)
		if err != nil {
			return err
		}

		doneFiles := int64(0)

		for _, entry := range entries {
			srcPath := filepath.Join(src, entry.Name())
			dstPath := filepath.Join(dst, entry.Name())

			if entry.IsDir() {
				if err := copyInternal(srcPath, dstPath, total, callback); err != nil {
					return err
				}
			} else {
				source, err := os.Open(srcPath)
				if err != nil {
					return err
				}
				defer source.Close()

				destination, err := os.Create(dstPath)
				if err != nil {
					return err
				}
				defer destination.Close()

				_, err = io.Copy(destination, source)
				if err != nil {
					return err
				}
				
				doneFiles++
				callback(doneFiles)
			}
		}
	} else {
		source, err := os.Open(src)
		if err != nil {
			return err
		}
		defer source.Close()

		destination, err := os.Create(dst)
		if err != nil {
			return err
		}
		defer destination.Close()

		_, err = io.Copy(destination, source)
		if err != nil {
			return err
		}

		callback(total)
	}

	return nil
}


func countFiles(src string) int64 {
	count := int64(0)
	filepath.Walk(src, func(path string, info os.FileInfo, err error) error {
		if !info.IsDir() {
			count++
		}
		return nil
	})
	return count
}