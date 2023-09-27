package main

import (
	"install-deps/tools"
	"flag"
	"fmt"
	"os"
	"os/exec"
    "io"
    "bufio"
	"strings"
	"strconv"
	"path/filepath"
	"runtime"
)

func getsqlite(projectRoot string, sqlite string, sqliteVerRelYear string) {
	depsDir := filepath.Join(projectRoot, "deps")
	if _, err := os.Stat(filepath.Join(depsDir, "sqlite-amalgamation")); !os.IsNotExist(err) {
		fmt.Println("SQLite folder already exists locally. The downloaded copy will not be used.")
		return
	}

	tempDir := filepath.Join(projectRoot, "temp")
	fileURL := fmt.Sprintf("https://www.sqlite.org/%s/%s.zip", sqliteVerRelYear, sqlite)

	err := tools.DownloadFile(fileURL, filepath.Join(tempDir, sqlite + ".zip"))
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	err = tools.ZIPExtract(filepath.Join(tempDir, sqlite + ".zip"), tempDir)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	if err := os.Rename(filepath.Join(tempDir, sqlite), filepath.Join(depsDir, "sqlite-amalgamation")); err != nil {
		fmt.Println("Error:", err)
		return
	}
}

func getopenssl(projectRoot string, openssl string, opensslVer string) {
	depsDir := filepath.Join(projectRoot, "deps")
	if _, err := os.Stat(filepath.Join(depsDir, "openssl")); !os.IsNotExist(err) {
		fmt.Println("OpenSSL folder already exists locally. The downloaded copy will not be used.")
		return
	}

	tempDir := filepath.Join(projectRoot, "temp")
	fileURL := fmt.Sprintf("https://www.openssl.org/source/openssl-%s.tar.gz", opensslVer)

	err := tools.DownloadFile(fileURL, filepath.Join(tempDir, openssl + ".tar.gz"))
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	err = tools.TARGZExtract(filepath.Join(tempDir, openssl + ".tar.gz"), tempDir)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	opensslDir := filepath.Join(depsDir, "openssl")
    if err := os.MkdirAll(opensslDir, os.ModePerm); err != nil {
        fmt.Println("Error:", err)
        return
    }

	ostype := runtime.GOOS
	if ostype == "windows" {
		testcmd := exec.Command("powershell.exe", "Get-ExecutionPolicy")

		stdout, err := testcmd.StdoutPipe()
		if err != nil {
			fmt.Println("Error:", err)
			return
		}

		err = testcmd.Start()
		if err != nil {
			fmt.Println("Error:", err)
			return
		}

		outputBytes, err := io.ReadAll(stdout)
		if err != nil {
			fmt.Println("Error:", err)
			return
		}

		output := strings.TrimSpace(string(outputBytes))
		if !strings.EqualFold(output, "RemoteSigned") {
			fmt.Println("PowerShell ExecutionPolicy must be RemoteSigned to perform OpenSSL compile.")
			return
		}

		psScriptPath, err := os.Getwd()
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		psScriptPath = filepath.Join(psScriptPath, "win-build-openssl.ps1")
	
		cmd := exec.Command("powershell.exe", "-File", psScriptPath, "-tempPath", tempDir, "-openssl", openssl)

		stdout, err = cmd.StdoutPipe()
	    if err != nil {
		    fmt.Println("Error:", err)
			return
	    }

		err = cmd.Start()
		if err != nil {
	        fmt.Println("Error:", err)
		    return
		}

		reader := bufio.NewReader(stdout)
		for {
			line, err := reader.ReadString('\n')
	        if err != nil && err == io.EOF {
		        break
			}
	        fmt.Print(line)
		}

		err = cmd.Wait()
		if err != nil {
			if exitErr, ok := err.(*exec.ExitError); ok {
				exitCode := exitErr.ExitCode()
	            fmt.Printf("PowerShell: %d\n", exitCode)
			} else {
				fmt.Println("Error:", err)
			}
		
			return
		}

		if err := os.Rename(filepath.Join(tempDir, openssl, "libcrypto.lib"), filepath.Join(opensslDir, "libcrypto.lib")); err != nil {
		    fmt.Println("Error:", err)
			return
		}

		if err := os.Rename(filepath.Join(tempDir, openssl, "libssl.lib"), filepath.Join(opensslDir, "libssl.lib")); err != nil {
			fmt.Println("Error:", err)
			return
		}
	} else if ostype == "linux" {
		perlcmd := exec.Command("perl", filepath.Join(tempDir, openssl, "Configure"), "no-shared", "--release", "--api=1.1.0", "no-deprecated", "no-ssl2", "no-ssl3", "no-md2", "no-rc4", "no-idea", "no-camellia", "no-ec", "no-engine", "no-tests", "linux-x86_64")
		makecmd := exec.Command("make", filepath.Join(tempDir, openssl))

		perlcmd.Stdout = os.Stdout
		perlcmd.Stderr = os.Stderr
		makecmd.Stdout = os.Stdout
		makecmd.Stderr = os.Stderr

		if err := perlcmd.Run(); err != nil {
			fmt.Println("Error:", err)
			return
		}

		if err := makecmd.Run(); err != nil {
			fmt.Println("Error:", err)
			return
		}

		if err := os.Rename(filepath.Join(tempDir, openssl, "libcrypto.a"), filepath.Join(opensslDir, "libcrypto.a")); err != nil {
		    fmt.Println("Error:", err)
			return
		}

		if err := os.Rename(filepath.Join(tempDir, openssl, "libssl.a"), filepath.Join(opensslDir, "libssl.a")); err != nil {
			fmt.Println("Error:", err)
			return
		}
	} else {
		fmt.Println("Other platforms are not supported. Supported platforms are Windows and Linux")
		return
	}

	if err := os.Rename(filepath.Join(tempDir, openssl, "include"), filepath.Join(opensslDir, "include")); err != nil {
        fmt.Println("Error:", err)
        return
    }
}

func main() {
	nosqlite3 := flag.Bool("no-sqlite3", false, "Disables SQLite3")
	noopenssl := flag.Bool("no-openssl", false, "Disables OpenSSL")
	helpPtr := flag.Bool("h", false, "Display help message")
	flag.BoolVar(helpPtr, "help", false, "Display help message")
	flag.Parse()

	if *helpPtr {
		fmt.Println("Usage: go run main.go [--no|XXX] [-h/--help]\n")
		fmt.Println("These dependencies can be disabled:")
		fmt.Println("sqlite3")
		fmt.Println("openssl")
		fmt.Print("\n")
		return
	}

	SQLITE_VERSION := tools.GetEnv("SQLITE_VERSION", "3.43.0")
	OPENSSL_VERSION := tools.GetEnv("OPENSSL_VERSION", "1.1.1w")

	// Used for SQLite download URL
	SQLITE_VERSION_RELEASE_YEAR := tools.GetEnv("SQLITE_VERSION_RELEASE_YEAR", "2023")
	SQLITE_VERSION_LIST := strings.Split(SQLITE_VERSION, ".")

	var SQLITE_VERSION_INT []int
	for i := 0; i < 4; i++ {
		if i < len(SQLITE_VERSION_LIST) {
			partInt, _ := strconv.Atoi(SQLITE_VERSION_LIST[i])
			SQLITE_VERSION_INT = append(SQLITE_VERSION_INT, partInt)
		} else {
			SQLITE_VERSION_INT = append(SQLITE_VERSION_INT, 0)
		}
	}

	SQLITE := fmt.Sprintf("sqlite-amalgamation-%d%02d%02d%02d", SQLITE_VERSION_INT[0], SQLITE_VERSION_INT[1], SQLITE_VERSION_INT[2], SQLITE_VERSION_INT[3])
	OPENSSL := fmt.Sprintf("openssl-%s", OPENSSL_VERSION)

	PROJECT_DIR, err := os.Getwd()
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	PROJECT_DIR = filepath.Dir(filepath.Dir(PROJECT_DIR))

	err = os.MkdirAll(filepath.Join(PROJECT_DIR, "deps"), os.ModePerm)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	err = os.MkdirAll(filepath.Join(PROJECT_DIR, "temp"), os.ModePerm)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	if !*nosqlite3 {
		getsqlite(PROJECT_DIR, SQLITE, SQLITE_VERSION_RELEASE_YEAR)
	}

	if !*noopenssl {
		getopenssl(PROJECT_DIR, OPENSSL, OPENSSL_VERSION)
	}

	if err := os.RemoveAll(filepath.Join(PROJECT_DIR, "temp")); err != nil {
        fmt.Println("Error", err)
        return
    }

}