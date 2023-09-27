package tools

import (
	"archive/zip"
	"archive/tar"
	"compress/gzip"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

func ZIPExtract(zipFileName string, extractPath string) error {
	fmt.Printf("Extracting %s.\n", zipFileName)
	zipFile, err := zip.OpenReader(zipFileName)
	if err != nil {
		return err
	}
	defer zipFile.Close()

	if err := os.MkdirAll(extractPath, os.ModePerm); err != nil {
		return err
	}

	progressBar := NewProgressBar(int64(len(zipFile.File)), 50)

	for _, file := range zipFile.File {
		src, err := file.Open()
		if err != nil {
			return err
		}
		defer src.Close()

		targetPath := fmt.Sprintf("%s/%s", extractPath, file.Name)
		if file.FileInfo().IsDir() {
			os.MkdirAll(targetPath, os.ModePerm)
		} else {
			dst, err := os.Create(targetPath)
			if err != nil {
				return err
			}
			defer dst.Close()

			_, err = io.Copy(dst, src)
			if err != nil {
				return err
			}
		}

		progressBar.Update(progressBar.Current + 1)
		fmt.Printf("\r%s", progressBar.Render())
	}

	progressBar.Update(int64(len(zipFile.File)))
	fmt.Printf("\r%s\nExtract Completed!\n", progressBar.Render())

	return nil
}

func TARGZExtract(targzFileName string, extractPath string) error {
	fmt.Printf("Extracting %s.\n", targzFileName)
	targzFile, err := os.Open(targzFileName)
	if err != nil {
		return err
	}
	defer targzFile.Close()

	gzipReader, err := gzip.NewReader(targzFile)
	if err != nil {
		return err
	}
	defer gzipReader.Close()
	tarReader := tar.NewReader(gzipReader)

	var totalFiles int64
	for {
		_, err := tarReader.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
		totalFiles++
	}

	if err := os.MkdirAll(extractPath, os.ModePerm); err != nil {
		return err
	}

	targzFile.Seek(0, 0)
	gzipReader.Reset(targzFile)
	progressBar := NewProgressBar(totalFiles, 50)
	tarReader = tar.NewReader(gzipReader)

	for {
        header, err := tarReader.Next()
        if err == io.EOF {
            break
        }
        if err != nil {
            return err
        }

        targetPath := filepath.Join(extractPath, header.Name)

        switch header.Typeflag {
        case tar.TypeDir:
            if err := os.MkdirAll(targetPath, os.ModePerm); err != nil {
                return err
            }
        case tar.TypeReg:
            file, err := os.Create(targetPath)
            if err != nil {
                return err
            }

            _, err = io.Copy(file, tarReader)
            if err != nil {
                file.Close()
                return err
            }

            if err := file.Close(); err != nil {
                return err
            }
        }

		progressBar.Update(progressBar.Current + 1)
		fmt.Printf("\r%s", progressBar.Render())
    }

	fmt.Printf("\r%s\nExtract Completed!\n", progressBar.Render())
    return nil
}