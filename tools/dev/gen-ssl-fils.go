package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
)

func main() {
	privateKeyFile := flag.String("privateKeyFile", "privatekey.pem", "Private key file name")
	publicKeyFile := flag.String("publicKeyFile", "publickey.pem", "Public key file name")
	flag.Parse()

	genpkeyCmd := exec.Command("openssl", "genpkey", "-algorithm", "RSA", "-out", *privateKeyFile)
	rsaCmd := exec.Command("openssl", "rsa", "-pubout", "-in", *privateKeyFile, "-out", *publicKeyFile)

	if err := genpkeyCmd.Run(); err != nil {
		fmt.Println("Error with generating private key:", err)
		os.Exit(1)
	}

	if err := rsaCmd.Run(); err != nil {
		fmt.Println("Error with generating public key:", err)
		os.Exit(1)
	}

	fmt.Println("Files generated successfully.")
}
