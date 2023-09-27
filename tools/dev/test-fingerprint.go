package main

import (
    "flag"
    "bytes"
    "fmt"
    "os/exec"
)

func main() {
	privateKeyFile := flag.String("privateKeyFile", "privatekey.pem", "Private Key File Name")
	flag.Parse()

	rsaCmd := exec.Command("openssl", "rsa", "-in", *privateKeyFile, "-pubout", "-outform", "DER")
    dgstCmd := exec.Command("openssl", "dgst", "-sha256", "-c")

    var err error
    dgstCmd.Stdin, err = rsaCmd.StdoutPipe()
    if err != nil {
        fmt.Println("Error:", err)
        return
    }

    var output bytes.Buffer
    dgstCmd.Stdout = &output
    
    if err := rsaCmd.Start(); err != nil {
        fmt.Println("Error:", err)
        return
    }
    if err := dgstCmd.Start(); err != nil {
        fmt.Println("Error:", err)
        return
    }

    if err := rsaCmd.Wait(); err != nil {
        fmt.Println("Error:", err)
        return
    }
    if err := dgstCmd.Wait(); err != nil {
        fmt.Println("Error:", err)
        return
    }

    fingerprint := output.String()
    fmt.Println(fingerprint)
}